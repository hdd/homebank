/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2008 Maxime DOYEN
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
#include "hb_account.h"
#include "ui_account.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ui_acc_comboboxentry_get_name:
 * 
 * get the name of the active account or -1
 * 
 * Return value: a new allocated name tobe freed with g_free
 *
 */
gchar *
ui_acc_comboboxentry_get_name(GtkComboBoxEntry *entry_box)
{
gchar *cbname;
gchar *name = NULL;

	cbname = (gchar *)gtk_entry_get_text(GTK_ENTRY (GTK_BIN (entry_box)->child));

	if( cbname != NULL)
	{

		name = g_strdup(cbname);
		g_strstrip(name);
	}

	return name;
}


/**
 * ui_acc_comboboxentry_get_key:
 * 
 * get the key of the active account
 * and create the account if it do not exists
 * 
 * Return value: the key or 0
 *
 */
guint32
ui_acc_comboboxentry_get_key(GtkComboBoxEntry *entry_box)
{
gchar *name;
Account *item;

	name = ui_acc_comboboxentry_get_name(entry_box);

	item = da_acc_get_by_name(name);

	g_free(name);

	if( item == NULL )
	{
		//todo: ask the user here		
		/*		
		item = da_acc_malloc();
		item->name = g_strdup(name);
		da_acc_insert(item);
		ui_acc_comboboxentry_add(entry_box, item);
		*/
		return 0;
	}



	return item->key;
}

gboolean
ui_acc_comboboxentry_set_active(GtkComboBoxEntry *entry_box, guint32 key)
{
Account *item;
	
	if( key > 0 )
	{	
		item = da_acc_get(key);
		if( item != NULL)
		{
			gtk_entry_set_text(GTK_ENTRY (GTK_BIN (entry_box)->child), item->name);
			return TRUE;
		}
	}
	gtk_entry_set_text(GTK_ENTRY (GTK_BIN (entry_box)->child), "");
	return FALSE;
}




/**
 * ui_acc_comboboxentry_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_acc_comboboxentry_add(GtkComboBoxEntry *entry_box, Account *acc)
{
	if( acc->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter  iter;

		model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, acc->name, -1);
	}
}

struct accPopContext
{
	GtkTreeModel *model;
	guint	except_key;
};

static void
ui_acc_comboboxentry_populate_ghfunc(gpointer key, gpointer value, struct accPopContext *ctx)
{
GtkTreeIter  iter;
Account *acc = value;

	if( acc->key != ctx->except_key )
	{
		gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter, 0, acc->name, -1);
	}
}

/**
 * ui_acc_comboboxentry_populate:
 * 
 * Populate the list and completion
 * 
 * Return value: --
 *
 */
void
ui_acc_comboboxentry_populate(GtkComboBoxEntry *entry_box, GHashTable *hash)
{
	ui_acc_comboboxentry_populate_except(entry_box, hash, 0);
}

void
ui_acc_comboboxentry_populate_except(GtkComboBoxEntry *entry_box, GHashTable *hash, guint except_key)
{
GtkTreeModel *model;
GtkEntryCompletion *completion;
struct accPopContext ctx;

    DB( g_print ("populate comboboxentry occured\n") );

    DB( g_print (" -> except is %d\n", except_key) );

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));
	completion = gtk_entry_get_completion(GTK_ENTRY (GTK_BIN (entry_box)->child));
	
	/* keep our model alive and detach from comboboxentry and completion */
	g_object_ref(model);
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), NULL);
	gtk_entry_completion_set_model (completion, NULL);

	/* clear and populate */
	ctx.model = model;
	ctx.except_key = except_key;
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_hash_table_foreach(hash, (GHFunc)ui_acc_comboboxentry_populate_ghfunc, &ctx);

	/* reatach our model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	gtk_entry_completion_set_model (completion, model);
	g_object_unref(model);

	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
	
}


/*
static gint
ui_acc_comboboxentry_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint ret = 0;
gchar *name1, *name2;

        gtk_tree_model_get(model, a, 0, &name1, -1);
        gtk_tree_model_get(model, b, 0, &name2, -1);

        if (name1 == NULL || name2 == NULL)
        {
          if (name1 == NULL && name2 == NULL)
            goto end;

          ret = (name1 == NULL) ? -1 : 1;
        }
        else
        {
          ret = g_utf8_collate(name1,name2);
        }

        g_free(name1);
        g_free(name2);

  end:
  	return ret;
  }
*/

/**
 * ui_acc_comboboxentry_new:
 * 
 * Create a new account comboboxentry
 * 
 * Return value: the new widget
 *
 */
GtkWidget *
ui_acc_comboboxentry_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *comboboxentry;
GtkEntryCompletion *completion;

	store = gtk_list_store_new (1, G_TYPE_STRING);
	//gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_comboboxentry_compare_func, NULL, NULL);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
    gtk_entry_completion_set_text_column (completion, 0);

	comboboxentry = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(store), 0);
	gtk_entry_set_completion (GTK_ENTRY (GTK_BIN (comboboxentry)->child), completion);

	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), comboboxentry);

	gtk_widget_set_size_request (comboboxentry, 10, -1);

	return comboboxentry;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
ui_acc_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFACC_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_acc_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Account *entry1, *entry2;
//gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFACC_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFACC_DATAS, &entry2, -1);

/*
	name1 = entry1->name;
	name2 = entry2->name;
    if (name1 == NULL || name2 == NULL)
    {
        result = (name1 == NULL) ? -1 : 1;
    }
    else
    {
        result = g_utf8_collate(name1,name2);
    }
*/
	result = entry1->pos - entry2->pos;

    return result;
}

static void
ui_acc_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, LST_DEFACC_DATAS, &entry, -1);
	if(entry->name == NULL)
		name = _("(none)");
	else
		name = entry->name;

	#if MYDEBUG
		string = g_strdup_printf ("[%d] %s", entry->key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}



/* = = = = = = = = = = = = = = = = */

/**
 * acc_list_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_acc_listview_add(GtkTreeView *treeview, Account *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFACC_TOGGLE, FALSE,
			LST_DEFACC_DATAS, item,
			-1);

		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}

guint32
ui_acc_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Account *item;

		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);
		
		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}

void
ui_acc_listview_remove_selected(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
}


static gint ui_acc_glist_compare_func(Account *a, Account *b)
{
	return ((gint)a->pos - b->pos);
}


void ui_acc_listview_populate(GtkWidget *view)
{
GtkTreeModel *model;
GtkTreeIter	iter;
GList *list;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	//g_hash_table_foreach(GLOBALS->h_acc, (GHFunc)ui_acc_listview_populate_ghfunc, model);
	list = g_hash_table_get_values(GLOBALS->h_acc);
	
	list = g_list_sort(list, (GCompareFunc)ui_acc_glist_compare_func);
	while (list != NULL)
	{
	Account *item = list->data;
	
		DB( g_print(" populate: %d\n", item->key) );

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFACC_TOGGLE	, FALSE,
			LST_DEFACC_DATAS, item,
			-1);

		list = g_list_next(list);
	}
	g_list_free(list);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}


GtkWidget *
ui_acc_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(NUM_LST_DEFACC,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFACC_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_acc_listview_toggled_cb), store);

	}

	// column 2: name
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFACC_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);
	
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_listview_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
** get widgets contents to the selected account
*/
/*
static void ui_acc_manage_get(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gchar *txt;
gboolean bool;
gdouble value;

Account *item;

gint field = GPOINTER_TO_INT(user_data);

	DB( g_printf("(ui_acc_manage_) get %d\n", field) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		data->change++;

		switch( field )
		{
			case FIELD_NAME:
				txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
				// ignore if entry is empty
				if (txt && *txt)
				{
					bool = account_rename(item, txt);					
					if(bool)
					{
						gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
					}
					else
					{
						gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
					}
				}
				break;

		//todo: for stock account			
		
			//case FIELD_TYPE:
			//	item->type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));
			//	break;
		
			case FIELD_BANK:
				g_free(item->bankname);
				item->bankname = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_bank)));
				break;

			case FIELD_NUMBER:
				g_free(item->number);
				item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));
				break;

			case FIELD_BUDGET:
				item->flags &= ~(AF_BUDGET);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budget));
				if(bool) item->flags |= AF_BUDGET;
				break;

			case FIELD_CLOSED:
				item->flags &= ~(AF_CLOSED);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_closed));
				if(bool) item->flags |= AF_CLOSED;
				break;

			case FIELD_INITIAL:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
				item->initial = value;
				break;

			case FIELD_MINIMUM:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
				item->minimum = value;
				break;

			case FIELD_CHEQUE1:
				item->cheque1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque1));
				break;

			case FIELD_CHEQUE2:
				item->cheque2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque2));
				break;
		}
	}

}
*/

static void ui_acc_manage_getlast(struct ui_acc_manage_data *data)
{
gchar *txt;
gboolean bool;
gdouble value;
Account *item;

	DB( g_printf("\n(ui_acc_manage_getlast)\n") );

	DB( g_printf(" -> for account id=%d\n", data->lastkey) );

	item = da_acc_get(data->lastkey);
	if(item != NULL)
	{	
		data->change++;

		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
				if (txt && *txt)
				{
					bool = account_rename(item, txt);					
					if(bool)
					{
						gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
					}
					else
					{
						gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
					}
				}

			g_free(item->bankname);
			item->bankname = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_bank)));

			g_free(item->number);
			item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));

				item->flags &= ~(AF_BUDGET);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budget));
				if(bool) item->flags |= AF_BUDGET;

				item->flags &= ~(AF_CLOSED);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_closed));
				if(bool) item->flags |= AF_CLOSED;

				gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_initial));
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
				item->initial = value;

				gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_minimum));
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
				item->minimum = value;

				gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_cheque1));
				item->cheque1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque1));

				gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_cheque2));
				item->cheque2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque2));
	}

}



/*
** set widgets contents from the selected account
*/
static void ui_acc_manage_set(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

Account *item;

	DB( g_printf("\n(ui_acc_manage_set)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		DB( g_printf(" -> set acc id=%d\n", item->key) );


		gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
		
		//todo: for stock account
		//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_type), item->type );
		
		if(item->bankname != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), item->bankname);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), "");

		if(item->number != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), item->number);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), "");


			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_budget), item->flags & AF_BUDGET);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_closed), item->flags & AF_CLOSED);


			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_initial), item->initial);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_minimum), item->minimum);

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque1), item->cheque1);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque2), item->cheque2);

	}

}

/*
static gboolean ui_acc_manage_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	ui_acc_manage_get(widget, user_data);
	return FALSE;
}
*/

/*
** update the widgets status and contents from action/selection value
*/
static void ui_acc_manage_update(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;
guint32 key;
//todo: for stock account
//gboolean is_new;

	DB( g_printf("\n(ui_acc_manage_update)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), &model, &iter);
	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));

	DB( g_printf(" -> selected = %d  action = %d key = %d\n", selected, data->action, key) );

	//todo amiga/linux
	/*
	if(acc)
	{
	// check for archives related
		for(i=0;;i++)
		{
		struct Archive *arc;

			DoMethod(data->mwd->LV_arc, MUIM_List_GetEntry, i, &arc);
			if(!arc) break;
			if(arc->arc_Account == acc->acc_Id)
			{ nbarc++; break; }
		}

	// check for operation related
		for(i=0;;i++)
		{
		struct Operation *ope;

			DoMethod(data->mwd->LV_ope, MUIM_List_GetEntry, i, &ope);
			if(!ope) break;
			if(ope->ope_Account == acc->acc_Id)
			{ nbope++; break; }
		}
	}	*/

	//todo: for stock account
	//todo: lock type if oldpos!=0
/*
	if( selected )
	{
		gtk_tree_model_get(model, &iter,
			LST_DEFACC_NEW, &is_new,
			-1);
		gtk_widget_set_sensitive(data->CY_type, is_new);
	}
*/

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name, sensitive);
	gtk_widget_set_sensitive(data->ST_number, sensitive);
	gtk_widget_set_sensitive(data->ST_bank, sensitive);
	gtk_widget_set_sensitive(data->CM_budget, sensitive);
	gtk_widget_set_sensitive(data->CM_closed, sensitive);

	gtk_widget_set_sensitive(data->ST_initial, sensitive);
	gtk_widget_set_sensitive(data->ST_minimum, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque1, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque2, sensitive);


	//sensitive = (data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->LV_acc, sensitive);
	//gtk_widget_set_sensitive(data->BT_new, sensitive);

	sensitive = (selected == TRUE && data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	if(selected)
	{
		if(key != data->lastkey)
		{
			DB( g_print(" -> should first do a get for account %d\n", data->lastkey) );
			ui_acc_manage_getlast(data);
		}

		ui_acc_manage_set(widget, NULL);
	}

	data->lastkey = key;

}


/*
** add an empty new account to our temp GList and treeview
*/
static void ui_acc_manage_add(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
Account *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_acc_manage_add) (data=%x)\n", (guint)data) );

	item = da_acc_malloc();
	item->name = g_strdup_printf( _("(account %d)"), da_acc_length()+1);

	da_acc_append(item);
	ui_acc_listview_add(GTK_TREE_VIEW(data->LV_acc), item);

	data->change++;
}

/*
** remove the selected account to our treeview and temp GList
*/
static void ui_acc_manage_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
Account *item;
guint32 key;
gboolean do_remove;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_acc_manage_remove) (data=%x)\n", (guint)data) );

	do_remove = TRUE;
	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
		if( account_is_used(key) == TRUE )
		{		
			item = da_acc_get(key);
			do_remove = FALSE;
			homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_INFO,
				_("Remove not allowed"),
				_("This account is used and cannot be removed."),
				NULL);
			
		}

		if( do_remove )
		{
			da_acc_remove(key);
			ui_acc_listview_remove_selected(GTK_TREE_VIEW(data->LV_acc));
			data->change++;
		}
	}
}

/*
** rename the selected account to our treeview and temp GList
*/
static void ui_acc_manage_rename(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
Account *item;
guint32 key;
gchar *txt;
gboolean bool;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_acc_manage_rename) (data=%x)\n", (guint)data) );

	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
		item = da_acc_get(key);
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
		if (txt && *txt)
		{
			bool = account_rename(item, txt);					
			if(bool)
			{
				gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
			}
		}
		
		data->change++;
	}
}





/*
**
*/
static void ui_acc_manage_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_acc_manage_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

//gint ui_acc_manage_list_sort(struct _Account *a, struct _Account *b) { return( a->acc_Id - b->acc_Id); }

/*
**
*/
static gboolean ui_acc_manage_cleanup(struct ui_acc_manage_data *data, gint result)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gint i;
guint32 key;
gboolean doupdate = FALSE;

	DB( g_printf("\n(ui_acc_manage_cleanup) %x\n", (guint)data) );

		key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
		if(key > 0)
		{
			data->lastkey = key;
			DB( g_print(" -> should first do a get for account %d\n", data->lastkey) );
			ui_acc_manage_getlast(data);
		}

		// test for change & store new position
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
		i=1; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Account *item;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFACC_DATAS, &item,
				-1);

			DB( g_printf(" -> check acc %d, pos is %d, %s\n", i, item->pos, item->name) );

			if(item->pos != i)
				data->change++;

			item->pos = i;

			// Make iter point to the next row in the list store 
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	GLOBALS->change += data->change;

	return doupdate;
}

/*
**
*/
static void ui_acc_manage_setup(struct ui_acc_manage_data *data)
{

	DB( g_printf("\n(ui_acc_manage_setup)\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->acc_list, sizeof(struct _Account));
	data->action = 0;
	data->change = 0;
	data->lastkey = 0;

	ui_acc_listview_populate(data->LV_acc);
	//populate_view_acc(data->LV_acc, GLOBALS->acc_list, TRUE);
}

/*
**
*/
GtkWidget *ui_acc_manage_dialog (void)
{
struct ui_acc_manage_data data;
GtkWidget *window, *mainbox;
GtkWidget *vbox, *table, *label, *entry1, *entry2, *entry3;
GtkWidget *spinner, *cheque1, *cheque2, *scrollwin;
GtkWidget *bbox, *check_button;
GtkWidget *alignment;
gint row;

	window = gtk_dialog_new_with_buttons (_("Manage Accounts"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
			    GTK_STOCK_CLOSE,
			    GTK_RESPONSE_ACCEPT,
				NULL);

	data.window = window;

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);

	//set the window icon
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "account.svg");

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(ui_acc_manage_) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

	//window contents
	mainbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);



	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), vbox, FALSE, FALSE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);

	data.LV_acc = ui_acc_listview_new(FALSE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data.LV_acc);

	// tools buttons
	bbox = gtk_hbox_new (TRUE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, TRUE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER (bbox), SP_BORDER);
	//gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), HB_BOX_SPACING);

	//data.BT_rem = gtk_button_new_with_mnemonic(_("_Remove"));
	data.BT_rem = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_rem);

	//data.BT_new = gtk_button_new_with_mnemonic(_("_New"));
	data.BT_new = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_new);





	//h_paned
	//vbox = gtk_vbox_new (FALSE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER (vbox), SP_BORDER);
	//gtk_box_pack_start (GTK_BOX (mainbox), vbox, TRUE, TRUE, 0);

	table = gtk_table_new (12, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.5, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (mainbox), alignment, TRUE, TRUE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Informations</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("_Heading:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry1 = make_string(label);
	data.ST_name = entry1;
	gtk_table_attach (GTK_TABLE (table), entry1, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//todo: for stock account
	/*	
	row++;
	label = make_label(_("_Type:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_ACCOUNT_TYPE);
	data.CY_type = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	*/

	row++;
	label = make_label(_("_Number:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry3 = make_string(label);
	data.ST_number = entry3;
	gtk_table_attach (GTK_TABLE (table), entry3, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Bank name:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1,(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry2 = make_string(label);
	data.ST_bank = entry2;
	gtk_table_attach (GTK_TABLE (table), entry2, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	check_button = gtk_check_button_new_with_mnemonic (_("Include in the budget"));
	data.CM_budget = check_button;
	gtk_table_attach (GTK_TABLE (table), check_button, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	check_button = gtk_check_button_new_with_mnemonic (_("This account was closed"));
	data.CM_closed = check_button;
	gtk_table_attach (GTK_TABLE (table), check_button, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);



//other


	//row = 0;
	row++;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Balances</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Initial:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	spinner = make_amount(label);
	data.ST_initial = spinner;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), spinner);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Overdrawn at:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	spinner = make_amount(label);
	data.ST_minimum = spinner;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), spinner);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


// frame 3
	row++;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Cheque number</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("Notebook _1:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	cheque1 = make_long (label);
	data.ST_cheque1 = cheque1;
	//hbox = gtk_hbox_new (FALSE, 0);
	//gtk_box_pack_start (GTK_BOX (hbox), cheque1, FALSE, FALSE, 0);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), cheque1);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("Notebook _2:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	cheque2 = make_long (label);
	data.ST_cheque2 = cheque2;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), cheque2);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);





	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_acc)), "changed", G_CALLBACK (ui_acc_manage_selection), NULL);

	g_signal_connect (G_OBJECT (data.ST_name), "changed", G_CALLBACK (ui_acc_manage_rename), NULL);





	g_signal_connect (G_OBJECT (data.BT_new), "clicked", G_CALLBACK (ui_acc_manage_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_acc_manage_remove), NULL);

	//setup, init and show window
	ui_acc_manage_setup(&data);
	ui_acc_manage_update(data.LV_acc, NULL);

//	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	// cleanup and destroy
	ui_acc_manage_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}


