/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2010 Maxime DOYEN
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

#include "def_wallet.h"
#include "ui_category.h"

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


struct defwallet_data
{
	GtkWidget	*ST_owner;
	GtkWidget	*PO_grp;
	GtkWidget	*NU_arc;
	gint		change;
};


/*
** get widgets contents from the selected account
*/
static void defwallet_get(GtkWidget *widget, gpointer user_data)
{
struct defwallet_data *data;
gchar	*owner;
gint	car;
gint	days;

	DB( g_printf("(defwallet) get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// get values
	owner = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_owner));
	car   = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_grp));
	days  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NU_arc));

	// check for changes
	if(strcasecmp(owner, GLOBALS->title)) data->change++;
	if(car != GLOBALS->car_category) data->change++;
	if(days != GLOBALS->auto_nbdays) data->change++;

	// update
	if (owner && *owner)
	{
		g_free(GLOBALS->title);
		GLOBALS->title = g_strdup(owner);
	}
	GLOBALS->car_category = car;
	GLOBALS->auto_nbdays  = days;

	DB( g_printf(" -> owner %s\n", GLOBALS->title) );
	DB( g_printf(" -> ccgrp %d\n", GLOBALS->car_category) );
	DB( g_printf(" -> autoinsert %d\n", GLOBALS->auto_nbdays) );

}



/*
** set widgets contents from the selected account
*/
static void defwallet_set(GtkWidget *widget, gpointer user_data)
{
struct defwallet_data *data;

	DB( g_printf("(defwallet) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_printf(" -> ccgrp %d\n", GLOBALS->car_category) );
	DB( g_printf(" -> autoinsert %d\n", GLOBALS->auto_nbdays) );



	if(GLOBALS->title) gtk_entry_set_text(GTK_ENTRY(data->ST_owner), GLOBALS->title);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_grp), GLOBALS->car_category);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NU_arc), GLOBALS->auto_nbdays);


}

/*
**
*/
static gboolean defwallet_cleanup(struct defwallet_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_printf("(defwallet) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		defwallet_get(data->ST_owner, NULL);


		DB( g_printf(" -> GLOBAL change = %d\n", GLOBALS->change) );

		DB( g_printf(" -> we update, change = %d\n", data->change) );


		GLOBALS->change += data->change;
	}
	return doupdate;
}

/*
**
*/
static void defwallet_setup(struct defwallet_data *data)
{
	DB( g_printf("(defwallet) setup\n") );

	data->change = 0;

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_grp), GLOBALS->h_cat);

	defwallet_set(data->ST_owner, NULL);

}



// the window creation
GtkWidget *create_defwallet_window (void)
{
struct defwallet_data data;
GtkWidget *window, *mainvbox, *table, *hbox;
GtkWidget *label, *entry, *combo, *spinner;
GtkWidget *alignment;
gint row;

	window = gtk_dialog_new_with_buttons (_("Wallet properties"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_REJECT,
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_printf("(defaccount) window=%08lx, inst_data=%08lx\n", window, &data) );

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);

	gtk_window_set_icon_name(GTK_WINDOW (window), GTK_STOCK_PROPERTIES);

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

    table = gtk_table_new (6, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.5, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_container_add (GTK_CONTAINER (mainvbox), alignment);

// part 1
	row = 0;
	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("_Owner:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry = make_string(label);
	data.ST_owner = entry;
	gtk_table_attach (GTK_TABLE (table), entry, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

// frame 2
	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Automatic transactions</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);


	label = make_label(_("_Insert:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	spinner = make_numeric(label, 0, 100);
	data.NU_arc = spinner;
    gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);
	//TRANSLATORS: there is a spinner on the left of this label, and so you have 0....x days into the future
	label = make_label(_("days into the future"), 1, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

// frame 3
	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Car cost</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Category:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	combo = ui_cat_comboboxentry_new(label);
	data.PO_grp = combo;
	gtk_table_attach (GTK_TABLE (table), combo, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	//setup, init and show window
	defwallet_setup(&data);
	//defwallet_update(data.LV_arc, NULL);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	// cleanup and destroy
	defwallet_cleanup(&data, result);
	gtk_widget_destroy (window);

	return window;
}
