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

#include "ui_account.h"
#include "ui_payee.h"
#include "ui_category.h"
#include "def_filter.h"
#include "hb_transaction.h"

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

struct deffilter_data
{
	Filter		*filter;

	GtkWidget	*notebook;

	GtkWidget	*CY_option[FILTER_MAX];

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CM_validated, *CM_reminded;

	GtkWidget	*CM_forceadd, *CM_forcechg;

	GtkWidget	*CM_paymode[NUM_PAYMODE_MAX];

	GtkWidget	*ST_minamount, *ST_maxamount;

	GtkWidget	*ST_info, *ST_wording, *ST_tag;

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
void filter_reset(Filter *flt);
void deffilter_clear(GtkWidget *widget, gpointer user_data);
void deffilter_acc_select(GtkWidget *widget, gpointer user_data);
void deffilter_pay_select(GtkWidget *widget, gpointer user_data);
void deffilter_cat_select(GtkWidget *widget, gpointer user_data);
void deffilter_option_update(GtkWidget *widget, gpointer user_data);
void deffilter_get(struct deffilter_data *data);
void deffilter_set(struct deffilter_data *data);
void deffilter_setup(struct deffilter_data *data);

GtkWidget *deffilter_part_date(struct deffilter_data *data);
GtkWidget *deffilter_part_text(struct deffilter_data *data);
GtkWidget *deffilter_part_amount(struct deffilter_data *data);
GtkWidget *deffilter_part_state(struct deffilter_data *data);
GtkWidget *deffilter_part_paymode(struct deffilter_data *data);

GtkWidget *deffilter_page_category (struct deffilter_data *data);
GtkWidget *deffilter_page_payee (struct deffilter_data *data);
GtkWidget *deffilter_page_account (struct deffilter_data *data);
GtkWidget *deffilter_page_general (struct deffilter_data *data);

/*
**
*/
void filter_reset(Filter *flt)
{
gint i;

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

	flt->validated = TRUE;
	flt->reminded = TRUE;

	flt->forceadd = FALSE;
	flt->forcechg = FALSE;

	for(i=0;i<NUM_PAYMODE_MAX;i++)
		flt->paymode[i] = TRUE;

	g_free(flt->info);
	g_free(flt->wording);
	g_free(flt->tag);
	flt->info = NULL;
	flt->wording = NULL;
	flt->tag = NULL;

}


gint filter_test(Filter *flt, Operation *ope)
{
gint insert;
Account *accitem;
Payee *payitem;
Category *catitem;

	//DB( g_printf("(filter) test\n") );

	insert = 1;

/*** start filtering ***/

	/* add/change force */
	if(flt->forceadd == TRUE && (ope->flags & OF_ADDED))
		goto end;

	if(flt->forcechg == TRUE && (ope->flags & OF_CHANGED))
		goto end;

/* date */
	if(flt->option[FILTER_DATE]) {
		insert = ( (ope->date >= flt->mindate) && (ope->date <= flt->maxdate) ) ? 1 : 0;
		if(flt->option[FILTER_DATE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* account */
	if(flt->option[FILTER_ACCOUNT]) {
		accitem = da_acc_get(ope->account);
		if(accitem)
		{	
			insert = ( accitem->filter == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_ACCOUNT] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* payee */
	if(flt->option[FILTER_PAYEE]) {
		payitem = da_pay_get(ope->payee);
		if(payitem)
		{
			insert = ( payitem->filter == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_PAYEE] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* category */
	if(flt->option[FILTER_CATEGORY]) {
		catitem = da_cat_get(ope->category);
		if(catitem)
		{
			insert = ( catitem->filter == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_CATEGORY] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* status */
	if(flt->option[FILTER_STATUS]) {
	gint insert1 = 0, insert2 = 0;
	
		if(flt->validated)
			insert1 = ( ope->flags & OF_VALID ) ? 1 : 0;
		if(flt->reminded)
			insert2 = ( ope->flags & OF_REMIND ) ? 1 : 0;
		
		insert = insert1 | insert2;

		if(flt->option[FILTER_STATUS] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* paymode */
	if(flt->option[FILTER_PAYMODE]) {
		insert = ( flt->paymode[ope->paymode] == TRUE) ? 1 : 0;
		if(flt->option[FILTER_PAYMODE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* amount */
	if(flt->option[FILTER_AMOUNT]) {
		insert = ( (ope->amount >= flt->minamount) && (ope->amount <= flt->maxamount) ) ? 1 : 0;

		if(flt->option[FILTER_AMOUNT] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* info/wording/tag */
	if(flt->option[FILTER_TEXT])
	{
	gchar *tags;
	gint insert1 = 0, insert2 = 0, insert3 = 0;

		if(flt->info)
		{
			if(ope->info)
			{
				if( g_strrstr(ope->info, flt->info) != NULL )
					insert1 = 1;
			}
		}
		else
			insert1 = 1;

		if(flt->wording)
		{
			if(ope->wording)
			{
				if( g_strrstr(ope->wording, flt->wording) != NULL )
					insert2 = 1;
			}
		}
		else
			insert2 = 1;
			
		if(flt->tag)
		{
			tags = transaction_get_tagstring(ope);
			if(tags)
			{			
				if( g_strrstr(tags, flt->tag) != NULL )
					insert3 = 1;
				
			}
			g_free(tags);
		}	
		else
			insert3 = 1;
			
		insert = insert1 && insert2 && insert3 ? 1 : 0;

		if(flt->option[FILTER_TEXT] == 2) insert ^= 1;
	}
	if(!insert) goto end;

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

	// status
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_STATUS]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_validated, sensitive);
	gtk_widget_set_sensitive(data->CM_reminded, sensitive);

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

	// text
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_TEXT]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->ST_wording, sensitive);
	gtk_widget_set_sensitive(data->ST_info, sensitive);
	gtk_widget_set_sensitive(data->ST_tag, sensitive);

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
gchar *txt;

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
		data->filter->validated = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_validated));
		data->filter->reminded  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_reminded));

		data->filter->forceadd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forceadd));
		data->filter->forcechg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forcechg));

	//paymode
		DB( g_printf(" paymode\n") );
		for(i=0;i<NUM_PAYMODE_MAX;i++)
			data->filter->paymode[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]));

	//amount
		data->filter->minamount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minamount));
		data->filter->maxamount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_maxamount));

	//text:wording
		//free any previous string
		if(	data->filter->wording )
		{
			g_free(data->filter->wording);
			data->filter->wording = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_wording));
	
		if (txt && *txt)	// ignore if entry is empty
		{
			data->filter->wording = g_strdup(txt);
		}	
	
	//text:info
		//free any previous string
		if(	data->filter->info )
		{
			g_free(data->filter->info);
			data->filter->info = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
		// ignore if entry is empty
		if (txt && *txt)
		{
			data->filter->info = g_strdup(txt);
		}

	//text:tag
		//free any previous string
		if(	data->filter->tag )
		{
			g_free(data->filter->tag);
			data->filter->tag = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tag));
		// ignore if entry is empty
		if (txt && *txt)
		{
			data->filter->tag = g_strdup(txt);
		}

	
	// account
		if(data->show_account == TRUE)
		{
			DB( g_printf(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Account *accitem;

				gtk_tree_model_get (model, &iter,
					LST_DEFACC_TOGGLE, &toggled,
					LST_DEFACC_DATAS, &accitem,
					-1);

				//data->filter->acc[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
				accitem->filter = toggled;

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
		Payee *payitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFPAY_TOGGLE, &toggled,
				LST_DEFPAY_DATAS, &payitem,
				-1);

			//data->filter->pay[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			payitem->filter = toggled;

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
		Category *catitem;
		
			gtk_tree_model_get (model, &iter,
				LST_DEFCAT_TOGGLE, &toggled,
				LST_DEFCAT_DATAS, &catitem,
				-1);

			//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			//data->filter->cat[i] = toggled;
			catitem->filter = toggled;

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get (model, &child,
					LST_DEFCAT_TOGGLE, &toggled,
					LST_DEFCAT_DATAS, &catitem,
					-1);

				
				//data->filter->cat[i] = toggled;
				//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &child);
				catitem->filter = toggled;

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
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_validated), data->filter->validated);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_reminded), data->filter->reminded);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forceadd), data->filter->forceadd);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forcechg), data->filter->forcechg);

	//paymode
		DB( g_printf(" paymode\n") );

		for(i=0;i<NUM_PAYMODE_MAX;i++)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]), data->filter->paymode[i]);

	//amount
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_minamount), data->filter->minamount);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_maxamount), data->filter->maxamount);

	//text
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), (data->filter->info != NULL) ? data->filter->info : "");
	gtk_entry_set_text(GTK_ENTRY(data->ST_wording), (data->filter->wording != NULL) ? data->filter->wording : "");
	gtk_entry_set_text(GTK_ENTRY(data->ST_tag), (data->filter->tag != NULL) ? data->filter->tag : "");

	//account
		if(data->show_account == TRUE)
		{
			DB( g_printf(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Account *accitem;

				gtk_tree_model_get (model, &iter,
					LST_DEFACC_DATAS, &accitem,
					-1);

				if(accitem->filter == TRUE)
					//gtk_tree_selection_select_iter(selection, &iter);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter,
						LST_DEFACC_TOGGLE, TRUE, -1);

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
		Payee *payitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFPAY_DATAS, &payitem,
				-1);
		
			if(payitem->filter == TRUE)
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
		Category *catitem;
		
			gtk_tree_model_get (model, &iter,
				LST_DEFCAT_DATAS, &catitem,
				-1);
		
			if(catitem->filter == TRUE)
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, TRUE, -1);

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get (model, &child,
					LST_DEFCAT_DATAS, &catitem,
					-1);

				if(catitem->filter == TRUE)
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

		ui_acc_listview_populate(data->LV_acc);
		//populate_view_acc(data->LV_acc, GLOBALS->acc_list, FALSE);
	}

	if(data->LV_pay)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay))), GTK_SELECTION_MULTIPLE);

		ui_pay_listview_populate(data->LV_pay);
		//populate_view_pay(data->LV_pay, GLOBALS->pay_list, FALSE);
	}

	if(data->LV_cat)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat))), GTK_SELECTION_MULTIPLE);

		//populate_view_cat(data->LV_cat, GLOBALS->cat_list, FALSE);
		ui_cat_listview_populate(data->LV_cat);
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
	data->CY_option[FILTER_CATEGORY] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_CATEGORY], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_cat = (GtkWidget *)ui_cat_listview_new(TRUE);
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
	data->CY_option[FILTER_PAYEE] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_PAYEE], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_pay = (GtkWidget *)ui_pay_listview_new(TRUE);
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
	data->CY_option[FILTER_ACCOUNT] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_ACCOUNT], TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), HB_BOX_SPACING);

	data->LV_acc = ui_acc_listview_new(TRUE);
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

GtkWidget *deffilter_part_date(struct deffilter_data *data)
{
GtkWidget *table, *label;
GtkWidget *alignment;
gint row;
	
	// filter date
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	

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
		data->CY_option[FILTER_DATE] = make_nainex(label);
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

	return alignment;
}


GtkWidget *deffilter_part_text(struct deffilter_data *data)
{
GtkWidget *table, *label;
GtkWidget *alignment;
gint row;
	
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);


	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter Text</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Option:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		//gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->CY_option[FILTER_TEXT] = make_nainex(label);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_TEXT], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_Description:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->ST_wording = make_string(label);
		gtk_table_attach (GTK_TABLE (table), data->ST_wording, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_Info:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->ST_info = make_string(label);
		gtk_table_attach (GTK_TABLE (table), data->ST_info, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = make_label(_("_Tag:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		data->ST_tag = make_string(label);
		gtk_table_attach (GTK_TABLE (table), data->ST_tag, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	return alignment;
}

GtkWidget *deffilter_part_amount(struct deffilter_data *data)
{
GtkWidget *table, *label;
GtkWidget *alignment;
gint row;


	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);

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
		data->CY_option[FILTER_AMOUNT] = make_nainex(label);
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
	

	return alignment;
}


GtkWidget *deffilter_part_state(struct deffilter_data *data)
{
GtkWidget *table, *label, *vbox, *widget;
GtkWidget *alignment;
gint row;


		// column 2

	// filter state
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);


	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Filter State</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

		row++;
		label = make_label("", 0.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = make_label(_("_Option:"), 0, 0.5);
		//----------------------------------------- l, r, t, b
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		data->CY_option[FILTER_STATUS] = make_nainex(label);
		gtk_table_attach (GTK_TABLE (table), data->CY_option[FILTER_STATUS], 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		row++;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach (GTK_TABLE (table), vbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
		
		widget = gtk_check_button_new_with_mnemonic (_("validated"));		
		data->CM_validated = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		widget = gtk_check_button_new_with_mnemonic (_("remind"));
		data->CM_reminded = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		row++;
		label = make_label(_("Force:"), 0, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach (GTK_TABLE (table), vbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

		widget = gtk_check_button_new_with_mnemonic (_("display 'Added'"));
		data->CM_forceadd = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		widget = gtk_check_button_new_with_mnemonic (_("display 'Edited'"));
		data->CM_forcechg = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	return alignment;
}


GtkWidget *deffilter_part_paymode(struct deffilter_data *data)
{
GtkWidget *table, *label, *table1, *image;
GtkWidget *alignment;
gint i, row;
	
	// Filter Payment
	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);


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
		data->CY_option[FILTER_PAYMODE] = make_nainex(label);
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



	return alignment;
}


/*
** general page: date, amount, state, payment
*/
GtkWidget *deffilter_page_general (struct deffilter_data *data)
{
GtkWidget *container, *part;

	//container = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	//gtk_container_set_border_width(GTK_CONTAINER(container), HB_BOX_SPACING);

	container = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (container), HB_TABROW_SPACING*2);
	gtk_table_set_col_spacings (GTK_TABLE (container), HB_TABCOL_SPACING*2);
	gtk_container_set_border_width(GTK_CONTAINER(container), HB_BOX_SPACING);

	// date: r=1, c=1
	part = deffilter_part_date(data);
	gtk_table_attach_defaults(GTK_TABLE (container), part, 0, 1, 0, 1);
	
	// amount: r=2, c=2
	part = deffilter_part_amount(data);
	gtk_table_attach_defaults (GTK_TABLE (container), part, 0, 1, 1, 2);

	// paymode:
	part = deffilter_part_paymode(data);
	gtk_table_attach_defaults (GTK_TABLE (container), part, 1, 2, 0, 2);

	// state: r=2, c=1
	part = deffilter_part_state(data);
	gtk_table_attach_defaults (GTK_TABLE (container), part, 2, 3, 0, 1);

	// text: r=2, c=1
	part = deffilter_part_text(data);
	gtk_table_attach_defaults (GTK_TABLE (container), part, 2, 3, 1, 2);
	
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
					    GTK_STOCK_CLEAR,
					    55,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "filter.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_FILTER);

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
    g_signal_connect (data.CY_option[FILTER_STATUS]  , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_DATE]    , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_AMOUNT]  , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_PAYMODE] , "changed", G_CALLBACK (deffilter_option_update), NULL);

    g_signal_connect (data.CY_option[FILTER_PAYEE]   , "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_CATEGORY], "changed", G_CALLBACK (deffilter_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_TEXT]    , "changed", G_CALLBACK (deffilter_option_update), NULL);

	if(show_account == TRUE)
	{
	    g_signal_connect (data.CY_option[FILTER_ACCOUNT] , "changed", G_CALLBACK (deffilter_option_update), NULL);

	    g_signal_connect (data.BT_acc[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_acc_select), GINT_TO_POINTER(BUTTON_ALL));
	    g_signal_connect (data.BT_acc[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_acc_select), GINT_TO_POINTER(BUTTON_NONE));
	    g_signal_connect (data.BT_acc[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_acc_select), GINT_TO_POINTER(BUTTON_INVERT));
	}

    g_signal_connect (data.BT_pay[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_pay_select), GINT_TO_POINTER(BUTTON_ALL));
    g_signal_connect (data.BT_pay[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_pay_select), GINT_TO_POINTER(BUTTON_NONE));
    g_signal_connect (data.BT_pay[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_pay_select), GINT_TO_POINTER(BUTTON_INVERT));

    g_signal_connect (data.BT_cat[BUTTON_ALL]   , "clicked", G_CALLBACK (deffilter_cat_select), GINT_TO_POINTER(BUTTON_ALL));
    g_signal_connect (data.BT_cat[BUTTON_NONE]  , "clicked", G_CALLBACK (deffilter_cat_select), GINT_TO_POINTER(BUTTON_NONE));
    g_signal_connect (data.BT_cat[BUTTON_INVERT], "clicked", G_CALLBACK (deffilter_cat_select), GINT_TO_POINTER(BUTTON_INVERT));

	//setup, init and show window
	deffilter_setup(&data);

	gtk_widget_show_all (window);

	deffilter_set(&data);
	
	deffilter_option_update(window, NULL);

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
