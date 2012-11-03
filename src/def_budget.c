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

#include "def_lists.h"
#include "def_budget.h"

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;



enum
{
  COL_NAME = 0,
  COL_OLDINDEX,
  NUM_COLS
};

enum {
	HID_CUSTOM,
	MAX_HID
};

#define FIELD_TYPE 15

struct defbudget_data
{
	GList	*tmp_list;
	gint	change;

	GtkWidget	*window;

	GtkWidget	*spinner[13];	//0 index is for All
	GtkWidget	*LV_cat;
	GtkWidget	*CM_type[2];

	GtkWidget	*BT_clear;
	GtkWidget	*BT_import, *BT_export;

	Category	*cat;

	gulong		spinner_hid[13];

	gulong		handler_id[MAX_HID];
};


gchar *months[] = {
"Jan",
"Feb",
"Mar",
"Apr",
"May",
"Jun",
"Jul",
"Aug",
"Sep",
"Oct",
"Nov",
"Dec"
};

void defbudget_update(GtkWidget *treeview, gpointer user_data);
void defbudget_set(GtkWidget *widget, gpointer user_data);
void defbudget_get(GtkWidget *widget, gpointer user_data);
void defbudget_selection_change(GtkWidget *treeview, gpointer user_data);
void defbudget_toggle(GtkRadioButton *radiobutton, gpointer user_data);
void defbudget_selection(GtkTreeSelection *treeselection, gpointer user_data);

/*
**
*/
gchar *defbudget_getcsvbudgetstr(Category *item)
{
gchar *retval = NULL;
char buf[G_ASCII_DTOSTR_BUF_SIZE];

	//DB( g_print(" get budgetstr for '%s'\n", item->name) );

	if( !(item->flags & GF_CUSTOM) )
	{
		if( item->budget[0] )
		{
			g_ascii_dtostr (buf, sizeof (buf), item->budget[0]);
			retval = g_strdup(buf);

			//DB( g_print(" => %d: %s\n", 0, retval) );
		}
	}
	else
	{
	gint i;

		for(i=1;i<13;i++)
		{
			if( item->budget[i] )
			{
			gchar *tmp = retval;

				g_ascii_dtostr (buf, sizeof (buf), item->budget[i]);

				if(retval != NULL)
				{
					retval = g_strconcat(retval, ";", buf, NULL);
					g_free(tmp);
				}
				else
					retval = g_strdup(buf);

				//DB( g_print(" => %d: %s\n", i, retval) );

			}
		}
	}

	return retval;
}



//todo amiga/linux
static void defbudget_load_csv( GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gchar *filename;
GIOChannel *io;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");


	DB( g_printf("(defbudget) load csv - data %x\n", data) );

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		GtkTreeModel *model;
		GtkTreeIter iter, newiter;
		gboolean error = FALSE;
		gchar *tmpstr, *lastcatname;
		gint io_stat;

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

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

						hb_string_strip_crlf(tmpstr);

						str_array = g_strsplit (tmpstr, ";", 15);
						// type; sign; name

						if( g_strv_length (str_array) < 4 )
						{
							error = TRUE;
							break;
						}
						DB( g_print(" read %s : %s : %s\n", str_array[0], str_array[1], str_array[2]) );

						gint pos = defcategory_exists(model, str_array[0], str_array[1], str_array[2], &iter);

						DB( g_print(" pos=%d\n", pos) );

						if( pos != 0 )
						{
						gboolean budget;
						Category *tmpitem;
						gint i;

							gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
								LST_DEFCAT_DATAS, &tmpitem,
								-1);

							DB( g_print(" should alter %s\n", tmpitem->name) );

							data->change++;

							tmpitem->flags &= ~(GF_CUSTOM);
							if( *str_array[1] == '*' )
							{
								tmpitem->budget[0] = g_ascii_strtod(str_array[3], NULL);

							}
							else
							{
								tmpitem->flags |= (GF_CUSTOM);

								for(i=1;i<=12;i++)
								{
									tmpitem->budget[i] = g_ascii_strtod(str_array[2+i], NULL);
								}
							}

							// if any value,set the flag to visual indicator
							budget = FALSE;
							tmpitem->flags &= ~(GF_BUDGET);
							for(i=0;i<=12;i++)
							{
								if(tmpitem->budget[i])
								{
									budget = TRUE;
									break;
								}
							}

							if(budget == TRUE)
								tmpitem->flags |= GF_BUDGET;
						}

						g_strfreev (str_array);
					}
					g_free(tmpstr);
				}

			}
			g_io_channel_unref (io);

			if( error == TRUE )
			{
				homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The csv file must contains the exact numbers of column,\nseparated by a semi-colon, read the help for more details.")
					);
			}

		}

	}
}

/*
**
*/
static void defbudget_save_csv( GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gchar *filename;
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
GIOChannel *io;

	DB( g_print("(defbudget) save csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename) == TRUE )
	{

		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

		    while (valid)
		    {
			gchar *outstr, *outvalstr;
			Category *category;
			gchar type;

				gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, LST_DEFCAT_DATAS, &category, -1);

				if( category->name != NULL )
				{

				//level 1: category
					if( category->flags & GF_BUDGET )
					{
						type = category->flags & GF_CUSTOM ? ' ' : '*';

						outvalstr = defbudget_getcsvbudgetstr(category);
						outstr = g_strdup_printf("1;%c;%s;%s", type, category->name, outvalstr);
						DB( g_print("%s\n", outstr) );
						g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
						g_free(outstr);
						g_free(outvalstr);
					}


				//level 2: subcategory
					gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
					gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
					while(n_child > 0)
					{
						gtk_tree_model_get(GTK_TREE_MODEL(model), &child, LST_DEFCAT_DATAS, &category, -1);

						outvalstr = defbudget_getcsvbudgetstr(category);
						if( outvalstr )
						{
							outstr = g_strdup_printf("2; ;%s;%s\n", category->name, outvalstr);
							DB( g_print("%s\n", outstr) );
							g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
							g_free(outstr);
						}
						g_free(outvalstr);

						n_child--;
						gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
					}
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
void defbudget_update(GtkWidget *treeview, gpointer user_data)
{
struct defbudget_data *data;
gboolean name, custom, sensitive;
gint i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n(defbudget) update %x\n", (gint)data) );


	name = FALSE;
	if(data->cat != NULL)
		name = data->cat->name == NULL ? FALSE : TRUE;

	sensitive = name;
	gtk_widget_set_sensitive(data->CM_type[0], sensitive);
	gtk_widget_set_sensitive(data->CM_type[1], sensitive);

	gtk_widget_set_sensitive(data->BT_clear, sensitive);

#if MYDEBUG == 1
	gint toto = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0]));
	DB( g_print(" monthly = %d\n", toto) );
#endif

	custom = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[1]));
	DB( g_print(" custom = %d\n\n", custom) );

	sensitive = name == FALSE ? FALSE : custom == TRUE ? FALSE: TRUE;
	gtk_widget_set_sensitive(data->spinner[0], sensitive);

	sensitive = name == FALSE ? FALSE : custom;
	for(i=0;i<12;i++)
	{
		gtk_widget_set_sensitive(data->spinner[i+1], sensitive);
	}


}

/*
**
*/
void defbudget_clear(GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gint i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) clear\n") );

	//g_signal_handler_block(data->CM_type[0], data->handler_id[HID_CUSTOM]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_type[0]), TRUE);
	//g_signal_handler_unblock(data->CM_type[0], data->handler_id[HID_CUSTOM]);

	for(i=0;i<=12;i++)
	{
		g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), 0);
		data->cat->budget[i] = 0;

		g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
	}

	data->cat->flags &= ~(GF_BUDGET);

}


/*
**
*/
void defbudget_set(GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gint active;
gint i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) set\n") );

	active = data->cat->flags & GF_CUSTOM ? 1 : 0;
	//data->custom = active;

	//DB( g_print(" set custom to %d\n", data->custom) );

	g_signal_handler_block(data->CM_type[0], data->handler_id[HID_CUSTOM]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_type[active]), TRUE);
	g_signal_handler_unblock(data->CM_type[0], data->handler_id[HID_CUSTOM]);

	for(i=0;i<=12;i++)
	{
		g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), data->cat->budget[i]);
		//DB( g_print(" %.2f\n", data->cat->budget[i]) );

		g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
	}

}

/*
**
*/
void defbudget_get(GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gboolean budget;
gint i;
gint field = GPOINTER_TO_INT(user_data);

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) get %d\n", field) );

	data->change++;

	if( field == FIELD_TYPE )
	{
		data->cat->flags &= ~(GF_CUSTOM);
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0])) == FALSE) data->cat->flags |= GF_CUSTOM;
		DB( g_print(" set custom to %d\n", gtk_toggle_button_get_active(data->CM_type[1])) );
	}
	else
	{
		data->cat->budget[field] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->spinner[field]));
	}

	// if any value,set the flag to visual indicator
	budget = FALSE;
	data->cat->flags &= ~(GF_BUDGET);
	for(i=0;i<=12;i++)
	{
		if(data->cat->budget[i])
		{
			budget = TRUE;
			break;
		}
	}

	if(budget == TRUE)
		data->cat->flags |= GF_BUDGET;

}




/*
**
*/
void defbudget_selection_change(GtkWidget *treeview, gpointer user_data)
{
struct defbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) changed\n") );

	//list
GtkTreeModel *model;
GtkTreeIter iter;

	data->cat = NULL;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DEFCAT_DATAS, &item,
			-1);

		DB( g_print(" selected %s\n", item->name) );

		data->cat = item;
		defbudget_set(treeview, NULL);
	}

	defbudget_update(treeview, NULL);

}

void defbudget_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{

struct defbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) toggle\n") );

	defbudget_get(GTK_WIDGET(radiobutton), (gpointer)FIELD_TYPE);

	//data->custom ^= 1;
	defbudget_update(GTK_WIDGET(radiobutton), NULL);
}

/*
**
*/
void defbudget_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defbudget_selection_change(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

/*
**
*/
gboolean defbudget_cleanup(struct defbudget_data *data, gint result)
{
gboolean doupdate = FALSE;
GList *s_list, *d_list;
gint i;

	DB( g_print("(defbudget) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		//do_application_specific_something ();
		DB( g_print(" accept\n") );

		s_list = g_list_first(data->tmp_list);
		d_list = g_list_first(GLOBALS->cat_list);
		while (s_list != NULL)
		{
		Category *s_item = s_list->data;
		Category *d_item = d_list->data;

			DB( g_printf(" slist %d, %s\n", s_item->key, s_item->name) );
			DB( g_printf(" dlist %d, %s\n", d_item->key, d_item->name) );

		/* copy flags & budget */
			d_item->flags = s_item->flags;
			for(i=0;i<13;i++)
			{
				d_item->budget[i] = s_item->budget[i];
			}

			s_list = g_list_next(s_list);
			d_list = g_list_next(d_list);
		}

		GLOBALS->change += data->change;

	}

	DB( g_print(" free tmp_list\n") );

	da_category_destroy(data->tmp_list);


	return doupdate;
}


/*
**
*/
void defbudget_setup(struct defbudget_data *data)
{
GList *list;

	DB( g_print("(defbudget) setup\n") );

	data->tmp_list = NULL;
	data->change = 0;
	data->cat = NULL;

	/* first: clone our cat glist, its simpler later ! */
	list = g_list_first(GLOBALS->cat_list);
	while (list != NULL)
	{
	Category *newitem = da_category_clone(list->data);

		data->tmp_list = g_list_append(data->tmp_list, newitem);

		//DB( g_printf(" clone_list (%d): %08x -> %08x\n", datalength, list->data, ptr) );

		list = g_list_next(list);
	}

	populate_view_cat(data->LV_cat, data->tmp_list, FALSE);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
}



// the window creation
GtkWidget *create_defbudget_window (void)
{
struct defbudget_data data;
GtkWidget *window, *hbox, *bbox, *mainbox, *treeview, *scrollwin, *vbox, *radio, *table, *label, *button, *separator;
GtkWidget *spinner, *apply;
GtkWidget *alignment;
guint i, row;

	memset(&data, 0, sizeof(struct defbudget_data));

      window = gtk_dialog_new_with_buttons (_("Edit Budget"),
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
	
	gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/budget.svg", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(defbudget) window=%08lx, inst_data=%08lx\n", window, &data) );


	//window contents
	mainbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	//gtk_container_set_border_width (GTK_CONTAINER (hbox), 50);
	gtk_box_pack_start (GTK_BOX (mainbox), vbox, TRUE, TRUE, 0);

	//listview
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);
 	treeview = (GtkWidget *)defbudget_list_new();
 	data.LV_cat = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	// clear button
	data.BT_clear = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_clear, FALSE, FALSE, 0);


	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), vbox, TRUE, TRUE, 0);

// parameters
	table = gtk_table_new (12, 5, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.5, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_container_add (GTK_CONTAINER (vbox), alignment);

    //gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Budget for each month</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 5, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	radio = gtk_radio_button_new_with_label (NULL, _("is the same"));
	data.CM_type[0] = radio;
 	gtk_table_attach_defaults (GTK_TABLE (table), radio, 1, 5, row, row+1);

	row++;
    //label = make_label(_("Each"), 1.0, 0.5);
	//gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, row, row+1);
	spinner = make_amount(label);
	data.spinner[0] = spinner;
	gtk_table_attach_defaults (GTK_TABLE (table), spinner, 2, 3, row, row+1);

	// propagate button
 	/*row++;
	button = gtk_button_new_with_label(_("Propagate"));
	gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, row, row+1);
	*/

 	row++;
    radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (radio), _("is different"));
	data.CM_type[1] = radio;
	gtk_table_attach_defaults (GTK_TABLE (table), radio, 1, 5, row, row+1);

	row++;
	for(i=0;i<12;i++)
	{
	gint col;

		col = ((i<6) ? 1 : 3);
		row = 4 + ((i<6) ? i : i-6);
		//col = 0;
		//row = 5 + i;

		label = make_label(months[i], 0, 0.5);
		gtk_table_attach_defaults (GTK_TABLE (table), label, col, col+1, row, row+1);

		spinner = make_amount(label);
		data.spinner[i+1] = spinner;
		data.spinner_hid[i+1] = g_signal_connect (spinner, "value-changed", G_CALLBACK (defbudget_get), (gpointer)i+1);
		gtk_table_attach_defaults (GTK_TABLE (table), spinner, col+1, col+2, row, row+1);

		DB( g_print("(defbudget) %s, col=%d, row=%d", months[i], col, row) );
	}

	// button box
	bbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), bbox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (bbox), HB_BOX_SPACING);

	data.BT_import = gtk_button_new_with_mnemonic(_("_Import"));
	//data.BT_import = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_box_pack_start (GTK_BOX (bbox), data.BT_import, FALSE, FALSE, 0);

	data.BT_export = gtk_button_new_with_mnemonic(_("E_xport"));
	//data.BT_export = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_box_pack_start (GTK_BOX (bbox), data.BT_export, FALSE, FALSE, 0);


	//connect all our signals
    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cat)), "changed", G_CALLBACK (defbudget_selection), NULL);
	//g_signal_connect (GTK_TREE_VIEW(data.LV_cat), "row-activated", G_CALLBACK (defbudget_onRowActivated), NULL);

	data.handler_id[HID_CUSTOM] = g_signal_connect (data.CM_type[0], "toggled", G_CALLBACK (defbudget_toggle), NULL);


	g_signal_connect (G_OBJECT (data.BT_clear), "clicked", G_CALLBACK (defbudget_clear), NULL);

	g_signal_connect (G_OBJECT (data.BT_import), "clicked", G_CALLBACK (defbudget_load_csv), NULL);
	g_signal_connect (G_OBJECT (data.BT_export), "clicked", G_CALLBACK (defbudget_save_csv), NULL);

	// modify events
	//data.handler_id[FIELD_INITIAL]
	for(i=0;i<13;i++)
	{
		data.spinner_hid[i] = g_signal_connect (data.spinner[i], "value-changed", G_CALLBACK (defbudget_get), (gpointer)i);
	}

	//data.custom = FALSE;
	//gtk_widget_set_sensitive(data.table, FALSE);

	//setup, init and show window
	defbudget_setup(&data);
	defbudget_update(window, NULL);



	gtk_widget_show_all (window);

	//result
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
	defbudget_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}
