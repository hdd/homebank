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

#include "def_operation.h"

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

struct defoperation_data
{
	gint	action;
	gint	accnum;
	gint	type;

	Operation *ope;

	GtkWidget	*window;

	GtkWidget	*PO_date;
	GtkWidget	*PO_pay;
	GtkWidget	*PO_arc;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_amount;
	GtkWidget	*TX_symbol;
	GtkWidget	*CM_valid;
	GtkWidget	*CM_remind;
	GtkWidget	*CM_cheque;
	GtkWidget	*notebook;

	GtkWidget	*NU_mode;
	GtkWidget	*ST_info;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*PO_accto;

};

void defoperation_add			(GtkWidget *widget, gpointer user_data);
void defoperation_remove		(GtkWidget *widget, gpointer user_data);
void defoperation_update		(GtkWidget *widget, gpointer user_data);
void defoperation_get			(GtkWidget *widget, gpointer user_data);
void defoperation_set			(GtkWidget *widget, gpointer user_data);
void defoperation_actionstart	(GtkWidget *widget, gpointer action);
void defoperation_actionend		(GtkWidget *widget, gint response);
gboolean defoperation_cleanup(struct defoperation_data *data, gint result);
void defoperation_setup(struct defoperation_data *data);

extern gchar *CYA_TYPE[];

gchar *CYA_OPERATION[] = {
	N_("Add operation(s)"),
	N_("Inherit operation"),
	N_("Modify operation")
};

/*
**
*/
void defoperation_toggleamount(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gdouble value;

	DB( g_printf("(defoperation) toggleamount\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	value *= -1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value);
}



/*
**
*/
void defoperation_paymode(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gint payment;
gint page;

	DB( g_printf("(defoperation) paymode change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	/* todo: prefill the cheque number ? */
	if( data->type != OPERATION_EDIT_MODIFY )
	{
	gboolean expense = (gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount)) > 0 ? FALSE : TRUE);

		if(payment == PAYMODE_CHEQUE)
		{
			DB( g_printf(" -> cheque ") );


			if(expense == TRUE)
			{
			Account *acc;
			gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));
			guint cheque;
			gchar *cheque_str;

				DB( g_printf(" -> should fill cheque number for account %d ", active) );

				if( active != -1 )
				{
					acc = g_list_nth_data(GLOBALS->acc_list, active);
					cheque = ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque))==TRUE ? acc->cheque2 : acc->cheque1 );
					cheque_str = g_strdup_printf("%d", cheque + 1);
					gtk_entry_set_text(GTK_ENTRY(data->ST_info), cheque_str);
					g_free(cheque_str);
				}
			}

		}
	}


	if(payment == PAYMODE_CHEQUE)
		page = 1;
	if(payment == PAYMODE_PERSTRANSFERT)
		page = 2;

	DB( g_printf(" payment: %d, page: %d\n", payment, page) );

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), page);
}

/*
** update the widgets status and contents from action/selection value
*/
void defoperation_update(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gboolean sensitive, bool;

	DB( g_printf("(defoperation) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(defoperation) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//valid & remind are exclusive
	bool  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
	sensitive = bool ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_remind, sensitive);
	if(bool)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), 0);
}

/*
**
*/
void defoperation_fillfrom(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
Operation *entry;
Archive *arc;
gint n_arc;

	DB( g_printf("(defoperation) fill from\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	entry = data->ope;

	n_arc = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_arc));

	DB( g_printf(" fill from %d\n", n_arc) );

	if(n_arc > 0)
	{
		arc = g_list_nth_data(GLOBALS->arc_list, n_arc-1);

		//fill it
		entry->amount	= arc->amount;
		entry->account	= arc->account;
		entry->dst_account	= arc->dst_account;
		entry->paymode		= arc->paymode;
		entry->flags	= arc->flags;
		entry->payee	= arc->payee;
		entry->category	= arc->category;
		entry->wording =	g_strdup(arc->wording);
		entry->info = NULL;

		DB( g_printf(" calls\n") );

		defoperation_set(widget, NULL);
		defoperation_paymode(widget, NULL);
		defoperation_update(widget, NULL);

		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_arc), 0);
	}
}

/*
** set widgets contents from the selected account
*/
void defoperation_set(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
Operation *entry;
gchar *txt;

	DB( g_printf("(defoperation) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_printf(" -> ope=%8x data=%x\n", (gint)data->ope, data) );

	//DB( g_printf(" set date to %d\n", entry->date) );
	//g_object_set(GTK_DATE_ENTRY(data->PO_date), "date", (guint32)entry->ope_Date);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_date), (guint)entry->date);

	txt = (entry->wording != NULL) ? entry->wording : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_word), txt);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), entry->amount);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_amount), (entry->ope_Flags & OF_INCOME) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_valid), (entry->flags & OF_VALID) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), (entry->flags & OF_REMIND) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_cheque), (entry->flags & OF_CHEQ2) ? 1 : 0);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), entry->paymode);

	txt = (entry->info != NULL) ? entry->info : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), txt);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_grp), entry->category);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_pay), entry->payee);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), entry->account);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_accto), entry->dst_account);
}

/*
** get widgets contents to the selected account
*/
void defoperation_get(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
Operation *entry;
gchar *txt;
gdouble value;
gint active;

	DB( g_printf("(defoperation) get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	//DB( g_printf(" get date to %d\n", entry->ope_Date) );
	entry->date = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_date));
	//g_object_get(GTK_DATE_ENTRY(data->PO_date), "date", entry->ope_Date);

	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->wording = g_strdup(txt);
	}
	else
	{
		g_free(entry->wording);
		entry->wording = NULL;
	}
	

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	entry->amount = value;

	entry->flags &= (OF_AUTO|OF_ADDED);

	if(	data->type == OPERATION_EDIT_ADD || data->type == OPERATION_EDIT_INHERIT)
	entry->flags |= OF_ADDED;

	if(	data->type == OPERATION_EDIT_MODIFY)
	entry->flags |= OF_CHANGED;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
	if(active == 1) entry->flags |= OF_VALID;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_remind));
	if(active == 1) entry->flags |= OF_REMIND;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque));
	if(active == 1) entry->flags |= OF_CHEQ2;

	//active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_amount));
	active = entry->amount > 0 ? TRUE : FALSE;
	if(active == TRUE) entry->flags |= OF_INCOME;

	entry->paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));

	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->info = g_strdup(txt);
	}
	else
	{
		g_free(entry->info);
		entry->info = NULL;
	}

	entry->category   = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_grp));
	entry->payee   = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_pay));
	entry->account = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));
	entry->dst_account = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_accto));


	/* store a new cheque number into account ? */
	if( (entry->paymode == PAYMODE_CHEQUE) && !(entry->flags & OF_INCOME) )
	{
	Account *acc;
	guint cheque;

		/* get the active account and the corresponding cheque number */
		acc = g_list_nth_data(GLOBALS->acc_list, entry->account);
		cheque = atol(entry->info);

		DB( g_printf(" -> should store cheque number %d to %d", cheque, entry->account) );
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque)))
			acc->cheque2 = cheque;
		else
			acc->cheque1 = cheque;
	}

}



/*
** called from outside
*/
void defoperation_set_operation(GtkWidget *widget, Operation *ope)
{
struct defoperation_data *data;


	DB( g_printf("(defoperation) set operation\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->ope = ope;

	DB( g_printf(" -> ope=%8x data=%x\n", (gint)data->ope, data) );




/*
	if(data->type == 0)
	{
		DB( g_printf(" -> add mode %08x\n", ope) );

		memset(ope, 0, sizeof(Operation));
	// setup the account
		ope->ope_Account = data->accnum;
		ope->ope_To = data->accnum;
		ope->ope_Date = GLOBALS->today;
		//set(data->BT_add, MUIA_ShowMe, 1);
		//set(data->SP_space, MUIA_ShowMe, 0);
	}
	else
	{
		DB( g_printf(" -> edit/inherit mode\n") );

		if(data->type == 1)
			ope->ope_Date = GLOBALS->today;
		//set(data->BT_add, MUIA_ShowMe, 0);
		//set(data->SP_space, MUIA_ShowMe, 1);
	}
	*/

	DB( g_printf(" -> call init\n") );


	defoperation_set(widget, NULL);
	//defoperation_paymode(widget, NULL);
	defoperation_update(widget, NULL);

}

void defoperation_dispose(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;

	DB( g_printf("(defoperation) dispose\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_free(data);
}


/*
**
*/
gboolean defoperation_cleanup(struct defoperation_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defoperation) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		DB( g_printf(" we always update our glist\n") );

		defoperation_get(data->ST_word, NULL);

	}


	return doupdate;
}

/*
**
*/
void defoperation_setup(struct defoperation_data *data)
{

	DB( g_printf("(defoperation) setup\n") );

    gtk_window_set_title (GTK_WINDOW (data->window), _(CYA_OPERATION[data->type]));

	make_poparchive_populate(GTK_COMBO_BOX(data->PO_arc), GLOBALS->arc_list);
	make_poppayee_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->pay_list);
	make_popcategory_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->cat_list);
	make_popaccount_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->acc_list);
	make_popaccount_populate(GTK_COMBO_BOX(data->PO_accto), GLOBALS->acc_list);

}




GtkWidget *defoperation_make_block2(struct defoperation_data *data)
{
GtkWidget *table, *hbox, *label, *widget;
gint row;

	table = gtk_table_new (7, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General infos</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new_with_mnemonic (_("_Date:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = gtk_dateentry_new();
	data->PO_date = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Wording:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_word = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//row 3
	row++;
	label = gtk_label_new_with_mnemonic (_("_Amount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	widget = make_amount(label);
	data->ST_amount = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("+/-");
	data->BT_amount = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("A_ccount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_popaccount(label);
	data->PO_acc = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Validated"));
	data->CM_valid = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Remind"));
	data->CM_remind = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 3, row, row+1);

	return table;
}

GtkWidget *defoperation_make_block3(struct defoperation_data *data)
{
GtkWidget *table, *hbox, *label, *widget, *notebook;
gint row;

	table = gtk_table_new (6, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Optional infos</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new_with_mnemonic (_("Pay_ment:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_paymode(label);
	data->NU_mode = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
	data->notebook = notebook;
	gtk_table_attach_defaults (GTK_TABLE (table), notebook, 2, 3, row, row+1);

		label = gtk_label_new(NULL);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), label, NULL);

		hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, NULL);
		widget = gtk_check_button_new_with_mnemonic(_("Of notebook _2"));
		data->CM_cheque = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, NULL);
		label = make_label(_("_To account:"), 0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
		widget = make_popaccount(label);
		data->PO_accto = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Info:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_info = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_poppayee(label);
	data->PO_pay = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_popcategory(label);
	data->PO_grp = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	return table;
}

gboolean defoperation_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{

	return FALSE;
}

// the window creation
GtkWidget *create_defoperation_window (Operation *ope, gint type, gint accnum)
{
struct defoperation_data *data;
GtkWidget *window, *hbox, *mainbox, *table, *label, *widget;
GtkWidget *alignment;

	data = g_malloc0(sizeof(struct defoperation_data));
	if(!data) return NULL;

	//parentwindow = gtk_widget_get_ancestor(treeview, GTK_TYPE_WINDOW);

      window = gtk_dialog_new_with_buttons (NULL,
					    /*GTK_WINDOW (parentwindow), */ NULL,
					    0,
					    NULL,
					    NULL);

	if(type == OPERATION_EDIT_MODIFY)
	{
		gtk_dialog_add_buttons (GTK_DIALOG(window),
		    GTK_STOCK_CANCEL,
		    GTK_RESPONSE_REJECT,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
		NULL);

		//set the window icon
		gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/ope_edit.svg", NULL);
	}
	else
	{
		gtk_dialog_add_buttons (GTK_DIALOG(window),
		    GTK_STOCK_CANCEL,
		    GTK_RESPONSE_REJECT,
		    GTK_STOCK_ADD,
		    GTK_RESPONSE_ADD,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
		NULL);

		//set the window icon
		gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/ope_add.svg", NULL);
	}

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);

	//gtk_window_set_icon_from_file(GTK_WINDOW (window), "./pixmaps/archive.png", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(defoperation) window=%08lx, inst_data=%08lx\n", window, data) );

	data->window = window;

	data->ope = ope;
	data->accnum = accnum;
	data->type = type;

	//window contents
	mainbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);

	// parameters
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	// frame 2
	table = defoperation_make_block2(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);


	// frame 3
	table = defoperation_make_block3(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);

	//fill from
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

	label = make_label(_("_Fill with archive:"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	widget = make_poparchive(label);
	data->PO_arc = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	//connect all our signals
	// connect our dispose function
    //g_signal_connect (window, "delete-event", G_CALLBACK (defoperation_delete), (gpointer)data);

	//g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	//g_signal_connect (data->LV_arc, "cursor-changed", G_CALLBACK (defoperation_update), NULL);
	//g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (defoperation_toto), treeview);

	g_signal_connect (G_OBJECT (data->BT_amount), "clicked", G_CALLBACK (defoperation_toggleamount), NULL);

	g_signal_connect (data->NU_mode, "changed", G_CALLBACK (defoperation_paymode), NULL);

	g_signal_connect (data->CM_valid, "toggled", G_CALLBACK (defoperation_update), NULL);

	g_signal_connect (data->PO_arc, "changed", G_CALLBACK (defoperation_fillfrom), NULL);



	//setup, init and show window

	defoperation_setup(data);

	gtk_widget_show_all (window);

/*

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	DB( g_printf("result=%d", result) );

	switch (result)
    {
	case GTK_RESPONSE_ACCEPT:

	   break;
    }


		// cleanup and destroy
		defoperation_cleanup(&data, result);
		gtk_widget_destroy (window);


	return result;
*/
	return window;
}
