/*  HomeBank -- Free, easy, personal ruleing for everyone.
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
#include "ui_assign.h"

#include "ui_category.h"
#include "ui_payee.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
ui_asg_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFASG_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFASG_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_asg_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Assign *entry1, *entry2;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFASG_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFASG_DATAS, &entry2, -1);


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


    return result;
}

static void
ui_asg_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Assign *entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, LST_DEFASG_DATAS, &entry, -1);
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
 * rul_list_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_asg_listview_add(GtkTreeView *treeview, Assign *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFASG_TOGGLE, FALSE,
			LST_DEFASG_DATAS, item,
			-1);

		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}

guint32
ui_asg_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Assign *item;

		gtk_tree_model_get(model, &iter, LST_DEFASG_DATAS, &item, -1);
		
		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}

void
ui_asg_listview_remove_selected(GtkTreeView *treeview)
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

/*
static gint ui_asg_glist_compare_func(Assign *a, Assign *b)
{
	return 0; //((gint)a->pos - b->pos);
}
*/

void ui_asg_listview_populate(GtkWidget *view)
{
GtkTreeModel *model;
GtkTreeIter	iter;
GList *list;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	//g_hash_table_foreach(GLOBALS->h_rul, (GHFunc)ui_asg_listview_populate_ghfunc, model);
	list = g_hash_table_get_values(GLOBALS->h_rul);
	
	//list = g_list_sort(list, (GCompareFunc)ui_asg_glist_compare_func);
	while (list != NULL)
	{
	Assign *item = list->data;
	
		DB( g_print(" populate: %d\n", item->key) );

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFASG_TOGGLE	, FALSE,
			LST_DEFASG_DATAS, item,
			-1);

		list = g_list_next(list);
	}
	g_list_free(list);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}


GtkWidget *
ui_asg_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(NUM_LST_DEFASG,
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
							     "active", LST_DEFASG_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_asg_listview_toggled_cb), store);

	}

	// column 2: name
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_asg_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFASG_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);
	
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_asg_listview_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
//todo move this
static gboolean
assign_rename(Assign *item, gchar *newname)
{
Account *existitem;

	existitem = da_acc_get_by_name(newname);
	if( existitem == NULL )
	{
		g_free(item->name);
		item->name = g_strdup(newname);
		return TRUE;	
	}

	return FALSE;
}



static void ui_asg_manage_getlast(struct ui_asg_manage_data *data)
{
gchar *txt;
gboolean bool;
Assign *item;

	DB( g_printf("\n(ui_asg_manage_getlast)\n") );

	DB( g_printf(" -> for assign id=%d\n", data->lastkey) );

	item = da_asg_get(data->lastkey);
	if(item != NULL)
	{	
		data->change++;

		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
				if (txt && *txt)
				{
					bool = assign_rename(item, txt);					
					if(bool)
					{
						gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_rul));
					}
					else
					{
						gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
					}
				}

			item->exact  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_exact));

			item->category    = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_cat));
			item->payee       = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_pay));


	}

}



/*
** set widgets contents from the selected assign
*/
static void ui_asg_manage_set(GtkWidget *widget, gpointer user_data)
{
struct ui_asg_manage_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

Assign *item;

	DB( g_printf("\n(ui_asg_manage_set)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_rul));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFASG_DATAS, &item, -1);

		DB( g_printf(" -> set rul id=%d\n", item->key) );


		gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_exact), item->exact);

		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_cat), item->category);
		ui_pay_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_pay), item->payee);


	}

}

/*
static gboolean ui_asg_manage_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	ui_asg_manage_get(widget, user_data);
	return FALSE;
}
*/

/*
** update the widgets status and contents from action/selection value
*/
static void ui_asg_manage_update(GtkWidget *widget, gpointer user_data)
{
struct ui_asg_manage_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;
guint32 key;
//todo: for stock assign
//gboolean is_new;

	DB( g_printf("\n(ui_asg_manage_update)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_rul)), &model, &iter);
	key = ui_asg_listview_get_selected_key(GTK_TREE_VIEW(data->LV_rul));

	DB( g_printf(" -> selected = %d  action = %d key = %d\n", selected, data->action, key) );


	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name, sensitive);
	gtk_widget_set_sensitive(data->CM_exact, sensitive);
	gtk_widget_set_sensitive(data->PO_pay, sensitive);
	gtk_widget_set_sensitive(data->PO_cat, sensitive);



	//sensitive = (data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->LV_rul, sensitive);
	//gtk_widget_set_sensitive(data->BT_new, sensitive);

	sensitive = (selected == TRUE && data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	if(selected)
	{
		if(key != data->lastkey)
		{
			DB( g_print(" -> should first do a get for assign %d\n", data->lastkey) );
			ui_asg_manage_getlast(data);
		}

		ui_asg_manage_set(widget, NULL);
	}

	data->lastkey = key;

}


/*
** add an empty new assign to our temp GList and treeview
*/
static void ui_asg_manage_add(GtkWidget *widget, gpointer user_data)
{
struct ui_asg_manage_data *data;
Assign *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_asg_manage_add) (data=%x)\n", (guint)data) );

	item = da_asg_malloc();
	item->name = g_strdup_printf( _("(assignment %d)"), da_asg_length()+1);

	da_asg_append(item);
	ui_asg_listview_add(GTK_TREE_VIEW(data->LV_rul), item);

	data->change++;
}

/*
** remove the selected assign to our treeview and temp GList
*/
static void ui_asg_manage_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_asg_manage_data *data;
guint32 key;
gboolean do_remove;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_asg_manage_remove) (data=%x)\n", (guint)data) );

	do_remove = TRUE;
	key = ui_asg_listview_get_selected_key(GTK_TREE_VIEW(data->LV_rul));
	if( key > 0 )
	{
		if( do_remove )
		{
			da_asg_remove(key);
			ui_asg_listview_remove_selected(GTK_TREE_VIEW(data->LV_rul));
			data->change++;
		}
	}
}

/*
** rename the selected assign to our treeview and temp GList
*/
static void ui_asg_manage_rename(GtkWidget *widget, gpointer user_data)
{
struct ui_asg_manage_data *data;
Assign *item;
guint32 key;
gchar *txt;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("\n(ui_asg_manage_rename) (data=%x)\n", (guint)data) );

	key = ui_asg_listview_get_selected_key(GTK_TREE_VIEW(data->LV_rul));
	if( key > 0 )
	{
		item = da_asg_get(key);
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
		if (txt && *txt)
		{
/*			bool = assign_rename(item, txt);					
			if(bool)
			{
				gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_rul));
			}
*/
		}
		
		data->change++;
	}
}





/*
**
*/
static void ui_asg_manage_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_asg_manage_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

//gint ui_asg_manage_list_sort(struct _Assign *a, struct _Assign *b) { return( a->rul_Id - b->rul_Id); }

/*
**
*/
static gboolean ui_asg_manage_cleanup(struct ui_asg_manage_data *data, gint result)
{
guint32 key;
gboolean doupdate = FALSE;

	DB( g_printf("\n(ui_asg_manage_cleanup) %x\n", (guint)data) );

		key = ui_asg_listview_get_selected_key(GTK_TREE_VIEW(data->LV_rul));
		if(key > 0)
		{
			data->lastkey = key;
			DB( g_print(" -> should first do a get for assign %d\n", data->lastkey) );
			ui_asg_manage_getlast(data);
		}

		// test for change & store new position
		/*
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_rul));
		i=1; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Assign *item;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFASG_DATAS, &item,
				-1);

			DB( g_printf(" -> check rul %d, pos is %d, %s\n", i, item->pos, item->name) );

			if(item->pos != i)
				data->change++;

			item->pos = i;

			// Make iter point to the next row in the list store 
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}
		*/

	GLOBALS->change += data->change;

	return doupdate;
}

/*
**
*/
static void ui_asg_manage_setup(struct ui_asg_manage_data *data)
{

	DB( g_printf("\n(ui_asg_manage_setup)\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->rul_list, sizeof(struct _Assign));
	data->action = 0;
	data->change = 0;
	data->lastkey = 0;

	ui_asg_listview_populate(data->LV_rul);

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_pay), GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_cat), GLOBALS->h_cat);
}

/*
**
*/
GtkWidget *ui_asg_manage_dialog (void)
{
struct ui_asg_manage_data data;
GtkWidget *window, *mainbox;
GtkWidget *vbox, *table, *label, *entry1;
GtkWidget *scrollwin;
GtkWidget *bbox;
GtkWidget *alignment, *widget;
gint row;

	window = gtk_dialog_new_with_buttons (_("Manage Assignments"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
			    GTK_STOCK_CLOSE,
			    GTK_RESPONSE_ACCEPT,
				NULL);

	data.window = window;

	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "assign.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_ASSIGN);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(ui_asg_manage_) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

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

	data.LV_rul = ui_asg_listview_new(FALSE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data.LV_rul);

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
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Search in Description</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("Contains the _text:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry1 = make_string(label);
	data.ST_name = entry1;
	gtk_table_attach (GTK_TABLE (table), entry1, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Case _sensitive"));
	data.CM_exact = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);



//other

	//row = 0;
	row++;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Automatic assignments</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_pay_comboboxentry_new(label);
	data.PO_pay = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Payee"));

	row++;
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_cat_comboboxentry_new(label);
	data.PO_cat = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Category"));


	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_rul)), "changed", G_CALLBACK (ui_asg_manage_selection), NULL);

	g_signal_connect (G_OBJECT (data.ST_name), "changed", G_CALLBACK (ui_asg_manage_rename), NULL);





	g_signal_connect (G_OBJECT (data.BT_new), "clicked", G_CALLBACK (ui_asg_manage_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_asg_manage_remove), NULL);

	//setup, init and show window
	ui_asg_manage_setup(&data);
	ui_asg_manage_update(data.LV_rul, NULL);

//	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	// cleanup and destroy
	ui_asg_manage_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}


