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
#include "gtkchart.h"
#include "dsp_wallet.h"
#include "rep_vehicle.h"
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
extern struct Preferences *PREFS;

enum {
	HID_MINDATE,
	HID_MAXDATE,
	HID_MONTH,
	HID_YEAR,
	HID_RANGE,
	HID_VEHICLE,
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


struct repvehicle_data
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

	GList		*vehicle_list;

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
	LST_CAR_DISTBYVOL,
	NUM_LST_CAR
};

/* prototypes */
static void repvehicle_date_change(GtkWidget *widget, gpointer user_data);
static void repvehicle_period_change(GtkWidget *widget, gpointer user_data);
static void repvehicle_range_change(GtkWidget *widget, gpointer user_data);
static void repvehicle_compute(GtkWidget *widget, gpointer user_data);
static void repvehicle_update(GtkWidget *widget, gpointer user_data);
static void repvehicle_toggle_minor(GtkWidget *widget, gpointer user_data);
static void repvehicle_setup(struct repvehicle_data *data);
static gboolean repvehicle_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static GtkWidget *create_list_repvehicle(void);


static void repvehicle_date_change(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;

	DB( g_print("(repvehicle) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), 0);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);

	
	repvehicle_compute(widget, NULL);

}


static void repvehicle_period_change(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
gint month, year;

	DB( g_print("(repvehicle) period change\n") );

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

	repvehicle_compute(widget, NULL);

}

static void repvehicle_range_change(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(repvehicle) range change\n") );

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

		repvehicle_compute(widget, NULL);
	}
}

static gint repvehicle_transaction_compare_func(CarCost *a, CarCost *b)
{
gint retval;

	//retval = (gint)(a->ope->date - b->ope->date);
	//if( retval == 0 )
		retval = a->meter - b->meter;

	return retval;
}

static void repvehicle_compute(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GList *list;
guint32 catkey;

	DB( g_print("(repvehicle) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* do nothing if no operation */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	// get the account key
	catkey = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_cat));

	DB( g_print(" -> active cat is %d\n", catkey) );

	if(catkey == -1) return;
	
	// clear the glist
	da_vehiclecost_destroy(data->vehicle_list);
	data->vehicle_list = NULL;

	// collect transactions
	// the purpose here is to collect all cat transaction
	// and precompute some datas
	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *ope = list->data;
	Category *cat;
    CarCost *item;

		cat = da_cat_get(ope->category);

		if( cat && (cat->key == catkey || cat->parent == catkey) && !(ope->flags & OF_REMIND) )
		{
			item = da_vehiclecost_malloc();

			item->ope	= ope;

			if( ope->wording != NULL)
			{
			gchar *d, *v1, *v2;
			gint len;

				len = strlen(ope->wording);
				d = g_strstr_len(ope->wording, len, "d=");
				v1 = g_strstr_len(ope->wording, len, "v=");
				v2 = g_strstr_len(ope->wording, len, "v~");
				if(d && (v1 || v2))
				{
					item->meter	= atol(d+2);
					if(v1)
					{
						item->fuel	= g_strtod(v1+2, NULL);
						item->partial = FALSE;
					}
					else
					{
						item->fuel	= g_strtod(v2+2, NULL);
						item->partial = TRUE;
					}
				}
			}
				
			data->vehicle_list = g_list_append(data->vehicle_list, item);
		}
		list = g_list_next(list);
	}

	// sort by meter #399170
	data->vehicle_list = g_list_sort(data->vehicle_list, (GCompareFunc)repvehicle_transaction_compare_func);

	repvehicle_update(widget, NULL);
}

static void repvehicle_update(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gchar *buf;
gint nb_refuel = 0;
guint lastmeter = 0;

	DB( g_print("(repvehicle) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// clear and detach our model
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_object_ref(model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL);

	data->total_misccost = 0;
	data->total_fuelcost = 0;
	data->total_fuel	 = 0;
	data->total_dist	 = 0;

	gdouble partial_fuel = 0;
	guint	partial_dist = 0;
	
	
	list = g_list_first(data->vehicle_list);
	while (list != NULL)
	{
	CarCost *item = list->data;
	gint dist;
	gdouble centkm;
	guint distbyvol;

		if( (item->ope->date >= data->mindate) && (item->ope->date <= data->maxdate) )
		{
			if( item->meter == 0 )
			{
				data->total_misccost += item->ope->amount;
			}
			else
			{
				if(nb_refuel > 0 )
				{
					//previtem = g_list_nth_data(data->vehicle_list, nb_refuel-1);
					//if(previtem != NULL) previtem->dist = item->meter - previtem->meter;
					//DB( g_print(" + previous item dist = %d\n", item->meter - previtem->meter) );
					item->dist = item->meter - lastmeter;

					//DB( g_print(" + last meter = %d\n", lastmeter) );

				}

				lastmeter = item->meter;
				nb_refuel++;	

				DB( g_print(" eval : d=%d v=%4.2f $%8.2f dist=%d\n", item->meter, item->fuel, item->ope->amount, item->dist) );

				//bugfix #159066
				if(item->partial == FALSE)
				{
					// full refuel after partial
					if(partial_fuel && partial_dist)
					{
						DB( g_print(" + full refuel after partial\n") );
						partial_fuel += item->fuel;
						partial_dist += item->dist;
						dist = item->dist;
						centkm = partial_dist != 0 ? partial_fuel * 100 / partial_dist : 0;
					}
					else
					{
						DB( g_print(" + real full refuel\n") );
						dist = item->dist;
						centkm = item->dist != 0 ? item->fuel * 100 / item->dist : 0;
					}
					partial_fuel = 0;
					partial_dist = 0;
				}
				// partial refuel
				else
				{
					DB( g_print(" + partial refuel\n") );
					partial_fuel += item->fuel;
					partial_dist += item->dist;
					dist = item->dist;
					centkm = 0;
				}

				distbyvol = 0;
				if(centkm != 0)
					distbyvol = arrondi((1/centkm)*100, 0);
				
				DB( g_print(" + pf=%.2f pd=%d :: dbv=%d\n", partial_fuel, partial_dist, distbyvol) );
				
				
		    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_CAR_DATE    , item->ope->date,
					LST_CAR_WORDING , item->ope->wording,
					LST_CAR_METER   , item->meter,
					LST_CAR_FUEL    , item->fuel,
					LST_CAR_PRICE   , ABS(item->ope->amount) / item->fuel,
					LST_CAR_AMOUNT  , item->ope->amount,
					LST_CAR_DIST    , dist,
					LST_CAR_100KM   , centkm,
				    LST_CAR_DISTBYVOL, distbyvol,
					-1);

				DB( g_print(" + insert d=%d v=%4.2f $%8.2f %d %5.2f\n", item->meter, item->fuel, item->ope->amount, dist, centkm) );

				if(item->dist)
				{
					data->total_fuelcost += item->ope->amount;
					data->total_fuel     += item->fuel;
					data->total_dist     += item->dist;
				}


			}
		}
		list = g_list_next(list);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
	g_object_unref(model);


	gdouble coef = data->total_dist ? 100 / (gdouble)data->total_dist : 0;

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


		DB( g_print(" coef = 100 / %.2f = %.2f\n", (gdouble)data->total_dist, coef) );

		// row 1 is for 100km
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][1]), "1:1");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][1]), "2:1");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][1]), "3:1");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][1]), "4:1");	//Total cost
		*/

		// 100km fuel
		buf = g_strdup_printf(PREFS->vehicle_unit_vol, data->total_fuel * coef);
		gtk_label_set_text(GTK_LABEL(data->LA_avera[CAR_RES_FUEL]), buf);
		g_free(buf);

		// 100km fuelcost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_FUELCOST]), data->total_fuelcost * coef, GLOBALS->minor);

		// 100km other cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_OTHERCOST]), data->total_misccost * coef, GLOBALS->minor);

		// 100km cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_TOTALCOST]), (data->total_fuelcost + data->total_misccost) * coef, GLOBALS->minor);


		// row 2 is for total
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][2]), "1:2");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][2]), "2:2");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][2]), "3:2");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][2]), "4:2");	//Total
		*/

		// total distance
		buf = g_strdup_printf(PREFS->vehicle_unit_dist, data->total_dist);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_METER]), buf);
		g_free(buf);

		// total fuel
		buf = g_strdup_printf(PREFS->vehicle_unit_vol, data->total_fuel);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_FUEL]), buf);
		g_free(buf);

		// total fuelcost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_FUELCOST]), data->total_fuelcost, GLOBALS->minor);

		// total other cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_OTHERCOST]), data->total_misccost, GLOBALS->minor);

		// total cost
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_TOTALCOST]), data->total_fuelcost + data->total_misccost, GLOBALS->minor);


}

static void repvehicle_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;

	DB( g_print("(repvehicle) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repvehicle_update(widget, NULL);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));


	/*
	statistic_update_total(widget,NULL);

	//wallet_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_show_minor(GTK_CHART(data->RE_bar), minor);
	gtk_chart_show_minor(GTK_CHART(data->RE_pie), minor);
	*/

}

/*
**
*/
static void repvehicle_setup(struct repvehicle_data *data)
{
	DB( g_print("(repvehicle) setup\n") );

	data->vehicle_list = NULL;

	/* if ope get date bounds */
	if(g_list_length(GLOBALS->ope_list) > 0)
	{
	GList *list;

		//get our min max date
		GLOBALS->ope_list = da_operation_sort(GLOBALS->ope_list);
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

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_cat), GLOBALS->h_cat);

	g_signal_handler_block(data->PO_cat, data->handler_id[HID_VEHICLE]);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_cat), GLOBALS->vehicle_category);
	g_signal_handler_unblock(data->PO_cat, data->handler_id[HID_VEHICLE]);



}


/*
**
*/
static gboolean repvehicle_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repvehicle_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("(repvehicle) dispose\n") );

	da_vehiclecost_destroy(data->vehicle_list);

	g_free(data);

	//store position and size
	wg = &PREFS->cst_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

	return FALSE;
}


// the window creation
GtkWidget *repcost_window_new(void)
{
struct repvehicle_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *treeview;
GtkWidget *label, *widget, *table, *alignment;
gint row, col;

	data = g_malloc0(sizeof(struct repvehicle_data));
	if(!data) return NULL;

	DB( g_print("(repvehicle) new\n") );

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Vehicle cost report"));

	//set the window icon
//	homebank_window_set_icon_from_file(GTK_WINDOW (window), "report_vehicle.svg");	
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_REP_CAR);



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
	label = make_label(_("Vehi_cle:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	widget = ui_cat_comboboxentry_new(label);
	gtk_widget_set_size_request (widget, 10, -1);
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
	//toolbar = create_repvehicle_toolbar(data);
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
	label = make_label(PREFS->vehicle_unit_100, 1.0, 0.5);
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
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	treeview = create_list_repvehicle();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);



	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);



	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repvehicle_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repvehicle_toggle_minor), NULL);

    data->handler_id[HID_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repvehicle_date_change), (gpointer)data);
    data->handler_id[HID_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repvehicle_date_change), (gpointer)data);

	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (repvehicle_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (repvehicle_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repvehicle_range_change), NULL);

	data->handler_id[HID_VEHICLE] = g_signal_connect (data->PO_cat, "changed", G_CALLBACK (repvehicle_compute), NULL);


	//setup, init and show window
	repvehicle_setup(data);

	/* toolbar */
	/*
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);
	*/

	//setup, init and show window
	wg = &PREFS->cst_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);



	if( PREFS->filter_range )
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
		repvehicle_compute(window, NULL);

	return(window);
}

/*
** ============================================================================
*/

static void repvehicle_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
GDate *date;
guint32 julian;
gchar buf[256];

	gtk_tree_model_get(model, iter,
		LST_CAR_DATE, &julian,
		-1);

	date = g_date_new_julian (julian);
	g_date_strftime (buf, 256-1, PREFS->date_format, date);
	g_date_free(date);

	g_object_set(renderer, "text", buf, NULL);
}

static void repvehicle_distance_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
guint distance;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &distance, -1);

	if(distance != 0)
	{
		text = g_strdup_printf(PREFS->vehicle_unit_dist, distance);
		g_object_set(renderer, "text", text, NULL);
		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

static void repvehicle_volume_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gdouble volume;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &volume, -1);

	if(volume != 0)
	{
		text = g_strdup_printf(PREFS->vehicle_unit_vol, volume);
		g_object_set(renderer, "text", text, NULL);
		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

static void repvehicle_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

	gtk_tree_model_get(model, iter,
		user_data, &value,
		-1);

	if( value )
	{
		mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, GLOBALS->minor);
		
		color = get_normal_color_amount(value);

		g_object_set(renderer, 
			"foreground",  color,
			"text", buf,
			NULL);	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}
}

static GtkTreeViewColumn *volume_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_volume_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *distance_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_distance_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *amount_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}


/*
** create our statistic list
*/
static GtkWidget *create_list_repvehicle(void)
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
		G_TYPE_DOUBLE,
	    G_TYPE_UINT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), PREFS->rules_hint);

	/* column date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_CAR_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_date_cell_data_function, NULL, NULL);

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
	//gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_text_cell_data_function, NULL, NULL);
*/

	/* column: Meter */
	column = distance_list_repvehicle_column(_("Meter"), LST_CAR_METER);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Fuel load */
	column = volume_list_repvehicle_column(_("Fuel"), LST_CAR_FUEL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Price by unit */
	column = amount_list_repvehicle_column(_("Price"), LST_CAR_PRICE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = amount_list_repvehicle_column(_("Amount"), LST_CAR_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Distance done */
	column = distance_list_repvehicle_column(_("Dist."), LST_CAR_DIST);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: consumption for 100Km */
	column = volume_list_repvehicle_column(PREFS->vehicle_unit_100, LST_CAR_100KM);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: km by liter (distance by volume */
	column = distance_list_repvehicle_column(PREFS->vehicle_unit_distbyvol, LST_CAR_DISTBYVOL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	
  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}
