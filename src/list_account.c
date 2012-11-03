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
#include "preferences.h"

#include "list_account.h"

/****************************************************************************/
/* Debug macros																*/
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

extern gchar *CYA_ACC_TYPE[];	//in ui_account.c

/*
** draw some icons according to the stored data structure
*/
static void
status_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
Account *acc;
gint dt;

	gtk_tree_model_get(model, iter, 
		LST_DSPACC_DATATYPE, &dt,
		LST_DSPACC_DATAS, &acc,
		-1);

	if( dt == DSPACC_TYPE_NORMAL )
	{
	
		switch(GPOINTER_TO_INT(user_data))
		{
			case 1:
				g_object_set(renderer, "pixbuf", (acc->flags & AF_ADDED) ? GLOBALS->lst_pixbuf[LST_PIXBUF_ADD] : NULL, NULL);
				break;
			case 2:
				g_object_set(renderer, "pixbuf", (acc->flags & AF_CHANGED) ? GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT] : NULL, NULL);
				break;
		}
	}
	else
		g_object_set(renderer, "pixbuf", NULL, NULL);
}

/*
** draw some text from the stored data structure
*/
static void
acc_type_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Account *acc;
gint dt;

	gtk_tree_model_get(model, iter,
		LST_DSPACC_DATATYPE, &dt,
		LST_DSPACC_DATAS, &acc,
		-1);

	if( dt == DSPACC_TYPE_NORMAL && acc->type > 0 )
	{
		g_object_set(renderer, "text", CYA_ACC_TYPE[acc->type], NULL);
	}
	else
		g_object_set(renderer, "text", NULL, NULL);
}


/*
** draw some text from the stored data structure
*/
static void
text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *acc;
gint dt;
gchar *groupname;

	gtk_tree_model_get(model, iter,
		LST_DSPACC_DATATYPE, &dt,
		LST_DSPACC_DATAS, &acc,
		LST_DSPACC_NAME, &groupname,
		-1);

	DB( g_print("acc %x\n", acc) );


	if( dt == DSPACC_TYPE_NORMAL )
	{
		switch(GPOINTER_TO_INT(user_data))
		{
			case 1:
				g_object_set(renderer, "text", acc->name, NULL);
				break;
			case 2:
				//g_object_set(renderer, "text", acc->number, NULL);
				break;

		}
	}
	else
		g_object_set(renderer, "markup", groupname, NULL);
}

static void
float_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gdouble value;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
Account *acc;
gint dt;
gint weight;
gchar *color;

	gtk_tree_model_get(model, iter,
		LST_DSPACC_DATATYPE, &dt,
		LST_DSPACC_DATAS, &acc,
		GPOINTER_TO_INT(user_data), &value,		//LST_DSPACC_(BANK/TODAY/FUTURE)
		-1);


	if( dt == DSPACC_TYPE_HEADER )
		g_object_set(renderer, "markup", NULL, NULL);
	else
	{
		mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, GLOBALS->minor);

		color = NULL;
		weight = PANGO_WEIGHT_NORMAL;

		if( dt == DSPACC_TYPE_NORMAL )
		{
			color = get_minimum_color_amount(value, acc->minimum);
		}
		else
		{
			color = get_normal_color_amount(value);
			weight = PANGO_WEIGHT_BOLD;
		}

		g_object_set(renderer, 
			"weight", weight,
			"foreground",  color,
			"text", buf,
			NULL);

	}		
}


static GtkTreeViewColumn *amount_list_account_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, float_cell_data_function, GINT_TO_POINTER(id), NULL);
	//gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_spacing( column, 16 );
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_BANK);

	return column;
}

static gint
list_account_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
gint dt1, dt2;
Account *entry1, *entry2;

    gtk_tree_model_get(model, a, LST_DSPACC_DATATYPE, &dt1, LST_DSPACC_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DSPACC_DATATYPE, &dt2, LST_DSPACC_DATAS, &entry2, -1);

	if( dt1 == DSPACC_TYPE_NORMAL && dt2 == DSPACC_TYPE_NORMAL )
	{
		result = entry1->pos - entry2->pos;
	}

    return result;
}


/*
 *
 */ 
static
gboolean list_account_selectionfunc(
GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
GtkTreeIter iter;

	if( gtk_tree_path_get_depth( path ) < 2 )
		return FALSE;

	if(gtk_tree_model_get_iter(model, &iter, path))
	{
	gint dt;

		gtk_tree_model_get(model, &iter,
			LST_DSPACC_DATATYPE, &dt,
			-1);	

		if( dt != DSPACC_TYPE_NORMAL )
			return FALSE;
	}

	return TRUE;
}


GtkWidget *create_list_account(void)
{
GtkTreeStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_tree_store_new(
	  	NUM_LST_DSPACC,
		G_TYPE_POINTER,
		G_TYPE_INT,		/* datatype */
		G_TYPE_INT,		/* fake: state */
		G_TYPE_STRING,	/* fake: name */
		G_TYPE_INT,		/* fake: type */
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), PREFS->rules_hint);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);


	/* Account */
	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, GINT_TO_POINTER(1), NULL);

    renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, GINT_TO_POINTER(2), NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, text_cell_data_function, GINT_TO_POINTER(1), NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_expander_column(GTK_TREE_VIEW (view), column);

	/* Type */
	column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("Type"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, acc_type_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


    /* State */
/*
	column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("State"));

    renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, GINT_TO_POINTER(1), NULL);

    renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, GINT_TO_POINTER(2), NULL);

	gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
*/

    /* Bank */
	column = amount_list_account_column(_("Bank"), LST_DSPACC_BANK);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


    /* Today */
	column = amount_list_account_column(_("Today"), LST_DSPACC_TODAY);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


    /* Future */
	column = amount_list_account_column(_("Future"), LST_DSPACC_FUTURE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

    /* column 7: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	/* disbale selection for level 1 of the tree */
	
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), list_account_selectionfunc, NULL, NULL);
	
	//sort etc
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), list_account_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);


	return(view);
}

