/* HomeBank -- Free easy personal accounting for all !
 * Copyright (C) 1995-2006 Maxime DOYEN
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

#include "def_payee.h"

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


struct defpayee_data
{
	GList	*tmp_list;
	gint	change;
	gint	action;

	GtkWidget	*window;


	GtkWidget	*LV_pay;
	GtkWidget	*TX_action;
	GtkWidget	*ST_name;
	GtkWidget	*PO_pay;
	GtkWidget	*GR_action;
	GtkWidget	*BT_okaction;
	GtkWidget	*BT_endaction;
	GtkWidget	*GR_tools;
	GtkWidget	*BT_add;
	GtkWidget	*BT_new;
	//GtkWidget	*BT_mov;
	GtkWidget	*BT_mod;
	GtkWidget	*BT_rem;
	GtkWidget	*BT_import, *BT_export;
};

enum
{
  COL_NAME = 0,
  COL_OLDINDEX,
  NUM_COLS
};

//todo amiga/linux
//save

void defpayee_real_add(struct defpayee_data *data, gchar *name);

/*
**
*/

static void defpayee_load_csv( GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
gchar *filename;
GIOChannel *io;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");


	DB( g_printf("(defpayee) load csv - data %x\n", data) );

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		gchar *tmpstr;
		gint io_stat;

			for(;;)
			{
				io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
				if( io_stat == G_IO_STATUS_EOF)
					break;
				if( io_stat == G_IO_STATUS_NORMAL)
				{
					if( tmpstr != "")
					{
						hb_string_strip_crlf(tmpstr);

						DB( g_print(" read %s\n", tmpstr) );

						defpayee_real_add(data, tmpstr);

						data->change++;
					}
					g_free(tmpstr);
				}

			}
			g_io_channel_unref (io);
		}

	}
}

/*
**
*/
static void defpayee_save_csv( GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
gchar *filename;
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
GIOChannel *io;

	DB( g_print("(defpayee) save csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename) == TRUE )
	{

		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));

			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

		    while (valid)
		    {
			gchar *outstr;
			Payee *payee;

				gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, COL_NAME, &payee, -1);

				if( payee->name != NULL )
				{

					outstr = g_strdup_printf("%s\n", payee->name);

					DB( g_print("%s", outstr) );

					g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

					g_free(outstr);
				}

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

	}

}


/*
**
*/
static void defpayee_update(GtkWidget *treeview, gpointer user_data)
{
struct defpayee_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;

	DB( g_printf("\n(defpayee) cursor changed\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");


	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay)), &model, &iter);

	DB( g_printf(" selected = %d\n", selected) );
	DB( g_printf(" action = %d\n", data->action) );

	//path 0 active ?
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter))
	{
		DB( g_printf(" 0 active = %d\n", 1) );


		selected = FALSE;
	}

	//sensitive = (data->action == ACTION_NEW || data->action == ACTION_MODIFY) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->ST_name, sensitive);

	//sensitive = (data->action == ACTION_MOVE) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->PO_pay, sensitive);

/*
	sensitive = (data->action == 0) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->LV_pay, sensitive);
	gtk_widget_set_sensitive(data->BT_new, sensitive);
*/

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->BT_mov, sensitive);
	gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

/*
	if(selected)
	{
	GtkTreeSelection *selection;
	GtkTreeModel		 *model;
	GtkTreeIter			 iter;
	struct _Payee *entry;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
		//if true there is a selected node
		if (gtk_tree_selection_get_selected(selection, &model, &iter))
		{
			gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &entry, -1);

			gtk_entry_set_text(GTK_ENTRY(data->ST_name), entry->pay_Name);


		}
	}
	*/
}


gint defpayee_exists (GtkTreeModel *liststore, gchar *name)
{
GtkTreeIter  iter;
gboolean     valid;
Payee *entry;
gint pos = 0;

    g_return_val_if_fail ( liststore != NULL, FALSE );

    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter);

    while (valid)
    {
	 /* ... do something with that row using the iter ...	    */
	 /* (Here column 0 of the list store is of type G_TYPE_STRING) */
		gtk_tree_model_get (liststore, &iter, LST_DEFACC_DATAS, &entry, -1);
		if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
		{
			return pos;
		}

	 /* Make iter point to the next row in the list store */
	 valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter);
	 pos++;
    }
	return 0;
}


gint defpayee_glist_exists(GList *src_list, gchar *name)
{
GList *list;
gint pos = 0;

	list = g_list_first(src_list);
	while (list != NULL)
	{
	Payee *entry = list->data;

		if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
		{
			return pos;
		}
		list = g_list_next(list);
		pos++;
	}
	return 0;
}


/*
** add an empty new payee to our temp GList and treeview
*/
void defpayee_real_add(struct defpayee_data *data, gchar *name)
{
GtkTreeModel *model;
GtkTreeIter  iter;
Payee *item;
gint exist;

	DB( g_printf("(defayee) real add (data=%08x)\n", data) );

	/* ignore if entry is empty */
	if (name && *name)
	{
		exist = defpayee_exists(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay)), name);
		if(exist == 0)
		{
			item = da_payee_malloc();
			item->name = g_strdup(name);

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);

			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DEFPAY_DATAS, item,
				LST_DEFPAY_OLDPOS, 0,
				-1);

			gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay)), &iter);

			data->change++;
		}
		else
		{
		GtkTreePath *path;

			/* activate the already exists */
			path = gtk_tree_path_new_from_indices(exist, -1);
			gtk_tree_selection_select_path (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay)), path);
			gtk_tree_path_free (path);
		}


	}



}



void defpayee_add(GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
gchar *name;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defayee) add (data=%08x)\n", data) );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));

	defpayee_real_add(data, name);

	gtk_entry_set_text(GTK_ENTRY(data->ST_name), "");
}


/*
** move
*/
/*
void defpayee_move(GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
GtkWidget *window, *mainvbox, *getwidget;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defayee) move\n", data) );

	// get selection ...
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	struct _Payee *entry;
	gushort oldpos;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &entry, -1);
		oldpos = entry->pay_Id;

		window = gtk_dialog_new_with_buttons ("Move to...",
						    //GTK_WINDOW (do_widget),
						    NULL,
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		getwidget = make_poppayee(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);

		gtk_combo_box_set_active(GTK_COMBO_BOX(getwidget), oldpos);

		gtk_widget_show_all(mainvbox);

		data->tmp_list = g_list_sort(data->tmp_list, (GCompareFunc)defpayee_list_sort);
		make_poppayee_populate(getwidget, data->tmp_list);
		gtk_widget_grab_focus (getwidget);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gint newpayee;

			newpayee = gtk_combo_box_get_active(GTK_COMBO_BOX(getwidget));

			DB( g_printf(" -> should move to %d, %s\n", newpayee, gtk_combo_box_get_active_text(GTK_COMBO_BOX(getwidget) ) ) );

			data->pos_vector[oldpos] = newpayee;



			// remove the old payee
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			data->tmp_list = g_list_remove(data->tmp_list, entry);
		}

		// cleanup and destroy
		gtk_widget_destroy (window);

	}

}
*/

/*
** modify
*/
void defpayee_modify(GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
GtkWidget *window, *mainvbox, *getwidget;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defayee) modify\n", data) );

	//get selection ...
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Payee *item;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &item, -1);

		window = gtk_dialog_new_with_buttons (_("Modify..."),
						    GTK_WINDOW (data->window),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		getwidget = gtk_entry_new();
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);
		gtk_widget_show_all(mainvbox);

		gtk_entry_set_text(GTK_ENTRY(getwidget), item->name);
		gtk_widget_grab_focus (getwidget);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			name = gtk_entry_get_text(GTK_ENTRY(getwidget));
			/* ignore if entry is empty */
			if (name && *name)
			{
				g_free(item->name);
				item->name = g_strdup(name);

				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), LST_DEFPAY_DATAS, GTK_SORT_DESCENDING);
				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), LST_DEFPAY_DATAS, GTK_SORT_ASCENDING);

				data->change++;
			}
	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}

/*
** remove the selected payee to our treeview and temp GList
*/
void defpayee_remove(GtkWidget *widget, gpointer user_data)
{
struct defpayee_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defpayee) remove (data=%08x)\n", data) );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Payee *item;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &item, -1);

		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		DB( g_printf(" remove =%08x (pos=%d)\n", item, g_list_index(data->tmp_list, item) ) );

		data->change++;
	}
}



/*
**
*/
void defpayee_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defpayee_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

void defpayee_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	{
		defpayee_modify(GTK_WIDGET(treeview), NULL);
	}
}

/*
**
*/

gboolean defpayee_cleanup(struct defpayee_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defpayee) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;
	gboolean valid;
	gushort i;
	guint *pos_vector;

		//do_application_specific_something ();
		DB( g_printf(" accept\n") );

		//allocate vector pos
		DB( g_print(" allocate %d guint\n", g_list_length(GLOBALS->pay_list)) );

		pos_vector = g_new0(guint, g_list_length(GLOBALS->pay_list));

		// test for change & store new position
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Payee *item;
		gint oldpos;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFPAY_DATAS, &item,
				LST_DEFPAY_OLDPOS, &oldpos,
				-1);

			item->key = i;
			data->tmp_list = g_list_append(data->tmp_list, item);

			if(pos_vector)
			{
				if(oldpos != 0)	// added payee have 0 has id and oldpos
				{
					pos_vector[oldpos] = i;
					if(doupdate == FALSE && oldpos != i) doupdate = TRUE;
				}

				 /* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}
		}

#if MYDEBUG == 1
		if(pos_vector)
		{
			g_print("vector change:\n");
			for(i=0;i<g_list_length(GLOBALS->pay_list);i++)
			{
				g_print(" %d => %d\n", i, pos_vector[i]);
			}
		}
#endif

		/* test if we should update */
		if(doupdate == TRUE)
		{
		GList *list;
		gint tmpval;

			DB( g_printf(" doing pos update\n") );

			/* -> change all archives payee */
			list = g_list_first(GLOBALS->arc_list);
			while (list != NULL)
			{
			Archive *entry = list->data;

				tmpval = entry->payee;
				entry->payee = pos_vector[tmpval];

				list = g_list_next(list);
			}

			/* -> all operations account & transfert */
			list = g_list_first(GLOBALS->ope_list);
			while (list != NULL)
			{
			Operation *entry = list->data;

				tmpval = entry->payee;
				entry->payee = pos_vector[tmpval];

				list = g_list_next(list);
			}

		}

		g_free(pos_vector);

		GLOBALS->change += data->change;

		//update our GLOBAL cat_list
		da_payee_destroy(GLOBALS->pay_list);
		GLOBALS->pay_list = data->tmp_list;
		data->tmp_list = NULL;

	}

	DB( g_printf(" free tmp_list\n") );
	da_payee_destroy(data->tmp_list);

	return doupdate;
}

/*
**
*/
void defpayee_setup(struct defpayee_data *data)
{

	DB( g_printf("(defpayee) setup\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->pay_list, sizeof(struct _Payee));
	data->action = 0;
	data->change = 0;

	populate_view_pay(data->LV_pay, GLOBALS->pay_list, TRUE);
}


/*
**
*/
GtkWidget *create_defpayee_window (void)
{
struct defpayee_data data;
GtkWidget *window, *mainvbox, *treeview, *scrollwin, *vbox, *table;
GtkWidget *separator;
gint row;

	window = gtk_dialog_new_with_buttons (_("Edit Payees"),
					    //GTK_WINDOW (do_widget),
					    NULL,
					    0,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);
	
	gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/payee.svg", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(defpayee) window=%08lx, inst_data=%08lx\n", window, &data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	//window contents
	mainvbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

    //our table
	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	//row 0
	row = 0;
	data.ST_name = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), data.ST_name, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.BT_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//list
	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_table_attach (GTK_TABLE (table), scrollwin, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 	treeview = (GtkWidget *)defpayee_list_new(FALSE);
 	data.LV_pay = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), vbox, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	data.BT_rem = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_rem, FALSE, FALSE, 0);

	data.BT_mod = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	//data.BT_mod = gtk_button_new_with_mnemonic(_("_Modify"));
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_mod, FALSE, FALSE, 0);

	//data.BT_mov = gtk_button_new_with_label("Move");
	//gtk_box_pack_start (GTK_BOX (vbox), data.BT_mov, FALSE, FALSE, 0);

	separator = gtk_hseparator_new();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, HB_BOX_SPACING);


	data.BT_import = gtk_button_new_with_mnemonic(_("_Import"));
	//data.BT_import = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_import, FALSE, FALSE, 0);

	data.BT_export = gtk_button_new_with_mnemonic(_("E_xport"));
	//data.BT_export = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_export, FALSE, FALSE, 0);



	//connect all our signals
	g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (defpayee_add), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_pay)), "changed", G_CALLBACK (defpayee_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_pay), "row-activated", G_CALLBACK (defpayee_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_add), "clicked", G_CALLBACK (defpayee_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_mod), "clicked", G_CALLBACK (defpayee_modify), NULL);
	//g_signal_connect (G_OBJECT (data.BT_mov), "clicked", G_CALLBACK (defpayee_move), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (defpayee_remove), NULL);

	g_signal_connect (G_OBJECT (data.BT_import), "clicked", G_CALLBACK (defpayee_load_csv), NULL);
	g_signal_connect (G_OBJECT (data.BT_export), "clicked", G_CALLBACK (defpayee_save_csv), NULL);

	//setup, init and show window
	defpayee_setup(&data);
	defpayee_update(data.LV_pay, NULL);

	gtk_window_resize(GTK_WINDOW(window), 200, 320);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	switch (result)
    {
	case GTK_RESPONSE_ACCEPT:
	   //do_application_specific_something ();
	   break;
	default:
	   //do_nothing_since_dialog_was_cancelled ();
	   break;
    }

	// cleanup and destroy
	defpayee_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}
