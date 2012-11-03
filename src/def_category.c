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

#include "def_lists.h"
#include "def_category.h"

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

struct defcategory_data
{
	GList	*tmp_list;
	gint	change;

	GtkWidget	*window;

	GtkWidget	*LV_cat;
	GtkWidget	*ST_name1, *ST_name2;

	GtkWidget	*BT_add1, *BT_add2;

	GtkWidget	*CM_type;

	//GtkWidget	*BT_mov;
	GtkWidget	*BT_mod;
	GtkWidget	*BT_rem;

	GtkWidget	*BT_import, *BT_export;

	GtkWidget	*LA_category;

};

//todo amiga/linux
//add exist function + check before add
//save
//load

void defcategory_update(GtkWidget *treeview, gpointer user_data);
void defcategory_add(GtkWidget *widget, gpointer user_data);
void defcategory_modify(GtkWidget *widget, gpointer user_data);
void defcategory_remove(GtkWidget *widget, gpointer user_data);
void defcategory_selection(GtkTreeSelection *treeselection, gpointer user_data);

static void defcategory_load_csv( GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;
gchar *filename = NULL;
GIOChannel *io;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");


	DB( g_printf("(defcategory) load csv - data %x\n", data) );

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		GtkTreeModel *model;
		GtkTreeIter iter, newiter;
		gboolean error = FALSE;
		gchar *tmpstr, *lastcatname = "";
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

						str_array = g_strsplit (tmpstr, ";", 3);
						// type; sign; name

						if( g_strv_length (str_array) != 3 )
						{
							error = TRUE;
							break;
						}
						DB( g_print(" read %s : %s : %s\n", str_array[0], str_array[1], str_array[2]) );

						gint pos = defcategory_exists(model, str_array[0], str_array[1], str_array[2], &iter);



						//insert if necessary

						if( pos == 0 )
						{
						Category *item;
						GtkTreeIter *parent = NULL;

							DB( g_print(" pos=%d, should insert %s\n", pos, str_array[2]) );


							//debug
							/*
							GtkTreePath *path;
							gchar *tree_path_str;

							path = gtk_tree_model_get_path(model, &iter);
							tree_path_str = gtk_tree_path_to_string(path);
							DB( g_print(" => pos=%d path=%s, depth=%d\n",
								pos,
								tree_path_str,
								gtk_tree_path_get_depth(path)
								) );
							g_free(tree_path_str);
							*/


							item = da_category_malloc();
							item->name = g_strdup(str_array[2]);

							if( *str_array[0] == '1' )
							{
								lastcatname = item->name;

								item->key = 0;
								item->parent = 0;
								parent = NULL;

								if( *str_array[1] == '+' )
									item->flags |= GF_INCOME;
							}
							else if( *str_array[0] == '2' )
							{
							Category *tmpitem;

								defcategory_exists(model, "1", "", lastcatname, &iter);

								gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
									LST_DEFCAT_DATAS, &tmpitem,
									-1);

								item->parent = tmpitem->key;
								item->flags |= (tmpitem->flags & GF_INCOME);
								item->flags |= GF_SUB;

								parent = &iter;
							}

							data->change++;

							//insert category
							gtk_tree_store_append(GTK_TREE_STORE(model), &newiter, parent);
							gtk_tree_store_set(GTK_TREE_STORE(model), &newiter,
								LST_DEFCAT_DATAS, item,	//data struct
								LST_DEFCAT_OLDPOS, 0,		//oldpos
								-1);

						}

						//defpayee_real_add(data, tmpstr);

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
					_("The csv file must contains the exact numbers of column,\nseparated by a semi-colon, please see the help for more details.")
					);
			}

		}

		g_free( filename );

	}
}

/*
**
*/
static void defcategory_save_csv( GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;
gchar *filename = NULL;
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
GIOChannel *io;

	DB( g_print("(defcategory) save csv\n") );

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
			gchar *outstr;
			Category *category;
			gchar type;

				gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, LST_DEFCAT_DATAS, &category, -1);

				if( category->name != NULL )
				{

				//level 1: category
					type = (category->flags & GF_INCOME) ? '+' : '-';
					outstr = g_strdup_printf("1;%c;%s\n", type, category->name);
					DB( g_print("%s", outstr) );
					g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
					g_free(outstr);
				//level 2: subcategory
					gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
					gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
					while(n_child > 0)
					{
						gtk_tree_model_get(GTK_TREE_MODEL(model), &child, LST_DEFCAT_DATAS, &category, -1);

						outstr = g_strdup_printf("2; ;%s\n", category->name);
						DB( g_print("%s", outstr) );
						g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
						g_free(outstr);

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

gint defcategory_exists (GtkTreeModel *model, gchar *level, gchar *type, gchar *name, GtkTreeIter *return_iter)
{
GtkTreeIter  iter, child;
gboolean     valid;
Category *entry;
gint pos = 0;

    g_return_val_if_fail ( model != NULL, FALSE );

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



/*
** return an array of pointer of each Category in the treeview
*/
/*
gint *defcategory_get_array(struct defcategory_data *data, gint *count_store)
{
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
gint i, count;
gint *array;

	DB( g_print("\n(defcategory) get array\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

	// first count the items
	count = 0;
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		count += gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
		count++;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	*count_store = count;

	DB( g_print(" counted %d\n", count) );

	array = g_malloc(count * sizeof(array));
	if(array != NULL)
	{

		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Category *item;
			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFCAT_DATAS, &item,
				-1);

			array[i] = item;
			DB( g_print(" %d %s\n", i, item->name) );

			gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get(GTK_TREE_MODEL(model), &child,
					LST_DEFCAT_DATAS, &item,
					-1);

				array[i] = item;
				DB( g_print(" + %d %s\n", i, item->name) );

				n_child--;
				gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
			}
			 // Make iter point to the next row in the list store
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}
	}

	return(array);
}
*/

void defcategory_update(GtkWidget *treeview, gpointer user_data)
{
struct defcategory_data *data;
gint count;
gboolean selected, sensitive;
GtkTreeSelection *selection;
GtkTreeModel     *model;
GtkTreeIter       iter;
GtkTreePath *path;
gchar *category;

	DB( g_print("\n(defcategory) cursor changed\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_print("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter);

	//path 0 active ?
	gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)), &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter))
	{
		DB( g_print(" 0 active = %d\n", 1) );
		selected = FALSE;
	}


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	count = gtk_tree_selection_count_selected_rows(selection);
	DB( g_print(" => select count=%d\n", count) );

	category = NULL;
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	gchar *tree_path_str;
	Category *item;
	gint oldpos;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DEFCAT_DATAS, &item,
			LST_DEFCAT_OLDPOS, &oldpos,
			-1);

		path = gtk_tree_model_get_path(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)), &iter);
		tree_path_str = gtk_tree_path_to_string(path);
		DB( g_print(" => select is=%s, depth=%d (id=%d, %s, oldpos=%d)\n",
			tree_path_str,
			gtk_tree_path_get_depth(path),
			item->key,
			item->name,
			oldpos
			) );
		g_free(tree_path_str);

		//get parent if subcategory selectd
		DB( g_print(" => get parent for title\n") );
		if(gtk_tree_path_get_depth(path) != 1)
			gtk_tree_path_up(path);

		if(gtk_tree_model_get_iter(model, &iter, path))
		{
		Category *tmpitem;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFCAT_DATAS, &tmpitem,
				-1);

			category = tmpitem->name;


			DB( g_print(" => parent is %s\n", category) );

		}

		gtk_tree_path_free(path);

	}

	gtk_label_set_text(GTK_LABEL(data->LA_category), category);

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name2, sensitive);
	gtk_widget_set_sensitive(data->BT_add2, sensitive);
	//gtk_widget_set_sensitive(data->BT_mov, sensitive);
	gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);
}







/*
** add an empty new category/subcategory to our temp GList and treeview
*/
void defcategory_add(GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;

GtkTreeModel *model;
GtkTreeIter  iter, newiter, *parent = NULL;
GtkTreePath *path;
const gchar *name;
gboolean subcat = (gboolean)user_data;
GtkWidget *tmpwidget;

Category *item;
gboolean type;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) add (data=%x) subcat=%d\n", data, subcat) );

	tmpwidget = (subcat == FALSE ? data->ST_name1 : data->ST_name2);
	name = gtk_entry_get_text(GTK_ENTRY(tmpwidget));
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

	/* ignore if item is empty */
	if (name && *name)
	{
		data->change++;

		item = da_category_malloc();
		item->name = g_strdup(name);

		/* if cat use new id */
		if(subcat == FALSE)
		{
			item->key = 0;
			item->parent = 0;
			parent = NULL;

			type = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type));
			if(type == TRUE)
				item->flags |= GF_INCOME;

			DB( g_print(" => add cat: %x %d, %s type=%d\n", item, subcat, item->name, type) );
		}
		/* if subcat use parent id & gf_income */
		else
		{
			//get the active iter
			if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter))
			{
			gchar *tree_path_str;

				path = gtk_tree_model_get_path(model, &iter);
				tree_path_str = gtk_tree_path_to_string(path);
				DB( g_print(" => path is %s\n", tree_path_str) );
				g_free(tree_path_str);

				//get parent if subcategory selectd
				if(gtk_tree_path_get_depth(path) != 1)
				{
					gtk_tree_path_up(path);
					tree_path_str = gtk_tree_path_to_string(path);
					DB( g_print(" => parent to path %s\n", tree_path_str) );
					g_free(tree_path_str);
				}

				if(gtk_tree_model_get_iter(model, &iter, path))
				{
				Category *tmpitem;

					gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
						LST_DEFCAT_DATAS, &tmpitem,
						-1);

					item->parent = tmpitem->key;
					item->flags |= (tmpitem->flags & GF_INCOME);
					item->flags |= GF_SUB;

					parent = &iter;

					DB( g_print(" => add subcat: %x %s\n", item, item->name) );

				}

				gtk_tree_path_free(path);
			}
		}

		DB( g_print(" => add %s\n", item->name) );

		//add our item pointer to the glist
		//data->tmp_list = g_list_append(data->tmp_list, item);

		//insert category
		gtk_tree_store_append(GTK_TREE_STORE(model), &newiter, parent);
		gtk_tree_store_set(GTK_TREE_STORE(model), &newiter,
			LST_DEFCAT_DATAS, item,	//data struct
			LST_DEFCAT_OLDPOS, 0,		//oldpos
			-1);

		path = gtk_tree_model_get_path(model, &newiter);
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW(data->LV_cat),path);
		gtk_tree_path_free(path);
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &newiter);

		// empty our widget
		gtk_entry_set_text(GTK_ENTRY(tmpwidget),"");
	}
}

/*
**
*/
void defcategory_modify(GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;
GtkWidget *window, *mainvbox, *w_name, *w_type = NULL;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter, child_iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) modify\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

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
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_MAINBOX_SPACING);

		w_name = gtk_entry_new();
		gtk_box_pack_start (GTK_BOX (mainvbox), w_name, TRUE, TRUE, 0);

		gtk_entry_set_text(GTK_ENTRY(w_name), item->name);
		gtk_widget_grab_focus (w_name);


		if(!(item->flags & GF_SUB))
		{
			w_type = gtk_check_button_new_with_mnemonic(_("_Income"));
			gtk_box_pack_start (GTK_BOX (mainvbox), w_type, TRUE, TRUE, 0);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_type), item->flags & GF_INCOME ? TRUE : FALSE);
		}

		gtk_widget_show_all(mainvbox);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			name = gtk_entry_get_text(GTK_ENTRY(w_name));
			/* ignore if item is empty */
			if (name && *name)
			{
				data->change++;

				g_free(item->name);
				item->name = g_strdup(name);

			// change the item flags
				if(!(item->flags & GF_SUB))
				{
				gboolean type;
				gboolean haschild;
				gint n_child;

					item->flags &= (~GF_INCOME);
					type = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_type));
					if(type == TRUE)
						item->flags |= GF_INCOME;

					//todo: modify subcat gf_income flag
					haschild = gtk_tree_model_iter_has_child(model, &iter);
					DB( g_print(" + should update child gf_income flag\n") );

					n_child = gtk_tree_model_iter_n_children(model, &iter);
					DB( g_print(" %d childs to change\n", n_child) );

					/* change our childs */
					gtk_tree_model_iter_children(model, &child_iter, &iter);
					while(n_child > 0)
					{
						data->change++;

						gtk_tree_model_get(model, &child_iter, LST_DEFCAT_DATAS, &item, -1);

						item->flags &= (~GF_INCOME);
						if(type == TRUE)
							item->flags |= GF_INCOME;

						gtk_tree_model_iter_next(model, &child_iter);
						n_child--;
					}
				}


			//hack to do a sort
				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), LST_DEFCAT_DATAS, GTK_SORT_DESCENDING);
				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);
			}
	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}

/*
**
*/
/*
void defcategory_move(GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;
GtkWidget *window, *mainvbox, *getwidget;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defcategory) move\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

		window = gtk_dialog_new_with_buttons ("Move...",
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
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), SP_BORDER);

		getwidget = make_popcategory(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);

		// populate popcateory
		Category **array;
		gint count, i;

		gtk_widget_show_all(mainvbox);

		//gtk_widget_grab_focus (getwidget);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gint active;

			active = gtk_combo_box_get_active(getwidget);

			DB( g_print(" moving to %d\n", active) );



	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}
*/

/*
** remove the selected payee to our treeview and temp GList
*/
void defcategory_remove(GtkWidget *widget, gpointer user_data)
{
struct defcategory_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
GtkTreeIter			 child_iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) remove (data=%x)\n", (guint)data) );


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;
	gboolean haschild;

		/* check for chil ? */
		haschild = gtk_tree_model_iter_has_child(model, &iter);
		DB( g_print(" haschild %d\n", haschild) );

		/* if no child */
		if(haschild == FALSE)
		{
			data->change++;

			gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
			//DB( g_print(" remove single %s (pos=%d)\n", item->grp_Name, g_list_index(data->tmp_list, item) ) );
			//data->tmp_list = g_list_remove(data->tmp_list, item);
		}
		else
		{
		gint n_child;

			n_child = gtk_tree_model_iter_n_children(model, &iter);
			DB( g_print(" %d childs to remove\n", n_child) );

			/* remove our root item ptr */
			gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);
			//data->tmp_list = g_list_remove(data->tmp_list, item);


			/* remove our childs */
			gtk_tree_model_iter_children(model, &child_iter, &iter);
			while(n_child > 0)
			{
				data->change++;

				gtk_tree_model_get(model, &child_iter, LST_DEFCAT_DATAS, &item, -1);
				//DB( g_print(" remove child %s (pos=%d)\n", item->grp_Name, g_list_index(data->tmp_list, item) ) );
				//data->tmp_list = g_list_remove(data->tmp_list, item);

				gtk_tree_model_iter_next(model, &child_iter);
				n_child--;
			}

			DB( g_print(" remove root %s\n", item->name) );
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		}
	}
}


/*
**
*/
void defcategory_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defcategory_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

void defcategory_onRowActivated (GtkTreeView        *treeview,
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
		defcategory_modify(GTK_WIDGET(treeview), NULL);
	}
}




/*
**
*/
gboolean defcategory_cleanup(struct defcategory_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(defcategory) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
	GtkTreeModel *model;
	GtkTreeIter	iter, child;
	gboolean valid;
	gushort i, lastparent;
	guint *pos_vector;

		//do_application_specific_something ();
		DB( g_print(" accept\n") );

		//allocate vector pos
		DB( g_print(" allocate %d guint\n", g_list_length(GLOBALS->cat_list)) );

		pos_vector = g_new0(guint, g_list_length(GLOBALS->cat_list));
		if(pos_vector)
		{

			// test for change & store new position
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Category *item;
			gint oldpos;

				gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
					LST_DEFCAT_DATAS, &item,
					LST_DEFCAT_OLDPOS, &oldpos,
					-1);

				item->key = i;
				lastparent = i;
				item->parent = 0;
				data->tmp_list = g_list_append(data->tmp_list, item);

				if(oldpos != 0)	// added cat have 0 has id, parent and oldpos
				{
					pos_vector[oldpos] = i;
					if(doupdate == FALSE && oldpos != i) doupdate = TRUE;
				}

				DB( g_print(" %2d : %s (%d->%d) %s\n", i, item->name, oldpos, i, doupdate==TRUE?"changed":"") );

				gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
				gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
				while(n_child > 0)
				{
					i++;

					gtk_tree_model_get(GTK_TREE_MODEL(model), &child,
						LST_DEFCAT_DATAS, &item,
						LST_DEFCAT_OLDPOS, &oldpos,
						-1);

				if(oldpos != 0)	// added cat have 0 as id, parent and oldpos
				{
					pos_vector[oldpos] = i;
					if(doupdate == FALSE && oldpos != i) doupdate = TRUE;
				}

					DB( g_print(" %2d : +%s (%d->%d) %s\n", i, item->name, oldpos, i, doupdate==TRUE?"changed":"") );

					item->key = i;
					item->parent = lastparent;
					data->tmp_list = g_list_append(data->tmp_list, item);

					n_child--;
					gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
				}
				 /* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

		#if MYDEBUG == 1
			DB( g_print("vector change:\n") );
			for(i=0;i<g_list_length(GLOBALS->cat_list);i++)
			{
				g_print(" %d => %d\n", i, pos_vector[i]);
			}
		#endif

			if(doupdate == TRUE)
			{
			GList *list;
			gint tmpval;

				/* ccgrp */
				tmpval = GLOBALS->car_category;
				GLOBALS->car_category = pos_vector[tmpval];

				/* -> all archive category */
				list = g_list_first(GLOBALS->arc_list);
				while (list != NULL)
				{
				Archive *item = list->data;

					tmpval = item->category;
					item->category = pos_vector[tmpval];

					list = g_list_next(list);
				}

				/* -> all operations category */
				list = g_list_first(GLOBALS->ope_list);
				while (list != NULL)
				{
				Operation *item = list->data;

					tmpval = item->category;
					item->category = pos_vector[tmpval];

					list = g_list_next(list);
				}

			}

			//update our GLOBAL cat_list
			da_category_destroy(GLOBALS->cat_list);
			GLOBALS->cat_list = data->tmp_list;
			//GLOBALS->cat_list = g_list_sort(data->tmp_list, (GCompareFunc)defcategory_list_sort);
			data->tmp_list = NULL;
		}

		g_free(pos_vector);

		GLOBALS->change += data->change;
	}

	DB( g_print(" free tmp_list\n") );

	da_category_destroy(data->tmp_list);

	return doupdate;
}

/*
**
*/
void defcategory_setup(struct defcategory_data *data)
{
	DB( g_print("(defcategory) setup\n") );

	//init GList
	data->tmp_list = NULL; //data->tmp_list = hb_glist_clone_list(GLOBALS->cat_list, sizeof(struct _Group));
	data->change = 0;

	populate_view_cat(data->LV_cat, GLOBALS->cat_list, TRUE);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));

}

// the window creation
GtkWidget *create_defcategory_window (void)
{
struct defcategory_data data;
GtkWidget *window, *mainvbox, *table, *hbox, *label, *scrollwin, *vbox, *separator, *treeview;
gint row;

      window = gtk_dialog_new_with_buttons (_("Edit Categories"),
					    //GTK_WINDOW (do_GtkWidget),
					    NULL,
					    0,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);
	

	homebank_window_set_icon_from_file(GTK_WINDOW (window), "category.svg");

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(defcategory) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	//window contents
	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

    //our table
	table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	// category item + add button
	row = 0;
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.ST_name1 = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), data.ST_name1, TRUE, TRUE, 0);
	data.CM_type = gtk_check_button_new_with_mnemonic(_("I_ncome"));
	gtk_box_pack_start (GTK_BOX (hbox), data.CM_type, FALSE, FALSE, 0);

	data.BT_add1 = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add1, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	// subcategory + add button
	row++;
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.LA_category = gtk_label_new(NULL);
	gtk_box_pack_start (GTK_BOX (hbox), data.LA_category, FALSE, FALSE, 0);
	label = gtk_label_new(":");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data.ST_name2 = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), data.ST_name2, TRUE, TRUE, 0);
	data.BT_add2 = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add2, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);




	//list
	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_table_attach (GTK_TABLE (table), scrollwin, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 	treeview = (GtkWidget *)defcategory_list_new(FALSE);
 	data.LV_cat = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), vbox, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	/*
	widget = gtk_check_button_new_with_mnemonic("Income type");
	data.CM_type = widget;
	gtk_box_pack_start (GTK_BOX (vbox), data.CM_type, FALSE, FALSE, 0);
	*/

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
	g_signal_connect (G_OBJECT (data.ST_name1), "activate", G_CALLBACK (defcategory_add), (gpointer)FALSE);
	g_signal_connect (G_OBJECT (data.ST_name2), "activate", G_CALLBACK (defcategory_add), (gpointer)TRUE);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cat)), "changed", G_CALLBACK (defcategory_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_cat), "row-activated", G_CALLBACK (defcategory_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_add1), "clicked", G_CALLBACK (defcategory_add), (gpointer)FALSE);
	g_signal_connect (G_OBJECT (data.BT_add2), "clicked", G_CALLBACK (defcategory_add), (gpointer)TRUE);
	g_signal_connect (G_OBJECT (data.BT_mod), "clicked", G_CALLBACK (defcategory_modify), NULL);
	//g_signal_connect (G_OBJECT (data.BT_mov), "clicked", G_CALLBACK (defcategory_move), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (defcategory_remove), NULL);

	g_signal_connect (G_OBJECT (data.BT_import), "clicked", G_CALLBACK (defcategory_load_csv), NULL);
	g_signal_connect (G_OBJECT (data.BT_export), "clicked", G_CALLBACK (defcategory_save_csv), NULL);

	//setup, init and show window
	defcategory_setup(&data);
	defcategory_update(data.LV_cat, NULL);

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
	defcategory_cleanup(&data, result);
	gtk_widget_destroy (window);

  return NULL;
}
