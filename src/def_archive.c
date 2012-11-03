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

#include "def_archive.h"
#include "ui_account.h"
#include "ui_category.h"
#include "ui_payee.h"

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

enum {
	HID_VALID,
	HID_REMIND,
	MAX_HID
};

struct defarchive_data
{
	GList	*tmp_list;
	gint	change;
	//guint32	lastkey;
	Archive		*lastarcitem;


	GtkWidget	*LV_arc;

	GtkWidget	*PO_pay;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_amount;
	GtkWidget	*notebook;
	GtkWidget	*CM_valid;
	GtkWidget	*CM_remind;
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

	gulong		handler_id[MAX_HID];

};



gchar *CYA_UNIT[] = { N_("Day"), N_("Week"), N_("Month"), N_("Year"), NULL };


static GtkWidget *defarchive_list_new(void);

/*
** add an empty new account to our temp GList and treeview
*/
static void defarchive_add(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;

Archive *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defarchive) add\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_arc));

	item = da_archive_malloc();
	item->wording = g_strdup_printf(_("(archive %d)"), g_list_length(GLOBALS->arc_list) + 1);
	item->unit = 2;

	GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, item);

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
static void defarchive_remove(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
Archive *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defarchive) remove (data=%08x)\n", data) );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &item, -1);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		GLOBALS->arc_list = g_list_remove(GLOBALS->arc_list, item);

		data->change++;
		//DB( g_printf(" remove =%08x (pos=%d)\n", entry, g_list_index(data->tmp_list, entry) ) );
	}
}


/*
** update the archive name everytime it changes
*/
static void defarchive_rename(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gchar *txt;
Archive *item;

	DB( g_print("(defarchive) rename\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_arc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &item, -1);
	
		DB( g_print(" -> %s\n", item->wording) );

		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
		// ignore if entry is empty
		if (txt && *txt)
		{
			g_free(item->wording);
			item->wording = g_strdup(txt);
		}

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_arc));

	}

}




/*
** set widgets contents from the selected account
*/
static void defarchive_set(GtkWidget *widget, gpointer user_data)
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

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), item->amount);

		g_signal_handler_block(data->CM_valid, data->handler_id[HID_VALID]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_valid), (item->flags & OF_VALID) ? 1 : 0);
		g_signal_handler_unblock(data->CM_valid, data->handler_id[HID_VALID]);
		
		g_signal_handler_block(data->CM_remind, data->handler_id[HID_REMIND]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), (item->flags & OF_REMIND) ? 1 : 0);
		g_signal_handler_unblock(data->CM_remind, data->handler_id[HID_REMIND]);
	
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), item->paymode);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_cheque), (item->flags & OF_CHEQ2) ? 1 : 0);

		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_grp), item->category);

	DB( g_print(" -> set payee %d\n", item->payee) );
		ui_pay_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_pay), item->payee);

	DB( g_print(" -> PO_acc %d\n", item->account) );
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_acc), item->account);

	DB( g_print(" -> PO_accto %d\n", item->dst_account) );
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_accto), item->dst_account);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_auto), (item->flags & OF_AUTO) ? 1 : 0);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_every), item->every);

		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_unit), item->unit);

		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_next), item->nextdate);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_limit), (item->flags & OF_LIMIT) ? 1 : 0);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_limit), item->limit);

	}

}

/*
** get widgets contents to the selected account
*/
static void defarchive_getlast(struct defarchive_data *data)
{
gchar *txt;
gboolean bool;
gdouble value;
gint active;
Archive *item;

	DB( g_print("(defarchive) getlast\n") );

	item = data->lastarcitem;
	
	if( item != NULL )
	{
		DB( g_print(" -> %s\n", item->wording) );

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

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_remind));
		if(bool == 1) item->flags |= OF_REMIND;

		item->paymode		= gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
		item->category		= ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_grp));
		item->payee			= ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_pay));
		item->account		= ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
		item->dst_account	= ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_accto));

		DB( g_print(" -> PO_acc %d\n", item->account) );
		DB( g_print(" -> PO_accto %d\n", item->dst_account) );

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_auto));
		if(bool) item->flags |= OF_AUTO;

		item->every   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_every));
		item->unit    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_unit));
		item->nextdate	= gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_next));

		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_limit));
		if(bool) item->flags |= OF_LIMIT;

		item->limit   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_limit));

		data->change++;
	}
}


/*
**
*/
static void defarchive_paymode(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
gint payment;
gint page;

	DB( g_printf("(defarchive) paymode\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	if(payment == PAYMODE_CHECK)
		page = 1;
	if(payment == PAYMODE_INTXFER)
		page = 2;

	DB( g_printf(" payment: %d, page: %d\n", payment, page) );

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), page);
}

/*
**
*/
static void defarchive_automatic(GtkWidget *widget, gpointer user_data)
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
** update the widgets status and contents from action/selection value
*/
static void defarchive_update(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;
Archive *arcitem;


	DB( g_printf("(defarchive) update\n") );

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

	gtk_widget_set_sensitive(data->CM_valid, sensitive);
	gtk_widget_set_sensitive(data->CM_remind, sensitive);
	
	gtk_widget_set_sensitive(data->CM_cheque, sensitive);

	gtk_widget_set_sensitive(data->NU_mode, sensitive);
	gtk_widget_set_sensitive(data->PO_grp, sensitive);
	gtk_widget_set_sensitive(data->PO_acc, sensitive);
	gtk_widget_set_sensitive(data->PO_accto, sensitive);

	gtk_widget_set_sensitive(data->CM_auto, sensitive);

	gtk_widget_set_sensitive(data->BT_rem, sensitive);



	if(selected)
	{
		gtk_tree_model_get(model, &iter, LST_DEFARC_DATAS, &arcitem, -1);

		if(data->lastarcitem != NULL && arcitem != data->lastarcitem)
		{
			DB( g_print(" -> should do a get for last selected (%s)\n", data->lastarcitem->wording) );
			defarchive_getlast(data);
		}
		data->lastarcitem = arcitem;


		defarchive_set(widget, NULL);
		
	}
	else
	{
		data->lastarcitem = NULL;
	
	}

	defarchive_automatic(widget, NULL);
	defarchive_paymode(widget,NULL);


}

static void defarchive_togglestatus(GtkWidget *widget, gpointer user_data)
{
struct defarchive_data *data;

	DB( g_printf("(defarchive) togglestatus\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//valid & remind are exclusive
	switch( GPOINTER_TO_INT(user_data) )
	{
		case HID_VALID:
			if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid)) )
			{
				g_signal_handler_block(data->CM_remind, data->handler_id[HID_REMIND]);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), FALSE);
				g_signal_handler_unblock(data->CM_remind, data->handler_id[HID_REMIND]);
			}
			break;
		
		case HID_REMIND:
			if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_remind)) )
			{
				g_signal_handler_block(data->CM_valid, data->handler_id[HID_VALID]);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_valid), FALSE);
				g_signal_handler_unblock(data->CM_valid, data->handler_id[HID_VALID]);
			}
	}




}



/*
**
*/
static void defarchive_toggleamount(GtkWidget *widget, gpointer user_data)
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
static void defarchive_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defarchive_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

/*
**
*/
static gboolean defarchive_cleanup(struct defarchive_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defarchive) cleanup\n") );


	if(data->lastarcitem != NULL)
	{
		DB( g_print(" -> should do a get for last selected (%s)\n", data->lastarcitem->wording) );
		defarchive_getlast(data);
	}

	GLOBALS->arc_list = da_archive_sort(GLOBALS->arc_list);
	
	GLOBALS->change += data->change;

	return doupdate;
}

/*
**
*/
static void defarchive_setup(struct defarchive_data *data)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	DB( g_printf("(defarchive) setup\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->arc_list, sizeof(struct _Archive));
	data->change = 0;
	data->lastarcitem = NULL;

	//hb_glist_populate_treeview(data->tmp_list, data->LV_arc, LST_DEFARC_DATAS, LST_DEFARC_OLDPOS);

	//insert all glist item into treeview
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_arc));
	i=0;
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFARC_DATAS, item,	//data struct
			LST_DEFARC_OLDPOS, i,		//oldpos
			-1);

		//DB( g_printf(" populate_treeview: %d %08x\n", i, list->data) );

		i++; list = g_list_next(list);
	}

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_pay)  , GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_grp)  , GLOBALS->h_cat);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_acc)  , GLOBALS->h_acc);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_accto), GLOBALS->h_acc);

	
}







// the window creation
GtkWidget *create_defarchive_window (void)
{
struct defarchive_data data;
GtkWidget *window, *mainbox, *hbox, *vbox, *bbox, *table;
GtkWidget *label, *widget, *treeview, *scrollwin, *notebook;
GtkWidget *alignment;
gint row;

      window = gtk_dialog_new_with_buttons (_("Manage Archives"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "archive.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_ARCHIVE);

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
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
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
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_pay_comboboxentry_new(label);	
	data.PO_pay = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_cat_comboboxentry_new(label);
	data.PO_grp = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


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
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		widget = ui_acc_comboboxentry_new(label);	
		data.PO_accto = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);


	//**

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Optional infos</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Description:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data.ST_word = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	
	row++;
	label = gtk_label_new_with_mnemonic (_("A_ccount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);	
	data.PO_acc = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Validated"));
	data.CM_valid = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Remind"));
	data.CM_remind = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);



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

	/* set default periodicity to month */
	//todo: ove elsewhere
	gtk_combo_box_set_active(GTK_COMBO_BOX(data.CY_unit), 2);



	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_arc)), "changed", G_CALLBACK (defarchive_selection), NULL);
	g_signal_connect (G_OBJECT (data.BT_amount), "clicked", G_CALLBACK (defarchive_toggleamount), NULL);

	g_signal_connect (G_OBJECT (data.BT_new), "clicked", G_CALLBACK (defarchive_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (defarchive_remove), NULL);

	g_signal_connect (G_OBJECT (data.ST_word), "changed", G_CALLBACK (defarchive_rename), NULL);

	 g_signal_connect (data.NU_mode, "changed", G_CALLBACK (defarchive_paymode), NULL);
	//data.handler_id[FIELD_CHEQUE] = g_signal_connect (data.CM_cheque, "toggled", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_CHEQUE));
	//data.handler_id[FIELD_CATEGORY] = g_signal_connect (data.PO_grp, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_CATEGORY));
	//data.handler_id[FIELD_ACCOUNT] = g_signal_connect (data.PO_acc, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_ACCOUNT));
	//data.handler_id[FIELD_TOACCOUNT] = g_signal_connect (data.PO_accto, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_TOACCOUNT));

	data.handler_id[HID_VALID]  = g_signal_connect (data.CM_valid , "toggled", G_CALLBACK (defarchive_togglestatus), GINT_TO_POINTER(HID_VALID));
	data.handler_id[HID_REMIND] = g_signal_connect (data.CM_remind, "toggled", G_CALLBACK (defarchive_togglestatus), GINT_TO_POINTER(HID_REMIND));

	g_signal_connect (data.CM_auto, "toggled", G_CALLBACK (defarchive_automatic), NULL);
	//data.handler_id[FIELD_EVERY] = g_signal_connect (data.NB_every, "value-changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_EVERY));
	//data.handler_id[FIELD_UNIT] = g_signal_connect (data.CY_unit, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_UNIT));
	//data.handler_id[FIELD_NEXT] = g_signal_connect (data.PO_next, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_NEXT));

	g_signal_connect (data.CM_limit, "toggled", G_CALLBACK (defarchive_automatic), NULL);
	//data.handler_id[FIELD_TIMES] = g_signal_connect (data.NB_limit, "changed", G_CALLBACK (defarchive_get), GINT_TO_POINTER(FIELD_TIMES));

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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
static gint defarchive_list_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Archive *entry1, *entry2;
gchar *name1, *name2;

	gtk_tree_model_get(model, a, LST_DEFARC_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFARC_DATAS, &entry2, -1);

	name1 = entry1->wording;
	name2 = entry2->wording;
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
**
*/
static void defarchive_auto_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
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
static void
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
static GtkWidget *defarchive_list_new(void)
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
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFARC_DATAS, defarchive_list_compare_func, GINT_TO_POINTER(LST_DEFARC_DATAS), NULL);


	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	/* column 1 */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, defarchive_text_cell_data_function, GINT_TO_POINTER(1), NULL);
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





