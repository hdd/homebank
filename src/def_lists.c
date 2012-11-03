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


static void
fixed_toggled (GtkCellRendererToggle *cell,
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

static void
fixed_toggled_tree (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFCAT_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

/**************************************************************************
 *
 * define account list
 *
 **************************************************************************/



/*
** draw some text from the stored data structure
*/
void
defaccount_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;
#if MYDEBUG
gchar *string;
gint oldpos;
#endif

	gtk_tree_model_get(model, iter, LST_DEFACC_DATAS, &entry, -1);

	name = entry->name;

	#if MYDEBUG
		gtk_tree_model_get(model, iter, LST_DEFACC_OLDPOS, &oldpos, -1);
		string = g_strdup_printf ("(%d->%d) %s", oldpos, entry->key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif
}

/*
** create a new defacount list
*/
GtkWidget *defaccount_list_new(gboolean dotoggle)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_list_store_new (
		NUM_LST_DEFACC,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	if( dotoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFACC_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (fixed_toggled), store);

	}

	// column 1
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defaccount_text_cell_data_function, (gpointer)1, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}

/**************************************************************************
 *
 * define payee list
 *
 **************************************************************************/

gint defpayee_list_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Payee *entry1, *entry2;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFPAY_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFPAY_DATAS, &entry2, -1);

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



/*
** draw some text from the stored data structure
*/
void
defpayee_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Payee *entry;
gchar *name;
#if MYDEBUG
gchar *string;
gint oldpos;
#endif

	gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &entry, -1);
	if(entry->name == NULL)
		name = _("(none)");
	else
		name = entry->name;

	#if MYDEBUG
		gtk_tree_model_get(model, iter, LST_DEFPAY_OLDPOS, &oldpos, -1);
		string = g_strdup_printf ("(%d->%d) %s", oldpos, entry->key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}

/*
**
*/
GtkWidget *defpayee_list_new(gboolean dotoggle)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_list_store_new (
		NUM_LST_DEFPAY,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//sortable
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, defpayee_list_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, GTK_SORT_ASCENDING);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	if( dotoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFPAY_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (fixed_toggled), store);

	}

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defpayee_text_cell_data_function, (gpointer)1, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}


/**************************************************************************
 *
 * define category list
 *
 **************************************************************************/

/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
gint defcategory_list_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
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
void defcategory_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar type;
gchar *string;

#if MYDEBUG
gint oldpos;
#endif

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);
	if(entry->name == NULL)
	{
		string = _("(none)");
		g_object_set(renderer, "text", string, NULL);
	}
	else
	{
		type = (entry->flags & GF_INCOME) ? '+' : '-';
		string = g_strdup_printf("%s [%c]", entry->name, type);
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	}

/*
	#if MYDEBUG
		gtk_tree_model_get(model, iter, LST_DEFCAT_OLDPOS, &oldpos, -1);
		type = (entry->flags & GF_INCOME) ? '+' : '-';
		string = g_strdup_printf("%s :: (k:%d, p:%d, old=%d, =%c)", entry->name, entry->key, entry->parent, oldpos, type );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif
*/
}

/*
**
*/
GtkWidget *defcategory_list_new(gboolean dotoggle)
{
GtkTreeStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_tree_store_new (
		NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//sortable
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, defcategory_list_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	if( dotoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFACC_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (fixed_toggled_tree), store);

	}

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defcategory_text_cell_data_function, (gpointer)1, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}

/**************************************************************************
 *
 * define archives list
 *
 **************************************************************************/

/*
**
*/
gint defarchive_list_sort(Archive *a, Archive *b)
{
gint result;

//	result = a->category - b->category;
//	if(result == 0)

	result = g_utf8_collate(a->wording, b->wording);

	return result;
}



/*
**
*/
void defarchive_auto_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *item;
GdkPixbuf *pixbuf = NULL;

	// get the operation
	gtk_tree_model_get(model, iter, LST_DEFARC_DATAS, &item, -1);

	if( item->flags & OF_AUTO )
		pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO];

	g_object_set(renderer, "pixbuf", pixbuf, NULL);
}


/*
** draw some text from the stored data structure
*/
void
defarchive_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Archive *item;
gchar *name;

	gtk_tree_model_get(model, iter, LST_DEFARC_DATAS, &item, -1);

	name = item->wording;

	g_object_set(renderer, "text", name, NULL);
}



/*
**
*/
GtkWidget *defarchive_list_new(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_list_store_new (
		NUM_LST_DEFARC,
		G_TYPE_POINTER,
		G_TYPE_UINT,
		G_TYPE_BOOLEAN
		);

	//sortable
	//gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, sort_iter_compare_func, GINT_TO_POINTER(LST_DEFPAY_DATAS), NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, GTK_SORT_ASCENDING);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defarchive_text_cell_data_function, (gpointer)1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column 2 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defarchive_auto_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}

/**************************************************************************
 *
 * define budget list
 *
 **************************************************************************/

/*
** draw some text from the stored data structure
*/
void defbudget_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar *name, *markuptxt;
#if MYDEBUG
gchar type;
gchar *string;
gint oldpos;
#endif

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);
	if(entry->name == NULL)
	{
		name = _("(none)");
		g_object_set(renderer, "text", name, NULL);
	}
	else
	{
		name = entry->name;

		#if MYDEBUG
			gtk_tree_model_get(model, iter, LST_DEFCAT_OLDPOS, &oldpos, -1);
			type = (entry->flags & GF_INCOME) ? '+' : '-';
			string = g_strdup_printf("%s :: (k:%d, p:%d, old=%d, =%c)", name, entry->key, entry->parent, oldpos, type );
			g_object_set(renderer, "text", string, NULL);
			g_free(string);
		#else
			if(entry->flags & GF_BUDGET)
			{
				markuptxt = g_strdup_printf("<b>%s</b>", name);
				g_object_set(renderer, "markup", markuptxt, NULL);
				g_free(markuptxt);
			}
			else
				g_object_set(renderer, "markup", name, NULL);
		#endif
	}

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
		NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//sortable
	//we use the category one
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, defcategory_list_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defbudget_text_cell_data_function, (gpointer)1, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	return(view);
}
