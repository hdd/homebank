/* HomeBank -- Free easy personal accounting for all !
 * Copyright (C) 1995-2007 Maxime DOYEN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "homebank.h"
#include "preferences.h"
#include "import.h"
#include "list_operation.h"


#ifndef NOOFX
#include <libofx/libofx.h>
#endif

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

#define SIDE_IMAGE 0

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;

struct import_data
{
	GtkWidget	*assistant;
	GtkWidget	*pages[7];

	GtkWidget	*filechooser;
	GtkWidget	*user_info;

	GtkWidget	*PO_acc;
	GtkWidget	*NB_decay;
	
	GtkWidget	*imported_ope;
	GtkWidget	*duplicat_ope;
	
	GtkWidget	*last_info;
	
	gchar		*filename;
	guint		filetype;

	guint		imported;
	guint		total;

	gboolean	valid;
	guint		step;
	guint		maxstep;

	GList		*ope_imp_list;
	
	//struct	Base base;
	//UBYTE	tmppass[12];
	//BOOL	check;
	//ULONG	change;
};


/*
** try to determine the file type (if supported for import by homebank
**
**
*/
gint homebank_alienfile_recognize(gchar *filename)
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
					
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		for(i=0;i<1;i++)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != "")
				{
					DB( g_print(" line %4d: %s\n", i, tmpstr) );

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

GList *homebank_csv_import(gchar *filename, guint accnum)
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

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != "")
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

						newope->date		= hb_date_get_julian_parse(str_array[0]);
						newope->paymode     = atoi(str_array[1]);
						newope->info        = g_strdup(str_array[2]);
						newope->payee       = da_payee_exists(GLOBALS->pay_list, str_array[3]);
						newope->wording     = g_strdup(str_array[4]);
						newope->amount      = g_ascii_strtod(str_array[5],NULL);
						newope->category    = da_category_exists(GLOBALS->cat_list, str_array[6]);
						
						
						newope->account     = accnum;
						newope->dst_account = accnum;

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
			_("%d operation(s) inserted\n%d error(s) in the file"),
			count, error);
		*/ 
	}


	return list;
}

GList	*ofx_acc_list;
GList	*ofx_ope_list;		//used as extern in list_operation
gint	ofx_acc_currentkey;

#ifndef NOOFX
/*
****
****
**** OFX part
****
****
*/

// globals datas


Account * ofx_find_account(gchar *number)
{
GList *list;
Account *item;


	DB( g_print("ofx_find_account\n") );

	DB( g_print(" -> searching for '%s'\n",number) );


	list = g_list_first(GLOBALS->acc_list);
	while (list != NULL)
	{
	Account *entry = list->data;

		// todo: maybe math should be done 

		if(entry->name && g_ascii_strcasecmp(number, entry->number) == 0)
		{
			ofx_acc_currentkey = entry->key;
			return entry;
		}
		list = g_list_next(list);
	}

	// no account found :: create it on the tmp account list
	item = da_account_malloc();
	
	ofx_acc_currentkey = g_list_length(GLOBALS->acc_list) + g_list_length(ofx_acc_list);
	item->key = ofx_acc_currentkey;
	
	item->name = g_strdup_printf(_("new%d"), ofx_acc_currentkey);
	item->number = g_strdup(number);
	
	DB( g_print(" -> creating tmp account: %d %s %x\n", ofx_acc_currentkey, number, item) );

	ofx_acc_list = g_list_append(ofx_acc_list, item);

	return NULL;
}


#define QIF_FILE_MAX_SIZE 256000

int ofx_proc_transaction_cb(const struct OfxTransactionData data, void * transaction_data)
{
  char dest_string[255];
  char trans_buff[4096];
  struct tm temp_tm;
  char trans_list_buff[QIF_FILE_MAX_SIZE];

  trans_list_buff[0]='\0';
  
  
  	Operation *item;
	
	item = da_operation_malloc();

  
  if(data.date_posted_valid==true){
    temp_tm = *localtime(&(data.date_posted));
    sprintf(trans_buff, "D%d%s%d%s%d%s", temp_tm.tm_mday, "/", temp_tm.tm_mon+1, "/", temp_tm.tm_year+1900, "\n");
    strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
    
    GDate *date;
    
    date = g_date_new();
    
    g_date_set_dmy(date, temp_tm.tm_mday, temp_tm.tm_mon+1, temp_tm.tm_year+1900);
    
    item->date = g_date_get_julian(date);
	
	g_date_free(date);
    
    
    
  }
  if(data.amount_valid==true){
    sprintf(trans_buff, "T%.2f%s",data.amount,"\n");
    strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
    
    item->amount = data.amount;

	if( item->amount > 0)
		item->flags |= OF_INCOME;
    
    
  //  g_print(" => %s %.f\n", data.name, data.amount);
    
  }
  if(data.check_number_valid==true){
    sprintf(trans_buff, "N%s%s",data.check_number,"\n");
    strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));

    item->info = g_strdup(data.check_number);
    
  }
  else if(data.reference_number_valid==true){
    sprintf(trans_buff, "N%s%s",data.reference_number,"\n");
      strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
}
if(data.name_valid==true){
    sprintf(trans_buff, "P%s%s",data.name,"\n");
        strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
        
        item->wording = g_strdup(data.name);
        
}
if(data.memo_valid==true){
    sprintf(trans_buff, "M%s%s",data.memo,"\n");
        strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
}
/* Add PAYEE and ADRESS here once supported by the library */


if(data.transactiontype_valid==true)
{
    switch(data.transactiontype)
    {
        case OFX_CREDIT: strncpy(dest_string, "Generic credit", sizeof(dest_string));
        //	item->info = g_strdup("Generic credit");
        break;
        case OFX_DEBIT: strncpy(dest_string, "Generic debit", sizeof(dest_string));
        //	item->info = g_strdup("Generic debit");
         	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_INT: strncpy(dest_string, "Interest earned or paid (Note: Depends on signage of amount)", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_DIV: strncpy(dest_string, "Dividend", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_FEE: strncpy(dest_string, "FI fee", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_SRVCHG: strncpy(dest_string, "Service charge", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_DEP: strncpy(dest_string, "Deposit", sizeof(dest_string));
			item->paymode = PAYMODE_CASH;
        break;
        case OFX_ATM: strncpy(dest_string, "ATM debit or credit (Note: Depends on signage of amount)", sizeof(dest_string));
        	item->paymode = PAYMODE_CARD;
        break;
        case OFX_POS: strncpy(dest_string, "Point of sale debit or credit (Note: Depends on signage of amount)", sizeof(dest_string));
        	item->paymode = PAYMODE_CARD;
        break;
        case OFX_XFER: strncpy(dest_string, "Transfer", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_CHECK: strncpy(dest_string, "Check", sizeof(dest_string));
        	item->paymode = PAYMODE_CHEQUE;
        break;
        case OFX_PAYMENT: strncpy(dest_string, "Electronic payment", sizeof(dest_string));
        	item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_CASH: strncpy(dest_string, "Cash withdrawal", sizeof(dest_string));
        	item->paymode = PAYMODE_CASH;
        break;
        case OFX_DIRECTDEP: strncpy(dest_string, "Direct deposit", sizeof(dest_string));
	        item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_DIRECTDEBIT: strncpy(dest_string, "Merchant initiated debit", sizeof(dest_string));
			item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_REPEATPMT: strncpy(dest_string, "Repeating payment/standing order", sizeof(dest_string));
			item->paymode = PAYMODE_BANKTRANSFERT;
        break;
        case OFX_OTHER: strncpy(dest_string, "Other", sizeof(dest_string));
        	//item->info = g_strdup("Other");
        break;
        default : strncpy(dest_string, "Unknown transaction type", sizeof(dest_string));
        	item->info = g_strdup("Unknown transaction type");
        break;
    }
    sprintf(trans_buff, "L%s%s",dest_string,"\n");
    strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
}
 strcpy(trans_buff, "^\n");
 strncat(trans_list_buff, trans_buff, sizeof(trans_list_buff)-1 - strlen(trans_list_buff));
 
 
 //fputs(trans_list_buff,stdout);

	// create an item in the glist
	


	//debug
	//item->account = 0;
	item->account = ofx_acc_currentkey;	
	
	
	
	ofx_ope_list = g_list_append(ofx_ope_list, item);

 return 0;
}/* end ofx_proc_transaction() */



int ofx_proc_statement_cb(const struct OfxStatementData data, void * statement_data)
{
  struct tm temp_tm;

  //printf("!Account\n");
  if(data.account_id_valid==true){
    /* Use the account id as the qif name of the account */
    //printf("N%s%s",data.account_id,"\n");
  }
  if(data.account_ptr->account_type_valid==true)
    {
      switch(data.account_ptr->account_type)
      {
	      case OFX_CHECKING :
	      	//printf("TBank\n");
		break;
	      case OFX_SAVINGS :
	      //printf("TBank\n");
		break;
	      case OFX_MONEYMRKT :
	      //printf("TOth A\n");
		break;
	      case OFX_CREDITLINE :
	      //printf("TOth L\n");
		break;
	      case OFX_CMA :
	      //printf("TOth A\n");
		break;
	      case OFX_CREDITCARD :
	      //printf("TCCard\n");
		break;
	      default:
	      perror("WRITEME: ofx_proc_account() This is an unknown account type!");
	      }
    }
  //printf("DOFX online account\n");

  if(data.ledger_balance_date_valid==true)
  {
    temp_tm = *localtime(&(data.ledger_balance_date));
    //printf("/%d%s%d%s%d%s", temp_tm.tm_mday, "/", temp_tm.tm_mon+1, "/", temp_tm.tm_year+1900, "\n");
  }
  if(data.ledger_balance_valid==true)
  {
    //printf("$%.2f%s",data.ledger_balance,"\n");
  }
  //printf("^\n");
  /*The transactions will follow, here is the header */
  /*
  if(data.account_ptr->account_type_valid==true){
    switch(data.account_ptr->account_type){
    case OFX_CHECKING : printf("!Type:Bank\n");
      break;
    case OFX_SAVINGS : printf("!Type:Bank\n");
      break;
    case OFX_MONEYMRKT : printf("!Type:Oth A\n");
      break;
    case OFX_CREDITLINE : printf("!Type:Oth L\n");
      break;
    case OFX_CMA : printf("!Type:Oth A\n");
      break;
    case OFX_CREDITCARD : printf("!Type:CCard\n");
      break;
    default: perror("WRITEME: ofx_proc_account() This is an unknown account type!");
    }
  }
  */

	Account *acc = ofx_find_account( (gchar *)data.account_id );

	DB( g_print(" hb account found result is %x\n", acc) );

	/*
		TODO:
		if account does not exist we should create one, but ask the user
		best will be the maintain a list of account to be created
		and only create it if the user answer yes or go further
	*/

  return 0;
}/* end ofx_proc_statement() */


int ofx_proc_account_cb(const struct OfxAccountData data, void * account_data)
{
  char dest_string[255]="** end of an account";
  
 
	//    strncat(trans_list_buff, dest_string, QIF_FILE_MAX_SIZE - strlen(trans_list_buff));
	fputs(dest_string,stdout);
	return 0;
}


GList *homebank_ofx_import(gchar *filename)
{
gchar *argv[2];

extern int ofx_PARSER_msg;
extern int ofx_DEBUG_msg;
extern int ofx_WARNING_msg;
extern int ofx_ERROR_msg;
extern int ofx_INFO_msg;
extern int ofx_STATUS_msg;

	DB( g_print("------------------------------------\n") );
	DB( g_print("(wizimport) ofx import\n") );

	ofx_PARSER_msg = false;
	ofx_DEBUG_msg = false;
	ofx_WARNING_msg = false;
	ofx_ERROR_msg = false;
	ofx_INFO_msg = false;
	ofx_STATUS_msg = false;

	// free resources
	da_account_destroy(ofx_acc_list);
	ofx_acc_list = NULL;
	
	//todo clean ope also


	LibofxContextPtr libofx_context = libofx_get_new_context();
	ofx_set_statement_cb(libofx_context, ofx_proc_statement_cb, 0);
	//ofx_set_account_cb(libofx_context, ofx_proc_account_cb, 0);
	ofx_set_transaction_cb(libofx_context, ofx_proc_transaction_cb, 0);

    argv[1] = filename;

	libofx_proc_file(libofx_context, argv[1], OFX);

	libofx_free_context(libofx_context);


	return ofx_ope_list;
}

#endif

/*
***
*** qif part
***
*/


GList *homebank_qif_import(gchar *filename)
{
GList *list = NULL;



	return list;
}



static void import_clearall(struct import_data *data)
{

	DB( g_print("(import) clear all\n") );

	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope))));


	da_account_destroy(ofx_acc_list);
	ofx_acc_list = NULL;

	if( data->filetype == FILETYPE_CSV_HB && g_list_length(GLOBALS->acc_list) == 0)
	{
	Account *item;

		DB( g_print("list is empty, account is created\n") );

		// no account found :: create it on the tmp account list
		item = da_account_malloc();
		item->key = 0;
		item->name = g_strdup_printf(_("new%d"), 0);
		//item->number = g_strdup(number);

		ofx_acc_list = g_list_append(ofx_acc_list, item);
	}

	da_operation_destroy(data->ope_imp_list);
	data->ope_imp_list = NULL;

#ifndef NOOFX
	ofx_ope_list = NULL;
#endif	
	
}

static void import_selchange(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;
gint page_number;

	page_number = gtk_assistant_get_current_page (GTK_ASSISTANT(data->assistant));

	DB( g_print("(import) selchange (page %d)\n", page_number) );

	if( page_number == 1 )
	{



	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	
	if(data->filename) 
		g_free( data->filename );
	data->filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(data->filechooser));
	DB( g_print(" filename -> %s\n", data->filename) );

	data->valid = FALSE;	
	if( data->filename != NULL )
	{
		data->filetype = homebank_alienfile_recognize(data->filename);
		switch(data->filetype)
		{
			case FILETYPE_UNKNOW:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("Unknow/Invalid file..."));
				break;
#ifndef NOOFX
			case FILETYPE_OFX:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("OFX file recognized !"));
				data->valid = TRUE;
				break;
#endif
			case FILETYPE_CSV_HB:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("CSV operation file recognized !"));
				data->valid = TRUE;
				break;
		
		
		}
	}

	//if(data->valid == TRUE)
	{
	GtkWidget *current_page;
	
		
		current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);

	    gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, data->valid);
	
	

	}
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

	if( data->ope_imp_list )
	{


		/* 1: get import min bound date */
		tmplist = g_list_first(data->ope_imp_list);
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

				implist = g_list_first(data->ope_imp_list);
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

	DB( g_print("(import) apply\n") );

	GList *tmplist = g_list_first(ofx_acc_list);
	while (tmplist != NULL)
	{
	Account *item = da_account_clone(tmplist->data);

		GLOBALS->acc_list = g_list_append(GLOBALS->acc_list, item);
		tmplist = g_list_next(tmplist);
	}

	// then import operations
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
			//DB(g_print("import %d to acc: %d\n", data->total, item->account)  );

			operation_add(item, NULL, 0);
		}

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	/* todo optimize this */
	if(data->imported > 0)
	{
		GLOBALS->change += data->imported;
		wallet_populate_listview(GLOBALS->mainwindow, NULL);
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
	}	

}


static void import_fill_imp_operations(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
GtkWidget *view;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *tmplist;

	DB( g_print("(import) fill\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	view = data->imported_ope;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	tmplist = g_list_first(data->ope_imp_list);
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
	Operation *item = tmplist->data;
	
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

				DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, item->same) );

				tmplist = g_list_next(tmplist);
			}
		}

	}


}






void import_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	import_fillsame(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

/*
**
*/
gboolean import_dispose(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;

	DB( g_print("(import) dispose\n") );

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_print(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

	g_free( data->filename );

	import_clearall(data);

	g_free(user_data);

	//delete-event TRUE abort/FALSE destroy
	return FALSE;
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

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	current_page = gtk_assistant_get_current_page (GTK_ASSISTANT (widget));
	n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

	DB( g_print("prepare %d of %d\n", current_page+1, n_pages) );

	switch( current_page + 1 )
	{
		case 1:
			break;
		case 3:
		case 4:
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[5], FALSE);
			break;
		case 5:		/* import */
			import_clearall(data);
			switch(data->filetype)
			{
#ifndef NOOFX
				/* ofx_acc_list & ofx_ope_list are filled here */
				case FILETYPE_OFX:
					data->ope_imp_list = homebank_ofx_import(data->filename);
					break;
#endif
				case FILETYPE_CSV_HB:
					if( g_list_length(GLOBALS->acc_list) == 0 )
						accnum = 0;
					else
						accnum = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));
					
					DB( g_print("accnum: %d\n", accnum) );
					
					data->ope_imp_list = homebank_csv_import(data->filename, accnum);
					break;
			}
								
			//sort by date
			data->ope_imp_list = da_operation_sort(data->ope_imp_list);
			import_find_duplicate_operations(widget, NULL);
			import_fill_imp_operations(widget, NULL);
			// progress ok, page is complete
   			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
	
		case 6:
		{
		gchar *txt;
		
			import_count_changes(data);

			if( g_list_length(ofx_acc_list) > 0 )
			{
			txt = g_strdup_printf(
				_(
				"%d account(s) will be created.\n\n" \
				"%d operation(s) will be imported.\n" \
				"%d operation(s) will be rejected."
				),
				g_list_length(ofx_acc_list),
				data->imported,
				data->total-data->imported
				);

			}
			else
			{
			txt = g_strdup_printf(
				_(
				"%d operation(s) will be imported.\n" \
				"%d operation(s) will be rejected."
				),
				data->imported,
				data->total-data->imported
				);
			}

			gtk_label_set_text(GTK_LABEL(data->last_info), txt);
			g_free(txt);		

   			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		}	
	}

	title = g_strdup_printf ( _("Import assistant (%d of %d)"), current_page + 1, n_pages);
	gtk_window_set_title (GTK_WINDOW (widget), title);
	g_free (title);
}

/*
 * forward function
 */
static gint gtk_assistant_forward_page_func(gint current_page, gpointer func_data)
{
struct import_data *data = func_data;
gint page = 0;

	page = current_page + 1;

	if(current_page == 1 && data->filetype == FILETYPE_OFX)
		page = 3;

	if(current_page == 1 && data->filetype == FILETYPE_CSV_HB && !g_list_length(GLOBALS->acc_list) )
		page = 3;


	DB( g_print("forward func: %d return %d\n", current_page, page) );

	return page;
}

/*
 * page 1: intro
 */
static GtkWidget *
create_page1(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label;
GdkPixbuf *pixbuf;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(_(
	
		"HomeBank can import operations from several file format:\n" \
		"\n" \
		"- CSV (Homebank operation csv export format only)\n" \
		"- OFX/QFX (optional)\n" \
		"\n" \
		"Other formats or not supported for the moment.\n" \
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
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Operation import wizard"));

#if SIDE_IMAGE == 1
  pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
  //pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif
	return vbox;
}



static GtkWidget *
create_page2 (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *widget, *label;
GdkPixbuf *pixbuf;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);


	widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
    data->filechooser = widget;
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, HB_BOX_SPACING);

	label = gtk_label_new("");
	data->user_info = label;
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, HB_BOX_SPACING);


  gtk_widget_show_all (vbox);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
  //gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Select a file to load"));

#if SIDE_IMAGE == 1
  pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
  //pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif
	return vbox;
}


/*
 * page 3: set default account
 */
static GtkWidget *
create_page3(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *label, *widget;
GdkPixbuf *pixbuf;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(
_("The file you selected appears to contains operations\n\
for just one account, without specifying its name.\n\n\
Please select the account to attach thoses operations to.")
		, 0.5, 0.5);
		
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, HB_BOX_SPACING);

	label = make_label(_("A_ccount:"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);
	widget = make_popaccount(label);
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, HB_BOX_SPACING);
	data->PO_acc = widget;

  gtk_widget_show_all (vbox);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
  //gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Set the default account"));


#if SIDE_IMAGE == 1
  pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
  //pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

	return vbox;
}


/*
 * page 4: 
 */
static GtkWidget *
create_page4(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *label, *widget;
GdkPixbuf *pixbuf;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(_(
		"Duplicate operations will be found and unselected\n" \
		"for import and can of course be reselected.\n"
		"\n" \
		"The match is done in order by account, amount and date.\n" \
		"For date, you can set a tolerance:\n\n" \
		"- 0 means an exact match.\n" \
		"- 1-14 to match +/- 1 to 14 days\n\n" )
		, 0.5, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, HB_BOX_SPACING);

	label = make_label(_("Date _tolerance:"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);
	widget = make_numeric(label, 0.0, 14.0);
	data->NB_decay = widget;
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, HB_BOX_SPACING);
	label = make_label(_("days"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);

  gtk_widget_show_all (vbox);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
  //gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Set the duplicate match option"));

#if SIDE_IMAGE == 1
  pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
  //pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif
	return vbox;
}





static GtkWidget *create_page5(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label, *sw, *widget, *expander;
GdkPixbuf *pixbuf;

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Operations to import</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	
	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	//gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	widget = create_list_import_operation();
	data->imported_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Possible duplicate for the above selected operation</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

   /* Create the expander */
    expander = gtk_expander_new (_("Details"));
    gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

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
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Select operations to import"));

#if SIDE_IMAGE == 1
  pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/wizard.svg", NULL);
  //pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_side_image (GTK_ASSISTANT (assistant), vbox, pixbuf);
  g_object_unref (pixbuf);
#endif

	return vbox;
}




/*
 * page 6: 
 */
static GtkWidget *create_page6(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label;
GdkPixbuf *pixbuf;

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
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _("Update your accounts"));


#if SIDE_IMAGE == 1
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

	data = g_malloc0(sizeof(struct import_data));
	if(!data) return NULL;


	assistant = gtk_assistant_new ();
	data->assistant = assistant;


	//gtk_window_set_transient_for(GTK_WINDOW(assistant), GTK_WINDOW(GLOBALS->mainwindow));


	screen = gtk_window_get_screen(GTK_WINDOW (assistant));
	width  = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);

#define SCOEF 0.6

	gtk_window_resize(GTK_WINDOW(assistant), SCOEF*width, SCOEF*height);
	//gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);


	//store our window private data
	g_object_set_data(G_OBJECT(assistant), "inst_data", (gpointer)data);
	DB( g_print("(import) window=%x, inst_data=%x\n", assistant, data) );


	data->pages[1] = create_page1 (assistant, data);	// intro
	data->pages[2] = create_page2 (assistant, data);	// file choose
	data->pages[3] = create_page3 (assistant, data);	// account choose
	data->pages[4] = create_page4 (assistant, data);	// duplicate option
	data->pages[5] = create_page5 (assistant, data);	// result
	data->pages[6] = create_page6 (assistant, data);	// confirm

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT(assistant), gtk_assistant_forward_page_func, data, NULL);

	//setup
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(data->filechooser), PREFS->path_import);
	make_popaccount_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->acc_list);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), 0);

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

}
