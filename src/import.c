/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2008 Maxime DOYEN
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

#ifndef NOOFX
#include <libofx/libofx.h>
#endif

/****************************************************************************/
/* Debug macros																														 */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

#define HEAD_IMAGE 1
#define SIDE_IMAGE 0

#define SCOEF 0.6

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;



static gchar *page_titles[] = 
{
	N_("Import assistant"),
	N_("Select a file to import"),
	N_("File content"),
	N_("Transaction selection"),
	N_("Update your accounts")
};

static void on_account_type_toggled(GtkRadioButton *radiobutton, gpointer user_data);

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
** try to determine the file type (if supported for import by homebank
**
**
*/
static gint homebank_alienfile_recognize(gchar *filename)
{
GIOChannel *io;
gint i, retval = FILETYPE_UNKNOW;
gchar *tmpstr;
gint io_stat;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};

	DB( g_print("(import) file recognise\n") );

					
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		for(i=0;i<4;i++)
		{
			if( retval != FILETYPE_UNKNOW )
				break;
		
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( *tmpstr != '\0' )
				{
					DB( g_print(" line %4d: %s\n", i, tmpstr) );

					// QIF file ?
					if( g_str_has_prefix(tmpstr, "!Type:") || g_str_has_prefix(tmpstr, "!Option:") || g_str_has_prefix(tmpstr, "!Account"))
					{
						DB( g_print(" type is QIF\n") );
						retval = FILETYPE_QIF;
					}
					else
					
					/* is it OFX ? */
					if( g_strstr_len(tmpstr, 10, "OFX") != NULL)
					{
						DB( g_print(" type is OFX\n") );
						retval = FILETYPE_OFX;
					}
					
					/* is it csv homebank ? */
					else
					{
					gboolean hbcsv;
					
						hbcsv = hb_string_csv_valid(tmpstr, 7, csvtype);
						
						DB( g_print(" hbcsv %d\n", hbcsv) );
						
						if( hbcsv == TRUE )
						{
							DB( g_print(" type is CSV homebank\n") );
							retval = FILETYPE_CSV_HB;
						}
						

					}
	
					g_free(tmpstr);
				}
			}

		}
		g_io_channel_unref (io);
	}

	return retval;
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

	DB( g_print("** (import) homebank csv\n") );

	//we should always set for csv
	ictx->has_unknow_account = TRUE;

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr;
	gint io_stat;
	gboolean valid;
	gint count = 0;
	gint error = 0;
	Payee *payitem;
	Category *catitem;

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
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
						newope->amount		 = g_ascii_strtod(str_array[5],NULL);

						/* category */
						g_strstrip(str_array[6]);
						catitem = da_cat_append_ifnew_by_fullname(str_array[6], TRUE);
						if( catitem != NULL )
						{
							newope->category = catitem->key;

							if( catitem->imported == TRUE && catitem->key > 0 )
								ictx->cnt_new_cat += 1;
						}
						
						newope->account		= 0;
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
*/

static Account * ofx_get_account_by_id(gchar *id)
{
GList *list;

	DB( g_print("(import) ofx_get_account_by_id\n") );
	DB( g_print(" -> searching for '%s'\n",id) );

	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *entry = list->data;

		// todo: maybe smartness should be done here
		if(entry->name && g_ascii_strcasecmp(id, entry->number) == 0)
		{
			return entry;
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
Account *tmp_acc;

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



	// test account exist here, create, etc...
	tmp_acc = ofx_get_account_by_id( data.account_id );
	
	DB( g_print(" ** hb account found result is %x\n", (unsigned int)tmp_acc) );

	if( tmp_acc != NULL )
	{
		ctx->curr_acc = tmp_acc;
		ctx->curr_acc_isnew = FALSE;

		DB( g_print(" -> use existing account: %d %s - %x\n", tmp_acc->key, data.account_id, (unsigned int)tmp_acc) );
	}
	else
	{
		tmp_acc = da_acc_malloc();
		tmp_acc->name     = g_strdup(data.account_name);
		tmp_acc->number   = g_strdup(data.account_id);
		tmp_acc->imported = TRUE;
		da_acc_append(tmp_acc);

		ctx->curr_acc = tmp_acc;
		ctx->curr_acc_isnew = TRUE;
	
		DB( g_print(" -> creating tmp account: %d %s - %x\n", tmp_acc->key, data.account_id, (unsigned int)tmp_acc) );
	}

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

	//todo: memo ?

// payment
	if(data.transactiontype_valid==true)
	{
		switch(data.transactiontype)
		{
			case OFX_CREDIT:
				
				break;
			case OFX_DEBIT:
			 		newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_INT:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_DIV:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_FEE:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_SRVCHG:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_DEP:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_ATM:
					newope->paymode = PAYMODE_CARD;
				break;
			case OFX_POS:
					newope->paymode = PAYMODE_CARD;
				break;
			case OFX_XFER:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_CHECK:
					newope->paymode = PAYMODE_CHEQUE;
				break;
			case OFX_PAYMENT:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_CASH:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_DIRECTDEP:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_DIRECTDEBIT:
					newope->paymode = PAYMODE_BANKTRANSFERT;
				break;
			case OFX_REPEATPMT:
					newope->paymode = PAYMODE_BANKTRANSFERT;
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

	DB( g_print("------------------------------------\n") );
	DB( g_print("(import) ofx import\n") );

	ofx_PARSER_msg	= false;
	ofx_DEBUG_msg	= false;
	ofx_WARNING_msg = false;
	ofx_ERROR_msg	= false;
	ofx_INFO_msg	= false;
	ofx_STATUS_msg	= false;

	LibofxContextPtr libofx_context = libofx_get_new_context();
	ofx_set_statement_cb  (libofx_context, (LibofxProcStatementCallback)ofx_proc_statement_cb  , &ctx);
	ofx_set_account_cb    (libofx_context, (LibofxProcAccountCallback)ofx_proc_account_cb    , &ctx);
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

	DB( g_print("** (import) homebank QIF\n") );


	//todo: context ?
	list = account_import_qif(filename, ictx);


	return list;
}


static void import_clearall(struct import_data *data)
{
GList *list;

	DB( g_print("** (import) clear all\n") );

	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope))));

	da_operation_destroy(data->ictx.trans_list);
	data->ictx.trans_list = NULL;

	data->ictx.has_unknow_account = FALSE;
	
	//todo: remove imported account ?
	// 1: remove imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> remove acc '%s'\n", item->name) );
			da_acc_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	data->ictx.cnt_initial_acc = da_acc_length();

	gtk_entry_set_text(GTK_ENTRY (GTK_BIN (data->PO_acc)->child), "");
	
	

}

static void import_selchange(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;
gint page_number;
GtkWidget *current_page;


	DB( g_print("** (import) selchange (page %d)\n", page_number) );

	page_number = gtk_assistant_get_current_page (GTK_ASSISTANT(data->assistant));

	if( page_number == PAGE_FILE )
	{


		if(data->filename) 
			g_free( data->filename );
		data->filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(data->filechooser));
		//DB( g_print(" filename -> %s\n", data->filename) );
	

		data->valid = FALSE;	
		if( data->filename == NULL )
		{
			gtk_label_set_text(GTK_LABEL(data->user_info), _("Please select a file..."));
		}
		else
		{
			if( page_number == PAGE_FILE )
			{
	
				data->filetype = homebank_alienfile_recognize(data->filename);
				switch(data->filetype)
				{
					case FILETYPE_UNKNOW:
						gtk_label_set_text(GTK_LABEL(data->user_info), _("Unknown/Invalid file..."));
						break;

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
		
		
				}
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

		current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);	
		gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, data->valid);
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

	DB( g_print("** (import) find duplicate\n") );

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

					if( 
						(impope->account == ope->account) && 
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

/* count account to be imported */
static void import_analysis_count(struct import_data *data)
{
GList *list;

	DB( g_print("** (import) count_new_account\n") );

	data->ictx.cnt_new_acc = 0;
	data->ictx.cnt_new_ope = 0;



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

	data->ictx.cnt_new_ope = g_list_length(data->ictx.trans_list);

}



/* count transaction with checkbox 'import'  */
static void import_count_changes(struct import_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;

	// then import operations
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));

	data->imported = 0;
	data->total = 0;

	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
			data->imported++;

		/* Make iter point to the next row in the list store */
		data->total++;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}

static void import_apply(struct import_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GList *list;

	DB( g_print("** (import) apply\n") );

	// 1: persist imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> persist acc '%s'\n", item->name) );
			item->imported = FALSE;
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
			DB( g_print(" -> persist pay '%s'\n", item->name) );
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
			DB( g_print(" -> persist cat '%s'\n", item->name) );
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
			//DB(g_print("import %d to acc: %d\n", data->total, item->account)	);

			operation_add(item, NULL, 0);
		}

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	// todo: optimize this
	if(data->imported > 0)
	{
		GLOBALS->change += data->imported;
		wallet_populate_listview(GLOBALS->mainwindow, NULL);
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
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

	DB( g_print("** (import) dispose\n") );

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_print(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

	g_free( data->filename );

	import_clearall(data);

	g_free(user_data);

	// 1: remove imported accounts
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> remove acc '%s'\n", item->name) );
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
			//DB( g_print(" -> remove pay '%s'\n", item->name) );
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

	DB( g_print("** (import) fill\n") );

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


	DB( g_print("** (import) fillsame\n") );

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






static void import_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	import_fillsame(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


static void
on_assistant_apply (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;

	/* Apply here changes, this is a fictional
		 example, so we just do nothing here */

	DB( g_print("apply\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	import_apply(data);

}

static void
on_assistant_close_cancel (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
	GtkWidget *assistant = (GtkWidget *) user_data;

	DB( g_print("close\n") );

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
gint accnum;
GList *tmplist;
gchar *txt;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	current_page = gtk_assistant_get_current_page (GTK_ASSISTANT (widget));
	n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

	DB( g_print("\n** (import) prepare %d of %d\n", current_page, n_pages) );

	switch( current_page  )
	{
		case PAGE_INTRO:
			DB( g_print(" -> 1 intro\n") );
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		case PAGE_FILE:
			DB( g_print(" -> 2 file choose\n") );
			break;
		case PAGE_ANALYSIS:
			DB( g_print(" -> 3 real import\n") );
			//gtk_assistant_set_current_page(GTK_ASSISTANT (widget), PAGE_IMPORT);
			
			import_clearall(data);

			data->ictx.cnt_new_pay = 0;
			data->ictx.cnt_new_cat = 0;


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
			
			//upadte
			gtk_label_set_text(GTK_LABEL(data->TX_filename), data->filename);

			
				txt = g_strdup_printf(
				_(
				"%d accounts\n" \
				"%d transactions\n" \
				"%d payees\n" \
				"%d categories\n"
				),
				
				data->ictx.cnt_new_acc,
				data->ictx.cnt_new_ope,
				data->ictx.cnt_new_pay,
				data->ictx.cnt_new_cat
				);

			gtk_label_set_text(GTK_LABEL(data->TX_details), txt);
			g_free(txt);		
	
			
			gchar *filename = homebank_get_filename_without_extension(data->filename);
			gtk_entry_set_text(GTK_ENTRY(data->ST_acc), filename);
			g_free(filename);

			on_account_type_toggled(GTK_RADIO_BUTTON(data->CM_type[0]), NULL);

		//determine page complete
			DB( g_print(" -> determine completion: nbtrans=%d, unknow account=%d\n", data->ictx.cnt_new_ope, data->ictx.has_unknow_account) );

			if( data->ictx.cnt_new_ope == 0 )
			{
				gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, FALSE);
			}
			else if( data->ictx.has_unknow_account == TRUE )
			{
			gboolean bnew;
			guint key;
			const gchar *txt;

				bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0]));
				if(!bnew)
				{
					key = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
					gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, key > 0 ? TRUE : FALSE);
				}
				else
				{
					txt = gtk_entry_get_text(GTK_ENTRY(data->ST_acc));
					gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, strlen(txt) > 0 ? TRUE : FALSE);
				}

			}
			else
				gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);

			break;		

		case PAGE_TRANSACTION:		/* import */
			DB( g_print(" -> 6 choose transaction\n") );

			// if an unknow account was found, transaction are attached to 0 account
			// so we must change this to user choice
			if( data->ictx.has_unknow_account == TRUE )
			{
			gboolean bnew;
			const gchar *txt;
			
				bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0]));
				if(bnew)
				{
				Account *acc;
				
				
					txt = gtk_entry_get_text(GTK_ENTRY(data->ST_acc));
					
					DB( g_print(" -> create new account named %s\n", txt) );

					acc = da_acc_malloc();
					acc->name = g_strdup(txt);
					acc->imported = TRUE;
					da_acc_append(acc);

					accnum = acc->key;
					
					data->ictx.cnt_new_acc += 1;
				
				}
				else
				{
					accnum = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
				}
	
				DB( g_print(" -> should update accnum from 0 to %d\n", accnum) );
				
				tmplist = g_list_first(data->ictx.trans_list);
				while (tmplist != NULL)
				{
				Operation *ope = tmplist->data;
					
					if(!ope->account)
						ope->account = accnum;

					tmplist = g_list_next(tmplist);
				}
			}

			// sort by date
			data->ictx.trans_list = da_operation_sort(data->ictx.trans_list);
			import_find_duplicate_operations(widget, NULL);
			import_fill_imp_operations(widget, NULL);

			// progress ok, page is complete
	 		gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);

			break;
	
		case PAGE_CONFIRM:
		{
		gchar *txt;
		
			DB( g_print(" -> 6 apply\n") );

			//todo:rework this
			import_count_changes(data);

			txt = g_strdup_printf(
				_(
				"%d accounts will be created.\n\n" \
				"%d transactions will be imported.\n" \
				"%d transactions will be rejected."
				),
				data->ictx.cnt_new_acc,
				data->imported,
				data->total-data->imported
				);

			gtk_label_set_text(GTK_LABEL(data->last_info), txt);
			g_free(txt);		

 			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		}	
	}

	title = g_strdup_printf ( _("Import assistant (%d of %d)"), current_page + 1 , n_pages );
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

//	DB( g_print("\n** (import) forward page\n") );

	// normal forward
	next_page = current_page + 1;

	//DB( g_print(" -> page is %s\n", page_titles[current_page]) );

	/*
	if( next_page == PAGE_SETACCOUNT )
	{
		if(	data->ictx.has_unknow_account == FALSE )
		{
			DB( g_print(" -> skip account page\n") );
			next_page++;
		}
	}
	*/

	//DB( g_print(" -> curr page: %d ==> next page: %d\n", current_page, next_page) );

	return next_page;
}





static void
import_refresh_transaction (GtkWidget *widget, gpointer data)
{

	import_find_duplicate_operations(widget, NULL);
	import_fill_imp_operations(widget, NULL);

}



static void
on_account_changed (GtkWidget *widget, gpointer data)
{
GtkAssistant *assistant = GTK_ASSISTANT (data);
GtkWidget *current_page;
gint page_number;
guint key;

	DB( g_print("** (import) account changed\n") );

	page_number = gtk_assistant_get_current_page (assistant);
	current_page = gtk_assistant_get_nth_page (assistant, page_number);
	key = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(widget));

	gtk_assistant_set_page_complete (assistant, current_page, key > 0 ? TRUE : FALSE);

}

static void on_account_type_toggled(GtkRadioButton *radiobutton, gpointer user_data)
{
struct import_data *data;
gboolean new_account;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("** (import) account type toggle\n") );

	DB( g_print(" -> cnt_initial_acc = %d\n", data->ictx.cnt_initial_acc) );
	DB( g_print(" -> has_unknow_account = %d\n",data->ictx.has_unknow_account) );

	gtk_widget_set_sensitive(data->CM_type[1], data->ictx.cnt_initial_acc == 0 ? FALSE : TRUE);
	gtk_widget_set_sensitive(data->PO_acc, data->ictx.cnt_initial_acc == 0 ? FALSE : TRUE);

	if( data->ictx.has_unknow_account == TRUE )
	{
		new_account = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0]));

		DB( g_print(" -> new? %d\n", new_account) );
		
		//si new account, on unsensitive PO_name
		//sinon l'inverse
		
		gtk_widget_set_sensitive(data->PO_acc, new_account^1);
		gtk_widget_set_sensitive(data->ST_acc, new_account);
		
		gtk_widget_set_sensitive(data->CM_type[0], TRUE);
	}
	else
	{

		gtk_widget_set_sensitive(data->PO_acc, FALSE);
		gtk_widget_set_sensitive(data->ST_acc, FALSE);
		
		gtk_widget_set_sensitive(data->CM_type[0], FALSE);
		gtk_widget_set_sensitive(data->CM_type[1], FALSE);				
		
	}	

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
GtkWidget *vbox, *label;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(_(
	
		"HomeBank can import transactions from several file formats:\n" \
		"\n" \
		"- QIF\n" \
		"- OFX/QFX (optional at compilation time)\n" \
		"- CSV (Homebank transaction CSV export format only)\n" \
		"\n" \
		"Other formats or not supported at the moment.\n" \
		"\n" \
		"The import process has several steps. Your HomeBank accounts\n" \
		"will not be changed until you click \"Apply\" at the end of this assistant.\n" \
		"\n" \
		"Now, click \"forward\" to start importing a file."
	), 0.5, 0.5);
	 
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	gtk_widget_show_all (vbox);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_INTRO]));

#if HEAD_IMAGE == 1
GdkPixbuf *pixbuf;
  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

#if SIDE_IMAGE == 1
GdkPixbuf *pixbuf;
	pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
	g_object_unref (pixbuf);
#endif
	return vbox;
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


	widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
		data->filechooser = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("OFX/QFX files"));
	gtk_file_filter_add_pattern (filter, "*.?fx");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("QIF files"));
	gtk_file_filter_add_pattern (filter, "*.qif");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.csv");
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
GdkPixbuf *pixbuf;
  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

#if SIDE_IMAGE == 1
GdkPixbuf *pixbuf;
	pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
	//pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
	g_object_unref (pixbuf);
#endif
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
GtkWidget *vbox, *table, *hbox, *label, *widget;
gint row;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);


	table = gtk_table_new (6, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, HB_BOX_SPACING);

	/* area 1 : file summary */

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>File analysis summary</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	
	label = gtk_label_new (_("Filename:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filename = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = gtk_label_new (_("Details:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_details = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);


	/* area 2 : account select */
	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Unamed account selection</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 3, row, row+1);
	
	widget = gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_SMALL_TOOLBAR );
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	label = gtk_label_new(
		_(
		"The file that you just loaded appears to contain transactions for an account without specifying\n" \
		"the name of that account. Please select the account to which these transactions can be attached.")
		);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);	

	row++;
	widget = gtk_radio_button_new_with_label (NULL, _("new account"));
	data->CM_type[0] = widget;
 	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = make_label(_("_Name:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	widget = make_string(label);
	data->ST_acc = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (data->CM_type[0]), _("existing account"));
	data->CM_type[1] = widget;
 	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = make_label(_("A_ccount:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);


	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (on_account_changed), assistant);

	g_signal_connect (data->CM_type[0], "toggled", G_CALLBACK (on_account_type_toggled), NULL);

	gtk_widget_show_all (vbox);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
//	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_PROGRESS);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_ANALYSIS]));

#if HEAD_IMAGE == 1
GdkPixbuf *pixbuf;
  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

#if SIDE_IMAGE == 1
GdkPixbuf *pixbuf;
	pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
	//pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
	g_object_unref (pixbuf);
#endif
	return vbox;
}

/**
 * create_page4:
 * 
 * page 4: transaction selection
 * 
 * Return value: a vbox widget
 *
 */
static GtkWidget *
create_page4(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *align, *label, *sw, *widget, *expander;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Duplicate match options</b>"));
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	align = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	/* -- gtk_alignment_set_padding                 t , b, l, r -- */
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 12, 0);
	gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);	
	gtk_container_add(GTK_CONTAINER(align), hbox);


	label = make_label(_("Date _tolerance:"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);




	widget = make_numeric(label, 0.0, 14.0);
	data->NB_decay = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	label = make_label(_("days"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

      g_signal_connect (widget, "clicked",
			G_CALLBACK (import_refresh_transaction), data);


	widget = gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_SMALL_TOOLBAR );
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	gtk_misc_set_padding(GTK_MISC(widget), 12, 0);
	

	label = gtk_label_new(_(
		"The match is done in order: by account, amount and date.\n" \
		"A date tolerance of 0 days means an exact match")
		);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);


	/* -------- */

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Transactions to import</b>"));
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	align = gtk_alignment_new(0.5, 0, 1.0, 1.0);
	/* -- gtk_alignment_set_padding                 t , b, l, r -- */
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 12, 0);
	gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	//gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(align), sw);

	widget = create_list_import_operation();
	data->imported_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Possible duplicate for the above selected transaction</b>"));
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	 /* Create the expander */
	expander = gtk_expander_new (_("Details"));
	gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	widget = create_list_operation(PREFS->lst_ope_columns);
	data->duplicat_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	//gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (expander), sw);
	
	//gtk_widget_set_size_request(sw, -1, 50);

	gtk_widget_show_all (vbox);
	
	
	
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
	//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONFIRM);
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_TRANSACTION]));

#if HEAD_IMAGE == 1
GdkPixbuf *pixbuf;
  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

#if SIDE_IMAGE == 1
GdkPixbuf *pixbuf;
	pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
	//pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
	g_object_unref (pixbuf);
#endif

	return vbox;
}




/**
 * create_page5:
 * 
 * page 5: confirmation
 * 
 * Return value: a vbox widget
 *
 */
static GtkWidget *
create_page5(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);


	label = make_label(
		_("Click \"Apply\" to update your accounts.\n\n"), 0.5, 0.5);
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	label = make_label(NULL, 0.5, 0.5);
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	data->last_info = label;


	gtk_widget_show_all (vbox);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONFIRM);
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_CONFIRM]));

#if HEAD_IMAGE == 1
GdkPixbuf *pixbuf;
  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

#if SIDE_IMAGE == 1
GdkPixbuf *pixbuf;

	pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
	//pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
	gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
	g_object_unref (pixbuf);
#endif

	return vbox;
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

	gtk_window_set_modal(GTK_WINDOW (assistant), TRUE);
	//gtk_window_set_transient_for(GTK_WINDOW(assistant), GTK_WINDOW(GLOBALS->mainwindow));




	screen = gtk_window_get_screen(GTK_WINDOW (assistant));
	width  = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);
	
	gtk_window_resize(GTK_WINDOW(assistant), SCOEF*width, SCOEF*height);
	gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);

	//store our window private data
	g_object_set_data(G_OBJECT(assistant), "inst_data", (gpointer)data);
	DB( g_print("** (import) window=%x, inst_data=%x\n", assistant, data) );

	pos = 0;
	data->pages[pos++] = create_page1 (assistant, data);	// intro
	data->pages[pos++] = create_page2 (assistant, data);	// file choose
	data->pages[pos++] = create_page3 (assistant, data);	// analysis & options
	data->pages[pos++] = create_page4 (assistant, data);	// result
	data->pages[pos++] = create_page5 (assistant, data);	// confirm

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT(assistant), gtk_assistant_forward_page_func, data, NULL);

	//setup
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(data->filechooser), PREFS->path_import);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_acc), GLOBALS->h_acc);
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

	gtk_widget_show (assistant);

	return assistant;
}
