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


/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


//debug
//extern gboolean minor_active;

//---- start debug
/*
static Account debug_accounts[] =
{
	{ 1, AF_ADDED, "Compte courant", "bank", "number", 500.15, -150, 12345678, 55553345 },
	{ 2, AF_ADDED|AF_CHANGED, "Epargne", "bank", "number", 8950, -150, 12345678, 55553345 },
	{ 3, AF_ADDED|AF_CHANGED, "Livret Z", "bank", "number", 65535, -150, 12345678, 55553345 },
};



void debug_populate_acc_list(void)
{
struct _Account *entry;
gint i = 0;

  for (i = 0; i < G_N_ELEMENTS (debug_accounts); i++)
    {
	entry = g_malloc(sizeof(struct _Account));
	//add our entry pointer to the glist
	memcpy(entry, &debug_accounts[i], sizeof(struct _Account));
	GLOBALS->acc_list = g_list_append(GLOBALS->acc_list, entry);
   }

}
*/
//---- end debug


/*
** draw some icons according to the stored data structure
*/
void
status_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
Account *acc;

	gtk_tree_model_get(model, iter, LST_DSPACC_DATAS, &acc, -1);

	switch((gint)user_data)
	{
		case 1:
			g_object_set(renderer, "pixbuf", (acc->flags & AF_ADDED) ? GLOBALS->lst_pixbuf[LST_PIXBUF_ADD] : NULL, NULL);
			break;
		case 2:
			g_object_set(renderer, "pixbuf", (acc->flags & AF_CHANGED) ? GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT] : NULL, NULL);
			break;
	}
}

/*
** draw some text from the stored data structure
*/
void
text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *acc;

	gtk_tree_model_get(model, iter, LST_DSPACC_DATAS, &acc, -1);

	switch((gint)user_data)
	{
		case 1:
			g_object_set(renderer, "text", acc->name, NULL);
			break;
		case 2:
			g_object_set(renderer, "text", acc->number, NULL);
			break;

	}
	//g_object_set(renderer, "text", "toto", NULL);
}

void
float_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
Account *acc;
gdouble value;

	//get the account & value
	gtk_tree_model_get(model, iter,
		LST_DSPACC_DATAS, &acc,
		user_data, &value,
		-1);

	if( !(value) )
	{
		g_object_set(renderer, "markup", NULL, NULL);
	}
	else
	{
	GtkWidget *widget;
	gchar   buf[128];
	gboolean minor;
	gchar *markuptxt;
	guint32 color;

		widget = g_object_get_data(G_OBJECT(model), "minor");
		if(GTK_IS_TOGGLE_BUTTON(widget))
		{
			minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
		}
		else
			minor = 0;


		hb_strfmon(buf, 127, value, minor);

		color = (value > 0) ? PREFS->color_inc : PREFS->color_exp;
		if(value < acc->minimum) color = DEFAULT_WARN_COLOR;

		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);

	}
}



GtkWidget *create_list_account(void)
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
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

  /* column 1: status */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, _("State"));

  renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, (gpointer)1, NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_set_cell_data_func(column, renderer, status_cell_data_function, (gpointer)2, NULL);

  /* set this column to a fixed sizing (of 50 pixels) */
  //gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
  //gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
  gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	/* column 2: Account */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Account name"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, text_cell_data_function, (gpointer)1, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column 3: Number */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Account number"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, text_cell_data_function, (gpointer)2, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NUMBER);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column 4: Bank */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Bank"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, float_cell_data_function, (gpointer)LST_DSPACC_BANK, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_BANK);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column 5: Today */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Today"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, float_cell_data_function, (gpointer)LST_DSPACC_TODAY, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column,1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_TODAY);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column 6: Future */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Future"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, float_cell_data_function, (gpointer)LST_DSPACC_FUTURE, NULL);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column,1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_FUTURE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column 7: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}

