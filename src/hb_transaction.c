/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2011 Maxime DOYEN
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
/* new transfer functions */

Operation *operation_strong_get_child_transfer(Operation *src)
{
GList *list;

	DB( g_print("(transaction) operation_strong_get_child_transfer\n") );

	DB( g_print(" - search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->dst_account) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		Operation *item = list->data;
		if( item->paymode == PAYMODE_INTXFER && item->account == src->dst_account && item->kxfer == src->kxfer )
		{
			DB( g_print(" - found : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->account, item->dst_account) );
			return item;			
		}
		list = g_list_next(list);
	}		
	DB( g_print(" - not found...\n") );
	return NULL;
}

/* 
 * this function retrieve into a glist the potential child transfer 
 * for the source transaction
 */
GList *operation_match_get_child_transfer(Operation *src)
{
GList *list;
GList *match = NULL;

	DB( g_print("(transaction) operation_match_get_child_transfer\n") );

	DB( g_print(" - search : %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->dst_account) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		Operation *item = list->data;
		if( src->date == item->date &&
		    src->dst_account == item->account &&
		    ABS(src->amount) == ABS(item->amount) &&
		    item->kxfer == 0)
		{
			DB( g_print(" - match : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->account, item->dst_account) );

			match = g_list_append(match, item);
		}
		list = g_list_next(list);
	}		

	DB( g_print(" - found : %d\n", g_list_length(match)) );
	
	return match;
}


//todo: move this to ui part
#include "list_operation.h"

struct xfer_data
{
	GtkWidget	*radio[2];
	GtkWidget	*treeview;
};


static void operation_on_action_toggled(GtkRadioButton *radiobutton, gpointer user_data)
{
struct xfer_data *data;
gboolean new;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(import) account type toggle\n") );

	new = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0]));

	gtk_widget_set_sensitive(data->treeview, new^1);

}


Operation *operation_xfer_select_child(GList *matchlist)
{
struct xfer_data data;
GtkWidget *window, *mainvbox, *vbox, *sw, *label;
GtkTreeModel		 *newmodel;
GtkTreeIter			 newiter;
Operation *retval = NULL;
	
			window = gtk_dialog_new_with_buttons (NULL,
						    //GTK_WINDOW (parentwindow),
			    			NULL,
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);

	
		//gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_window_set_default_size (GTK_WINDOW (window), 400, -1);

		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		gtk_window_set_title (GTK_WINDOW (window), _("Select among possible transactions..."));

		label = make_label(_(
		"HomeBank has found some transaction that may be " \
		"the associated transaction for the internal transfer."), 0.0, 0.5
		);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, HB_BOX_SPACING);


	vbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainvbox), vbox, FALSE, TRUE, HB_BOX_SPACING);

	label = make_label(NULL, 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Select an action:</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	
	
	data.radio[0] = gtk_radio_button_new_with_label (NULL, _("create a new transaction"));
	gtk_box_pack_start (GTK_BOX (vbox), data.radio[0], FALSE, FALSE, 0);

	data.radio[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (data.radio[0]), _("select an existing transaction"));
	gtk_box_pack_start (GTK_BOX (vbox), data.radio[1], FALSE, FALSE, 0);

	g_signal_connect (data.radio[0], "toggled", G_CALLBACK (operation_on_action_toggled), NULL);

	
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	data.treeview = create_list_operation(TRN_LIST_TYPE_BOOK, PREFS->lst_ope_columns);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(data.treeview)), GTK_SELECTION_SINGLE);
	gtk_container_add (GTK_CONTAINER (sw), data.treeview);

	gtk_box_pack_start (GTK_BOX (mainvbox), sw, TRUE, TRUE, 0);

	/* populate */
	newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data.treeview));



	
	gtk_list_store_clear (GTK_LIST_STORE(newmodel));

	
	GList *tmplist = g_list_first(matchlist);
	while (tmplist != NULL)
	{
	Operation *tmp = tmplist->data;

		/* append to our treeview */
			gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

			gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
			LST_DSPOPE_DATAS, tmp,
			-1);

		//DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, (unsigned int)item->same) );

		tmplist = g_list_next(tmplist);
	}

	//initialize
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data.radio[1]), TRUE);

	
		gtk_widget_show_all(mainvbox);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gboolean bnew;
			
			bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data.radio[0]));
			if( bnew == FALSE)
			{
			GtkTreeSelection *selection;
			GtkTreeModel		 *model;
			GtkTreeIter			 iter;

				selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data.treeview));
				if (gtk_tree_selection_get_selected(selection, &model, &iter))
				{
					gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &retval, -1);
				}
			}

		}

		// cleanup and destroy
		gtk_widget_destroy (window);

	return retval;
}



void operation_xfer_search_or_add_child(Operation *ope, GtkWidget *treeview)
{
GList *matchlist = operation_match_get_child_transfer(ope);

guint count = g_list_length(matchlist);


	DB( g_print("(transaction) operation_xfer_search_or_add_child\n") );

	DB( g_printf(" - found result is %d, switching\n", count) );
	
	switch(count)
	{
		case 0:		//we should create the child
			operation_xfer_create_child(ope, treeview);
			break;


		//todo: maybe with just 1 match the user must choose ?
		
		case 1:		//transform the transaction to a child transfer
		{
			GList *list = g_list_first(matchlist);
			operation_xfer_change_to_child(ope, list->data);
			break;
		}
		
		default:	//the user must choose himself
		{
		Operation *child;

			child = operation_xfer_select_child(matchlist);
			if(child == NULL)
				operation_xfer_create_child(ope, treeview);
			else
				operation_xfer_change_to_child(ope, child);
			break;
		}
	}

	g_list_free(matchlist);
	
}






void operation_xfer_create_child(Operation *ope, GtkWidget *treeview)
{
Operation *child;
Account *acc;
gchar swap;

	DB( g_printf("(transaction) operation_xfer_create_child\n") );

	if( ope->dst_account > 0 )
	{
		child = da_operation_clone(ope);

		child->amount = -child->amount;
		child->flags ^= (OF_INCOME);	//xor
		child->flags &= (~OF_REMIND);

		swap = child->account;
		child->account = child->dst_account;
		child->dst_account = swap;

		/* update acc flags */
		acc = da_acc_get( child->account);
		if(acc != NULL)
			acc->flags |= AF_ADDED;

		//strong link
		guint maxkey = da_operation_get_max_kxfer();

		DB( g_printf(" + maxkey is %d\n", maxkey) );

		
		ope->kxfer = maxkey+1;
		child->kxfer = maxkey+1;

		DB( g_printf(" + strong link to %d\n", ope->kxfer) );
		

		DB( g_printf(" + add transfer, %x\n", child) );

		da_operation_append(child);
		if(treeview != NULL)
			operation_add_treeview(child, treeview, ope->account);
	}

}


void operation_xfer_change_to_child(Operation *ope, Operation *child)
{
Account *acc;

	DB( g_printf("(transaction) operation_xfer_change_to_child\n") );

	child->paymode = PAYMODE_INTXFER;
	
	ope->dst_account = child->account;
	child->dst_account = ope->account;

	/* update acc flags */
	acc = da_acc_get( child->account);
	if(acc != NULL)
		acc->flags |= AF_CHANGED;

	//strong link
	guint maxkey = da_operation_get_max_kxfer();
	ope->kxfer = maxkey+1;
	child->kxfer = maxkey+1;

}


void operation_xfer_sync_child(Operation *ope, Operation *child)
{

	DB( g_printf("(transaction) operation_xfer_sync_child\n") );

	child->date			= ope->date;
	child->amount		= -ope->amount;
	child->flags		= child->flags | OF_CHANGED;
	child->payee		= ope->payee;
	child->category	= ope->category;
	if(child->wording)
		g_free(child->wording);
	child->wording		= g_strdup(ope->wording);
	if(child->info)
		g_free(child->info);
	child->info		= g_strdup(ope->info);

	//todo: synchronise tags here also ?
	
}


void operation_xfer_delete_child(Operation *src)
{
Operation *dst;

	DB( g_printf("(transaction) operation_xfer_delete_child\n") );

	dst = operation_strong_get_child_transfer( src );
	
	DB( g_print(" -> return is %s, %d\n", dst, dst) );
	
	if( dst != NULL )
	{
		DB( g_print("deleting...") );
		
		GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, dst);
	}
}


Operation *operation_old_get_child_transfer(Operation *src)
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




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


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
	acc = da_acc_get( newope->account);
	if(acc != NULL)
		acc->flags |= AF_ADDED;

	DB( g_printf(" + add normal %x\n", newope) );
	da_operation_append(newope);
	if(treeview != NULL) operation_add_treeview(newope, treeview, accnum);

	if(newope->paymode == PAYMODE_INTXFER)
	{
		operation_xfer_search_or_add_child(newope, treeview);
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
		
		
			if( ope->payee == 0 || ope->category == 0 )
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
								if( ope->payee == 0 )
								{	
									ope->payee    = rul->payee;
									ope->flags |= OF_CHANGED;
									changes++;
								}
								if( ope->category == 0 )
								{	
									ope->category = rul->category;
									ope->flags |= OF_CHANGED;
									changes++;
								}
							}
						}
						else
						{
						gchar *word   = g_utf8_casefold(ope->wording, -1);
						gchar *needle = g_utf8_casefold(rul->name, -1);
			
							if( g_strrstr(word, needle) != NULL )
							{
								DB( g_print(" found nocase\n") );				
								if( ope->payee == 0 )
								{	
									ope->payee    = rul->payee;
									ope->flags |= OF_CHANGED;
									changes++;
								}
								if( ope->category == 0 )
								{	
									ope->category = rul->category;
									ope->flags |= OF_CHANGED;
									changes++;
								}
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





