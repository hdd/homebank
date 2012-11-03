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
#include "def_archive.h"

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
	FIELD_PAYEE,
	FIELD_WORDING,
	FIELD_AMOUNT,
	FIELD_VALID,

	FIELD_PAYMODE,
	FIELD_CHEQUE,
	FIELD_CATEGORY,
	FIELD_ACCOUNT,
	FIELD_TOACCOUNT,

	FIELD_AUTO,
	FIELD_EVERY,
	FIELD_UNIT,
	FIELD_NEXT,
	FIELD_LIMIT,
	FIELD_TIMES,
	MAX_FIELD
};

struct defarchive_data
{
	GList	*tmp_list;
	gint	change;

	GtkWidget	*LV_arc;

	GtkWidget	*PO_pay;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_amount;
	GtkWidget	*notebook;
	GtkWidget	*CM_valid;
	GtkWidget	*GR_cheque;
	GtkWidget	*CM_cheque;

	GtkWidget	*NU_mode;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*PO_accto;

	GtkWidget	*CM_auto;
	GtkWidget	*NB_every;
	GtkWidget	*CY_unit;
	GtkWidget	*PO_next;
	GtkWidget	*CM_limit;
	GtkWidget	*NB_limit;

	GtkWidget	*BT_new;
	GtkWidget	*BT_ren;
	GtkWidget	*BT_rem;

	gulong		handler_id[MAX_FIELD];

};

void defarchive_add(GtkWidget *widget, gpointer user_data);
void defarchive_remove(GtkWidget *widget, gpointer user_data);
void defarchive_update(GtkWidget *widget, gpointer user_data);
void defarchive_set(GtkWidget *widget, gpointer user_data);
void defarchive_get(GtkWidget *widget, gpointer user_data);
void defarchive_toggleamount(GtkWidget *widget, gpointer user_data);
void defarchive_paymode(GtkWidget *widget, gpointer user_data);
void defarchive_automatic(GtkWidget *widget, gpointer user_data);
gboolean defarchive_cleanup(struct defarchive_data *data, gint result);
void defarchive_setup(struct defarchive_data *data);
void defarchive_dispose(struct defarchive_data *data);

gchar *CYA_UNIT[] = { N_("Day"), N_("Week"), N_("Month"), N_("Year"), NULL };


gboolean defarchive_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	defarchive_get(widget, user_data);
	return FALSE;
}
/*
** add an empty new account to our temp GList and treeview
*/
void defarchive_add(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;

Archive *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defarchive) add\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_arc));

	item = da_archive_malloc();
	item->wording = g_strdup(_("(new archive)"));

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		LST_DEFARC_DATAS, item,
		LST_DEFARC_OLDPOS, 0,
		-1);

	gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc)), &iter);

	data->change++;
}

/*
** remove the selected account to our treeview and temp GList
*/
void defarchive_remove(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defarchive) remove (data=%08x)\n", data) );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		//gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &entry, -1);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		data->change++;
		//DB( g_printf(" remove =%08x (pos=%d)\n", entry, g_list_index(data->tmp_list, entry) ) );
	}
}

/*
** update the widgets status and contents from action/selection value
*/
void defarchive_update(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;

	DB( g_printf("\n****(defarchive) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(defarchive) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc)), &model, &iter);

	DB( g_printf(" selected = %d\n", selected) );


	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->PO_pay, sensitive);
	gtk_widget_set_sensitive(data->ST_word, sensitive);
	gtk_widget_set_sensitive(data->ST_amount, sensitive);
	gtk_widget_set_sensitive(data->BT_amount, sensitive);
	//gtk_widget_set_sensitive(data->CY_amount, sensitive);
	gtk_widget_set_sensitive(data->CM_valid, sensitive);
	gtk_widget_set_sensitive(data->CM_cheque, sensitive);

	gtk_widget_set_sensitive(data->NU_mode, sensitive);
	gtk_widget_set_sensitive(data->PO_grp, sensitive);
	gtk_widget_set_sensitive(data->PO_acc, sensitive);
	gtk_widget_set_sensitive(data->PO_accto, sensitive);

	gtk_widget_set_sensitive(data->CM_auto, sensitive);

	//sensitive = (data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->LV_arc, sensitive);
	//gtk_widget_set_sensitive(data->BT_new, sensitive);

	//sensitive = (selected == TRUE && data->action == 0) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	if(selected)
	{
		defarchive_set(widget, NULL);
	}

	defarchive_paymode(widget,NULL);
	defarchive_automatic(widget,NULL);


}

/*
** set widgets contents from the selected account
*/
void defarchive_set(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

Archive *item;

	DB( g_printf("(defarchive) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &item, -1);


		gtk_entry_set_text(GTK_ENTRY(data->ST_word), item->wording);

		g_signal_handler_block(data->ST_amount, data->handler_id[FIELD_AMOUNT]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), item->amount);
		g_signal_handler_unblock(data->ST_amount, data->handler_id[FIELD_AMOUNT]);

		//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_amount), (item->Flags & OF_INCOME) ? 1 : 0);

		g_signal_handler_block(data->CM_valid, data->handler_id[FIELD_VALID]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_valid), (item->flags & OF_VALID) ? 1 : 0);
		g_signal_handler_unblock(data->CM_valid, data->handler_id[FIELD_VALID]);

		g_signal_handler_block(data->NU_mode, data->handler_id[FIELD_PAYMODE]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), item->paymode);
		g_signal_handler_unblock(data->NU_mode, data->handler_id[FIELD_PAYMODE]);

		g_signal_handler_block(data->CM_cheque, data->handler_id[FIELD_CHEQUE]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_cheque), (item->flags & OF_CHEQ2) ? 1 : 0);
		g_signal_handler_unblock(data->CM_cheque, data->handler_id[FIELD_CHEQUE]);

		g_signal_handler_block(data->PO_grp, data->handler_id[FIELD_CATEGORY]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_grp), item->category);
		g_signal_handler_unblock(data->PO_grp, data->handler_id[FIELD_CATEGORY]);

		g_signal_handler_block(data->PO_pay, data->handler_id[FIELD_PAYEE]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_pay), item->payee);
		g_signal_handler_unblock(data->PO_pay, data->handler_id[FIELD_PAYEE]);

		g_signal_handler_block(data->PO_acc, data->handler_id[FIELD_ACCOUNT]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), item->account);
		g_signal_handler_unblock(data->PO_acc, data->handler_id[FIELD_ACCOUNT]);

		g_signal_handler_block(data->PO_accto, data->handler_id[FIELD_TOACCOUNT]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_accto), item->dst_account);
		g_signal_handler_unblock(data->PO_accto, data->handler_id[FIELD_TOACCOUNT]);

		g_signal_handler_block(data->CM_auto, data->handler_id[FIELD_AUTO]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_auto), (item->flags & OF_AUTO) ? 1 : 0);
		g_signal_handler_unblock(data->CM_auto, data->handler_id[FIELD_AUTO]);

		g_signal_handler_block(data->NB_every, data->handler_id[FIELD_EVERY]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_every), item->every);
		g_signal_handler_unblock(data->NB_every, data->handler_id[FIELD_EVERY]);

		g_signal_handler_block(data->CY_unit, data->handler_id[FIELD_UNIT]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_unit), item->unit);
		g_signal_handler_unblock(data->CY_unit, data->handler_id[FIELD_UNIT]);

		g_signal_handler_block(data->PO_next, data->handler_id[FIELD_NEXT]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_next), item->nextdate);
		g_signal_handler_unblock(data->PO_next, data->handler_id[FIELD_NEXT]);

		g_signal_handler_block(data->CM_limit, data->handler_id[FIELD_LIMIT]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_limit), (item->flags & OF_LIMIT) ? 1 : 0);
		g_signal_handler_unblock(data->CM_limit, data->handler_id[FIELD_LIMIT]);

		g_signal_handler_block(data->NB_limit, data->handler_id[FIELD_TIMES]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_limit), item->limit);
		g_signal_handler_unblock(data->NB_limit, data->handler_id[FIELD_TIMES]);

	}

}

/*
** get widgets contents to the selected account
*/
void defarchive_get(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gchar *txt;
gboolean bool;
gdouble value;
gint active;

Archive *item;
gint field = GPOINTER_TO_INT(user_data);

	DB( g_print("(defarchive) get %d\n", field) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &item, -1);

		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
		// ignore if entry is empty
		if (txt && *txt)
		{
			g_free(item->wording);
			item->wording = g_strdup(txt);
		}

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_arc));

		value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		item->amount = value;

		item->flags = 0;

		active = item->amount > 0 ? TRUE : FALSE;
		//active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_amount));
		if(active == 1) item->flags |= OF_INCOME;

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque));
		if(bool) item->flags |= OF_CHEQ2;

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
		if(bool) item->flags |= OF_VALID;

		item->paymode		= gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
		item->category		= gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_grp));
		item->payee			= gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_pay));
		item->account		= gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));
		item->dst_account	= gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_accto));

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_auto));
		if(bool) item->flags |= OF_AUTO;

		item->every   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_every));
		item->unit    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_unit));
		item->nextdate	= gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_next));

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_limit));
		if(bool) item->flags |= OF_LIMIT;

		item->limit   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_limit));

		switch( field )
		{
			case FIELD_PAYMODE:
				defarchive_paymode(widget, NULL);
				break;

			case FIELD_AUTO:
			case FIELD_LIMIT:
				defarchive_automatic(widget, NULL);
				break;

		}

	}
}

/*
**
*/
void defarchive_toggleamount(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
gdouble value;

	DB( g_printf("(defarchive) toggleamount\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	value *= -1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value);


	/*
	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	type = gtk_widget_get_sensitive(data->CY_amount);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value * type);
	*/
}

/*
**
*/
void defarchive_paymode(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
gint payment;
gint page;

	DB( g_printf("(defarchive) paymode\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	if(payment == PAYMODE_CHEQUE)
		page = 1;
	if(payment == PAYMODE_PERSTRANSFERT)
		page = 2;

	DB( g_printf(" payment: %d, page: %d\n", payment, page) );

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), page);
}

/*
**
*/
void defarchive_automatic(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
gboolean sensitive;

	DB( g_printf("(defarchive) automatic\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//automated
	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_auto)) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->PO_next, sensitive);
	gtk_widget_set_sensitive(data->NB_every, sensitive);
	gtk_widget_set_sensitive(data->CY_unit, sensitive);
	gtk_widget_set_sensitive(data->CM_limit, sensitive);

	sensitive = (sensitive == TRUE) ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_limit)) : sensitive;
	gtk_widget_set_sensitive(data->NB_limit, sensitive);


}

/*
**
*/
void defarchive_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defarchive_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

/*
**
*/
gboolean defarchive_cleanup(struct defarchive_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defarchive) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;
	gboolean valid;
	guint i;

		DB( g_printf(" we always update our glist\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_arc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Archive *item;
			gint oldpos;

				gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
					LST_DEFARC_DATAS, &item,
					LST_DEFARC_OLDPOS, &oldpos,
					-1);

				//item->key = i;
				data->tmp_list = g_list_append(data->tmp_list, item);

				/* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

		//update our GLOBAL pay_list
		da_archive_destroy(GLOBALS->arc_list);
		GLOBALS->arc_list = g_list_sort(data->tmp_list, (GCompareFunc)defarchive_list_sort);
		data->tmp_list = NULL;

		GLOBALS->change += data->change;
	}
	DB( g_printf(" free tmp_list\n") );

	da_archive_destroy(data->tmp_list);

	return doupdate;
}

/*
**
*/
void defarchive_setup(struct defarchive_data *data)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	DB( g_printf("(defarchive) setup\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->arc_list, sizeof(struct _Archive));
	data->change = 0;

	//hb_glist_populate_treeview(data->tmp_list, data->LV_arc, LST_DEFARC_DATAS, LST_DEFARC_OLDPOS);

	//insert all glist item into treeview
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_arc));
	i=0; list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = da_archive_clone(list->data);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFARC_DATAS, item,	//data struct
			LST_DEFARC_OLDPOS, i,		//oldpos
			-1);

		//DB( g_printf(" populate_treeview: %d %08x\n", i, list->data) );

		i++; list = g_list_next(list);
	}

	make_poppayee_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->pay_list);
	make_popcategory_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->cat_list);
	make_popaccount_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->acc_list);
	make_popaccount_populate(GTK_COMBO_BOX(data->PO_accto), GLOBALS->acc_list);

}







// the window creation
GtkWidget *create_defarchive_window (void)
{
struct defarchive_data data;
GtkWidget *window, *mainbox, *hbox, *vbox, *bbox, *table;
GtkWidget *label, *widget, *treeview, *scrollwin, *notebook, *separator;
GtkWidget *alignment;
gint row;

      window = gtk_dialog_new_with_buttons (_("Edit Archives"),
					    //GTK_WINDOW (do_widget),
					    NULL,
					    0,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);
	
	gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/archive.svg", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(defarchive) window=%08lx, inst_data=%08lx\n", window, &data) );

	//window contents
	mainbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);

	//hbox 1 : list | other
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), vbox, FALSE, FALSE, 0);

// listview
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 	treeview = (GtkWidget *)defarchive_list_new();
  	data.LV_arc = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	// tools buttons
	bbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER (bbox), HB_BOX_SPACING);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), HB_BOX_SPACING);

	//data.BT_rem = gtk_button_new_with_mnemonic(_("_Remove"));
	data.BT_rem = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_rem);

	//data.BT_new = gtk_button_new_with_mnemonic(_("_New"));
	data.BT_new = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_new);



	//right side
	table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.5, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (mainbox), alignment, TRUE, TRUE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General infos</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new_with_mnemonic (_("_Wording:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data.ST_word = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Amount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	widget = make_amount(label);
	data.ST_amount = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("+/-");
	data.BT_amount = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("A_ccount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_popaccount(label);
	data.PO_acc = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Validated"));
	data.CM_valid = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//**

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Optional infos</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("Pay_ment:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_paymode(label);
	data.NU_mode = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
	data.notebook = notebook;
	gtk_table_attach (GTK_TABLE (table), notebook, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		label = gtk_label_new(NULL);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), label, NULL);

		hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, NULL);
		widget = gtk_check_button_new_with_mnemonic(_("Of notebook _2"));
		data.CM_cheque = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, NULL);
		label = make_label(_("_To account:"), 1, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
		widget = make_popaccount(label);
		data.PO_accto = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_poppayee(label);
	data.PO_pay = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_popcategory(label);
	data.PO_grp = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	// ----

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Automated insertion</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic(_("_Activate"));
	data.CM_auto = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		widget = gtk_check_button_new_with_mnemonic(_("_Limit to:"));
		data.CM_limit = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

		widget = make_numeric(label, 1, 100);
		data.NB_limit = widget;
	    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		label = gtk_label_new_with_mnemonic (_("t_imes"));
	    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	row++;
	label = gtk_label_new_with_mnemonic (_("Ever_y:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 1, 100);
	data.NB_every = widget;
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("_Unit:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	widget = make_cycle(label, CYA_UNIT);
	data.CY_unit = widget;
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Next on:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = gtk_dateentry_new();
	data.PO_next = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);




	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_arc)), "changed", G_CALLBACK (defarchive_selection), NULL);
	g_signal_connect (G_OBJECT (data.BT_amount), "clicked", G_CALLBACK (defarchive_toggleamount), NULL);

	g_signal_connect (G_OBJECT (data.BT_new), "clicked", G_CALLBACK (defarchive_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (defarchive_remove), NULL);

	// modify events
	data.handler_id[FIELD_PAYEE] = g_signal_connect (data.PO_pay, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_PAYEE);
	g_signal_connect (G_OBJECT (data.ST_word), "activate", G_CALLBACK (defarchive_get), (gpointer)FIELD_WORDING);
	g_signal_connect (G_OBJECT (data.ST_word), "focus-out-event", G_CALLBACK (defarchive_focus_out), (gpointer)FIELD_WORDING);
	data.handler_id[FIELD_AMOUNT] = g_signal_connect (data.ST_amount, "value-changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_AMOUNT);
	data.handler_id[FIELD_VALID] = g_signal_connect (data.CM_valid, "toggled", G_CALLBACK (defarchive_get), (gpointer)FIELD_VALID);

	data.handler_id[FIELD_PAYMODE] = g_signal_connect (data.NU_mode, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_PAYMODE);
	data.handler_id[FIELD_CHEQUE] = g_signal_connect (data.CM_cheque, "toggled", G_CALLBACK (defarchive_get), (gpointer)FIELD_CHEQUE);
	data.handler_id[FIELD_CATEGORY] = g_signal_connect (data.PO_grp, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_CATEGORY);
	data.handler_id[FIELD_ACCOUNT] = g_signal_connect (data.PO_acc, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_ACCOUNT);
	data.handler_id[FIELD_TOACCOUNT] = g_signal_connect (data.PO_accto, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_TOACCOUNT);

	data.handler_id[FIELD_AUTO] = g_signal_connect (data.CM_auto, "toggled", G_CALLBACK (defarchive_get), (gpointer)FIELD_AUTO);
	data.handler_id[FIELD_EVERY] = g_signal_connect (data.NB_every, "value-changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_EVERY);
	data.handler_id[FIELD_UNIT] = g_signal_connect (data.CY_unit, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_UNIT);
	data.handler_id[FIELD_NEXT] = g_signal_connect (data.PO_next, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_NEXT);

	data.handler_id[FIELD_LIMIT] = g_signal_connect (data.CM_limit, "toggled", G_CALLBACK (defarchive_get), (gpointer)FIELD_LIMIT);
	data.handler_id[FIELD_TIMES] = g_signal_connect (data.NB_limit, "changed", G_CALLBACK (defarchive_get), (gpointer)FIELD_TIMES);

	//setup, init and show window
	defarchive_setup(&data);
	defarchive_update(data.LV_arc, NULL);

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
	defarchive_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}
