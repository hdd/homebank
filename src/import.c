/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2011 Maxime DOYEN
 *
 *	This file is part of HomeBank.
 *
 *	HomeBank is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	HomeBank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"
#include "preferences.h"
#include "import.h"
#include "list_operation.h"
#include "dsp_wallet.h"
#include "dsp_account.h"
#include "def_operation.h"
#include "ui_account.h"
#include "imp_qif.h"
#include "da_encoding.h"
#include "hb_account.h"
#include "hb_transaction.h"



#ifndef NOOFX
#include <libofx/libofx.h>
#endif

/****************************************************************************/
/* Debug macros																*/
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

#define FORCE_SIZE 1
#define HEAD_IMAGE 1
#define SIDE_IMAGE 0

#define SCOEF 0.6

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;



static gchar *page_titles[] = 
{
	N_("HomeBank Import Assistant"),
	N_("Select a file"),
	N_("Control the import"),
	N_("Update your accounts")
};

static void on_account_type_toggled(GtkRadioButton *radiobutton, gpointer user_data);
static GtkWidget *ui_acc_affect_listview_new(void);
guint32 ui_acc_affect_listview_get_selected_key(GtkTreeView *treeview);
void ui_acc_affect_listview_add(GtkTreeView *treeview, Account *item);


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static gchar *homebank_utf8_convert(gchar *buffer, const gchar **charset)
{
GError *conv_error;
gchar* conv_buffer = NULL;
gsize new_len;
guint i;
gboolean valid;
const struct _GeditEncoding *enc;

	DB( g_print("(homebank) homebank_utf8_convert\n") );

	for (i=0 ; i<GEDIT_ENCODING_LAST ; i++)
	{
		conv_error = NULL;

		enc = gedit_encoding_get_from_index(i);
		DB( g_print("-> should try %s\n",  enc->charset) );

		conv_buffer = g_convert(buffer, -1, "UTF-8", enc->charset, NULL, &new_len, &conv_error); 
		valid = g_utf8_validate (conv_buffer, -1, NULL);
		if ((conv_error != NULL) || !valid )
		{
			DB( g_print ("  -> Couldn't convert from %s to UTF-8.\n", enc->charset) );
		}				
		else
		{
			DB( g_print ("  -> file compatible with '%s'\n", enc->charset) );
			if(charset != NULL)
				*charset = enc->charset;
			return conv_buffer;
		}
	}

	if(charset != NULL)
		*charset = NULL;	
	return NULL;
}


/* 
 * Ensure a buffer to be utf-8, and convert if necessary
 *
 */
gchar *homebank_utf8_ensure(gchar *buffer)
{
gboolean isvalid;
gchar *converted;

	DB( g_print("(homebank) homebank_utf8_ensure\n") );

	if(buffer == NULL)
		return NULL;
	
	isvalid = g_utf8_validate(buffer, -1, NULL);
	DB( g_print(" -> is valid utf8: %d\n", isvalid) );

	if(!isvalid)
	{
		converted = homebank_utf8_convert(buffer, NULL);
		if(converted != NULL)
		{
			//g_warn here ?
			g_free(buffer);
			return converted;
		}
		//g_warn here ?
	}
	return buffer;
}


const gchar *homebank_file_getencoding(gchar *filename)
{
const gchar *charset = NULL;
gchar *buffer;
gsize length;
GError *error = NULL;
gboolean isutf8;
const gchar *locale_charset;
const struct _GeditEncoding *enc;

	DB( g_print("(homebank) test encoding\n") );

	if (g_get_charset (&locale_charset) == FALSE)
	{
		//unknown_encoding.charset = g_strdup (locale_charset);
		
	}

	DB( g_print(" -> locale charset is '%s'\n", locale_charset) );

	if (g_file_get_contents (filename, &buffer, &length, &error))
	{
	
		isutf8 = g_utf8_validate(buffer, -1, NULL);
		DB( g_print(" -> is valid utf8: %d\n", isutf8) );

		if( isutf8 == FALSE )
		{
		gchar *converted;
			
			converted = homebank_utf8_convert(buffer, &charset);

			DB( g_print(" -> converted charset match: '%s'\n", charset) );
			DB( g_print(" -> converted: '%x' %s\n", converted, converted) );
			
			if(converted != NULL)
				g_free(converted);
		}
		else
		{
			enc = gedit_encoding_get_utf8();
			charset = enc->charset;
		}
		
	
		g_free(buffer);	
	}

	DB( g_print ("  -> charset is '%s'\n", charset) );

	return charset;
}



Account *import_create_account(gchar *name, gchar *number)
{
Account *accitem, *existitem;

	//first check we do not have already this imported account
	existitem = da_acc_get_by_imp_name(name);
	if(existitem != NULL)
		return existitem;

	accitem = da_acc_malloc();
	accitem->key  = da_acc_get_max_key() + 1;
	accitem->pos  = da_acc_length() + 1;

	// then we check a same named account
	existitem = da_acc_get_by_name(name);
	if(existitem == NULL)
	{
		accitem->name = g_strdup(name);
	}
	else
	{
		accitem->name = g_strdup_printf(_("(account %d)"), accitem->key);
	}

	accitem->imp_name = g_strdup(name);

	if(number)
		accitem->number = g_strdup(number);
	
	accitem->imported = TRUE;
	da_acc_insert(accitem);

	return accitem;
}


static GList *homebank_csv_import(gchar *filename, ImportContext *ictx)
{
GIOChannel *io;
GList *list = NULL;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};

	DB( g_print("(import) homebank csv\n") );

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr;
	gint io_stat;
	gboolean valid;
	gint count = 0;
	gint error = 0;
	Account *tmp_acc;
	Payee *payitem;
	Category *catitem;
	GError *err = NULL; 

	
		gchar *accname = g_strdup_printf(_("(account %d)"), da_acc_get_max_key() + 1);
		tmp_acc = import_create_account(accname, NULL);
		g_free(accname);


		if( ictx->encoding != NULL )
		{
			g_io_channel_set_encoding(io, ictx->encoding, NULL);
		}

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, &err);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( *tmpstr != '\0' )
				{
				gchar **str_array;
				Operation *newope = da_operation_malloc();

					hb_string_strip_crlf(tmpstr);

					/* control validity here */
					valid = hb_string_csv_valid(tmpstr, 7, csvtype);
				
					 //DB( g_print("valid %d, '%s'\n", valid, tmpstr) );
				
					if( !valid )
					{
						error++;
					}
					else
					{
						count++;
					
						str_array = g_strsplit (tmpstr, ";", 7);
						// 0:date; 1:paymode; 2:info; 3:payee, 4:wording; 5:amount; 6:category

						DB( g_print(" ->%s\n", tmpstr ) );

						newope->date		 = hb_date_get_julian_parse(str_array[0]);
						newope->paymode		 = atoi(str_array[1]);
						newope->info		 = g_strdup(str_array[2]);

						/* payee */
						g_strstrip(str_array[3]);
						payitem = da_pay_get_by_name(str_array[3]);
						if(payitem == NULL)
						{
							payitem = da_pay_malloc();
							payitem->name = g_strdup(str_array[3]);
							payitem->imported = TRUE;
							da_pay_append(payitem);

							if( payitem->imported == TRUE )
								ictx->cnt_new_pay += 1;
						}

						newope->payee = payitem->key;
						newope->wording		 = g_strdup(str_array[4]);
						newope->amount		 = hb_qif_parser_get_amount(str_array[5]);

						/* category */
						g_strstrip(str_array[6]);
						catitem = da_cat_append_ifnew_by_fullname(str_array[6], TRUE);
						if( catitem != NULL )
						{
							newope->category = catitem->key;

							if( catitem->imported == TRUE && catitem->key > 0 )
								ictx->cnt_new_cat += 1;
						}
						
						newope->account		= tmp_acc->key;
						//newope->dst_account = accnum;

						newope->flags |= OF_ADDED;

						if( newope->amount > 0)
							newope->flags |= OF_INCOME;

						/*
						DB( g_print(" storing %s : %s : %s :%s : %s : %s : %s : %s\n", 
							str_array[0], str_array[1], str_array[2],
							str_array[3], str_array[4], str_array[5],
							str_array[6], str_array[7]
							) );
						*/

						list = g_list_append(list, newope);

						g_strfreev (str_array);
					}
				}
				g_free(tmpstr);
			}

		}
		g_io_channel_unref (io);

	/*
		homebank_message_dialog(data->window, error > 0 ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
			_("Operation CSV import result"),
			_("%d operations inserted\n%d errors in the file"),
			count, error);
		*/ 
	}


	return list;
}

#ifndef NOOFX
/*
**** OFX part
**** 
**** this part is quite weird,but works
** id is ACCTID

*/

static Account * ofx_get_account_by_id(gchar *id)
{
GList *list;

	DB( g_print("(import) ofx_get_account_by_id\n") );
	DB( g_print(" -> searching for '%s'\n",id) );

	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *accitem = list->data;

		if( accitem->imported == FALSE)
		{
			if(accitem->name && accitem->number && strlen(accitem->number) )
			{
				// todo: maybe smartness should be done here
				if(g_strstr_len(id, -1, accitem->number) != NULL)
				{
					return accitem;
				}
			}			
		}
		list = g_list_next(list);
	}
	g_list_free(list);
	return NULL;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ofx_proc_account_cb:
 *
 * The ofx_proc_account_cb event is always generated first, to allow the application to create accounts
 * or ask the user to match an existing account before the ofx_proc_statement and ofx_proc_transaction 
 * event are received. An OfxAccountData is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_account_cb(const struct OfxAccountData data, OfxContext *ctx)
{
Account *tmp_acc, *dst_acc;

	DB( g_print("** ofx_proc_account_cb()\n") );

	if(data.account_id_valid==true)
	{
		DB( g_print("  account_id: %s\n", data.account_id) );
		DB( g_print("  account_name: %s\n", data.account_name) );
	}

	//if(data.account_number_valid==true)
	//{
		DB( g_print("  account_number: %s\n", data.account_number) );
	//}


	if(data.account_type_valid==true)
	{
		DB( g_print("  account_type: %d\n", data.account_type) );
	}

	if(data.currency_valid==true)
	{
		DB( g_print("  currency: %s\n", data.currency) );
	}


	//find target account
	dst_acc = ofx_get_account_by_id( (gchar *)data.account_id );
	DB( g_print(" ** hb account found result is %x\n", (unsigned int)dst_acc) );


	// in every case we create an account here
	tmp_acc = import_create_account((gchar *)data.account_name, (gchar *)data.account_id);
	DB( g_print(" -> creating tmp account: %d %s - %x\n", tmp_acc->key, data.account_id, (unsigned int)tmp_acc) );

	if( dst_acc != NULL )
	{
		tmp_acc->imp_key = dst_acc->key;
	}


	ctx->curr_acc = tmp_acc;
	ctx->curr_acc_isnew = TRUE;




	



	DB( fputs("\n",stdout) );
	return 0;
}


/**
 * ofx_proc_statement_cb:
 *
 * The ofx_proc_statement_cb event is sent after all ofx_proc_transaction events have been sent.
 * An OfxStatementData is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_statement_cb(const struct OfxStatementData data, OfxContext *ctx)
{
	DB( g_print("** ofx_proc_statement_cb()\n") );

#ifdef MYDEBUG
	if(data.ledger_balance_date_valid==true)
	{
	struct tm temp_tm;	

		temp_tm = *localtime(&(data.ledger_balance_date));
		DB( printf("ledger_balance_date : %d%s%d%s%d%s", temp_tm.tm_mday, "/", temp_tm.tm_mon+1, "/", temp_tm.tm_year+1900, "\n") );
	}
#endif

	if(data.ledger_balance_valid==true)
	{
		if( ctx->curr_acc != NULL && ctx->curr_acc_isnew == TRUE )
		{	
			ctx->curr_acc->initial = data.ledger_balance;
		}
		DB( printf("ledger_balance: $%.2f%s",data.ledger_balance,"\n") );
	}

	return 0;
}

/**
 * ofx_proc_statement_cb:
 *
 * An ofx_proc_transaction_cb event is generated for every transaction in the ofx response, 
 * after ofx_proc_statement (and possibly ofx_proc_security is generated.
 * An OfxTransactionData structure is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_transaction_cb(const struct OfxTransactionData data, OfxContext *ctx)
{
struct tm temp_tm;
GDate *date;
Operation *newope;

	DB( g_print("** ofx_proc_transaction_cb()\n") );

	newope = da_operation_malloc();

// date	
	if(data.date_posted_valid==true)
	{
		temp_tm = *localtime(&(data.date_posted));
		date = g_date_new();
		g_date_set_dmy(date, temp_tm.tm_mday, temp_tm.tm_mon+1, temp_tm.tm_year+1900);
		newope->date = g_date_get_julian(date);
		g_date_free(date);
	}
// amount
	if(data.amount_valid==true)
	{
		newope->amount = data.amount;

		if( newope->amount > 0)
			newope->flags |= OF_INCOME;
	}
// check number
	if(data.check_number_valid==true)
	{
		newope->info = g_strdup(data.check_number);
	}
	//todo: reference_number ?

// wording
	if(data.name_valid==true)
	{
		newope->wording = g_strdup(data.name);
	}

//memo ( new for v4.2)

	DB( g_print(" -> memo is='%d'\n", data.memo_valid) );

	
	if(data.memo_valid==true)
	{
	gchar *old = NULL;
		
		switch(PREFS->dtex_ofxmemo)
		{
			case 1:	//add to info
				old = newope->info;
				if(old == NULL)
					newope->info = g_strdup(data.memo);
				else
				{
					newope->info = g_strjoin(" ", old, data.memo, NULL);
					g_free(old);
				}
				break;
				
			case 2: //add to description
				old = newope->wording;
				if(old == NULL)
					newope->wording = g_strdup(data.memo);
				else
				{	
					newope->wording = g_strjoin(" ", old, data.memo, NULL);
					g_free(old);
				}

				DB( g_print(" -> should concatenate ='%s'\n", data.memo) );
				DB( g_print(" -> old='%s', new ='%s'\n", old, newope->wording) );

				break;
		}
	}
	
// payment
	if(data.transactiontype_valid==true)
	{
		switch(data.transactiontype)
		{
			case OFX_CREDIT:
				
				break;
			case OFX_DEBIT:
			 		newope->paymode = PAYMODE_XFER;
				break;
			case OFX_INT:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_DIV:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_FEE:
					newope->paymode = PAYMODE_FEE;
				break;
			case OFX_SRVCHG:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_DEP:
					newope->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_ATM:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_POS:
				if(ctx->curr_acc && ctx->curr_acc->type == ACC_TYPE_CREDITCARD)
					newope->paymode = PAYMODE_CCARD;
				else
					newope->paymode = PAYMODE_DCARD;
				break;
			case OFX_XFER:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_CHECK:
					newope->paymode = PAYMODE_CHECK;
				break;
			case OFX_PAYMENT:
					newope->paymode = PAYMODE_EPAYMENT;
				break;
			case OFX_CASH:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_DIRECTDEP:
					newope->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_DIRECTDEBIT:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_REPEATPMT:
					newope->paymode = PAYMODE_REPEATPMT;
				break;
			case OFX_OTHER:
					
				break;
			default :
					
				break;
		}
	}

	if( ctx->curr_acc )
	{

		newope->account = ctx->curr_acc->key;
		newope->flags |= OF_ADDED;	

		/* ensure utf-8 here, has under windows, libofx not always return utf-8 as it should */
	#ifndef G_OS_UNIX
		DB( printf(" ensure UTF-8\n") );

		newope->info = homebank_utf8_ensure(newope->info);
		newope->wording = homebank_utf8_ensure(newope->wording);
	#endif
		
		ctx->trans_list = g_list_append(ctx->trans_list, newope);

		DB( printf(" insert newope: acc=%d\n", newope->account) );
		
		if( ctx->curr_acc_isnew == TRUE )
		{	
			ctx->curr_acc->initial -= data.amount;
		}
	}
	else
	{
		da_operation_free(newope);
	}

	return 0;
}








static GList *homebank_ofx_import(gchar *filename, ImportContext *ictx)
{
OfxContext ctx = { 0 };
gchar *argv[2];

extern int ofx_PARSER_msg;
extern int ofx_DEBUG_msg;
extern int ofx_WARNING_msg;
extern int ofx_ERROR_msg;
extern int ofx_INFO_msg;
extern int ofx_STATUS_msg;

	DB( g_print("(import) ofx import\n") );

	ofx_PARSER_msg	= false;
	ofx_DEBUG_msg	= false;
	ofx_WARNING_msg = false;
	ofx_ERROR_msg	= false;
	ofx_INFO_msg	= false;
	ofx_STATUS_msg	= false;

	LibofxContextPtr libofx_context = libofx_get_new_context();
	ofx_set_statement_cb  (libofx_context, (LibofxProcStatementCallback)  ofx_proc_statement_cb  , &ctx);
	ofx_set_account_cb    (libofx_context, (LibofxProcAccountCallback)    ofx_proc_account_cb    , &ctx);
	ofx_set_transaction_cb(libofx_context, (LibofxProcTransactionCallback)ofx_proc_transaction_cb, &ctx);

	argv[1] = filename;
	libofx_proc_file(libofx_context, argv[1], OFX);
	libofx_free_context(libofx_context);

	return ctx.trans_list;
}

#endif




static GList *homebank_qif_import(gchar *filename, ImportContext *ictx)
{
GList *list = NULL;

	DB( g_print("(import) homebank QIF\n") );


	//todo: context ?
	list = account_import_qif(filename, ictx);


	return list;
}











static void import_clearall(struct import_data *data)
{
GList *list;

	DB( g_print("(import) clear all\n") );

	// clear the active file into filechooser
	//gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(data->filechooser), PREFS->path_import);
	//gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(data->filechooser), "");
	
	// clear transactions
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope))));
	da_operation_destroy(data->ictx.trans_list);
	data->ictx.trans_list = NULL;

	//todo: remove imported account ?
	// 1: remove imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE)
		{
			//DB( g_print(" -> remove acc %x '%s'\n", item, item->name) );
			da_acc_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	data->ictx.cnt_initial_acc = da_acc_length();

	//gtk_entry_set_text(GTK_ENTRY (GTK_BIN (data->PO_acc)->child), "");
	
	

}

static void import_selchange(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;
gint page_number;
GtkWidget *current_page;
gchar *filename;
	
	page_number = gtk_assistant_get_current_page (GTK_ASSISTANT(data->assistant));

	DB( g_print("(import) selchange (page %d)\n", page_number+1) );

	data->valid = FALSE;	

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(data->filechooser));
	if( filename == NULL )
	{
		gtk_label_set_text(GTK_LABEL(data->user_info), _("Please select a file..."));
		//current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);	
		//gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, FALSE);
	}
	else
	{
		if( page_number == PAGE_FILE )
		{	
			if(data->filename) 
				g_free( data->filename );
			data->filename = filename;
			//DB( g_print(" filename -> %s\n", data->filename) );
	
			data->filetype = homebank_alienfile_recognize(data->filename);
			switch(data->filetype)
			{
				case FILETYPE_QIF:
					gtk_label_set_text(GTK_LABEL(data->user_info), _("QIF file recognised !"));
					data->valid = TRUE;
					break;

				case FILETYPE_OFX:
					#ifndef NOOFX
					gtk_label_set_text(GTK_LABEL(data->user_info), _("OFX file recognised !"));
					data->valid = TRUE;
					#else
					gtk_label_set_text(GTK_LABEL(data->user_info), _("** OFX support is disabled **"));
					#endif
					break;

				case FILETYPE_CSV_HB:
					gtk_label_set_text(GTK_LABEL(data->user_info), _("CSV operation file recognised !"));
					data->valid = TRUE;
					break;

				default:
					data->filetype = FILETYPE_UNKNOW;
					gtk_label_set_text(GTK_LABEL(data->user_info), _("Unknown/Invalid file..."));
					break;
			}

			current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);	
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, data->valid);

		}

	}

	if(data->valid == TRUE)
	{
		gtk_widget_show(data->ok_image);
		gtk_widget_hide(data->ko_image);
	}
	else
	{
		gtk_widget_show(data->ko_image);
		gtk_widget_hide(data->ok_image);
	}	

}



/*
 * find duplicate operations
 *
 * for 
 *
 */

static void import_find_duplicate_operations(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
GList *tmplist, *implist;
Operation *item;
guint32 mindate;
guint decay;

	DB( g_print("(import) find duplicate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( data->ictx.trans_list )
	{
		/* 1: get import min bound date */
		tmplist = g_list_first(data->ictx.trans_list);
		item = tmplist->data;
		mindate = item->date;
		decay = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_decay));

		tmplist = g_list_first(GLOBALS->ope_list);
		while (tmplist != NULL)
		{
		Operation *ope = tmplist->data;

			//if( ope->account == accnum && ope->date >= mindate )
			if( ope->date >= mindate )
			{
				//DB( g_print("should check here %d: %s\n", ope->date, ope->wording) );

				implist = g_list_first(data->ictx.trans_list);
				while (implist != NULL)
				{
				Operation *impope = implist->data;
				Account *acc;
				guint acckey;
				
					//here we should test acc->imp_key and not impope->account
					acckey = impope->account;
					acc = da_acc_get(acckey);
					if( acc )
					{
						if( acc->imp_key > 0  )
							acckey = acc->imp_key;
					}

					if( 
						(acckey == ope->account) && 
						(impope->amount == ope->amount) && 
						(ope->date <= impope->date+decay) && (ope->date >= impope->date-decay)
						)
					{
						//DB( g_print(" found %d: %s\n", impope->date, impope->wording) );

						impope->same = g_list_append(impope->same, ope);
					}

					implist = g_list_next(implist);
				}
			}

			tmplist = g_list_next(tmplist);
		}
	}
	

}


static void import_populate_account(struct import_data *data)
{
GList *list;

	DB( g_print("(import) populate account\n") );

	// clear accounts
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))));


	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			ui_acc_affect_listview_add(GTK_TREE_VIEW(data->LV_acc), item);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

}




/* count account to be imported */
static void import_analysis_count(struct import_data *data)
{
GList *list;

	DB( g_print("(import) count_new_account\n") );

	data->ictx.cnt_new_acc = 0;
	data->ictx.cnt_new_ope = 0;

	/* count account */
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			data->ictx.cnt_new_acc++;
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	/* count transaction */
	data->ictx.cnt_new_ope = g_list_length(data->ictx.trans_list);

}



/* count transaction with checkbox 'import'  */
static void import_count_changes(struct import_data *data)
{
GList *list;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;

	DB( g_print("(import) count_final_changes\n") );

	data->imp_cnt_acc = 0;

	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE && item->imp_key != 0)
		{
			data->imp_cnt_acc++;
		}
		list = g_list_next(list);
	}
	g_list_free(list);


	// then import operations
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));

	data->imp_cnt_trn = 0;

	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
			data->imp_cnt_trn++;

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}







static void import_apply(struct import_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GList *list;

	DB( g_print("(import) apply\n") );

	// 1: persist imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			//only persist user selected to new account
			if( item->imp_key == 0)
			{
				//DB( g_print(" -> persist acc %x '%s'\n", item, item->name) );
				item->imported = FALSE;
				g_free(item->imp_name);
				item->imp_name = NULL;
			}
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// 2: persist imported payees
	list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> persist pay '%s'\n", item->name) );
			item->imported = FALSE;
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// 3: persist imported categories
	list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> persist cat '%s'\n", item->name) );
			item->imported = FALSE;
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// 4: insert every operations
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Operation *item;
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DSPOPE_DATAS, &item,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
		{
		Account *acc;
			//DB(g_print("import %d to acc: %d\n", data->total, item->account)	);

			//todo: here also test imp_key on account and change the key into the transaction
			
			acc = da_acc_get(item->account);
			if( acc != NULL)
			{
				if( acc->imp_key > 0)
				{
					item->account = acc->imp_key;
				}
			}

			operation_add(item, NULL, 0);
		}

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}


}

/*
**
*/
static gboolean
import_dispose(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;
GList *list;

	DB( g_print("(import) dispose\n") );

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_print(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

	g_free( data->filename );

	import_clearall(data);


	// 1: remove imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> remove acc %x '%s'\n", item, item->name) );
			da_acc_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// 2: remove imported payees
	list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> remove pay '%s'\n", item->name) );
			da_pay_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// 3: remove imported category
	list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> remove cat '%s'\n", item->name) );
			da_cat_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	// todo: optimize this
	if(data->imp_cnt_trn > 0)
	{
		GLOBALS->change += data->imp_cnt_trn;
		wallet_populate_listview(GLOBALS->mainwindow, NULL);
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
	}	


	g_free(user_data);


	//delete-event TRUE abort/FALSE destroy
	return FALSE;
}


static void import_fill_imp_operations(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
GtkWidget *view;
GtkTreeModel *model;
GtkTreeIter	iter;
GList *tmplist;

	DB( g_print("(import) fill imp operatoin\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	view = data->imported_ope;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	tmplist = g_list_first(data->ictx.trans_list);
	while (tmplist != NULL)
	{
	Operation *item = tmplist->data;

		/* append to our treeview */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);

				//DB( g_print(" populate: %s\n", ope->ope_Word) );

				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, item,
				LST_OPE_IMPTOGGLE, item->same == NULL ? TRUE : FALSE,
				-1);

		//DB( g_print(" - fill: %d, %s %.2f %x\n", item->account, item->wording, item->amount, item->same) );

		tmplist = g_list_next(tmplist);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */

	g_object_unref(model);



}

static void import_fillsame(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model, *newmodel;
GtkTreeIter			 iter, newiter;
GList *tmplist;
GtkWidget *view;


	DB( g_print("(import) fillsame\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	view = data->duplicat_ope;
	newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(newmodel));



	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Operation *item;
	
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &item, -1);

		if( item->same != NULL )
		{
			tmplist = g_list_first(item->same);
			while (tmplist != NULL)
			{
			Operation *tmp = tmplist->data;

				/* append to our treeview */
					gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

					gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
					LST_DSPOPE_DATAS, tmp,
					-1);

				DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, (unsigned int)item->same) );

				tmplist = g_list_next(tmplist);
			}
		}

	}


}


/*
** modify target account
*/


static void on_account_type_toggled(GtkRadioButton *radiobutton, gpointer user_data)
{
struct import_target_data *data;
gboolean new_account;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(import) account type toggle\n") );

	new_account = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0]));

	gtk_widget_set_sensitive(data->getwidget1, new_account);
	gtk_widget_set_sensitive(data->getwidget2, new_account^1);

}


static void import_edit_target_account(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
struct import_target_data ddata;
GtkWidget *window, *mainvbox, *table, *label ;
guint32 key;
gint row;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(import) edit target account\n") );

	key = ui_acc_affect_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
	Account *item;

		item = da_acc_get( key );

		window = gtk_dialog_new_with_buttons (_("Change HomeBank account target"),
						    GTK_WINDOW (data->assistant),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		//store our window private data
		g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&ddata);

		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

		table = gtk_table_new (3, 2, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
		gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
		gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, HB_BOX_SPACING);

		/* area 1 : file summary */

		row = 0;
		ddata.radio[0] = gtk_radio_button_new_with_label (NULL, _("new account"));
		gtk_table_attach (GTK_TABLE (table), ddata.radio[0], 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Name:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		ddata.getwidget1 = gtk_entry_new();
		gtk_table_attach (GTK_TABLE (table), ddata.getwidget1, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		row++;
		ddata.radio[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (ddata.radio[0]), _("existing account"));
		gtk_table_attach (GTK_TABLE (table), ddata.radio[1], 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("A_ccount:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		ddata.getwidget2 = ui_acc_comboboxentry_new(NULL);
		gtk_table_attach (GTK_TABLE (table), ddata.getwidget2, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//initialize
		if( data->ictx.cnt_initial_acc > 0 )	//if there were already some accounts
		{
			gtk_widget_set_sensitive(ddata.radio[1], TRUE);
			if( item->imp_key > 0 )
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ddata.radio[1]), TRUE);
			}
		}
		else
		{
			gtk_widget_set_sensitive(ddata.radio[1], FALSE);
			
		}
		
		gtk_entry_set_text(GTK_ENTRY(ddata.getwidget1), item->name);
		ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(ddata.getwidget2), GLOBALS->h_acc);
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(ddata.getwidget2), item->imp_key);
		
		on_account_type_toggled(GTK_RADIO_BUTTON (ddata.radio[0]), NULL);

		gtk_widget_show_all(mainvbox);

		g_signal_connect (ddata.radio[0], "toggled", G_CALLBACK (on_account_type_toggled), NULL);


		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gchar *name;
		gboolean bnew;
		guint key;

			key = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(ddata.getwidget2));

			bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ddata.radio[0]));
			if( bnew )
			{

				name = (gchar *)gtk_entry_get_text(GTK_ENTRY(ddata.getwidget1));
		
				if(strcasecmp(name, item->name))
				{
		
					DB( g_print("name '%s', existing acc %d\n", name, key) );
		
					if (name && *name)
					{
						if( account_rename(item, name) == FALSE )
						{
							homebank_message_dialog(GTK_WINDOW(window), GTK_MESSAGE_ERROR,
								_("Error"),
								_("Cannot rename this Account,\n"
								"from '%s' to '%s',\n"
								"this name already exists."),
								item->name,
								name
								);
						}
					}
				}	
				else
				{
					item->imp_key = 0;
				}
			}
			else
			{
				item->imp_key = key;
			}

			//we should refresh duplicate
			import_find_duplicate_operations(widget, NULL);
			import_fill_imp_operations(widget, NULL);

	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}





static void import_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	import_fillsame(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


static void ui_import_update_filecontent(struct import_data *data)
{
gchar *tmpstr;

	/* filename */
	tmpstr = g_path_get_basename(data->filename);
	gtk_label_set_text(GTK_LABEL(data->TX_filename), tmpstr);
	g_free(tmpstr);

	/* file content detail */
	//TODO: difficult translation here
	tmpstr = g_strdup_printf(_("account: %d - transaction: %d - payee: %d - categorie: %d"),
				data->ictx.cnt_new_acc,
				data->ictx.cnt_new_ope,
				data->ictx.cnt_new_pay,
				data->ictx.cnt_new_cat
				);
	gtk_label_set_text(GTK_LABEL(data->TX_filedetails), tmpstr);
	g_free(tmpstr);	
}

static void ui_import_integer_to_label(guint value, GtkWidget *label)
{
gchar *tmpstr;

	tmpstr = g_strdup_printf("%d", value);
	gtk_label_set_text(GTK_LABEL(label), tmpstr);
	g_free(tmpstr);	

}


static void ui_import_update_summary(struct import_data *data)
{

	/* account summary */
	ui_import_integer_to_label(data->imp_cnt_acc     , data->TX_acc_upd);
	ui_import_integer_to_label(data->ictx.cnt_new_acc - data->imp_cnt_acc, data->TX_acc_new);

	/* transaction summary */
	ui_import_integer_to_label(data->imp_cnt_trn     , data->TX_trn_imp);
	ui_import_integer_to_label(data->ictx.cnt_new_ope - data->imp_cnt_trn, data->TX_trn_nop);
	ui_import_integer_to_label(data->imp_cnt_asg     , data->TX_trn_asg);

}

static void
on_assistant_apply (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;

	/* Apply here changes, this is a fictional
		 example, so we just do nothing here */

	DB( g_print("(import) apply\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	import_apply(data);

}

static void
on_assistant_close_cancel (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
	GtkWidget *assistant = (GtkWidget *) user_data;

	DB( g_print("(import) close\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	import_dispose(widget, data);


	//g_free(data);


	gtk_widget_destroy (assistant);
	//assistant = NULL;
}

static void
on_assistant_prepare (GtkWidget *widget, GtkWidget *page, gpointer user_data)
{
struct import_data *data;
gint current_page, n_pages;
gchar *title;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	current_page = gtk_assistant_get_current_page (GTK_ASSISTANT (widget));
	n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

	DB( g_print("(import) prepare %d of %d\n", current_page, n_pages) );

	switch( current_page  )
	{
		case PAGE_INTRO:
			DB( g_print(" -> 1 intro\n") );
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		case PAGE_FILE:
			DB( g_print(" -> 2 file choose\n") );

			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_OPTIONS], FALSE);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_CONFIRM], FALSE);

			
			// the page complete is contextual in import_selchange
			break;
		case PAGE_OPTIONS:
			DB( g_print(" -> 3 real import\n") );
			//gtk_assistant_set_current_page(GTK_ASSISTANT (widget), PAGE_IMPORT);

			/* remind folder to preference */
			gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data->filechooser));
			g_free(PREFS->path_import);
			PREFS->path_import = folder;

			
			import_clearall(data);
			

			data->ictx.cnt_new_pay = 0;
			data->ictx.cnt_new_cat = 0;

			data->ictx.encoding = homebank_file_getencoding(data->filename);
			DB( g_print(" -> encoding is '%s'\n", data->ictx.encoding) );


			switch(data->filetype)
			{
#ifndef NOOFX
				/* ofx_acc_list & ofx_ope_list are filled here */
				case FILETYPE_OFX:
					data->ictx.trans_list = homebank_ofx_import(data->filename, &data->ictx);
					break;
#endif

				/* qif file */
				case FILETYPE_QIF:
					data->ictx.trans_list = homebank_qif_import(data->filename, &data->ictx);
					break;


				case FILETYPE_CSV_HB:
					data->ictx.trans_list = homebank_csv_import(data->filename, &data->ictx);
					break;
			}
		
		
			import_analysis_count(data);
			
			ui_import_update_filecontent(data);	

			
			
			//gchar *filename = homebank_get_filename_without_extension(data->filename);
			//gtk_entry_set_text(GTK_ENTRY(data->ST_acc), filename);
			//g_free(filename);

			import_populate_account(data);


			// sort by date
			data->ictx.trans_list = da_operation_sort(data->ictx.trans_list);
			import_find_duplicate_operations(widget, NULL);
			import_fill_imp_operations(widget, NULL);

			//todo: optional
			data->imp_cnt_asg = transaction_auto_assign(data->ictx.trans_list, -1);

		
			DB( g_print(" -> determine completion: nbtrans=%d\n", data->ictx.cnt_new_ope) );

			//TODO: Check this is sufficient
			if( data->ictx.cnt_new_ope == 0 )
			{
				gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, FALSE);
			}
			else
				gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);

			break;		

		case PAGE_CONFIRM:
		{
			DB( g_print(" -> 6 apply\n") );

			//todo:rework this
			import_count_changes(data);

			ui_import_update_summary(data);
			


 			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		}	
	}

	title = g_strdup_printf ( _("HomeBank Import Assistant - (%d of %d)"), current_page + 1 , n_pages );
	gtk_window_set_title (GTK_WINDOW (widget), title);
	g_free (title);
}


/**
 * gtk_assistant_forward_page_func:
 * 
 * define the page to be called when the user forward
 * 
 * Return value: the page number
 *
 */
static gint
gtk_assistant_forward_page_func(gint current_page, gpointer func_data)
{
//struct import_data *data = func_data;
gint next_page;

	DB( g_print("(import) forward page\n") );

	// normal forward
	next_page = current_page + 1;

	DB( g_print(" -> curr page: %d ==> next page: %d\n", current_page, next_page) );
	DB( g_print(" -> page is %s\n", page_titles[current_page]) );

	return next_page;
}





static void
import_refresh_transaction (GtkWidget *widget, gpointer data)
{

	DB( g_print("(import) refresh transaction\n") );

	import_find_duplicate_operations(widget, NULL);
	import_fill_imp_operations(widget, NULL);

}



static void import_acc_affect_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
GtkTreeModel		 *model;

	model = gtk_tree_view_get_model(treeview);
	//gtk_tree_model_get_iter_first(model, &iter);
	//if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	//{
		import_edit_target_account(GTK_WIDGET(treeview), NULL);
	//}
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * create_page1:
 * 
 * page 1: intro
 * 
 * Return value: a vbox widget
 *
 */
static GtkWidget *
create_page1(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label, *align;

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 0);
	
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	
	label = make_label(
	    _("Welcome to the HomeBank Import Assistant.\n" \
		"With this assistant you will be guided throught the process\n" \
		"of importing an external file into HomeBank.\n" \
	    "No changes will be made until you click \"Apply\" at the end\n" \
	    "of this assistant.")
			, 0., 0.0);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, HB_BOX_SPACING);

	/* supported format */


	label = make_label(
	    _("HomeBank can import files in the following formats:\n" \
		"- QIF\n" \
		"- OFX/QFX (optional at compilation time)\n" \
		"- CSV (format is specific to HomeBank, see the documentation)\n" \
	), 0.0, 0.0);
	 
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, HB_BOX_SPACING);


	gtk_widget_show_all (align);
	
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), align);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), align, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), align, _(page_titles[PAGE_INTRO]));
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), align, TRUE);

#if HEAD_IMAGE == 1
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), align, data->head_pixbuf);
#endif

#if SIDE_IMAGE == 1
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), align, data->side_pixbuf);
#endif

	
	return align;
}


/**
 * create_page2:
 * 
 * page 2: file selection
 * 
 * Return value: a vbox widget
 *
 */
static GtkWidget *
create_page2 (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *align, *widget, *label;
GtkFileFilter *filter;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);


//	widget = gtk_file_chooser_button_new ("Pick a File", GTK_FILE_CHOOSER_ACTION_OPEN);

	widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);

	data->filechooser = widget;
	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("QIF files"));
	gtk_file_filter_add_pattern (filter, "*.[Qq][Ii][Ff]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

	#ifndef NOOFX
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("OFX/QFX files"));
	gtk_file_filter_add_pattern (filter, "*.[OoQq][Ff][Xx]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);
	#endif

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.[Cc][Ss][Vv]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

/* our addon message */
	align = gtk_alignment_new(0.65, 0, 0, 0);
		gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
		gtk_container_add(GTK_CONTAINER(align), hbox);

	label = gtk_label_new("");
	data->user_info = label;
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);

	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
							PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);



	widget = gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
	data->ok_image = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		
	widget = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
	data->ko_image = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	gtk_widget_show_all (vbox);
	gtk_widget_hide(data->ok_image);
	gtk_widget_hide(data->ko_image);


	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
	//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_FILE]));

		                        
#if HEAD_IMAGE == 1
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, data->head_pixbuf);
#endif

#if SIDE_IMAGE == 1
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, data->side_pixbuf);
#endif

	
	return vbox;
}




static GtkWidget *
create_page3_summary (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *table, *label, *widget;
gint row;
	
	table = gtk_table_new (2, 2, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	
	/* area 1 : file summary */
	row = 0;
	label = make_label(_("Filename:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filename = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);
	//gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Content:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1,  row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filedetails = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);
	//gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	
	/*
	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Content of the file</b>"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), scrollwin, 1, 3, row, row+1);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);

	widget = gtk_text_view_new ();
	gtk_container_add(GTK_CONTAINER(scrollwin), widget);
	*/
	
	return table;
}

static GtkWidget *
create_page3_account (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *widget, *scrollwin;

	vbox = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), HB_BOX_SPACING);
	
 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	widget = ui_acc_affect_listview_new();
	gtk_widget_set_size_request(widget, -1, 100);
	data->LV_acc = widget;
	gtk_container_add(GTK_CONTAINER(scrollwin), widget);
	gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);	
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	widget = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	data->BT_edit = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	/* signal and other stuff */
	g_signal_connect (G_OBJECT (data->BT_edit), "clicked", G_CALLBACK (import_edit_target_account), data);
	g_signal_connect (GTK_TREE_VIEW(data->LV_acc), "row-activated", G_CALLBACK (import_acc_affect_onRowActivated), NULL);

	return vbox;
}


static GtkWidget *
create_page3_transaction (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *align, *hbox, *label, *sw, *widget, *expander;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_BOX_SPACING);

	
	/* transaction list */
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	widget = create_list_import_operation();
	data->imported_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	
	 /* Create the expander */
	expander = gtk_expander_new (_("Duplicate transaction found"));
	gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	widget = create_list_operation(TRN_LIST_TYPE_DETAIL, PREFS->lst_ope_columns);
	data->duplicat_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	//gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (expander), sw);
	
	//gtk_widget_set_size_request(sw, -1, 50);

	
	/* duplicate control */
	expander = gtk_expander_new (_("Change match options"));
	gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);
	
	align = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
	/* -- gtk_alignment_set_padding                 t , b, l, r -- */
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 12, 0);
	gtk_container_add(GTK_CONTAINER(expander), align);

	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);	
	gtk_container_add(GTK_CONTAINER(align), hbox);


	label = make_label(_("Date _tolerance:"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	widget = make_numeric(label, 0.0, 14.0);
	data->NB_decay = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	//TRANSLATORS: there is a spinner on the left of this label, and so you have 0....x days of date tolerance
	label = make_label(_("days"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

      g_signal_connect (widget, "clicked",
			G_CALLBACK (import_refresh_transaction), data);


	widget = gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_SMALL_TOOLBAR );
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	gtk_misc_set_padding(GTK_MISC(widget), HB_BOX_SPACING, 0);
	

	label = gtk_label_new(_(
		"The match is done in order: by account, amount and date.\n" \
		"A date tolerance of 0 day means an exact match")
		);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

	
	gtk_widget_show_all (vbox);

	return vbox;
}


/**
 * create_page3:
 * 
 * page 3: analysis & options
 * 
 * Return value: a table widget
 *
 */
static GtkWidget *
create_page3 (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *table, *label, *expander, *align;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	/* file information */
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>File informations</b>"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

		align = gtk_alignment_new(0.5, 0.0, 1.0, 1.0);
		gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, HB_HSPACE_SPACING, 0);
		gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);
	
		table = create_page3_summary(assistant, data);
		gtk_container_add(GTK_CONTAINER(align), table);

	/* account selection */
	expander = gtk_expander_new (NULL);
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Account to import</b>"));
	gtk_expander_set_label_widget(GTK_EXPANDER(expander), label);
	gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);	

		align = gtk_alignment_new(0.5, 0.0, 1.0, 1.0);
		gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, HB_HSPACE_SPACING, 0);
		gtk_container_add(GTK_CONTAINER(expander), align);
	
		table = create_page3_account(assistant, data);
		gtk_container_add(GTK_CONTAINER(align), table);
		 
	/* transaction selection */
	expander = gtk_expander_new (NULL);
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Transactions to import</b>"));
	gtk_expander_set_label_widget(GTK_EXPANDER(expander), label);
	gtk_box_pack_start (GTK_BOX (vbox), expander, TRUE, TRUE, 0);

		align = gtk_alignment_new(0.5, 0.0, 1.0, 1.0);
		gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, HB_HSPACE_SPACING, 0);
		gtk_container_add(GTK_CONTAINER(expander), align);

		table = create_page3_transaction(assistant, data);
		gtk_container_add(GTK_CONTAINER(align), table);

	gtk_expander_set_expanded (GTK_EXPANDER(expander), TRUE);
	
	gtk_widget_show_all (vbox);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
//	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_PROGRESS);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_OPTIONS]));

#if HEAD_IMAGE == 1
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, data->head_pixbuf);
#endif

#if SIDE_IMAGE == 1
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, data->side_pixbuf);
#endif

	
	return vbox;
}


/**
 * create_page6:
 * 
 * page 6: confirmation
 * 
 * Return value: a vbox widget
 *
 */
static GtkWidget *
create_page6(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label, *align, *widget, *table;
gint row;

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 0);
	
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	
	label = make_label(
		_("Click \"Apply\" to update your accounts.\n"), 0.5, 0.5);
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	/* the summary */
	table = gtk_table_new (7, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING/2);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
	
	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Accounts</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	/* acc update */
	row++;
	label = make_label(NULL, 0.0, 0.5);
	//gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_acc_upd = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	label = make_label(_("to update"), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, row, row+1);

	/* acc create */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_acc_new = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	label = make_label(_("to create"), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, row, row+1);

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Transactions</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	/* trn import */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_imp = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	label = make_label(_("to import"), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, row, row+1);
	
	/* trn reject */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_nop = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	label = make_label(_("to reject"), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, row, row+1);
	
	/* trn auto-assigned */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_asg = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	label = make_label(_("auto-assigned"), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, row, row+1);

	
	gtk_widget_show_all (align);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), align);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), align, GTK_ASSISTANT_PAGE_CONFIRM);
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), align, _(page_titles[PAGE_CONFIRM]));

#if HEAD_IMAGE == 1
	gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), align, data->head_pixbuf);
#endif

#if SIDE_IMAGE == 1
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), align, data->side_pixbuf);
#endif

	return align;
}


GtkWidget *create_import_window (void)
{
struct import_data *data;
GtkWidget *assistant;
GdkScreen *screen;
gint width, height;
gint pos;
	
	data = g_malloc0(sizeof(struct import_data));
	if(!data) return NULL;


	assistant = gtk_assistant_new ();
	data->assistant = assistant;

	//store our window private data
	g_object_set_data(G_OBJECT(assistant), "inst_data", (gpointer)data);
	//DB( g_print("** (import) window=%x, inst_data=%x\n", assistant, data) );
	
	
	gtk_window_set_modal(GTK_WINDOW (assistant), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(assistant), GTK_WINDOW(GLOBALS->mainwindow));

	/* set pixbuf */
#if HEAD_IMAGE == 1
	data->head_pixbuf = gtk_widget_render_icon (assistant, "hb-file-import", GTK_ICON_SIZE_DIALOG, NULL);
	g_object_unref (data->head_pixbuf);
#endif

#if SIDE_IMAGE == 1
	gchar *pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), "wizard.svg", NULL);
	data->side_pixbuf = gdk_pixbuf_new_from_file (pathfilename, NULL);
	g_free(pathfilename);
	g_object_unref (data->side_pixbuf);
#endif	

#if FORCE_SIZE == 1
	screen = gtk_window_get_screen(GTK_WINDOW (assistant));
	// fix #379372 : manage multiple monitor case
	if( gdk_screen_get_n_monitors(screen) > 1 )
	{
	GdkRectangle rect;
		
		gdk_screen_get_monitor_geometry(screen, 1, &rect);
		width = rect.width;
		height = rect.height;
	}
	else
	{
		width  = gdk_screen_get_width(screen);
		height = gdk_screen_get_height(screen);
	}
	
	gtk_window_resize(GTK_WINDOW(assistant), SCOEF*width, SCOEF*height);
	gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER_ON_PARENT);
	//gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);
#endif


	pos = 0;
	data->pages[pos++] = create_page1 (assistant, data);	// intro
	data->pages[pos++] = create_page2 (assistant, data);	// file choose
	data->pages[pos++] = create_page3 (assistant, data);	// file content
	data->pages[pos++] = create_page6 (assistant, data);	// confirm

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT(assistant), gtk_assistant_forward_page_func, data, NULL);

	//gtk_assistant_set_current_page(GTK_ASSISTANT (assistant), PAGE_FILE);
	
	//setup
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(data->filechooser), PREFS->path_import);
	//ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_acc), GLOBALS->h_acc);
	data->ictx.cnt_initial_acc = da_acc_length();

	import_selchange(assistant, data);

	//connect all our signals
	g_signal_connect (G_OBJECT (data->filechooser), "selection-changed",
		G_CALLBACK (import_selchange), (gpointer)data);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope)), "changed",
		G_CALLBACK (import_selection), NULL);

	g_signal_connect (G_OBJECT (assistant), "cancel",
		G_CALLBACK (on_assistant_close_cancel), assistant);

	g_signal_connect (G_OBJECT (assistant), "close",
		G_CALLBACK (on_assistant_close_cancel), assistant);

	g_signal_connect (G_OBJECT (assistant), "apply",
		G_CALLBACK (on_assistant_apply), NULL);

	g_signal_connect (G_OBJECT (assistant), "prepare",
		G_CALLBACK (on_assistant_prepare), NULL);

//    g_signal_connect (window, "delete-event", G_CALLBACK (wallet_dispose), (gpointer)data);

	
	gtk_widget_show (assistant);

	return assistant;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* account affect listview */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint32
ui_acc_affect_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Account *item;

		gtk_tree_model_get(model, &iter, 0, &item, -1);
		
		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}


static void
ui_acc_affect_listview_srcname_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;
gchar *string;

	gtk_tree_model_get(model, iter, 0, &entry, -1);

	name = entry->imp_name;

	#if MYDEBUG
		string = g_markup_printf_escaped("<i>[%d] %s</i>", entry->key, name );
	#else
		string = g_markup_printf_escaped("<i>%s</i>", name);
	#endif
	g_object_set(renderer, "markup", string, NULL);
	g_free(string);
}

static void
ui_acc_affect_listview_new_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;

	gtk_tree_model_get(model, iter, 0, &entry, -1);
	name = NULL;
	if(entry->imp_key == 0)
		name = _("<b>Create new</b>");
	else
		name = _("Import into");	
		
	g_object_set(renderer, "markup", name, NULL);

}

static void
ui_acc_affect_listview_dstname_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry, *dst_entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, 0, &entry, -1);
	name = NULL;
	if(entry->imp_key == 0)
		name = entry->name;
	else
	{
		dst_entry = da_acc_get(entry->imp_key);
		if( dst_entry != NULL )
			name = dst_entry->name;
	}

	#if MYDEBUG
		string = g_strdup_printf ("[%d] %s", entry->imp_key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}

void
ui_acc_affect_listview_add(GtkTreeView *treeview, Account *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, item,
			-1);

		//gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}


static GtkWidget *
ui_acc_affect_listview_new(void)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(1,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	// column: import account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("Imported name"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_srcname_cell_data_function, GINT_TO_POINTER(LST_DEFACC_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column: target account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("Action"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_new_cell_data_function, GINT_TO_POINTER(LST_DEFACC_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column: target account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("HomeBank name"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_dstname_cell_data_function, GINT_TO_POINTER(LST_DEFACC_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);



	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), TRUE);
	
	//gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_listview_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}



