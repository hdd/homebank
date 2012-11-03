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
#include "def_account.h"

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
	ACTION_NEW,
	ACTION_MODIFY,
	ACTION_REMOVE,
};

enum
{
	FIELD_NAME,
	//todo: for stock account	
	//FIELD_TYPE,
	FIELD_BANK,
	FIELD_NUMBER,
	FIELD_BUDGET,
	FIELD_CLOSED,
	FIELD_INITIAL,
	FIELD_MINIMUM,
	FIELD_CHEQUE1,
	FIELD_CHEQUE2,
	MAX_FIELD
};


struct defaccount_data
{
	GList	*tmp_list;
	gint	change;
	gint	action;

	GtkWidget	*LV_acc;
	GtkWidget	*ST_name;
	//todo: for stock account	
	//GtkWidget	*CY_type;
	GtkWidget	*ST_bank;
	GtkWidget	*ST_number;
	GtkWidget	*CM_budget;
	GtkWidget	*CM_closed;
	GtkWidget	*ST_initial;
	GtkWidget	*ST_minimum;
	GtkWidget	*ST_cheque1;
	GtkWidget	*ST_cheque2;

	GtkWidget	*BT_new, *BT_rem;

	gulong		handler_id[MAX_FIELD];

};

//todo: for stock account
//gchar *CYA_ACCOUNT_TYPE[] = { N_("Bank Account"), N_("Stocks Account"), NULL };


void defaccount_add			(GtkWidget *widget, gpointer user_data);
void defaccount_remove		(GtkWidget *widget, gpointer user_data);
void defaccount_update		(GtkWidget *widget, gpointer user_data);
void defaccount_get			(GtkWidget *widget, gpointer user_data);
void defaccount_set			(GtkWidget *widget, gpointer user_data);
gboolean defaccount_cleanup(struct defaccount_data *data, gint result);
void defaccount_setup(struct defaccount_data *data);
void defaccount_dispose(struct defaccount_data *data);




gboolean defaccount_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	defaccount_get(widget, user_data);
	return FALSE;
}

/*
** update the widgets status and contents from action/selection value
*/
void defaccount_update(GtkWidget *widget, gpointer user_data)
{
struct defaccount_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;
//todo: for stock account
//gboolean is_new;

	DB( g_printf("(defaccount) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), &model, &iter);

	DB( g_printf(" selected = %d\n", selected) );
	DB( g_printf(" action = %d\n", data->action) );

	//todo amiga/linux
	/*
	if(acc)
	{
	// check for archives related
		for(i=0;;i++)
		{
		struct Archive *arc;

			DoMethod(data->mwd->LV_arc, MUIM_List_GetEntry, i, &arc);
			if(!arc) break;
			if(arc->arc_Account == acc->acc_Id)
			{ nbarc++; break; }
		}

	// check for operation related
		for(i=0;;i++)
		{
		struct Operation *ope;

			DoMethod(data->mwd->LV_ope, MUIM_List_GetEntry, i, &ope);
			if(!ope) break;
			if(ope->ope_Account == acc->acc_Id)
			{ nbope++; break; }
		}
	}	*/

	//todo: for stock account
	//todo: lock type if oldpos!=0
/*
	if( selected )
	{
		gtk_tree_model_get(model, &iter,
			LST_DEFACC_NEW, &is_new,
			-1);
		gtk_widget_set_sensitive(data->CY_type, is_new);
	}
*/

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name, sensitive);
	gtk_widget_set_sensitive(data->ST_number, sensitive);
	gtk_widget_set_sensitive(data->ST_bank, sensitive);
	gtk_widget_set_sensitive(data->CM_budget, sensitive);
	gtk_widget_set_sensitive(data->CM_closed, sensitive);

	gtk_widget_set_sensitive(data->ST_initial, sensitive);
	gtk_widget_set_sensitive(data->ST_minimum, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque1, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque2, sensitive);


	//sensitive = (data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->LV_acc, sensitive);
	//gtk_widget_set_sensitive(data->BT_new, sensitive);

	sensitive = (selected == TRUE && data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	if(selected)
	{
		defaccount_set(widget, NULL);
	}


}


/*
** add an empty new account to our temp GList and treeview
*/
void defaccount_add(GtkWidget *widget, gpointer user_data)
{
struct defaccount_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;

Account *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defaccount) add (data=%x)\n", (guint)data) );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));

	item = da_account_malloc();
	item->name = g_strdup( _("(new account)"));

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		LST_DEFACC_DATAS, item,
		LST_DEFACC_OLDPOS, 0,
		//todo: for stock account		
		//LST_DEFACC_NEW, TRUE,
		-1);

	gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), &iter);

	data->change++;
}

/*
** remove the selected account to our treeview and temp GList
*/
void defaccount_remove(GtkWidget *widget, gpointer user_data)
{
struct defaccount_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(defaccount) remove (data=%x)\n", (guint)data) );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{

		//gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &entry, -1);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}

	data->change++;
}


/*
** get widgets contents to the selected account
*/
void defaccount_get(GtkWidget *widget, gpointer user_data)
{
struct defaccount_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gchar *txt;
gboolean bool;
gdouble value;

Account *item;

gint field = GPOINTER_TO_INT(user_data);

	DB( g_printf("(defaccount) get %d\n", field) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		data->change++;

		switch( field )
		{
			case FIELD_NAME:
				txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
				/* ignore if entry is empty */
				if (txt && *txt)
				{
					g_free(item->name);
					item->name = g_strdup(txt);

					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

				}
				break;

		//todo: for stock account			
		/*
			case FIELD_TYPE:
				item->type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));
				break;
		*/
			case FIELD_BANK:
				g_free(item->bankname);
				item->bankname = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_bank)));
				break;

			case FIELD_NUMBER:
				g_free(item->number);
				item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));
				break;

			case FIELD_BUDGET:
				item->flags &= ~(AF_BUDGET);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budget));
				if(bool) item->flags |= AF_BUDGET;
				break;

			case FIELD_CLOSED:
				item->flags &= ~(AF_CLOSED);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_closed));
				if(bool) item->flags |= AF_CLOSED;
				break;

			case FIELD_INITIAL:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
				item->initial = value;
				break;

			case FIELD_MINIMUM:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
				item->minimum = value;
				break;

			case FIELD_CHEQUE1:
				item->cheque1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque1));
				break;

			case FIELD_CHEQUE2:
				item->cheque2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque2));
				break;
		}
	}

}

/*
** set widgets contents from the selected account
*/
void defaccount_set(GtkWidget *widget, gpointer user_data)
{
struct defaccount_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

Account *item;

	DB( g_printf("(defaccount) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
		
		//todo: for stock account
		//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_type), item->type );
		
		if(item->bankname != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), item->bankname);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), "");

		if(item->number != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), item->number);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), "");

		g_signal_handler_block(data->CM_budget, data->handler_id[FIELD_BUDGET]);
		g_signal_handler_block(data->CM_closed, data->handler_id[FIELD_CLOSED]);

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_budget), item->flags & AF_BUDGET);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_closed), item->flags & AF_CLOSED);

		g_signal_handler_unblock(data->CM_budget, data->handler_id[FIELD_BUDGET]);
		g_signal_handler_unblock(data->CM_closed, data->handler_id[FIELD_CLOSED]);


		g_signal_handler_block(data->ST_initial, data->handler_id[FIELD_INITIAL]);
		g_signal_handler_block(data->ST_minimum, data->handler_id[FIELD_MINIMUM]);
		g_signal_handler_block(data->ST_cheque1, data->handler_id[FIELD_CHEQUE1]);
		g_signal_handler_block(data->ST_cheque2, data->handler_id[FIELD_CHEQUE2]);

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_initial), item->initial);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_minimum), item->minimum);

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque1), item->cheque1);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque2), item->cheque2);

		g_signal_handler_unblock(data->ST_initial, data->handler_id[FIELD_INITIAL]);
		g_signal_handler_unblock(data->ST_minimum, data->handler_id[FIELD_MINIMUM]);
		g_signal_handler_unblock(data->ST_cheque1, data->handler_id[FIELD_CHEQUE1]);
		g_signal_handler_unblock(data->ST_cheque2, data->handler_id[FIELD_CHEQUE2]);

	}

}


/*
**
*/
void defaccount_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	defaccount_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

//gint defaccount_list_sort(struct _Account *a, struct _Account *b) { return( a->acc_Id - b->acc_Id); }

/*
**
*/
gboolean defaccount_cleanup(struct defaccount_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defaccount) cleanup %x\n", (guint)data) );

	if(result == GTK_RESPONSE_ACCEPT)
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;
	gboolean valid;
	guint i, count;
	guint *pos_vector;

		//do_application_specific_something ();
		DB( g_printf(" accept\n") );

		//allocate vector pos
		count = g_list_length(GLOBALS->acc_list);
		pos_vector = g_new0(guint, count);

		DB( g_printf(" pos v=%x\n", (gint)pos_vector) );

		// test for change & store new position
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Account *item;
		gint oldpos;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFACC_DATAS, &item,
				LST_DEFACC_OLDPOS, &oldpos,
				-1);

			item->key = i;
			data->tmp_list = g_list_append(data->tmp_list, item);

			if(pos_vector)
			{
				if(oldpos != 0)	// added account have 0 has id and oldpos
				{
					pos_vector[oldpos] = i;
					if(doupdate == FALSE && oldpos != i) doupdate = TRUE;
				}
			}

			DB( g_print(" %2d : %s (%d->%d) %s\n", i, item->name, oldpos, i, doupdate==TRUE?"changed":"") );

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

#if MYDEBUG == 1
		if(pos_vector)
		{
			g_print("vector change:\n");
			for(i=0;i<g_list_length(GLOBALS->acc_list);i++)
			{
				g_print(" %d => %d\n", i, pos_vector[i]);
			}
		}
#endif

	/* test if we should update */
		if(doupdate == TRUE)
		{
		GList *list;
		gint tmpval;

			DB( g_printf(" we should update pos\n") );

			/* -> change all archives account & transfert */
			list = g_list_first(GLOBALS->arc_list);
			while (list != NULL)
			{
			Archive *entry = list->data;

				tmpval = entry->account;
				entry->account = pos_vector[tmpval];
				tmpval = entry->dst_account;
				entry->dst_account = pos_vector[tmpval];

				list = g_list_next(list);
			}

			/* -> all operations account & transfert */
			list = g_list_first(GLOBALS->ope_list);
			while (list != NULL)
			{
			Operation *entry = list->data;

				tmpval = entry->account;
				entry->account = pos_vector[tmpval];

				list = g_list_next(list);
			}


		}

		g_free(pos_vector);

		GLOBALS->change += data->change;

		//modify our GLOBAL acc_list
		da_account_destroy(GLOBALS->acc_list);
		GLOBALS->acc_list = data->tmp_list;
		data->tmp_list = NULL;

	}

	DB( g_printf(" free tmp_list\n") );

	da_account_destroy(data->tmp_list);

	return doupdate;
}

/*
**
*/
void defaccount_setup(struct defaccount_data *data)
{

	DB( g_printf("(defaccount) setup\n") );

	//init GList
	data->tmp_list = NULL; //hb_glist_clone_list(GLOBALS->acc_list, sizeof(struct _Account));
	data->action = 0;
	data->change = 0;

	populate_view_acc(data->LV_acc, GLOBALS->acc_list, TRUE);
}

/*
**
*/
GtkWidget *create_editaccount_window (void)
{
struct defaccount_data data;
GtkWidget *window, *mainbox;
GtkWidget *vbox, *table, *label, *entry1, *entry2, *entry3, *widget;
GtkWidget *spinner, *cheque1, *cheque2, *scrollwin;
GtkWidget *bbox, *check_button;
GtkWidget *alignment;
gint row;

	window = gtk_dialog_new_with_buttons (_("Edit Accounts"),
				//GTK_WINDOW (do_widget),
				NULL,
				0,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_REJECT,
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL);

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);

	//set the window icon
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "account.svg");

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(defaccount) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

	//window contents
	mainbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);



	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), vbox, FALSE, FALSE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);

	data.LV_acc = (GtkWidget *)defaccount_list_new(FALSE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data.LV_acc);

	// tools buttons
	bbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, TRUE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER (bbox), SP_BORDER);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), HB_BOX_SPACING);

	//data.BT_rem = gtk_button_new_with_mnemonic(_("_Remove"));
	data.BT_rem = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_rem);

	//data.BT_new = gtk_button_new_with_mnemonic(_("_New"));
	data.BT_new = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_new);





	//h_paned
	//vbox = gtk_vbox_new (FALSE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER (vbox), SP_BORDER);
	//gtk_box_pack_start (GTK_BOX (mainbox), vbox, TRUE, TRUE, 0);

	table = gtk_table_new (12, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.5, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (mainbox), alignment, TRUE, TRUE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Informations</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("_Heading:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry1 = make_string(label);
	data.ST_name = entry1;
	gtk_table_attach (GTK_TABLE (table), entry1, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//todo: for stock account
	/*	
	row++;
	label = make_label(_("_Type:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_ACCOUNT_TYPE);
	data.CY_type = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	*/

	row++;
	label = make_label(_("_Number:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry3 = make_string(label);
	data.ST_number = entry3;
	gtk_table_attach (GTK_TABLE (table), entry3, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Bank name:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1,(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry2 = make_string(label);
	data.ST_bank = entry2;
	gtk_table_attach (GTK_TABLE (table), entry2, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	check_button = gtk_check_button_new_with_mnemonic (_("Include in the budget"));
	data.CM_budget = check_button;
	gtk_table_attach (GTK_TABLE (table), check_button, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	check_button = gtk_check_button_new_with_mnemonic (_("This account was closed"));
	data.CM_closed = check_button;
	gtk_table_attach (GTK_TABLE (table), check_button, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);



//other


	//row = 0;
	row++;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Balances</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Initial:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	spinner = make_amount(label);
	data.ST_initial = spinner;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), spinner);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Overdrawn at:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	spinner = make_amount(label);
	data.ST_minimum = spinner;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), spinner);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


// frame 3
	row++;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Cheque number</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("Notebook _1:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	cheque1 = make_long (label);
	data.ST_cheque1 = cheque1;
	//hbox = gtk_hbox_new (FALSE, 0);
	//gtk_box_pack_start (GTK_BOX (hbox), cheque1, FALSE, FALSE, 0);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), cheque1);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("Notebook _2:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	cheque2 = make_long (label);
	data.ST_cheque2 = cheque2;
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.5, 0.33, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), cheque2);
	gtk_table_attach (GTK_TABLE (table), alignment, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);





	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_acc)), "changed", G_CALLBACK (defaccount_selection), NULL);

	// modify events
	g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (defaccount_get), (gpointer)FIELD_NAME);
	g_signal_connect (G_OBJECT (data.ST_name), "focus-out-event", G_CALLBACK (defaccount_focus_out), (gpointer)FIELD_NAME);

	//todo: for stock account	
	//g_signal_connect (G_OBJECT (data.CY_type), "changed", G_CALLBACK (defaccount_get), (gpointer)FIELD_TYPE);

	g_signal_connect (G_OBJECT (data.ST_bank), "activate", G_CALLBACK (defaccount_get), (gpointer)FIELD_BANK);
	g_signal_connect (G_OBJECT (data.ST_bank), "focus-out-event", G_CALLBACK (defaccount_focus_out), (gpointer)FIELD_BANK);

	g_signal_connect (G_OBJECT (data.ST_number), "activate", G_CALLBACK (defaccount_get), (gpointer)FIELD_NUMBER);
	g_signal_connect (G_OBJECT (data.ST_number), "focus-out-event", G_CALLBACK (defaccount_focus_out), (gpointer)FIELD_NUMBER);

	data.handler_id[FIELD_BUDGET] = g_signal_connect (data.CM_budget, "toggled", G_CALLBACK (defaccount_get), (gpointer)FIELD_BUDGET);
	data.handler_id[FIELD_CLOSED] = g_signal_connect (data.CM_closed, "toggled", G_CALLBACK (defaccount_get), (gpointer)FIELD_CLOSED);

	data.handler_id[FIELD_INITIAL] = g_signal_connect (data.ST_initial, "value-changed", G_CALLBACK (defaccount_get), (gpointer)FIELD_INITIAL);
	data.handler_id[FIELD_MINIMUM] = g_signal_connect (data.ST_minimum, "value-changed", G_CALLBACK (defaccount_get), (gpointer)FIELD_MINIMUM);
	data.handler_id[FIELD_CHEQUE1] = g_signal_connect (data.ST_cheque1, "value-changed", G_CALLBACK (defaccount_get), (gpointer)FIELD_CHEQUE1);
	data.handler_id[FIELD_CHEQUE2] = g_signal_connect (data.ST_cheque2, "value-changed", G_CALLBACK (defaccount_get), (gpointer)FIELD_CHEQUE2);

	g_signal_connect (G_OBJECT (data.BT_new), "clicked", G_CALLBACK (defaccount_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (defaccount_remove), NULL);

	//setup, init and show window
	defaccount_setup(&data);
	defaccount_update(data.LV_acc, NULL);

//	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	// cleanup and destroy
	defaccount_cleanup(&data, result);
	gtk_widget_destroy (window);

	return NULL;
}
