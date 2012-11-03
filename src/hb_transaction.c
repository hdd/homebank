/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2010 Maxime DOYEN
 *
 *  This file is part of HomeBank.
 *
 *  HomeBank is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  HomeBank is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "hb_transaction.h"

/****************************************************************************/
/* Debug macros										 */
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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

Operation *
operation_get_child_transfer(Operation *src)
{
GList *list;
Operation *item;

	DB( g_print("(transaction) operation_get_child_transfer\n") );

	DB( g_print(" search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->dst_account) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		item = list->data;
		if( item->paymode == PAYMODE_INTXFER)
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


void operation_warn_transfer(Operation *src, gchar *msg2)
{
gchar str_amount[128];
gchar str_date[128];
GDate *date;
Account *acc;
gchar *accname;

	DB( g_print("(transaction) operation_warn_transfer\n") );
	
		
		mystrfmon(str_amount, 127, src->amount, 0);

		date = g_date_new_julian (src->date);
		g_date_strftime (str_date, 128-1, PREFS->date_format, date);
		g_date_free(date);
		
		acc = da_acc_get( src->dst_account);
		
		//fix 379760
		accname = _("Unknown");
		if( acc != NULL )
			accname = acc->name;
		
		//warn the user
		homebank_message_dialog(NULL, GTK_MESSAGE_INFO,
			_("No corresponding transfer can be found."),
			msg2,
			accname,	//account name
			str_date,	//date
			str_amount 	//amount
			
			);


}


void operation_delete_child_transfer(Operation *src)
{
Operation *dst;

	DB( g_printf("(transaction) delete_child_transfer\n") );

	dst = operation_get_child_transfer( src );
	
	DB( g_print(" -> return is %s, %d\n", dst, dst) );
	
	if( dst == NULL )
	{
		/* warn the user we have notfound the child internal transfer */
		operation_warn_transfer( src, _("Could not delete child transfer:\n\n"
			"account:  %s\n"
			"date:  %s\n"
			"amount:  %s\n"
			"\nYou should fix the problem manually.") );
	}
	else
	{
		DB( g_print("deleting...") );
		
		GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, dst);
	}
}

void operation_add_transfer(Operation *ope, GtkWidget *treeview)
{
Operation *newope;
Account *acc;
gchar swap;

	DB( g_printf("(transaction) operation add transfer\n") );

	if( ope->dst_account > 0 )
	{
		newope = da_operation_clone(ope);

		newope->amount = -newope->amount;
		newope->flags ^= (OF_INCOME);	//xor
		newope->flags &= (~OF_REMIND);

		swap = newope->account;
		newope->account = newope->dst_account;
		newope->dst_account = swap;


		/* update acc flags */
		acc = da_acc_get( newope->account);
		if(acc != NULL)
			acc->flags |= AF_ADDED;


		DB( g_printf(" + add transfer, %x\n", newope) );

		da_operation_append(newope);
		if(treeview != NULL) operation_add_treeview(newope, treeview, ope->account);
	}

}

void operation_add(Operation *ope, GtkWidget *treeview, gint accnum)
{
Operation *newope;
Account *acc;

	DB( g_printf("(transaction) operation add\n") );

	//allocate a new entry and copy from our edited structure
	newope = da_operation_clone(ope);

	//init flag and keep remind state
	// already done in defoperation_get
	//ope->flags |= (OF_ADDED);
	//remind = (ope->flags & OF_REMIND) ? TRUE : FALSE;
	//ope->flags &= (~OF_REMIND);

	/* cheque number is already stored in defoperation_get */
	/* todo:move this to operation add
		store a new cheque number into account ? */
		
	if( (newope->info) && (newope->paymode == PAYMODE_CHECK) && !(newope->flags & OF_INCOME) )
	{
	Account *acc;
	guint cheque;

		/* get the active account and the corresponding cheque number */
		acc = da_acc_get( newope->account);
		cheque = atol(newope->info);

		DB( g_printf(" -> should store cheque number %d to %d", cheque, newope->account) );
		if( newope->flags & OF_CHEQ2 )
			acc->cheque2 = cheque;
		else
			acc->cheque1 = cheque;
	}


	/* add normal operation */
	acc = da_acc_get( ope->account);
	if(acc != NULL)
		acc->flags |= AF_ADDED;

	DB( g_printf(" + add normal %x\n", newope) );
	da_operation_append(newope);
	if(treeview != NULL) operation_add_treeview(newope, treeview, accnum);

	if(ope->paymode == PAYMODE_INTXFER)
	{
		operation_add_transfer(ope, treeview);
	}
}




void operation_add_treeview(Operation *ope, GtkWidget *treeview, gint accnum)
{
GtkTreeModel *model;
GtkTreeIter  iter;
//GtkTreePath *path;
//GtkTreeSelection *sel;

	DB( g_printf("(transaction) operation add treeview\n") );

	if(ope->account == accnum)
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);

		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, ope,
				-1);
	
		//activate that new line
		//path = gtk_tree_model_get_path(model, &iter);
		//gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeview), path);
		
		//sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		//gtk_tree_selection_select_iter(sel, &iter);
		
		//gtk_tree_path_free(path);
	
	}
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

//todo: this is only a test
gint transaction_auto_assign(GList *ope_list, guint key)
{
GList *l_ope;
GList *l_rul, *c_rul;
gint changes = 0;

	l_ope = g_list_first(ope_list);
	l_rul = g_hash_table_get_values(GLOBALS->h_rul);

	while (l_ope != NULL)
	{
	Operation *ope = l_ope->data;

		//DB( g_print("ope '%s' %d, %d\n", ope->wording, ope->payee, ope->category) );
		
		if( key == -1 || key == ope->account )
		{
		
		
			if( ope->payee == 0 && ope->category == 0 )
			{	
				c_rul = g_list_first(l_rul);
				while (c_rul != NULL)
				{
				Assign *rul = c_rul->data;

					DB( g_print("search %s in %s\n", rul->name, ope->wording) );
					if( rul->name != NULL)
					{
						if( rul->exact )
						{				
							if( g_strrstr(ope->wording, rul->name) != NULL )
							{
								DB( g_print(" found case\n") );
				
								ope->payee    = rul->payee;
								ope->category = rul->category;

								ope->flags |= OF_CHANGED;
								changes++;
							}
						}
						else
						{
						gchar *word   = g_utf8_casefold(ope->wording, -1);
						gchar *needle = g_utf8_casefold(rul->name, -1);
			
							if( g_strrstr(word, needle) != NULL )
							{
								DB( g_print(" found nocase\n") );
				
								ope->payee    = rul->payee;
								ope->category = rul->category;
					
								ope->flags |= OF_CHANGED;
								changes++;
							}
		
							g_free(word);
							g_free(needle);
			
						}
					}

					c_rul = g_list_next(c_rul);
				}
			}
		
		}
		
		l_ope = g_list_next(l_ope);
	}

	g_list_free(l_rul);

	return changes;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint
transaction_count_tags(Operation *ope)
{
guint count = 0;
guint32 *ptr = ope->tags;

	if( ope->tags == NULL )
		return 0;

	while(*ptr++ != 0 && count < 32)
		count++;

	return count;
}

guint
transaction_set_tags(Operation *ope, const gchar *tagstring)
{
gchar **str_array;
guint count, i;
Tag *tag;

	DB( g_print("(transaction_set_tags)\n") );

	str_array = g_strsplit (tagstring, " ", 0);
	count = g_strv_length( str_array );
	
	g_free(ope->tags);
	ope->tags = NULL;

	DB( g_print(" -> reset storage %x\n", ope->tags) );

	
	if( count > 0 )
	{
		
		ope->tags = g_new0(guint32, count + 1);

		DB( g_print(" -> storage %x\n", ope->tags) );

		for(i=0;i<count;i++)
		{
			tag = da_tag_get_by_name(str_array[i]);
			if(tag == NULL)
			{
			Tag *newtag = da_tag_malloc();
			
				newtag->name = g_strdup(str_array[i]);
				da_tag_append(newtag);
				tag = da_tag_get_by_name(str_array[i]);
			}
	
			DB( g_print(" -> storing %d=>%s at tags pos %d\n", tag->key, tag->name, i) );
	
			ope->tags[i] = tag->key;
		}
	}

	//hex_dump(ope->tags, sizeof(guint32*)*count+1);

	g_strfreev (str_array);

	return count;
}

gchar *
transaction_get_tagstring(Operation *ope)
{
guint count, i;
gchar **str_array;
gchar *tagstring;
Tag *tag;

	DB( g_print("transaction_get_tagstring\n") );

	DB( g_print(" -> tags at=%x\n", ope->tags) );

	if( ope->tags == NULL )
	{
	
		return NULL;
	}
	else
	{	
		count = transaction_count_tags(ope);

		DB( g_print(" -> tags at=%x, nbtags=%d\n", ope->tags, count) );

		str_array = g_new0(gchar*, count+1);	

		DB( g_print(" -> str_array at %x\n", str_array) );

		//hex_dump(ope->tags, sizeof(guint32*)*(count+1));

		for(i=0;i<count;i++)
		{
			DB( g_print(" -> try to get tag %d\n", ope->tags[i]) );
			
			tag = da_tag_get(ope->tags[i]);
			if( tag )
			{
				DB( g_print(" -> get %s at %d\n", tag->name, i) );
				str_array[i] = tag->name;
			}
			else
				str_array[i] = NULL;
			

		}

		tagstring = g_strjoinv(" ", str_array);

		g_free (str_array);

	}

	return tagstring;
}





