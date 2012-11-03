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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty ofdefoperation_amountchanged
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "def_operation.h"
#include "dsp_account.h"
#include "hb_transaction.h"
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
extern struct Preferences *PREFS;

enum {
	HID_AMOUNT,
	MAX_HID
};

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
	GtkWidget	*ST_tags;

	gulong		handler_id[MAX_HID];
};

static void defoperation_update		(GtkWidget *widget, gpointer user_data);
//static gboolean defoperation_cleanup(struct defoperation_data *data, gint result);
static void defoperation_setup(struct defoperation_data *data);
static void defoperation_set(GtkWidget *widget, gpointer user_data);
static void defoperation_update_transfer(GtkWidget *widget, gpointer user_data);

extern gchar *CYA_TYPE[];

gchar *CYA_OPERATION[] = {
	N_("Add transaction"),
	N_("Inherit transaction"),
	N_("Modify transaction")
};


/*
** set widgets contents from the selected account
*/
static void defoperation_set(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
Operation *entry;
gchar *tagstr, *txt;

	DB( g_printf("(ui_operation) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_printf(" -> ope=%8x data=%x\n", (gint)data->ope, data) );

	DB( g_printf(" -> tags at: '%x'\n", entry->tags) );


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

	txt = (entry->info != NULL) ? entry->info : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), txt);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_grp), entry->category);
	ui_pay_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_pay), entry->payee);

	tagstr = transaction_get_tagstring(entry);

	DB( g_print(" -> tags: '%s'\n", txt) );

	txt = (tagstr != NULL) ? tagstr : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_tags), txt);
	g_free(tagstr);

	//as we trigger an event on this
	//let's place it at the end to avoid misvalue on the trigger function
	
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_acc), entry->account);
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_accto), entry->dst_account);
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), entry->paymode);
	
	DB( g_print(" -> acc is: %d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc)) ) );
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

	DB( g_printf("(ui_operation) get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_printf(" -> ope = %x\n", entry) );

	//DB( g_printf(" get date to %d\n", entry->ope_Date) );
	entry->date = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_date));
	//g_object_get(GTK_DATE_ENTRY(data->PO_date), "date", entry->ope_Date);

	//free any previous string
	if(	entry->wording )
	{
		g_free(entry->wording);
		entry->wording = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->wording = g_strdup(txt);
	}

	entry->paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	entry->amount = value;

	/* for internal transfer add, amount must be expense */
	// #617936
	/*
	if( entry->paymode == PAYMODE_INTXFER && data->type == OPERATION_EDIT_ADD )
	{
		if( entry->amount > 0 )
			entry->amount *= -1;
	}
	*/

	//free any previous string
	if(	entry->info )
	{
		g_free(entry->info);
		entry->info = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->info = g_strdup(txt);
	}

	entry->category    = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_grp));
	entry->payee       = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(data->PO_pay));
	entry->account     = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
	entry->dst_account = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_accto));

	/* tags */
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tags));
	DB( g_print(" -> tags: '%s'\n", txt) );
	transaction_set_tags(entry, txt);

	/* flags */
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

}



/*
** called from outside
*/
void defoperation_set_operation(GtkWidget *widget, Operation *ope)
{
struct defoperation_data *data;


	DB( g_printf("(ui_operation) set out operation\n") );

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

static gboolean defoperation_amount_focusout(GtkWidget     *widget,
                                                        GdkEventFocus *event,
                                                        gpointer       user_data)
{
struct defoperation_data *data;
gushort paymode;
gdouble amount;
	
	DB( g_print("(ui_operation) amount focus-out-event %d\n", gtk_widget_is_focus(widget)) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));

	// for internal transfer add, amount must be expense by default
	if( paymode == PAYMODE_INTXFER && data->type == OPERATION_EDIT_ADD )
	{
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if(amount > 0)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);		
	}

	return FALSE;
}

/*
**
*/
static void defoperation_toggleamount(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gdouble value;

	DB( g_printf("(ui_operation) toggleamount\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	value *= -1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value);
}



/*
**
*/
static void defoperation_paymode(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gint payment;
gint page;

	DB( g_printf("(ui_operation) paymode change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	/* todo: prefill the cheque number ? */
	if( data->type != OPERATION_EDIT_MODIFY )
	{
	gboolean expense = (gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount)) > 0 ? FALSE : TRUE);

		DB( g_print(" -> payment: %d\n", PAYMODE_CHECK) );
		DB( g_print(" -> expense: %d\n", expense) );
		DB( g_print(" -> acc is: %d\n", ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc)) ) );

		if(payment == PAYMODE_CHECK)
		{


			if(expense == TRUE)
			{
			Account *acc;
			gint active = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
			guint cheque;
			gchar *cheque_str;

				DB( g_printf(" -> should fill cheque number for account %d\n", active) );

				if( active != -1 )
				{
					acc = da_acc_get( active );
					cheque = ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque))==TRUE ? acc->cheque2 : acc->cheque1 );
					cheque_str = g_strdup_printf("%d", cheque + 1);
					gtk_entry_set_text(GTK_ENTRY(data->ST_info), cheque_str);
					g_free(cheque_str);
				}
			}

		}
	}


	if(payment == PAYMODE_CHECK)
		page = 1;
	
	if(payment == PAYMODE_INTXFER)
	{
		page = 2;
			// for internal transfer add, amount must be expense by default
		if( data->type == OPERATION_EDIT_ADD )
		{
			gdouble amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
			if(amount > 0)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);		
		}

	}

	/*
	if( payment == PAYMODE_INTXFER && data->type == OPERATION_EDIT_ADD )
	{
		// #617936 : for internal trn: value must be seized > 0
		gtk_spin_button_set_range(data->ST_amount, 0, G_MAXDOUBLE);
		gtk_widget_set_sensitive(data->BT_amount, FALSE);
	}
	else
	{
		gtk_spin_button_set_range(data->ST_amount, -G_MAXDOUBLE, G_MAXDOUBLE);
		gtk_widget_set_sensitive(data->BT_amount, TRUE);
	}
*/
		
	DB( g_printf(" payment: %d, page: %d\n", payment, page) );

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), page);
	
	defoperation_update_transfer(widget, user_data);
}

static void defoperation_update_accto(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
guint kacc;

	DB( g_printf("(ui_operation) update accto\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));

	DB( g_printf(" acc is %d\n", kacc) );


	ui_acc_comboboxentry_populate_except(GTK_COMBO_BOX_ENTRY(data->PO_accto), GLOBALS->h_acc, kacc);

	defoperation_update_transfer(widget, user_data);
}



static void defoperation_update_transfer(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gboolean sensitive;
	guint kacc, kdst;

	DB( g_printf("(ui_operation) update transfer\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	sensitive = TRUE;

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));

	if(kacc == 0) sensitive = FALSE;

	/* coherent seizure */
	if( gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode)) == PAYMODE_INTXFER )
	{

		kdst = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_accto));
	
		//sensitive = kdst > 0 ? (kacc == kdst ? FALSE : TRUE) : FALSE;
		if(kdst == 0) sensitive = FALSE;
		if(kdst == kacc) sensitive = FALSE;
		
		DB( g_printf(" sensitive %d\n", sensitive) );
		
		
	}	

	gtk_widget_set_sensitive(GTK_DIALOG(data->window)->action_area, sensitive);

}


/*
** update the widgets status and contents from action/selection value
*/
static void defoperation_update(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
gboolean sensitive, bool;

	DB( g_printf("(ui_operation) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_printf("(ui_operation) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//valid & remind are exclusive
	bool  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
	sensitive = bool ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_remind, sensitive);
	if(bool)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), 0);

	/* transfer integrity */
	//todo: see bug with transfer
/*
	sensitive = TRUE;
	if( data->type == OPERATION_EDIT_MODIFY && data->ope !=NULL )
	{
		if( data->ope->paymode == PAYMODE_INTXFER )
		{
			sensitive = FALSE;
		}
	
	}
	gtk_widget_set_sensitive(data->PO_acc, sensitive);
	gtk_widget_set_sensitive(data->PO_accto, sensitive);
	gtk_widget_set_sensitive(data->NU_mode, sensitive);
*/
	 
}

/*
**
*/
static void defoperation_fillfrom(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;
Operation *entry;
Archive *arc;
gint n_arc;

	DB( g_printf("(ui_operation) fill from\n") );

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


void defoperation_dispose(GtkWidget *widget, gpointer user_data)
{
struct defoperation_data *data;

	DB( g_printf("(ui_operation) dispose\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_free(data);
}


/*
**
*/
/*
static gboolean defoperation_cleanup(struct defoperation_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(ui_operation) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		DB( g_printf(" we always update our glist\n") );

		defoperation_get(data->ST_word, NULL);

	}


	return doupdate;
}
*/

/*
**
*/
static void defoperation_setup(struct defoperation_data *data)
{

	DB( g_printf("(ui_operation) setup\n") );

    gtk_window_set_title (GTK_WINDOW (data->window), _(CYA_OPERATION[data->type]));

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_pay), GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_grp), GLOBALS->h_cat);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_acc), GLOBALS->h_acc);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_accto), GLOBALS->h_acc);

	if( data->type != OPERATION_EDIT_MODIFY )
		make_poparchive_populate(GTK_COMBO_BOX(data->PO_arc), GLOBALS->arc_list);

}




static GtkWidget *defoperation_make_block1(struct defoperation_data *data)
{
GtkWidget *table, *hbox, *label, *widget, *notebook;
gint row;

	table = gtk_table_new (7, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General information</b>"));
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

	gtk_widget_set_tooltip_text(widget, _("Date accepted here are:\nday,\nday/month or month/day,\nand complete date into your locale"));


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
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_pay_comboboxentry_new(label);
	data->PO_pay = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Payee"));

	row++;
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_cat_comboboxentry_new(label);
	data->PO_grp = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Category"));

		row++;
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
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		widget = ui_acc_comboboxentry_new(label);
		data->PO_accto = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	
	return table;
}

static GtkWidget *defoperation_make_block2(struct defoperation_data *data)
{
GtkWidget *table, *label, *widget;
gint row;

	table = gtk_table_new (6, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Optional information</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	label = gtk_label_new_with_mnemonic (_("_Description:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	//widget = make_string(label);
	widget = make_memo_entry(label);
	data->ST_word = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Info:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_info = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("_Tags:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_tags = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	label = gtk_label_new_with_mnemonic (_("A_ccount:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);
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

/*
gboolean defoperation_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{

	return FALSE;
}
*/

// the window creation
GtkWidget *create_defoperation_window (GtkWindow *parent, Operation *ope, gint type, gint accnum)
{
struct defoperation_data *data;
GtkWidget *window, *hbox, *mainbox, *table, *label, *widget;
GtkWidget *alignment;

	DB( g_printf("(ui_operation) new\n") );


	data = g_malloc0(sizeof(struct defoperation_data));
	if(!data) return NULL;

	//parentwindow = gtk_widget_get_ancestor(treeview, GTK_TYPE_WINDOW);

      window = gtk_dialog_new_with_buttons (NULL,
					    GTK_WINDOW (parent),
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
	}
	else
	{
		gtk_dialog_add_buttons (GTK_DIALOG(window),
			GTK_STOCK_CLOSE,
		    GTK_RESPONSE_REJECT,
		    GTK_STOCK_ADD,
		    GTK_RESPONSE_ADD,
		NULL);
	}

	switch(type)
	{
		case OPERATION_EDIT_ADD:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_add.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_ADD);
			break;
		case OPERATION_EDIT_INHERIT:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_herit.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_HERIT);
			break;
		case OPERATION_EDIT_MODIFY:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_edit.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_EDIT);
			break;
	}


	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf(" -> window=%08lx, inst_data=%08lx\n", window, data) );

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
	table = defoperation_make_block1(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);


	// frame 3
	table = defoperation_make_block2(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.0, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);

	//fill from
	if( type != OPERATION_EDIT_MODIFY )
	{
		hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
		gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

		label = make_label(_("_Fill with archive:"), 0, 0.5);
		widget = make_poparchive(label);
		data->PO_arc = widget;
		gtk_box_pack_end (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

		g_signal_connect (data->PO_arc, "changed", G_CALLBACK (defoperation_fillfrom), NULL);
	}

	//connect all our signals
	// connect our dispose function
    //g_signal_connect (window, "delete-event", G_CALLBACK (defoperation_delete), (gpointer)data);

	//g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	//g_signal_connect (data->LV_arc, "cursor-changed", G_CALLBACK (defoperation_update), NULL);
	//g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (defoperation_toto), treeview);

	g_signal_connect (GTK_OBJECT (data->ST_amount), "focus-out-event",
				G_CALLBACK (defoperation_amount_focusout), data);

	
	
	g_signal_connect (G_OBJECT (data->BT_amount), "clicked", G_CALLBACK (defoperation_toggleamount), NULL);

	g_signal_connect (data->NU_mode, "changed", G_CALLBACK (defoperation_paymode), NULL);

	g_signal_connect (data->CM_cheque, "toggled", G_CALLBACK (defoperation_paymode), NULL);

	g_signal_connect (data->CM_valid, "toggled", G_CALLBACK (defoperation_update), NULL);

	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (defoperation_update_accto), NULL);


	g_signal_connect (data->PO_accto, "changed", G_CALLBACK (defoperation_update_transfer), NULL);

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
