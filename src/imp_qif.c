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

#include "import.h"
#include "imp_qif.h"

/****************************************************************************/
/* Debug macros																														 */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


/* = = = = = = = = = = = = = = = = */
static QIF_Tran *
da_qif_tran_malloc(void)
{
	return g_malloc0(sizeof(QIF_Tran));
}

static void
da_qif_tran_free(QIF_Tran *item)
{
	if(item != NULL)
	{
		if(item->date != NULL)
			g_free(item->date);
		if(item->payee != NULL)
			g_free(item->payee);
		if(item->memo != NULL)
			g_free(item->memo);
		if(item->category != NULL)
			g_free(item->category);
		if(item->account != NULL)
			g_free(item->account);

		g_free(item);
	}
}




static void
da_qif_tran_destroy(QifContext *ctx)
{
GList *qiflist = g_list_first(ctx->q_tra);

	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;
		da_qif_tran_free(item);
		qiflist = g_list_next(qiflist);
	}
	g_list_free(ctx->q_tra);
	ctx->q_tra = NULL;
}

static void 
da_qif_tran_new(QifContext *ctx)
{
	ctx->q_tra = NULL;
}



static void
da_qif_tran_move(QIF_Tran *sitem, QIF_Tran *ditem)
{
	if(sitem != NULL && ditem != NULL)
	{
		memcpy(ditem, sitem, sizeof(QIF_Tran));
		memset(sitem, 0, sizeof(QIF_Tran));
	}
}



static void
da_qif_tran_append(QifContext *ctx, QIF_Tran *item)
{
	ctx->q_tra = g_list_append(ctx->q_tra, item);
}


/* = = = = = = = = = = = = = = = = */

static gdouble
hb_qif_parser_get_amount(gchar *string)
{
gdouble amount;
gint l, i;
char *new_str, *p;

	amount = 0.0;

	// is there a non digit char at length-3 pos, it should be a decimal separator
	l = strlen(string);
	
	if( !g_ascii_isdigit( string[l-3] && string[l-3] != '-' ))
	{
		//DB( g_print(" -> %s it seems a decimal char is %c (%d)\n", string, string[l-3],l ));	

		new_str = g_malloc (l+1);
		p = new_str;
		for(i=0;i<l;i++)
		{
			if( g_ascii_isdigit( string[i] ) || string[i] == '-' )
			{
				*p++ = string[i];
			}
			else
			if( string[i] == string[l-3] )
				*p++ = '.';
				
		}
		*p++ = '\0';
		amount = g_ascii_strtod(new_str, NULL);
		g_free(new_str);
	}

	//DB( g_print(" -> amount is '%s'  %.2f\n", string, amount ));	

	return amount;
}




static gboolean
hb_qif_parser_get_dmy(gchar *string, gint *d, gint *m, gint *y)
{
gboolean retval;
gchar **str_array;

	retval = FALSE;
	str_array = g_strsplit (string, "/", 3);
	if( g_strv_length( str_array ) != 3 )
	{
		g_strfreev (str_array);
		str_array = g_strsplit (string, ".", 3);
	}

	if( g_strv_length( str_array ) == 3 )
	{
		*d = atoi(str_array[0]);
		*m = atoi(str_array[1]);
		*y = atoi(str_array[2]);

		//correct for 2 digits year
		if(*y < 1970)
		{
			if(*y < 60)
				*y += 2000;
			else
				*y += 1900;
		}

		retval = TRUE;
	}

	g_strfreev (str_array);

	return retval;
}

static guint32
hb_qif_date_get_julian(gchar *string, gboolean isodate)
{
GDate *date;
gint d, m, y;
guint32 julian = 0;

	if( hb_qif_parser_get_dmy(string, &d, &m, &y) )
	{
		date = g_date_new();
		if( isodate )
			g_date_set_dmy(date, d, m, y);
		else
			g_date_set_dmy(date, m, d, y);
		julian = g_date_get_julian (date);
		g_date_free(date);
	}

	return julian;
}




/*

	return:0 for iso date (dd/mm/yy[yy])
	return:1 for american date (/mm/dd/yy[yy])
*/

static gboolean
hb_qif_parser_check_iso_date(QifContext *ctx)
{
gboolean retval = TRUE;
GList *qiflist;
gboolean r, valid;
gint d, m, y;

	DB( g_print("(qif) get_datetype\n") );

	qiflist = g_list_first(ctx->q_tra);
	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;

		r = hb_qif_parser_get_dmy(item->date, &d, &m, &y);
		valid = g_date_valid_dmy(d, m, y);
		
		DB( g_print(" -> date: %s :: %d %d %d :: %d\n", item->date, d, m, y, valid ) );

		if(valid == FALSE)
		{
			retval = FALSE;
			break;
		}

		qiflist = g_list_next(qiflist);
	}


	return retval;
}


static gint
hb_qif_parser_get_block_type(gchar *tmpstr)
{
gchar **typestr;
gint type = QIF_NONE;

	//DB( g_print("--------\n(account) block type\n") );

	//DB( g_print(" -> str: %s type: %d\n", tmpstr, type) );


	if(g_str_has_prefix(tmpstr, "!Account"))
	{
		type = QIF_ACCOUNT;
	}
	else
	{
		typestr = g_strsplit(tmpstr, ":", 2);

		if( g_strv_length(typestr) == 2 )
		{

			//DB( g_print(" -> str[1]: %s\n", typestr[1]) );

			if( g_str_has_prefix(typestr[1], "Bank") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "Cash") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "CCard") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "Invst") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "Oth A") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "Oth L") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(typestr[1], "Security") )
			{
				type = QIF_SECURITY;
			}
			else
			if( g_str_has_prefix(typestr[1], "Prices") )
			{		
				type = QIF_PRICES;
			}
		}		
		g_strfreev(typestr);							
	}

	//DB( g_print(" -> return type: %d\n", type) );


	return type;
}

static void
hb_qif_parser_parse(QifContext *ctx, gchar *filename)
{
GIOChannel *io;
QIF_Tran tran = { 0 };

	DB( g_print("(qif) hb_qif_parser_parse\n") );

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr;
	GError *err = NULL; 
	gint io_stat;
	gint type = QIF_NONE;
	gchar *value = NULL;
	gchar *cur_acc;
		
		
		DB( g_print(" -> encoding is %s\n", g_io_channel_get_encoding(io)) );
		
		g_io_channel_set_encoding(io, NULL, NULL);
		
		cur_acc = g_strdup(QIF_UNKNOW_ACCOUNT_NAME);

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, &err);
			
			DB (g_print(" line read %s\n", tmpstr));
			
			if( io_stat == G_IO_STATUS_EOF )
				break;
			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_NORMAL )
			{
				
				
				
				hb_string_strip_crlf(tmpstr);

				//start qif parsing
				if(g_str_has_prefix(tmpstr, "!")) /* !Type: or !Option: or !Account otherwise ignore */
				{
					type = hb_qif_parser_get_block_type(tmpstr);
					DB ( g_print(" ----------------\n") );
					DB ( g_print("* QIF block: '%s' (type = %d)\n", tmpstr, type) );
				}

				value = &tmpstr[1];

				if( type == QIF_ACCOUNT )
				{

					// Name
					if(g_str_has_prefix(tmpstr, "N"))
					{
		
						g_free(cur_acc);
						g_strstrip(value);
						cur_acc = g_strdup(value);
		
		
						DB ( g_print(" name: '%s'\n", value) );
					}
					else
					
					// Type of account
					if(g_str_has_prefix(tmpstr, "T"))
					{
		
						DB ( g_print(" type: '%s'\n", value) );
					}
					else
					
					// Credit limit (only for credit card accounts)
					if(g_str_has_prefix(tmpstr, "L"))
					{
		
						DB ( g_print(" credit limit: '%s'\n", value) );
					}
					else
					
					// Statement balance amount
					if(g_str_has_prefix(tmpstr, "$"))
					{
		
						DB ( g_print(" balance: '%s'\n", value) );
					}
					else
					
					// end 
					if(g_str_has_prefix(tmpstr, "^"))
					{
						DB ( g_print(" ----------------\n") );
					}
					

				}

		/* transaction */
				if( type == QIF_TRANSACTION )
				{
				//date
					if(g_str_has_prefix(tmpstr, "D"))
					{
					gchar *ptr;
					
						// US Quicken seems to be using the ' to indicate post-2000 two-digit years
						//(such as 01/01'00 for Jan 1 2000)
						ptr = g_strrstr (value, "\'");
						if(ptr != NULL) { *ptr = '/'; }

						ptr = g_strrstr (value, " ");
						if(ptr != NULL) { *ptr = '0'; }

						g_free(tran.date);
						tran.date = g_strdup(value);
					}
					else

				// amount
					if(g_str_has_prefix(tmpstr, "T"))
					{
						tran.amount = hb_qif_parser_get_amount(value);
					}
					else

				// payee
					if(g_str_has_prefix(tmpstr, "P"))
					{
						if(*value != '\0')
						{
							g_free(tran.payee);
							g_strstrip(value);
							tran.payee = g_strdup(value);
						}
					}
					else

				// memo
					if(g_str_has_prefix(tmpstr, "M"))
					{
						if(*value != '\0')
						{
							g_free(tran.memo);
							tran.memo = g_strdup(value);
						}
					}
					else

				// category
					if(g_str_has_prefix(tmpstr, "L"))
					{
						// LCategory of transaction
						// L[Transfer account name]
						// LCategory of transaction/Class of transaction
						// L[Transfer account]/Class of transaction

						/*		
						if(g_str_has_prefix(&tmpstr[1], "["))	// this is a transfert account name
						{
							DB ( g_print(" transfert to: '%s'\n", value) );
						}
						else
						{
							DB ( g_print(" category: '%s'\n", value) );
						}
						*/					

						if(*value != '\0')
						{
							g_free(tran.category);
							g_strstrip(value);
							tran.category = g_strdup(value);
						}
					}
					else

				// end 
					if(g_str_has_prefix(tmpstr, "^"))
					{
					QIF_Tran *newitem;

						tran.account = g_strdup(cur_acc);

						DB ( g_print(" storing qif transaction: dat:'%s' amt:%.2f pay:'%s' mem:'%s' cat:'%s' acc:'%s'\n", tran.date, tran.amount, tran.payee, tran.memo, tran.category, tran.account) );

						newitem = da_qif_tran_malloc();
						da_qif_tran_move(&tran, newitem);
						da_qif_tran_append(ctx, newitem);
					}
				}
				// end QIF_TRANSACTION


			}
			g_free(tmpstr);
		}
		g_io_channel_unref (io);

		g_free(cur_acc);
		
	}


}



Operation *
account_qif_get_child_transfert(Operation *src, GList *list)
{
Operation *item;

	DB( g_print("(operation) operation_get_child_transfert\n") );

	DB( g_print(" search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->dst_account) );

	list = g_list_first(list);
	while (list != NULL)
	{
		item = list->data;
		if( item->paymode == PAYMODE_PERSTRANSFERT)
		{
			if( src->date == item->date &&
			    src->account == item->dst_account &&
			    src->dst_account == item->account &&
			    ABS(src->amount) == ABS(item->amount) )
			{
				DB( g_print(" found : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->account, item->dst_account) );

				return item;			
			}
		}
		list = g_list_next(list);
	}		

	DB( g_print(" not found...\n") );

	return NULL;
}

/*
** this is ourmain qif entry point
**
*/
GList *
account_import_qif(gchar *filename, ImportContext *ictx)
{
QifContext ctx = { 0 };
gboolean isodate;
GList *qiflist;
GList *list = NULL;

	DB( g_print("(qif) account import qif\n") );


	// allocate our GLists
	da_qif_tran_new(&ctx);

	// parse !!
	hb_qif_parser_parse(&ctx, filename);

	// check iso date format in file
	isodate = hb_qif_parser_check_iso_date(&ctx);
	DB( g_print(" 1) date is dd/mm/yy: %d\n", isodate) );

	DB( g_print(" 2) transform to HomeBank\n") );

	ictx->has_unknow_account = FALSE;

	// transform our qif transactions to homebank ones
	qiflist = g_list_first(ctx.q_tra);
	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;
	Operation *newope, *child;
	Account *accitem;
	Payee *payitem;
	Category *catitem;

		newope = da_operation_malloc();

		newope->date		 = hb_qif_date_get_julian(item->date, isodate);
		//newope->paymode	 = atoi(str_array[1]);
		//newope->info		 = g_strdup(str_array[2]);

		// payee + append
		if( item->payee != NULL )
		{
			payitem = da_pay_get_by_name(item->payee);
			if(payitem == NULL)
			{
				DB( g_print(" -> append pay: '%s'\n", item->payee ) );

				payitem = da_pay_malloc();
				payitem->name = g_strdup(item->payee);
				payitem->imported = TRUE;
				da_pay_append(payitem);
				
				ictx->cnt_new_pay += 1;
			}
			newope->payee = payitem->key;
		}

		newope->wording		 = g_strdup(item->memo);
		newope->amount		 = item->amount;
		
		
		// LCategory of transaction
		// L[Transfer account name]
		// LCategory of transaction/Class of transaction
		// L[Transfer account]/Class of transaction
		if( item->category != NULL )
		{
							
			if(g_str_has_prefix(item->category, "["))	// this is a transfert account name
			{
			gchar *accname;
			
				DB ( g_print(" -> transfert to: '%s'\n", item->category) );

				//remove brackets
				accname = hb_strdup_nobrackets(item->category);
				


				// account + append
				accitem = da_acc_get_by_name(accname);
				if(accitem == NULL)
				{
					DB( g_print(" -> append acc: '%s'\n", accname ) );

					accitem = da_acc_malloc();
					accitem->name = g_strdup(accname);
					accitem->imported = TRUE;
					da_acc_append(accitem);
				}
				
				newope->dst_account = accitem->key;
				newope->paymode = PAYMODE_PERSTRANSFERT;
				
				
				g_free(accname);
			}
			else
			{
				DB ( g_print(" -> append cat: '%s'\n", item->category) );
			
				catitem = da_cat_append_ifnew_by_fullname(item->category, TRUE);
				if( catitem != NULL)
				{
					ictx->cnt_new_cat += 1;
					newope->category = catitem->key;
				}
			}
		}

		// account + append
		if(!strcmp(QIF_UNKNOW_ACCOUNT_NAME, item->account))
		{
			ictx->has_unknow_account = TRUE;
			newope->account = 0;
		}
		else
		{
			accitem = da_acc_get_by_name(item->account);
			if(accitem == NULL)
			{
				DB( g_print(" -> append acc: '%s'\n", item->account ) );

				accitem = da_acc_malloc();
				accitem->name = g_strdup(item->account);
				accitem->imported = TRUE;
				da_acc_append(accitem);
			}
			newope->account = accitem->key;
		}


		newope->flags |= OF_ADDED;
		if( newope->amount > 0)
			newope->flags |= OF_INCOME;

		child = NULL;
		
		child = account_qif_get_child_transfert(newope, list);
		if( child != NULL)
		{
			DB( g_print(" -> already exist\n" ) );
		
			da_operation_free(newope);
		}
		else
		{
			DB( g_print(" -> append trans. acc:'%s', memo:'%s', val:%.2f\n", item->account, item->memo, item->amount ) );

			list = g_list_append(list, newope);
		}
		

		qiflist = g_list_next(qiflist);
	}

	// destroy our GLists
	da_qif_tran_destroy(&ctx);

	return list;
}

/* = = = = = = = = = = = = = = = = = = = = */

void test_qif_export (void)
{
FILE *fp;
GString *buffer;
GList *list, *list2;
GDate *date;
char amountbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *filename = NULL;
size_t rc;

	DB( g_print("(qif) test qif export\n\n") );

	if( homebank_chooser_open_qif(NULL, &filename) == TRUE )
	{


		//filename = g_strdup("/home/max/Desktop/export.qif");


		buffer = g_string_new (NULL);	
		
		//todo: save accounts in order
		//todo: save transfert transaction once

		list = g_hash_table_get_values(GLOBALS->h_acc);
		while (list != NULL)
		{
		Account *item = list->data;
		
			buffer = g_string_append (buffer, "!Account\n");
			g_string_append_printf (buffer, "N%s\n", item->name);
			buffer = g_string_append (buffer, "^\n");

			// transaction export
			buffer = g_string_append (buffer, "!Type:Bank\n");

			list2 = g_list_first(GLOBALS->ope_list);
			while (list2 != NULL)
			{
			Operation *trans = list2->data;
			Payee *payee;
			Category *cat;
			gchar *txt;

				if( trans->account == item->key )
				{
					date = g_date_new_julian (trans->date);
					g_string_append_printf (buffer, "D%02d/%02d/%04d\n",
						g_date_get_day(date),
						g_date_get_month(date),
						g_date_get_year(date)
						);
					g_date_free(date);

					//g_ascii_dtostr (amountbuf, sizeof (amountbuf), trans->amount);
					g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", trans->amount);
					
					g_string_append_printf (buffer, "T%s\n", amountbuf);
				
					if( trans->paymode == PAYMODE_CHEQUE)
						g_string_append_printf (buffer, "N%s\n", trans->info);

					payee = da_pay_get(trans->payee);
					g_string_append_printf (buffer, "P%s\n", payee->name);

					g_string_append_printf (buffer, "M%s\n", trans->wording);

					// category/transfert

						// LCategory of transaction
						// L[Transfer account name]
						// LCategory of transaction/Class of transaction
						// L[Transfer account]/Class of transaction

					if( trans->paymode == PAYMODE_PERSTRANSFERT && trans->account == item->key)
					{
						g_string_append_printf (buffer, "L[%s]\n", item->name);
					}
					else
					{
						cat = da_cat_get(trans->category);
						txt = da_cat_get_fullname(cat);
						g_string_append_printf (buffer, "L%s\n", txt);
						g_free(txt);			
					}

					buffer = g_string_append (buffer, "^\n");
				}

				list2 = g_list_next(list2);
			}

			list = g_list_next(list);
		}
		g_list_free(list);
		
		
		
		if ((fp = fopen(filename, "w")) == NULL)
		{
			g_message("file error on: %s", filename);
		}
		else
		{
			rc = fwrite (buffer->str, buffer->len, 1, fp);

			//g_print( buffer->str );

			fclose(fp);
		}
		
		
	
		g_string_free (buffer, TRUE);
	}

	g_free( filename );

}


