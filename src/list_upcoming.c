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
#include "preferences.h"
#include "list_upcoming.h"

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;

/*
** date cell function
*/
static void date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *item;
gchar buffer[256];
GDate *date;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &item,
		-1);

	date = g_date_new_julian (item->nextdate);
	g_date_strftime (buffer, 256-1, PREFS->date_format, date);
	g_date_free(date);

	//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

    g_object_set(renderer, "text", buffer, NULL);
}

/*
** payee cell function
*/
static void payee_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
Payee *pay;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
		-1);
	pay = g_list_nth_data(GLOBALS->pay_list, arc->payee);
    //g_object_set(renderer, "text", pay->name, NULL);
}

/*
** wording cell function
*/
static void wording_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
		-1);
    //if( arc != NULL && arc->wording != NULL)
        g_object_set(renderer, "text", arc->wording, NULL);
}


/*
** amount cell function
*/
static void amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *item;
gint type;

	// get the operation

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &item,
		-1);

	type = (item->flags & OF_INCOME) ? 1 : -1;

	if( !(item->amount) )
	{
		g_object_set(renderer, "markup", NULL, NULL);
	}
	else
	{
	gchar   buf[128];
	gchar *markuptxt;
	guint32 color;

		mystrfmon(buf, 127, item->amount, GLOBALS->minor);

		color = (item->flags & OF_INCOME) ? PREFS->color_inc : PREFS->color_exp;
		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);

	}
	
}

/*
** remaining cell function
*/
static void remaining_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *item;
gchar *markuptxt;
guint decay;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &item,
		-1);

	decay = item->nextdate - GLOBALS->today;

	//markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);

	markuptxt = g_strdup_printf("%d days", decay);
	

    g_object_set(renderer, "markup", markuptxt, NULL);
    g_free(markuptxt);
}


GtkWidget *create_list_upcoming(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_DSPACC,
		G_TYPE_POINTER,
		G_TYPE_BOOLEAN,	/* fake column */
		G_TYPE_BOOLEAN,	/* fake column */
		G_TYPE_BOOLEAN,	/* fake column */
		G_TYPE_BOOLEAN,	/* fake column */
		G_TYPE_INT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

	/* column: Payee */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, payee_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Wording */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Description"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, wording_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", 2);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Amount"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, amount_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Next on */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Next on"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, date_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Next on */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Remaining"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, remaining_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPUPC_REMAINING);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);



  /* column: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

    /* set initial sort order */
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DSPUPC_REMAINING, GTK_SORT_ASCENDING);

	return(view);
}

