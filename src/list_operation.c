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

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


//debug
//extern gboolean minor_active;
extern GdkPixbuf *paymode_icons[];

/* This is not pretty. Of course you can also use a
   *  separate compare function for each sort ID value */

  gint
  ope_sort_iter_compare_func (GtkTreeModel *model,
                          GtkTreeIter  *a,
                          GtkTreeIter  *b,
                          gpointer      userdata)
  {
    gint sortcol = GPOINTER_TO_INT(userdata);
    gint ret = 0;
	Operation *ope1, *ope2;
	gdouble tmpval = 0;

	gtk_tree_model_get(model, a, LST_DSPOPE_DATAS, &ope1, -1);
	gtk_tree_model_get(model, b, LST_DSPOPE_DATAS, &ope2, -1);

    switch (sortcol)
    {
		case LST_DSPOPE_DATE:
 			ret = ope1->date - ope2->date;
			break;

		case LST_DSPOPE_INFO:
			if(!(ret = ope1->paymode - ope2->paymode))
			{
				if(ope1->info != NULL && ope2->info != NULL)
					ret = g_utf8_collate(ope1->info, ope2->info);
			}
			break;

		case LST_DSPOPE_PAYEE:
			ret = ope1->payee - ope2->payee;
			break;

		case LST_DSPOPE_WORDING:
			if(ope1->wording != NULL && ope2->wording != NULL)
				ret = g_utf8_collate(ope1->wording, ope2->wording);
			break;

		case LST_DSPOPE_AMOUNT:
		case LST_DSPOPE_EXPENSE:
		case LST_DSPOPE_INCOME:
			tmpval = ope1->amount - ope2->amount;
			ret = tmpval > 0 ? 1 : -1;
			break;

		case LST_DSPOPE_CATEGORY:
			ret = ope1->category - ope2->category;
			break;

		default:
			g_return_val_if_reached(0);
    }

    return ret;
  }

/*
** date cell function
*/
void ope_state_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *entry;
GdkPixbuf *pixbuf = NULL;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &entry, -1);

	/*
		stat[0] = ( entry->ope_Flags & OF_ADDED  ) ? data->istatus[2] : data->istatus[0];
		stat[1] = ( entry->ope_Flags & OF_CHANGED) ? data->istatus[3] : data->istatus[0];
		stat[2] = ( entry->ope_Flags & OF_VALID  ) ? data->istatus[4] : data->istatus[0];
		if( entry->ope_Flags & OF_REMIND ) stat[2] = data->istatus[1];
	*/

	switch((gint)user_data)
	{
		case 1:
			pixbuf = ( entry->flags & OF_AUTO  ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO] : ( entry->flags & OF_ADDED ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_ADD] : NULL;
			break;
		case 2:
			pixbuf = ( entry->flags & OF_CHANGED  ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT] : NULL;
			break;
		case 3:
			if( entry->flags & OF_VALID )
				pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_VALID];
			else
			{
				if( entry->flags & OF_REMIND )
					pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND];
			}
			break;
	}
	g_object_set(renderer, "pixbuf", pixbuf, NULL);
}

/*
** date cell function
*/
void ope_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
gchar buffer[256];
GDate *date;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	date = g_date_new_julian (ope->date);
	g_date_strftime (buffer, 256-1, PREFS->date_format, date);
	g_date_free(date);

	//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

    g_object_set(renderer, "text", buffer, NULL);
}

/*
** info cell function
*/
void ope_info_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	switch((gint)user_data)
	{
		case 1:
			g_object_set(renderer, "pixbuf", paymode_icons[ope->paymode], NULL);
			break;
		case 2:
		    g_object_set(renderer, "text", ope->info, NULL);
			break;
	}
}

/*
** payee cell function
*/
void ope_payee_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
Payee *pay;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	pay = g_list_nth_data(GLOBALS->pay_list, ope->payee);
	if(pay != NULL)
		g_object_set(renderer, "text", pay->name, NULL);
}

/*
** wording cell function
*/
void ope_wording_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
    g_object_set(renderer, "text", ope->wording, NULL);
}

/*
** amount cell function
*/
void ope_amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
gint column = (gint)user_data;
gint type;

	// get the operation
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	type = (ope->flags & OF_INCOME) ? LST_DSPOPE_INCOME : LST_DSPOPE_EXPENSE;

	if( !(ope->amount) || (column != type) && column != LST_DSPOPE_AMOUNT )
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

		mystrfmon(buf, 127, ope->amount, minor);

		color = (ope->flags & OF_INCOME) ? PREFS->color_inc : PREFS->color_exp;
		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);

	}
}


/*
** category cell function
*/
void ope_category_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
Category *grp;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	//g_snprintf(buf, sizeof(buf), "(%d) %s", ope->ope_Group, NULL);
    //g_object_set(renderer, "text", buf, NULL);

	grp = g_list_nth_data(GLOBALS->cat_list, ope->category);
	if(grp != NULL)
		g_object_set(renderer, "text", grp->name, NULL);
}




/* column 3: Infos */
static GtkTreeViewColumn *info_list_operation_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Info"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, (gpointer)1, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, (gpointer)2, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_INFO);
	//gtk_tree_view_column_set_resizable(column, TRUE);

	return column;
}


/* column 4: Payee */
static GtkTreeViewColumn *payee_list_operation_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_payee_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_PAYEE);
	gtk_tree_view_column_set_resizable(column, TRUE);

	return column;
}

/* column 5: Wording */
static GtkTreeViewColumn *wording_list_operation_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Description"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_wording_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_WORDING);
	gtk_tree_view_column_set_resizable(column, TRUE);

	return column;
}




/* column 6/7: Amount/Expense/Income */

static GtkTreeViewColumn *amount_list_operation_column(gchar *title, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_amount_cell_data_function, (gpointer)id, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_column_set_sort_column_id (column, id);
	gtk_tree_view_column_set_resizable(column, TRUE);

	return column;
}




/* column 8: Category */
static GtkTreeViewColumn *category_list_operation_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_category_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_CATEGORY);
	gtk_tree_view_column_set_resizable(column, TRUE);

	return column;
}



void list_operation_visible_columns(GtkTreeView *treeview, gint *visibility)
{
GtkTreeViewColumn *column;
GList *list, *tmp;
gint i = 0, idx;

	list = gtk_tree_view_get_columns( treeview );
	tmp = g_list_first(list);
	while (tmp != NULL)
	{
		column = tmp->data;
		gtk_tree_view_column_set_visible (column, visibility[i]);
		tmp = g_list_next(tmp);
		i++;
	}

	g_list_free(list );
}




/*
** create our operation list
** State, Date, Info, Payee, Wording, (Amount), Expense, Income, Category
*/
GtkWidget *create_list_operation(gboolean *columns)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_DSPOPE,
		G_TYPE_POINTER,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_MULTIPLE);

	/* column 1: Status */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("State"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_state_cell_data_function, (gpointer)1, NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_state_cell_data_function, (gpointer)2, NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_state_cell_data_function, (gpointer)3, NULL);

	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column 2: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_date_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	column = info_list_operation_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = payee_list_operation_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = wording_list_operation_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_operation_column(_("Amount"), LST_DSPOPE_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_operation_column(_("Expense"), LST_DSPOPE_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_operation_column(_("Income"), LST_DSPOPE_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = category_list_operation_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column 9: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_DATE    , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_DATE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INFO    , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INFO), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_PAYEE   , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_PAYEE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_WORDING , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_WORDING), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_AMOUNT  , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_AMOUNT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_EXPENSE , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_EXPENSE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INCOME  , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INCOME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_CATEGORY, ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_CATEGORY), NULL);

  /* set initial sort order */
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DSPOPE_DATE, GTK_SORT_ASCENDING);

  /* assign visibility */
	list_operation_visible_columns(GTK_TREE_VIEW(view), columns);

	return(view);
}


/* ---------------------------------------------- */

/*
** account cell function
*/
extern GList *ofx_acc_list;

void ope_account_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
Account *acc;
gchar *name;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	name = NULL;
	acc = g_list_nth_data(GLOBALS->acc_list, ope->account);
	if( acc != NULL)
		name = acc->name;
	else
	{
		acc = g_list_nth_data(ofx_acc_list, ope->account - g_list_length(GLOBALS->acc_list) );
		name = acc->name;

		//debug
		//g_print("get fromofx acc %d - %x - %s\n", ope->account - g_list_length(GLOBALS->acc_list), acc, acc->name);
		//g_print("get fromofx acc %d - %x\n", ope->account - g_list_length(GLOBALS->acc_list), acc);
	}
	g_object_set(renderer, "text", name, NULL);
}

/*
** amount cell function
*/
void ope_importamount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;

	// get the operation
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	gchar   buf[128];
	gchar *markuptxt;
	guint32 color;

		mystrfmon(buf, 127, ope->amount, 0);

		color = (ope->flags & OF_INCOME) ? PREFS->color_inc : PREFS->color_exp;
		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);

}

/*
**
*/
void ope_importstate_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Operation *ope;
GdkPixbuf *pixbuf = NULL;

	// get the operation
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	if( ope->same != NULL )
		pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_WARNING];

	g_object_set(renderer, "pixbuf", pixbuf, NULL);
}


static void
ope_importfixed_toggled (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_OPE_IMPTOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_OPE_IMPTOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

/*
** create our operation list
*/
GtkWidget *create_list_import_operation(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	8,
		G_TYPE_POINTER,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		//G_TYPE_BOOLEAN,
		//G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	//gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_MULTIPLE);

	/* column: Selection */
	renderer = gtk_cell_renderer_toggle_new ();

	g_signal_connect (renderer, "toggled",
		    G_CALLBACK (ope_importfixed_toggled), store);


	column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "active", LST_OPE_IMPTOGGLE,
						     NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Duplicate */
	column = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title(column, _("Import ?"));
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importstate_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

//#ifndef NOOFX
	/* column: Account */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Account"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_account_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
//#endif	
	
	/* column: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_date_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Infos */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Info"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, (gpointer)1, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, (gpointer)2, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_INFO);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Payee */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_payee_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_PAYEE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
*/
	/* column: Wording */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Description"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_wording_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_WORDING);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Amount"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importamount_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_EXPENSE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	/* column : Category */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_category_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_CATEGORY);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
*/


	/* column 6: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}
