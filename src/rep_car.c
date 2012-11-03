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

#include "rep_car.h"

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
	HID_MINDATE,
	HID_MAXDATE,
	HID_MONTH,
	HID_YEAR,
	HID_RANGE,
	HID_CAR,
	MAX_HID
};

enum {
	CAR_RES_METER = 1,
	CAR_RES_FUEL,
	CAR_RES_FUELCOST,
	CAR_RES_OTHERCOST,
	CAR_RES_TOTALCOST,
	MAX_CAR_RES
};


struct repcar_data
{
	GtkWidget	*window;

	GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*LV_report;
	GtkWidget	*PO_cat;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*LA_avera[MAX_CAR_RES];
	GtkWidget	*LA_total[MAX_CAR_RES];

	GList		*car_list;

	guint		total_dist;
	gdouble		total_fuel;
	gdouble		total_fuelcost;
	gdouble		total_misccost;


	guint32		mindate, maxdate;

	gulong		handler_id[MAX_HID];
};

extern gchar *CYA_RANGE[];
extern gchar *CYA_SELECT[];

/* list stat */
enum
{
	LST_CAR_DATE,
	LST_CAR_WORDING,
	LST_CAR_METER,
	LST_CAR_FUEL,
	LST_CAR_PRICE,
	LST_CAR_AMOUNT,
	LST_CAR_DIST,
	LST_CAR_100KM,
	NUM_LST_CAR
};

/* prototypes */
void repcar_date_change(GtkWidget *widget, gpointer user_data);
void repcar_period_change(GtkWidget *widget, gpointer user_data);
void repcar_range_change(GtkWidget *widget, gpointer user_data);
void repcar_compute(GtkWidget *widget, gpointer user_data);
void repcar_update(GtkWidget *widget, gpointer user_data);
void repcar_toggle_minor(GtkWidget *widget, gpointer user_data);
void repcar_setup(struct repcar_data *data);
gboolean repcar_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data);
GtkWidget *create_list_repcar(void);


void repcar_date_change(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;

	DB( g_print("(repcar) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	repcar_compute(widget, NULL);

}


void repcar_period_change(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;
gint month, year;

	DB( g_print("(repcar) period change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

/*
	data->flt.flt_option[FLT_DATE] = 1;
*/

	month = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_month));
	year = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_year));

	DB( g_print(" month=%d, year=%d\n", month, year) );


	if(month != 0)
		get_period_minmax(month-1, year, &data->mindate, &data->maxdate);
	else
		get_period_minmax(0, year, &data->mindate, &data->maxdate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), 0);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);

	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->mindate);
	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->maxdate);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

	repcar_compute(widget, NULL);

}

void repcar_range_change(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(repcar) range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

/*
	data->flt.flt_option[FLT_DATE] = 1;
*/

	if(g_list_length(GLOBALS->ope_list) == 0) return;

	//get our min max date
	GLOBALS->ope_list = da_operation_sort(GLOBALS->ope_list);
	list = g_list_first(GLOBALS->ope_list);
	data->mindate = ((Operation *)list->data)->date;
	list = g_list_last(GLOBALS->ope_list);
	data->maxdate   = ((Operation *)list->data)->date;

	refdate = GLOBALS->today;
	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != 0)
	{
		if(range > 0)
			get_range_minmax(refdate, range-1, &data->mindate, &data->maxdate);

		/* update the year */
		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		date = g_date_new_julian(data->maxdate);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), g_date_get_year(date));
		g_date_free(date);
		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

		g_signal_handler_block(data->CY_month, data->handler_id[HID_MONTH]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_month), 0);
		g_signal_handler_unblock(data->CY_month, data->handler_id[HID_MONTH]);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->mindate);
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->maxdate);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

		repcar_compute(widget, NULL);
	}
}


void repcar_compute(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;
GList *list;
gint catkey, nb_refuel;

	DB( g_print("(repcar) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* do nothing if no operation */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	// get the account key
	catkey = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_cat));

	// clear the glist
	da_carcost_destroy(data->car_list);
	data->car_list = NULL;

	nb_refuel = 0;
	data->total_misccost = 0;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *ope = list->data;
    CarCost *item, *previtem;
	gchar *d, *v;
	gint len;

		if( (ope->category == catkey) && !(ope->flags & OF_REMIND) )
		{
			len = strlen(ope->wording);
			d = g_strstr_len(ope->wording, len, "d=");
			v = g_strstr_len(ope->wording, len, "v=");

			if(d && v)
			{
		    	item = da_carcost_malloc();

				if(item)
				{
			    	item->ope	= ope;
			    	item->meter	= atol(d+2);
			    	item->fuel	= g_strtod(v+2, NULL);
			    	item->dist	= 0;

			    	data->car_list = g_list_append(data->car_list, item);
					/*
					if(d[-1] == '(') d--;
					if(d[-1] == ' ') d--;
					len = (UWORD)d - (UWORD)&ope->ope_Word;
					strncpy(ac.ac_Word, ope->ope_Word, len);
					ac.ac_Word[len] = 0;
					*/

					DB( g_print("store refuel d=%d v=%.2f $%.2f\n", item->meter, item->fuel, item->ope->amount) );

					if(nb_refuel > 0 )
					{
				    	previtem = g_list_nth_data(data->car_list, nb_refuel-1);
						if(previtem != NULL) previtem->dist = item->meter - previtem->meter;
					}

					nb_refuel++;
				}
			}
			else
			{
				data->total_misccost += ope->amount;
			}
		}
		list = g_list_next(list);
	}

	repcar_update(widget, NULL);
}

void repcar_update(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gchar *buf;

	DB( g_print("(repcar) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// clear and detach our model
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_object_ref(model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL);

	data->total_fuelcost = 0;
	data->total_fuel	 = 0;
	data->total_dist	 = 0;

	list = g_list_first(data->car_list);
	while (list != NULL)
	{
	CarCost *item = list->data;
	gint dist;
	gdouble centkm;

		if( (item->ope->date >= data->mindate) && (item->ope->date <= data->maxdate) )
		{
	    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

	    	dist = item->dist;
	    	centkm = item->dist != 0 ? item->fuel * 100 / item->dist : 0;

	    	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_CAR_DATE    , item->ope->date,
				LST_CAR_WORDING , item->ope->wording,
				LST_CAR_METER   , item->meter,
				LST_CAR_FUEL    , item->fuel,
				LST_CAR_PRICE   , ABS(item->ope->amount) / item->fuel,
				LST_CAR_AMOUNT  , item->ope->amount,
				LST_CAR_DIST    , dist,
				LST_CAR_100KM   , centkm,
				-1);

			DB( g_print("insert d=%d v=%.2f $%.2f %d %.2f\n", item->meter, item->fuel, item->ope->amount, dist, centkm) );

			if(item->dist)
			{
				data->total_fuelcost += item->ope->amount;
				data->total_fuel     += item->fuel;
				data->total_dist     += item->dist;
			}

			//todo
			//data->othercost :: problem not filtered;


		}
		list = g_list_next(list);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
	g_object_unref(model);


	gdouble coef = data->total_dist ? 100 / (gdouble)data->total_dist : 0;
	gboolean minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


		DB( g_print(" coef = 100 / %.2f = %.2f\n", (gdouble)data->total_dist, coef) );

		// row 1 is for 100km
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][1]), "1:1");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][1]), "2:1");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][1]), "3:1");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][1]), "4:1");	//Total cost
		*/

		// 100km fuel
		buf = g_strdup_printf("%.2f L", data->total_fuel * coef);
		gtk_label_set_text(GTK_LABEL(data->LA_avera[CAR_RES_FUEL]), buf);
		g_free(buf);

		// 100km fuelcost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_FUELCOST]), data->total_fuelcost * coef, minor);

		// 100km other cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_OTHERCOST]), data->total_misccost * coef, minor);

		// 100km cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_TOTALCOST]), (data->total_fuelcost + data->total_misccost) * coef, minor);


		// row 2 is for total
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][2]), "1:2");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][2]), "2:2");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][2]), "3:2");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][2]), "4:2");	//Total
		*/

		// total distance
		buf = g_strdup_printf("%d Km", data->total_dist);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_METER]), buf);
		g_free(buf);

		// total fuel
		buf = g_strdup_printf("%.2f L", data->total_fuel);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_FUEL]), buf);
		g_free(buf);

		// total fuelcost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_FUELCOST]), data->total_fuelcost, minor);

		// total other cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_OTHERCOST]), data->total_misccost, minor);

		// total cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_TOTALCOST]), data->total_fuelcost + data->total_misccost, minor);


}

void repcar_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct repcar_data *data;

	DB( g_print("(repcar) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repcar_update(widget, NULL);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));


	/*
	statistic_update_total(widget,NULL);

	//wallet_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_set_minor(GTK_CHART(data->RE_bar), minor);
	gtk_chart_set_minor(GTK_CHART(data->RE_pie), minor);
	*/

}

/*
**
*/
void repcar_setup(struct repcar_data *data)
{
	DB( g_print("(repcar) setup\n") );

	data->car_list = NULL;

	/* if ope get date bounds */
	if(g_list_length(GLOBALS->ope_list) > 0)
	{
	GList *list;

		//get our min max date
		da_operation_sort(GLOBALS->ope_list);
		list = g_list_first(GLOBALS->ope_list);
		data->mindate = ((Operation *)list->data)->date;
		list = g_list_last(GLOBALS->ope_list);
		data->maxdate   = ((Operation *)list->data)->date;

		/*  */
		GDate *date;
		gdouble min,max;

		date = g_date_new_julian(data->mindate);
		min = g_date_get_year(date);
		g_date_set_julian(date, data->maxdate);
		max = g_date_get_year(date);
		g_date_free(date);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->mindate);
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->maxdate);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->NB_year), min, max);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), max);
		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

	}

	make_popcategory_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->cat_list);

	g_signal_handler_block(data->PO_cat, data->handler_id[HID_CAR]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_cat), GLOBALS->car_category);
	g_signal_handler_unblock(data->PO_cat, data->handler_id[HID_CAR]);

}


/*
**
*/
gboolean repcar_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repcar_data *data = user_data;

	DB( g_print("(repcar) dispose\n") );

	da_carcost_destroy(data->car_list);

	g_free(data);

	//enable define windows
	GLOBALS->define_off--;
	wallet_update(GLOBALS->mainwindow, (gpointer)2);

	return FALSE;
}


// the window creation
GtkWidget *repcar_window_new(void)
{
struct repcar_data *data;
GtkWidget *window, *mainvbox, *hbox, *vbox, *treeview;
GtkWidget *label, *widget, *table, *alignment;
gint row, col;

	data = g_malloc0(sizeof(struct repcar_data));
	if(!data) return NULL;

	DB( g_print("(repcar) new\n") );

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, (gpointer)2);

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Car report"));

	//set the window icon
	gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/report_car.svg", NULL);



	//window contents
	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//control part
	table = gtk_table_new (6, 2, FALSE);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
    gtk_box_pack_start (GTK_BOX (hbox), alignment, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Display</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, row, row+1);

	row++;
	label = make_label(_("_Car category:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	widget = make_popcategory(label);
	data->PO_cat = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Minor currency"));
	data->CM_minor = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_hseparator_new();
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, row, row+1);

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Date filter</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, row, row+1);

	row++;
	label = make_label(_("_From:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->PO_mindate = gtk_dateentry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), data->PO_mindate, 1, 2, row, row+1);


	row++;
	label = make_label(_("_To:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->PO_maxdate = gtk_dateentry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), data->PO_maxdate, 1, 2, row, row+1);

	row++;
	label = make_label(_("_Range:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->CY_range = make_cycle(label, CYA_RANGE);
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_range, 1, 2, row, row+1);

	row++;
	label = make_label(_("_Month:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->CY_month = make_cycle(label, CYA_SELECT);
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_month, 1, 2, row, row+1);

	row++;
	label = make_label(_("_Year:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->NB_year = make_year(label);
	gtk_table_attach_defaults (GTK_TABLE (table), data->NB_year, 1, 2, row, row+1);


	//part: info + report
	vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	//toobar
	//toolbar = create_repcar_toolbar(data);
    //gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);

	//infos
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);


	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

	// total
	table = gtk_table_new (5, 3, FALSE);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0; col = 0;
	label = make_label(_("100 Km"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("Total"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Meter:"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Consumption:"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Fuel cost:"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Other cost:"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Total cost:"), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	col++;
	for(row = 1;row<MAX_CAR_RES;row++)
	{
		label = make_label(NULL, 1.0, 0.5);
		gtk_table_attach_defaults (GTK_TABLE (table), label, col, col+1, row, row+1);
		data->LA_avera[row] = label;
	}

	col++;
	for(row = 1;row<MAX_CAR_RES;row++)
	{
		label = make_label(NULL, 1.0, 0.5);
		gtk_table_attach_defaults (GTK_TABLE (table), label, col, col+1, row, row+1);
		data->LA_total[row] = label;
	}

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	treeview = create_list_repcar();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);



	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);



	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repcar_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repcar_toggle_minor), NULL);

    data->handler_id[HID_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repcar_date_change), (gpointer)data);
    data->handler_id[HID_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repcar_date_change), (gpointer)data);

	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (repcar_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (repcar_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repcar_range_change), NULL);

	data->handler_id[HID_CAR] = g_signal_connect (data->PO_cat, "changed", G_CALLBACK (repcar_compute), NULL);


	//setup, init and show window
	repcar_setup(data);





    /* finish & show */
    gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);


	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);


	if( PREFS->filter_range )
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
		repcar_compute(window, NULL);

	return(window);
}

/*
** ============================================================================
*/

void repcar_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
GDate *date;
guint32 julian;
gchar buf[256];

	gtk_tree_model_get(model, iter,
		LST_CAR_DATE, &julian,
		-1);

	date = g_date_new_julian (julian);
	g_date_strftime (buf, 256-1, "%x", date);
	g_date_free(date);

	g_object_set(renderer, "text", buf, NULL);
}

void repcar_distance_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
guint distance;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &distance, -1);

	if(distance != 0)
	{
		text = g_strdup_printf("%d Km", distance);
		g_object_set(renderer, "text", text, NULL);
		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

void repcar_volume_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gdouble volume;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &volume, -1);

	if(volume != 0)
	{
		text = g_strdup_printf("%.2f L", volume);
		g_object_set(renderer, "text", text, NULL);
		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

void repcar_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
GtkWidget *widget;
gdouble  value;
gchar   buf[128];
gboolean minor;
gchar *markuptxt;
guint32 color;

	//get datas
	gtk_tree_model_get(model, iter,
		user_data, &value,
		-1);

	widget = g_object_get_data(G_OBJECT(model), "minor");
	if(GTK_IS_TOGGLE_BUTTON(widget))
	{
		minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	}
	else
		minor = 0;

	hb_strfmon(buf, 127, value, minor);

	color = (value > 0) ? PREFS->color_inc : PREFS->color_exp;

	markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
	g_object_set(renderer, "markup", markuptxt, NULL);
	g_free(markuptxt);

}

GtkTreeViewColumn *volume_list_repcar_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repcar_volume_cell_data_function, (gpointer)id, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

GtkTreeViewColumn *distance_list_repcar_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repcar_distance_cell_data_function, (gpointer)id, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

GtkTreeViewColumn *amount_list_repcar_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repcar_amount_cell_data_function, (gpointer)id, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}


/*
** create our statistic list
*/
GtkWidget *create_list_repcar(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_CAR,
		G_TYPE_UINT,
		G_TYPE_STRING,
		G_TYPE_UINT,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_UINT,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);

	/* column date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_CAR_DATE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repcar_date_cell_data_function, NULL, NULL);

/*
	LST_CAR_DATE,
	LST_CAR_WORDING,
	LST_CAR_METER,
	LST_CAR_FUEL,
	LST_CAR_PRICE,
	LST_CAR_AMOUNT,
	LST_CAR_DIST,
	LST_CAR_100KM

*/

	/* column: Wording */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Wording"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_CAR_WORDING);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, repcar_text_cell_data_function, NULL, NULL);
*/

	/* column: Meter */
	column = distance_list_repcar_column(_("Meter"), LST_CAR_METER);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Fuel load */
	column = volume_list_repcar_column(_("Fuel"), LST_CAR_FUEL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Price by unit */
	column = amount_list_repcar_column(_("Price"), LST_CAR_PRICE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = amount_list_repcar_column(_("Amount"), LST_CAR_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Distance done */
	column = distance_list_repcar_column(_("Dist."), LST_CAR_DIST);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: consumption for 100Km */
	column = volume_list_repcar_column(_("100 Km"), LST_CAR_100KM);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}
