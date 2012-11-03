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

#include "ui_category.h"
#include "def_budget.h"
#include "ui_category.h"
#include "import.h"

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

static gchar *defbudget_getcsvbudgetstr(Category *item);
static void defbudget_update(GtkWidget *treeview, gpointer user_data);
static void defbudget_set(GtkWidget *widget, gpointer user_data);
static void defbudget_getlast(struct defbudget_data *data);
static void defbudget_selection_change(GtkWidget *treeview, gpointer user_data);
static void defbudget_toggle(GtkRadioButton *radiobutton, gpointer user_data);
static void defbudget_selection(GtkTreeSelection *treeselection, gpointer user_data);

GtkWidget *defbudget_list_new(void);

/*
** index 0 is all month, then 1 -> 12 are months
*/
static gchar *defbudget_getcsvbudgetstr(Category *item)
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

		for(i=1;i<=12;i++)
		{
			//if( item->budget[i] )
			//{
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

			//}
		}
	}

	return retval;
}


static gint defbudget_category_exists (GtkTreeModel *model, gchar *level, gchar *type, gchar *name, GtkTreeIter *return_iter)
{
GtkTreeIter  iter, child;
gboolean     valid;
Category *entry;
gint pos = 0;

    if(model == NULL)
		return 0;

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

    while (valid)
    {
		gtk_tree_model_get (model, &iter, LST_DEFCAT_DATAS, &entry, -1);

		if(*level == '1')
		{
			if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
			{
				*return_iter = iter;
				return pos;
			}
		}
		else
		{
			if(*level == '2')
			{
				gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
				gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
				while(n_child > 0)
				{

					gtk_tree_model_get(GTK_TREE_MODEL(model), &child,
						LST_DEFCAT_DATAS, &entry,
						-1);

					if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
					{
						*return_iter = child;
						return pos;
					}

					n_child--;
					gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
					pos++;
				}
			}
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		pos++;
    }

	return 0;
}


static void defbudget_load_csv( GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gchar *filename = NULL;
GIOChannel *io;
const gchar *encoding;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");


	DB( g_printf("(defbudget) load csv - data %x\n", data) );

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		encoding = homebank_file_getencoding(filename);

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		GtkTreeModel *model;
		GtkTreeIter iter;
		gboolean error = FALSE;
		gchar *tmpstr;
		gint io_stat;

			DB( g_print(" -> encoding should be %s\n", encoding) );
			if( encoding != NULL )
			{
				g_io_channel_set_encoding(io, encoding, NULL);
			}
			
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

			
			for(;;)
			{
				io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
				if( io_stat == G_IO_STATUS_EOF)
					break;
				if( io_stat == G_IO_STATUS_NORMAL)
				{
					if( tmpstr != NULL)
					{
					gchar **str_array;

						hb_string_strip_crlf(tmpstr);

						str_array = g_strsplit (tmpstr, ";", 15);
						// type; sign; name

						if( (g_strv_length (str_array) < 4 || *str_array[1] != '*') && (g_strv_length (str_array) < 15))
						{
							error = TRUE;
							break;
						}
						
						DB( g_print(" csv read '%s : %s : %s ...'\n", str_array[0], str_array[1], str_array[2]) );

						gint pos = defbudget_category_exists(model, str_array[0], str_array[1], str_array[2], &iter);

						DB( g_print(" pos=%d\n", pos) );

						if( pos != 0 )
						{
						gboolean budget;
						Category *tmpitem;
						gint i;

							gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
								LST_DEFCAT_DATAS, &tmpitem,
								-1);

							DB( g_print(" found cat, updating '%s'\n", tmpitem->name) );

							data->change++;

							tmpitem->flags &= ~(GF_CUSTOM);
							if( *str_array[1] == '*' )
							{
								tmpitem->budget[0] = g_ascii_strtod(str_array[3], NULL);

								DB( g_print(" monthly '%.2f'\n", tmpitem->budget[0]) );
							}
							else
							{
								tmpitem->flags |= (GF_CUSTOM);

								for(i=1;i<=12;i++)
								{
									tmpitem->budget[i] = g_ascii_strtod(str_array[2+i], NULL);
									DB( g_print(" month %d '%.2f'\n", i, tmpitem->budget[i]) );
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

			//update the treeview
			gtk_tree_selection_unselect_all (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)));

			
			g_io_channel_unref (io);

			if( error == TRUE )
			{
				homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The csv file must contains the exact numbers of column,\nseparated by a semi-colon, read the help for more details.")
					);
			}

		}

		g_free( filename );

	}
}

/*
**
*/
static void defbudget_save_csv( GtkWidget *widget, gpointer user_data)
{
struct defbudget_data *data;
gchar *filename = NULL;
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
GIOChannel *io;

	DB( g_print("(defbudget) save csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
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
						type = (category->flags & GF_CUSTOM) ? ' ' : '*';

						outvalstr = defbudget_getcsvbudgetstr(category);
						outstr = g_strdup_printf("1;%c;%s;%s\n", type, category->name, outvalstr);
						DB( g_print("%s", outstr) );
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

						type = (category->flags & GF_CUSTOM) ? ' ' : '*';
						
						outvalstr = defbudget_getcsvbudgetstr(category);
						if( outvalstr )
						{
							outstr = g_strdup_printf("2;%c;%s;%s\n", type, category->name, outvalstr);
							DB( g_print("%s", outstr) );
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

		g_free( filename );

	}

}






/*
**
*/
static void defbudget_update(GtkWidget *treeview, gpointer user_data)
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
static void defbudget_clear(GtkWidget *widget, gpointer user_data)
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
		//g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), 0);
		data->cat->budget[i] = 0;

		//g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
	}

	data->cat->flags &= ~(GF_BUDGET);

}


/*
**
*/
static void defbudget_set(GtkWidget *widget, gpointer user_data)
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
		//g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), data->cat->budget[i]);
		//DB( g_print(" %.2f\n", data->cat->budget[i]) );

		//g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_force), (data->cat->flags & GF_FORCED) ? 1 : 0);
	
}

/*
**
*/
static void defbudget_getlast(struct defbudget_data *data)
{
gboolean budget, change;
gint i;
Category *item;
gdouble oldvalue;
	gint active;

	item = data->lastcatitem;

	DB( g_print("****\n(defbudget) getlast for '%s'\n", item->name ) );

	if( item != NULL )
	{
	gushort old_flags = item->flags;

		item->flags &= ~(GF_CUSTOM);
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0])) == FALSE)
			item->flags |= GF_CUSTOM;
		
		DB( g_print(" custom flag=%d\n", gtk_toggle_button_get_active(data->CM_type[1])) );

		// if any value,set the flag to visual indicator
		budget = FALSE;
		change = FALSE;
		item->flags &= ~(GF_BUDGET);
		for(i=0;i<=12;i++)
		{
			gtk_spin_button_update(GTK_SPIN_BUTTON(data->spinner[i]));
			oldvalue = item->budget[i];
			
			item->budget[i] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->spinner[i]));
			
			if( oldvalue != item->budget[i])
				change = TRUE;
			
			DB( g_print(" set value %d to %.2f\n", i, item->budget[i]) );
			if(item->budget[i])
			{
				budget = TRUE;
			}
		}

		item->flags &= ~(GF_FORCED);
		active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_force));
		if(active == 1)
			item->flags |= GF_FORCED;
		
		if(budget == TRUE || active == 1)
			item->flags |= GF_BUDGET;
		
		// compute chnages
		if( (old_flags != item->flags) || change )
			data->change++;
			
	}
	
}




/*
**
*/
static void defbudget_selection_change(GtkWidget *treeview, gpointer user_data)
{
struct defbudget_data *data;
GtkTreeModel *model;
GtkTreeIter iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) changed\n") );

	data->cat = NULL;
	
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DEFCAT_DATAS, &item,
			-1);

		DB( g_print(" selected %s\n", item->name) );

		if(data->lastcatitem != NULL && item != data->lastcatitem)
		{
			DB( g_print(" -> should do a get for last selected (%s)\n", data->lastcatitem->name) );
			defbudget_getlast(data);
		}


		data->cat = item;
		data->lastcatitem = item;
		
		defbudget_set(treeview, NULL);
	}
	else
	{
		data->lastcatitem = NULL;
	}

	defbudget_update(treeview, NULL);

}

static void defbudget_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{

struct defbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(defbudget) toggle\n") );

	//defbudget_get(GTK_WIDGET(radiobutton), GINT_TO_POINTER(FIELD_TYPE));

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
static gboolean defbudget_cleanup(struct defbudget_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(defbudget) cleanup\n") );


		if(data->lastcatitem != NULL)
		{
			DB( g_print(" -> should do a get for last selected (%s)\n", data->lastcatitem->name) );
			defbudget_getlast(data);
		}


		//do_application_specific_something ();
		DB( g_print(" accept\n") );

		GLOBALS->change += data->change;

	DB( g_print(" free tmp_list\n") );

	return doupdate;
}


/*
**
*/
static void defbudget_setup(struct defbudget_data *data)
{

	DB( g_print("(defbudget) setup\n") );

	data->tmp_list = NULL;
	data->change = 0;
	data->cat = NULL;
	data->lastcatitem = NULL;

	ui_cat_listview_populate(data->LV_cat);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
}



// the window creation
GtkWidget *create_defbudget_window (void)
{
struct defbudget_data data;
GtkWidget *window, *bbox, *mainbox, *treeview, *scrollwin, *vbox, *radio, *table, *label, *widget;
GtkWidget *spinner;
GtkWidget *alignment;
guint i, row;

	memset(&data, 0, sizeof(struct defbudget_data));

      window = gtk_dialog_new_with_buttons (_("Manage Budget"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;

	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "budget.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_BUDGET);

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
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
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
	alignment = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
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
		gtk_table_attach_defaults (GTK_TABLE (table), spinner, col+1, col+2, row, row+1);

		DB( g_print("(defbudget) %s, col=%d, row=%d", months[i], col, row) );
	}

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Force monitoring this category"));
	data.CM_force = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 5, row, row+1);

	
	// button box
	bbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	//gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
static gint defbudget_list_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Category *entry1, *entry2;
gchar *name1, *name2;

	gtk_tree_model_get(model, a, LST_DEFCAT_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFCAT_DATAS, &entry2, -1);

	result = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
	if(!result)
	{
		name1 = entry1->name;
		name2 = entry2->name;
        if (name1 == NULL || name2 == NULL)
        {
          //if (name1 == NULL && name2 == NULL)
          result = (name1 == NULL) ? -1 : 1;
        }
        else
        {
          result = g_utf8_collate(name1,name2);
        }
	}
	
    return result;
}

/*
** draw some text from the stored data structure
*/
static void defbudget_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar *name;
gchar *string;

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);

	if(entry->key == 0)
		name = g_strdup(_("(none)"));
	else
		name = entry->name;

	#if MYDEBUG
		gchar type;
		type = (entry->flags & GF_INCOME) ? '+' : '-';
		string = g_markup_printf_escaped("%s ::(f=%d, %c)", name, entry->flags, type );
	#else
		if(entry->flags & GF_BUDGET)
		{
			if( entry->parent == 0 )
				string = g_markup_printf_escaped("<b>%s</b>", name);
			else
				string = g_markup_printf_escaped("<b><i>%s</i></b>", name);
		}
		else
		{
			if( entry->parent == 0 )
				string = g_markup_printf_escaped("%s", name);
			else
				string = g_markup_printf_escaped("<i>%s</i>", name);
		}
	#endif

	g_object_set(renderer, "markup", string, NULL);

	g_free(string);
}

/*
**
*/
GtkWidget *defbudget_list_new(void)
{
GtkTreeStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_tree_store_new (
		3,
		//NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//sortable
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, defbudget_list_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defbudget_text_cell_data_function, GINT_TO_POINTER(1), NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}



