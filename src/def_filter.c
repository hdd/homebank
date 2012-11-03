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
#include "def_filter.h"

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

enum
{
	BUTTON_ALL,
	BUTTON_NONE,
	BUTTON_INVERT,
	MAX_BUTTON
};


extern char *paymode_label_names[];
extern GdkPixbuf *paymode_icons[];

gchar *CYA_NAINOUT[] = { N_("Inactive"), N_("Include"), N_("Exclude"), NULL };

struct deffilter_data
{
	Filter		*filter;

	GtkWidget	*notebook;

	GtkWidget	*CY_option[FILTER_MAX];

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CM_forceadd, *CM_forcechg;

	GtkWidget	*CM_paymode[NUM_PAYMODE_MAX];

	GtkWidget	*ST_minamount, *ST_maxamount;

	GtkWidget	*LV_acc, *BT_acc[MAX_BUTTON];
	GtkWidget	*LV_pay, *BT_pay[MAX_BUTTON];
	GtkWidget	*LV_cat, *BT_cat[MAX_BUTTON];

	gboolean	show_account;
/*
	Object	*originator;
	Object	*GR_page;
	Object	*ST_pattern, *CY_in;
	struct	Wallet_Data *mwd;
	struct	Filter *flt;
	BOOL	account;
*/

};


void deffilter_set(struct deffilter_data *data);


/*
**
*/
void filter_reset(Filter *flt)
{
gint i;
guint count;

	DB( g_printf("(filter) reset %x\n", flt) );

	for(i=0;i<FILTER_MAX;i++)
		flt->option[i] = 0;

	flt->option[FILTER_DATE] = 1;
	flt->option[FILTER_ACCOUNT] = 0;

/*
	flt->flt_range = prefs.dsp_range;

	// a voir
	flt->flt_mindate = date_from(today, 1);
	flt->flt_maxdate = date_to(today);
	flt->flt_year    = getyear(today);

*/

	flt->forceadd = TRUE;
	flt->forcechg = TRUE;

	for(i=0;i<NUM_PAYMODE_MAX;i++)
		flt->paymode[i] = TRUE;

	g_free(flt->acc);
	count = g_list_length(GLOBALS->acc_list);
	DB( g_printf(" %d account(s)\n", count) );
	flt->acc = g_malloc0(sizeof(gboolean)*count);
	for(i=0;i<count;i++)
		flt->acc[i] = TRUE;

	g_free(flt->pay);
	count = g_list_length(GLOBALS->pay_list);
	DB( g_printf(" %d payee(s)\n", count) );
	flt->pay = g_malloc0(sizeof(gboolean)*count);
	for(i=0;i<count;i++)
		flt->pay[i] = TRUE;

	g_free(flt->cat);
	count = g_list_length(GLOBALS->cat_list);
	DB( g_printf(" %d categorie(s)\n", count) );
	flt->cat = g_malloc0(sizeof(gboolean)*count);
	for(i=0;i<count;i++)
		flt->cat[i] = TRUE;

}


gint filter_test(Filter *flt, Operation *ope)
{
gint insert, count;

	//DB( g_printf("(filter) test\n") );

	insert = 1;
	count = 0;

/*** start filtering ***/
	if( ope->flags & OF_REMIND )
		goto end;

	/* add/change force */
	if(flt->forceadd == TRUE && (ope->flags & OF_ADDED))
		goto end;

	if(flt->forcechg == TRUE && (ope->flags & OF_CHANGED))
		goto end;

/* date */
	if(flt->option[FILTER_DATE]) {
		insert = ( (ope->date >= flt->mindate) && (ope->date <= flt->maxdate) ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_DATE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* account */
	if(flt->option[FILTER_ACCOUNT]) {
		insert = ( flt->acc[ope->account] == TRUE ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_ACCOUNT] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* payee */
	if(flt->option[FILTER_PAYEE]) {
		insert = ( flt->pay[ope->payee] == TRUE ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_PAYEE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* category */
	if(flt->option[FILTER_CATEGORY]) {
		insert = ( flt->cat[ope->category] == TRUE ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_CATEGORY] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* status */
	if(flt->option[FILTER_STATUS]) {
		insert = ( ope->flags & OF_VALID ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_STATUS] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* paymode */
	if(flt->option[FILTER_PAYMODE]) {
		insert = ( flt->paymode[ope->paymode] == TRUE) ? 1 : 0;
		count++;
		if(flt->option[FILTER_PAYMODE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* amount */
	if(flt->option[FILTER_AMOUNT]) {
		insert = ( (ope->amount >= flt->minamount) && (ope->amount <= flt->maxamount) ) ? 1 : 0;
		count++;
		if(flt->option[FILTER_AMOUNT] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* info/wording */

end:
//	DB( g_printf(" %d :: %d :: %d\n", flt->mindate, ope->date, flt->maxdate) );
//	DB( g_printf(" [%d] %s => %d (%d)\n", ope->account, ope->wording, insert, count) );
	return(insert);
}


/*
**
*/
void deffilter_clear(GtkWidget *widget, gpointer user_data)
{
struct deffilter_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(deffilter) clear\n") );

	filter_reset(data->filter);

	deffilter_set(data);

}


/*
**
*/
void deffilter_acc_select(GtkWidget *widget, gpointer user_data)
{
struct deffilter_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean toggle;

	DB( g_printf("(deffilter) acc select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFACC_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, toggle, -1);
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}

/*
**
*/
void deffilter_pay_select(GtkWidget *widget, gpointer user_data)
{
struct deffilter_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean toggle;

	DB( g_printf("(deffilter) pay select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFPAY_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, toggle, -1);
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}

/*
**
*/
void deffilter_cat_select(GtkWidget *widget, gpointer user_data)
{
struct deffilter_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
gint n_child;
gboolean toggle;

	DB( g_printf("(deffilter) pay select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFCAT_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, toggle, -1);
				break;
		}

		n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
		gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
		while(n_child > 0)
		{

			switch(select)
			{
				case BUTTON_ALL:
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, TRUE, -1);
					break;
				case BUTTON_NONE:
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, FALSE, -1);
					break;
				case BUTTON_INVERT:
						gtk_tree_model_get (model, &child, LST_DEFCAT_TOGGLE, &toggle, -1);
						toggle ^= 1;
						gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, toggle, -1);
					break;
			}

			n_child--;
			gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
		}

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

}

/*
**
*/
void deffilter_option_update(GtkWidget *widget, gpointer user_data)
{
struct deffilter_data *data;
gint active, i;
gboolean sensitive;

	DB( g_printf("(deffilter) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// date
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_DATE]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->PO_mindate, sensitive);
	gtk_widget_set_sensitive(data->PO_maxdate, sensitive);

	// amount
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_AMOUNT]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->ST_minamount, sensitive);
	gtk_widget_set_sensitive(data->ST_maxamount, sensitive);

	//paymode
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_PAYMODE]));
	sensitive = active == 0 ? FALSE : TRUE;
	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		gtk_widget_set_sensitive(data->CM_paymode[i], sensitive);
	}

	//account
	if(data->show_account == TRUE)
	{
		active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_ACCOUNT]));
		sensitive = active == 0 ? FALSE : TRUE;
		gtk_widget_set_sensitive(data->LV_acc, sensitive);
		for(i=0;i<MAX_BUTTON;i++)
		{
			gtk_widget_set_sensitive(data->BT_acc[i], sensitive);
		}


	}

	//payee
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_PAYEE]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->LV_pay, sensitive);
	for(i=0;i<MAX_BUTTON;i++)
	{
		gtk_widget_set_sensitive(data->BT_pay[i], sensitive);
	}

	//category
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_CATEGORY]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->LV_cat, sensitive);
	for(i=0;i<MAX_BUTTON;i++)
	{
		gtk_widget_set_sensitive(data->BT_cat[i], sensitive);
	}



}


/*
**
*/
void deffilter_get(struct deffilter_data *data)
{
gint i;

	DB( g_printf("(deffilter) get\n") );

	if(data->filter !=NULL)
	{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter	iter, child;
	gint n_child;
	gboolean valid;
	gboolean toggled;

		for(i=0;i<FILTER_MAX;i++)
		{
			if(data->show_account == FALSE && i == FILTER_ACCOUNT)
				continue;

			data->filter->option[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[i]));
		}

	//date
		DB( g_printf(" date\n") );
		data->filter->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
		data->filter->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	//status
		DB( g_printf(" status\n") );
		data->filter->forceadd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forceadd));
		data->filter->forcechg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forcechg));

	//paymode
		DB( g_printf(" paymode\n") );
		for(i=0;i<NUM_PAYMODE_MAX;i++)
			data->filter->paymode[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]));

	// account
		if(data->show_account == TRUE)
		{
			DB( g_printf(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
				gtk_tree_model_get (model, &iter, LST_DEFACC_TOGGLE, &toggled, -1);

				//data->filter->acc[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
				data->filter->acc[i] = toggled;

				/* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}
		}

	// payee
		DB( g_printf(" payee\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
			gtk_tree_model_get (model, &iter, LST_DEFPAY_TOGGLE, &toggled, -1);

			//data->filter->pay[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			data->filter->pay[i] = toggled;

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// category
		DB( g_printf(" category\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
			gtk_tree_model_get (model, &iter, LST_DEFCAT_TOGGLE, &toggled, -1);

			//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			data->filter->cat[i] = toggled;

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get (model, &child, LST_DEFCAT_TOGGLE, &toggled, -1);
				data->filter->cat[i] = toggled;
				//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &child);

				n_child--;
				gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
			}

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// active tab
	data->filter->last_tab =gtk_notebook_get_current_page(GTK_NOTEBOOK(data->notebook));

	}
}


/*
**
*/
void deffilter_set(struct deffilter_data *data)
{

	DB( g_printf("(deffilter) set\n") );

	if(data->filter != NULL)
	{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter	iter, child;
	gint n_child;
	gboolean valid;
	gint i;

		DB( g_printf(" options\n") );

		for(i=0;i<FILTER_MAX;i++)
		{
			if(data->show_account == FALSE && i == FILTER_ACCOUNT)
				continue;

			gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_option[i]), data->filter->option[i]);
		}

		//DB( g_printf(" setdate %d to %x\n", data->filter->mindate, data->PO_mindate) );
		//DB( g_printf(" setdate %d to %x\n", 0, data->PO_mindate) );
	//date
		DB( g_printf(" date\n") );
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);

	//status
		DB( g_printf(" status\n") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forceadd), data->filter->forceadd);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forcechg), data->filter->forcechg);

	//paymode
		DB( g_printf(" paymode\n") );

		for(i=0;i<NUM_PAYMODE_MAX;i++)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]), data->filter->paymode[i]);

	// account
		if(data->show_account == TRUE)
		{
			DB( g_printf(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
				if(data->filter->acc[i] == TRUE)
					//gtk_tree_selection_select_iter(selection, &iter);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, TRUE, -1);

				/* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}
		}

	// payee
		DB( g_printf(" payee\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
			if(data->filter->pay[i] == TRUE)
				//gtk_tree_selection_select_iter(selection, &iter);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, TRUE, -1);

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// category
		DB( g_printf(" category\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
			if(data->filter->cat[i] == TRUE)
				//gtk_tree_selection_select_iter(selection, &iter);
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, TRUE, -1);

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				if(data->filter->cat[i] == TRUE)
					//gtk_tree_selection_select_iter(selection, &child);
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, TRUE, -1);

				n_child--;
				gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
			}

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// active tab
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), data->filter->last_tab);


	}
}



/*
**
*/
void deffilter_setup(struct deffilter_data *data)
{

	DB( g_printf("(deffilter) setup\n") );

	if(data->show_account == TRUE && data->LV_acc != NULL)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc))), GTK_SELECTION_MULTIPLE);

		populate_view_acc(data->LV_acc, GLOBALS->acc_list, FALSE);
	}

	if(data->LV_pay)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay))), GTK_SELECTION_MULTIPLE);

		populate_view_pay(data->LV_pay, GLOBALS->pay_list, FALSE);
	}

	if(data->LV_cat)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat))), GTK_SELECTION_MULTIPLE);

		populate_view_cat(data->LV_cat, GLOBALS->cat_list, FALSE);
		gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
	}
}

/*
**
*/
GtkWidget *deffilter_page_category (struct deffilter_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (container), HB_MAINBOX_SPACING);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label(_("_Option:"), 1.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_CATEGORY] = make_cycle(label, CYA_NAINOUT);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_CATEGORY], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_cat = (GtkWidget *)defcategory_list_new(TRUE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_cat);

	vbox = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_INVERT] = widget;

	return(container);
}

/*
**
*/
GtkWidget *deffilter_page_payee (struct deffilter_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (container), HB_MAINBOX_SPACING);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label(_("_Option:"), 1.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_PAYEE] = make_cycle(label, CYA_NAINOUT);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_PAYEE], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_pay = (GtkWidget *)defpayee_list_new(TRUE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_pay);

	vbox = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_INVERT] = widget;

	return(container);
}

/*
** account filter
*/
GtkWidget *deffilter_page_account (struct deffilter_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (container), HB_MAINBOX_SPACING);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label(_("_Option:"), 1.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_ACCOUNT] = make_cycle(label, CYA_NAINOUT);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_ACCOUNT], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_acc = (GtkWidget *)defaccount_list_new(TRUE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_acc);

	vbox = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_INVERT] = widget;

	return(container);
}


/*
** general page: date, amount, state, payment
*/
GtkWidget *deffilter_page_general (struct deffilter_data *data)
{
GtkWidget *container, *table, *table1, *label, *widget, *image;
GtkWidget *alignment;
gint row, i;

	//container = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	//gtk_container_set_border_width(GTK_CONTAINER(container), HB_BOX_SPACING);

	container = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (container), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (container), HB_TABCOL_SPACING);
	gtk_container_set_border_width(GTK_CONTAINER(container), HB_BOX_SPACING);

	// filter date & state
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	
	gtk_table_attach_defaults (GTK_TABLE (container), alignment, 0, 1, 0, 1);

	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter Date</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);
	//gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Option:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->CY_option[FILTER_DATE] = make_cycle(label, CYA_NAINOUT);
		//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_DATE], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_From:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->PO_mindate = gtk_dateentry_new();
		//data->PO_mindate = gtk_entry_new();
		//gtk_table_attach_defaults (GTK_TABLE (table), data->PO_mindate, 1, 2, row, row+1);
		gtk_table_attach (GTK_TABLE (table), data->PO_mindate, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_To:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->PO_maxdate = gtk_dateentry_new();
		//data->PO_maxdate = gtk_entry_new();
		//gtk_table_attach_defaults (GTK_TABLE (table), data->PO_maxdate, 1, 2, row, row+1);
		gtk_table_attach (GTK_TABLE (table), data->PO_maxdate, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	// filter date & state
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);

	gtk_table_attach_defaults (GTK_TABLE (container), alignment, 0, 1, 1, 2);

	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter State</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("Validated:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		//gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->CY_option[FILTER_STATUS] = make_cycle(label, CYA_NAINOUT);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_STATUS], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("Force:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		//label = gtk_label_new(NULL);
		//----------------------------------------- l, r, t, b
		//gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
		widget = gtk_check_button_new_with_mnemonic (_("display 'Added'"));
		gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
		data->CM_forceadd = widget;

		row++;
		//label = gtk_label_new(NULL);
		//gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
		widget = gtk_check_button_new_with_mnemonic (_("display 'Edited'"));
		gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
		data->CM_forcechg = widget;

	// filter date & state
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);

	gtk_table_attach_defaults (GTK_TABLE (container), alignment, 1, 2, 0, 1);

	// Amount section
	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter Amount</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Option:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		//gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->CY_option[FILTER_AMOUNT] = make_cycle(label, CYA_NAINOUT);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_AMOUNT], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_From:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->ST_minamount = make_amount(label);
		gtk_table_attach (GTK_TABLE (table), data->ST_minamount, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_To:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->ST_maxamount = make_amount(label);
		gtk_table_attach (GTK_TABLE (table), data->ST_maxamount, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	// Filter Payment
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);

	gtk_table_attach_defaults (GTK_TABLE (container), alignment, 1, 2, 1, 2);

	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter Payment</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);


		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Option:"), 1.0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->CY_option[FILTER_PAYMODE] = make_cycle(label, CYA_NAINOUT);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_PAYMODE], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		table1 = gtk_table_new (1, 1, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (table1), 0);
		gtk_table_set_col_spacings (GTK_TABLE (table1), 2);

		row++;
		gtk_table_attach (GTK_TABLE (table), table1, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		for(i=0;i<NUM_PAYMODE_MAX;i++)
		{
			row = i;

			image = gtk_image_new_from_pixbuf(paymode_icons[i]);
			gtk_table_attach (GTK_TABLE (table1), image, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
			
			data->CM_paymode[i] = gtk_check_button_new();
			gtk_table_attach (GTK_TABLE (table1), data->CM_paymode[i], 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);


			label = make_label(_(paymode_label_names[i]), 0.0, 0.5);
			gtk_table_attach (GTK_TABLE (table1), label, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		}



	return(container);
}




/*
**
*/
gint create_deffilter_window (Filter *filter, gboolean show_account)
{
struct deffilter_data data;
GtkWidget *window, *mainbox, *notebook, *label, *page;

	//data = g_malloc0(sizeof(struct deffilter_data));
	//if(!data) return NULL;
	memset(&data, 0, sizeof(data));

	data.filter = filter;

	window = gtk_dialog_new_with_buttons (_("Edit Filter"),
					    //GTK_WINDOW (do_widget),
					    NULL,
					    0,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT,
					    GTK_STOCK_CLEAR,
					    55,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);

	
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "filter.svg");

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(deffilter) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	mainbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);


	notebook = gtk_notebook_new();
    gtk_box_pack_start (GTK_BOX (mainbox), notebook, TRUE, TRUE, 0);
	data.notebook = notebook;

	//common (date + state + amount)
	label = gtk_label_new(_("General"));
	page = deffilter_page_general(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	//category
	page = deffilter_page_category(&data);
	label = gtk_label_new(_("Category"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	//payee
	page = deffilter_page_payee(&data);
	label = gtk_label_new(_("Payee"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);


	//account
	data.show_account = show_account;
	if(show_account == TRUE)
	{
		page = deffilter_page_account(&data);
		label = gtk_label_new(_("Account"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
	}

	//text
	//maybe later

	/* signal connect */
    g_signal_connect (data.CY_option[FILTER_DATE]    , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_AMOUNT]  , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_PAYMODE] , "changed", G_CALLBACK (deffilter_option_update), NULL);

    g_signal_connect (data.CY_option[FILTER_PAYEE]   , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_CATEGORY], "changed", G_CALLBACK (deffilter_option_update), NULL);

	if(show_account == TRUE)
	{
	    g_signal_connect (data.CY_option[FILTER_ACCOUNT] , "changed", G_CALLBACK (deffilter_option_update), NULL);

	    g_signal_connect (data.BT_acc[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_acc_select), (gpointer)BUTTON_ALL);
	    g_signal_connect (data.BT_acc[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_acc_select), (gpointer)BUTTON_NONE);
	    g_signal_connect (data.BT_acc[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_acc_select), (gpointer)BUTTON_INVERT);
	}

    g_signal_connect (data.BT_pay[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_pay_select), (gpointer)BUTTON_ALL);
    g_signal_connect (data.BT_pay[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_pay_select), (gpointer)BUTTON_NONE);
    g_signal_connect (data.BT_pay[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_pay_select), (gpointer)BUTTON_INVERT);

    g_signal_connect (data.BT_cat[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_cat_select), (gpointer)BUTTON_ALL);
    g_signal_connect (data.BT_cat[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_cat_select), (gpointer)BUTTON_NONE);
    g_signal_connect (data.BT_cat[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_cat_select), (gpointer)BUTTON_INVERT);

	//setup, init and show window
	deffilter_setup(&data);

	gtk_widget_show_all (window);

	deffilter_set(&data);

	//wait for the user
	gint result = 55;
	
	//while( result == 55 )
	//{
		result = gtk_dialog_run (GTK_DIALOG (window));

		switch (result)
	    {
		case GTK_RESPONSE_ACCEPT:
		   //do_application_specific_something ();
		   deffilter_get(&data);
		   break;
		case 55:
			deffilter_clear(window, NULL);
		   deffilter_get(&data);
			break;
		default:
		   //do_nothing_since_dialog_was_cancelled ();
		   break;
	    }
	//}

	// cleanup and destroy
	//deffilter_cleanup(&data, result);


	DB( g_printf(" free\n") );
	//g_free(data);

	DB( g_printf(" destroy\n") );
	gtk_widget_destroy (window);

	DB( g_printf(" all ok\n") );

	return result;
}
