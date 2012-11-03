/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2008 Maxime DOYEN
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
#include "rep_stats.h"
#include "list_operation.h"
#include "def_filter.h"
#include "dsp_wallet.h"

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
	HID_VIEW,
	MAX_HID
};

struct statistic_data
{
	GtkWidget	*window;

	gint	busy;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkWidget	*TB_bar;
	
	GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*CY_for;
	GtkWidget	*CY_view;
	GtkWidget	*LV_report;
	GtkWidget	*CM_byamount;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*TX_total[3];

	GtkWidget	*RE_bar;
	GtkWidget	*RE_pie;

	GtkWidget	*GR_detail;
	GtkWidget	*LV_detail;

	gdouble		total_expense;
	gdouble		total_income;

	gboolean	detail;
	gboolean	legend;
	gboolean	rate;

	gulong		handler_id[MAX_HID];

	Filter		*filter;

/*
	struct	Wallet_Data *mwd;
	Object	*LV_detail;
	Object	*WI_filter;
	Object	*LV_per;

	struct	Filter flt;
	UWORD	page;
	BOOL	detail;
	double	expense;
	double	income;
	double	balance;
	double	total;
	*/
};

/* prototypes */
static void statistic_action_viewlist(GtkAction *action, gpointer user_data);
static void statistic_action_viewbar(GtkAction *action, gpointer user_data);
static void statistic_action_viewpie(GtkAction *action, gpointer user_data);
static void statistic_action_detail(GtkAction *action, gpointer user_data);
static void statistic_action_legend(GtkAction *action, gpointer user_data);
static void statistic_action_rate(GtkAction *action, gpointer user_data);
static void statistic_action_filter(GtkAction *action, gpointer user_data);
static void statistic_action_refresh(GtkAction *action, gpointer user_data);

static void statistic_busy(GtkWidget *widget, gboolean state);


static GtkActionEntry entries[] = {
  { "List"    , "hb-stock-view-list" , N_("List")   , NULL,    N_("View results as list"), G_CALLBACK (statistic_action_viewlist) },
  { "Bar"     , "hb-stock-view-bar"  , N_("Bar")    , NULL,    N_("View results as bars"), G_CALLBACK (statistic_action_viewbar) },
  { "Pie"     , "hb-stock-view-pie"  , N_("Pie")    , NULL,    N_("View results as pies"), G_CALLBACK (statistic_action_viewpie) },

  { "Detail"  , "hb-stock-ope-show"  , N_("Detail") , NULL,    N_("Toggle detail"), G_CALLBACK (statistic_action_detail) },
  { "Legend"  , "hb-stock-legend"    , N_("Legend") , NULL,    N_("Toggle legend"), G_CALLBACK (statistic_action_legend) },
  { "Rate"    , "hb-stock-rate"      , N_("Rate")   , NULL,    N_("Toggle rate")  , G_CALLBACK (statistic_action_rate) },

  { "Filter"  , "hb-stock-filter"    , N_("Filter") , NULL,   N_("Edit the filter"), G_CALLBACK (statistic_action_filter) },
  { "Refresh" , "hb-stock-refresh"   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (statistic_action_refresh) },
};

static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Bar'/>"
"    <toolitem action='Pie'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"    <toolitem action='Legend'/>"
"    <toolitem action='Rate'/>"
"      <separator/>"
"    <toolitem action='Filter'/>"
"    <toolitem action='Refresh'/>"
"  </toolbar>"
"</ui>";

/* list stat */
enum
{
	LST_STAT_POS,
	LST_STAT_KEY,
	LST_STAT_NAME,
	LST_STAT_EXPENSE,
	LST_STAT_EXPRATE,
	LST_STAT_INCOME,
	LST_STAT_INCRATE,
	LST_STAT_BALANCE,
	LST_STAT_BALRATE,
	NUM_LST_STAT
};


static void statistic_date_change(GtkWidget *widget, gpointer user_data);
static void statistic_period_change(GtkWidget *widget, gpointer user_data);
static void statistic_range_change(GtkWidget *widget, gpointer user_data);
static void statistic_detail(GtkWidget *widget, gpointer user_data);
static void statistic_update(GtkWidget *widget, gpointer user_data);
static void statistic_update_total(GtkWidget *widget, gpointer user_data);
static void statistic_compute(GtkWidget *widget, gpointer user_data);
static void statistic_sensitive(GtkWidget *widget, gpointer user_data);
static void statistic_toggle_detail(GtkWidget *widget, gpointer user_data);
static void statistic_toggle_legend(GtkWidget *widget, gpointer user_data);
static void statistic_toggle_minor(GtkWidget *widget, gpointer user_data);
static void statistic_toggle_rate(GtkWidget *widget, gpointer user_data);
static GtkWidget *create_list_statistic(void);

static gint stat_list_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata);

enum
{
	STAT_CATEGORY,
	STAT_SUBCATEGORY,
	STAT_PAYEE,
	STAT_TAG,
	STAT_MONTH,
	STAT_YEAR,
};

gchar *CYA_STATSELECT[] = { N_("Category"), N_("Subcategory"), N_("Payee"), N_("Tag"), N_("Month"), N_("Year"), NULL };



gchar *CYA_KIND2[] = { N_("Exp. & Inc."), N_("Expense"), N_("Income"), N_("Balance"), NULL };

gchar *CYA_RANGE[] = {
"----",
N_("All date"),
N_("Current month"),
N_("Current year"),
N_("Previous month"),
N_("Previous year"),
N_("Last 30 days"),
N_("Last 2 months"),
N_("Last 3 months"),
N_("Last 4 months"),
N_("Last 6 months"),
N_("Last 12 months"),
NULL
};

gchar *CYA_SELECT[] =
{
"----",
N_("All month"),
N_("January"),
N_("February"),
N_("March"),
N_("April"),
N_("May"),
N_("June"),
N_("July"),
N_("August"),
N_("September"),
N_("October"),
N_("November"),
N_("December"),
NULL
};

gchar *CYA_MONTHS[] =
{
NULL,
N_("January"),
N_("February"),
N_("March"),
N_("April"),
N_("May"),
N_("June"),
N_("July"),
N_("August"),
N_("September"),
N_("October"),
N_("November"),
N_("December"),
};

/* action functions -------------------- */

static void statistic_action_viewlist(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	statistic_sensitive(data->window, NULL);
}

static void statistic_action_viewbar(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	statistic_sensitive(data->window, NULL);
}

static void statistic_action_viewpie(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;
gint tmpview;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 2);
	statistic_sensitive(data->window, NULL);

	tmpview = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

	// ensure not exp & inc for piechart
	if( tmpview == 0 )
	{
		//g_signal_handler_block(data->CY_view, data->handler_id[HID_VIEW]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_view), 1);
		//g_signal_handler_unblock(data->CY_view, data->handler_id[HID_VIEW]);
	}

}

static void statistic_action_detail(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	statistic_toggle_detail(data->window, NULL);
}

static void statistic_action_legend(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	statistic_toggle_legend(data->window, NULL);
}

static void statistic_action_rate(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	statistic_toggle_rate(data->window, NULL);
}

static void statistic_action_filter(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	//debug
	//create_deffilter_window(data->filter, TRUE);

	if(create_deffilter_window(data->filter, TRUE) == GTK_RESPONSE_ACCEPT)
		statistic_compute(data->window, NULL);
}

static void statistic_action_refresh(GtkAction *action, gpointer user_data)
{
struct statistic_data *data = user_data;

	statistic_compute(data->window, NULL);
}



/* ======================== */



/*
** ============================================================================
*/




/*
** return the month list position correponding to the passed date
*/
static gint DateInPer(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
gint pos;

	//debug
	// this return sometimes -1, -2 which is wrong

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	pos = ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1);

	//g_print(" from=%d-%d ope=%d-%d => %d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos);

	g_date_free(date2);
	g_date_free(date1);

	return(pos);
}

/*
** return the year list position correponding to the passed date
*/
static gint DateInYear(guint32 from, guint32 opedate)
{
GDate *date;
gint year_from, year_ope, pos;

	date = g_date_new_julian(from);
	year_from = g_date_get_year(date);
	g_date_set_julian(date, opedate);
	year_ope = g_date_get_year(date);
	g_date_free(date);

	pos = year_ope - year_from;

	//g_print(" from=%d ope=%d => %d\n", year_from, year_ope, pos);

	return(pos);
}

static void statistic_date_change(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;

	DB( g_print("(statistic) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	statistic_compute(widget, NULL);

}


static void statistic_period_change(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gint month, year;

	DB( g_print("(statistic) period change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

/*
	data->flt.flt_option[FLT_DATE] = 1;
*/

	month = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_month));
	year = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_year));

	DB( g_print(" month=%d, year=%d\n", month, year) );


	if(month != 0)
		get_period_minmax(month-1, year, &data->filter->mindate, &data->filter->maxdate);
	else
		get_period_minmax(0, year, &data->filter->mindate, &data->filter->maxdate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), 0);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);

	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

	data->filter->month = month;
	data->filter->year = year;
	data->filter->range = 0;

	statistic_compute(widget, NULL);

}

static void statistic_range_change(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(statistic) range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

/*
	data->flt.flt_option[FLT_DATE] = 1;
*/

	if(g_list_length(GLOBALS->ope_list) == 0) return;

	//get our min max date
	GLOBALS->ope_list = da_operation_sort(GLOBALS->ope_list);
	list = g_list_first(GLOBALS->ope_list);
	data->filter->mindate = ((Operation *)list->data)->date;
	list = g_list_last(GLOBALS->ope_list);
	data->filter->maxdate   = ((Operation *)list->data)->date;

	refdate = GLOBALS->today;
	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != 0)
	{
		if(range > 0)
			get_range_minmax(refdate, range-1, &data->filter->mindate, &data->filter->maxdate);

		/* update the year */
		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		date = g_date_new_julian(data->filter->maxdate);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), g_date_get_year(date));

		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

		g_signal_handler_block(data->CY_month, data->handler_id[HID_MONTH]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_month), 0);
		g_signal_handler_unblock(data->CY_month, data->handler_id[HID_MONTH]);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

		data->filter->month = 0;
		data->filter->year = g_date_get_year(date);
		data->filter->range = range;

		g_date_free(date);

		statistic_compute(widget, NULL);
	}
}



static void statistic_detail(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
guint active = GPOINTER_TO_INT(user_data);
guint tmpfor;
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(statistic) detail\n") );

	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if(data->detail && active != -1)
	{
		tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));

		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		/* fill in the model */
		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope = list->data;

			//DB( g_print(" get %s\n", ope->ope_Word) );

			//filter here
			if(filter_test(data->filter, ope) == 1)
			{
			gint pos = 0;

				if( tmpfor != STAT_TAG )
				{

					switch(tmpfor)
					{
						case STAT_CATEGORY:
							pos = ope->category;
							break;
						case STAT_SUBCATEGORY:
							pos = ope->category;
							break;
						case STAT_PAYEE:
							pos = ope->payee;
							break;

						case STAT_MONTH:
							pos = DateInPer(data->filter->mindate, ope->date);
							break;
						case STAT_YEAR:
							pos = DateInYear(data->filter->mindate, ope->date);
							break;
					}

					//insert
					if( pos == active )
					{

				    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							LST_DSPOPE_DATAS, ope,
							-1);

					/*
					if(ApplyFilter(&data->flt, ope))
					{
						DoMethod(data->LV_detail, MUIM_NList_InsertSingle, ope, MUIV_NList_Insert_Bottom);
					}
					*/
					}


				}
				else
				/* the TAG process is particular */
				{
					if(ope->tags != NULL)
					{
					guint32 *tptr = ope->tags;
					
						while(*tptr)
						{
							pos = *tptr - 1;
	
							DB( g_print(" -> storing tag %d %.2f\n", pos, ope->amount) ); 

							if( pos == active )
							{
						    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
					     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									LST_DSPOPE_DATAS, ope,
									-1);

							}
	
							tptr++;
						} 	

					}
				}



			}
			list = g_list_next(list);
		}

		/* Re-attach model to view */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), model);
		g_object_unref(model);

	}
	else
		DB( g_print(" ->nothing done\n") );


}


static void statistic_update(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gboolean byamount;
GtkTreeModel		 *model;
gint page, tmpfor, tmpkind, column;
gboolean xval;

	DB( g_print("(statistic) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	byamount = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_byamount));
	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpkind = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

	// ensure not exp & inc for piechart
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	if( page == 2 && tmpkind == 0 )
	{
		g_signal_handler_block(data->CY_view, data->handler_id[HID_VIEW]);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_view), 1);
		g_signal_handler_unblock(data->CY_view, data->handler_id[HID_VIEW]);
		tmpkind = 1;
	}


	DB( g_print(" tmpkind %d\n\n", tmpkind) );


	column = byamount ? LST_STAT_EXPENSE+(tmpkind-1)*2 : LST_STAT_POS;
	DB( g_print(" sort on column %d\n\n", column) );

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), column, GTK_SORT_DESCENDING);

	column = LST_STAT_EXPENSE+(tmpkind-1)*2;

	/* update bar chart */
	DB( g_print(" set bar to %d %s\n\n", column, _(CYA_KIND2[tmpkind])) );
	if( tmpkind == 0 )
		gtk_chart_set_dualdatas(GTK_CHART(data->RE_bar), model, LST_STAT_EXPENSE, LST_STAT_INCOME);
	else
		gtk_chart_set_datas(GTK_CHART(data->RE_bar), model, column);
	gtk_chart_set_title(GTK_CHART(data->RE_bar), _(CYA_KIND2[tmpkind]));
	
	
	/* show xval for month/year and no by amount display */
	xval = FALSE;
	
	
	if( !byamount && (tmpfor == STAT_MONTH || tmpfor == STAT_YEAR) )
	{
		xval = TRUE;
		switch( tmpfor)
		{
			case STAT_MONTH:
				gtk_chart_set_decy_xval(GTK_CHART(data->RE_bar), 4);
				break;
			case STAT_YEAR:
				gtk_chart_set_decy_xval(GTK_CHART(data->RE_bar), 2);
				break;
	
		}
		
		
	}
	
	gtk_chart_show_xval(GTK_CHART(data->RE_bar), xval);
	
	/* update pie chart */
	DB( g_print(" set pie to %d %s\n\n", column, _(CYA_KIND2[tmpkind])) );
	if( tmpkind != 0 )
		gtk_chart_set_datas(GTK_CHART(data->RE_pie), model, column);
	else
		gtk_chart_set_datas(GTK_CHART(data->RE_pie), NULL, 0);
	gtk_chart_set_title(GTK_CHART(data->RE_pie), _(CYA_KIND2[tmpkind]));
	
}

static void statistic_update_total(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gboolean minor;

	DB( g_print("(statistic) update total\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	hb_label_set_colvalue(GTK_LABEL(data->TX_total[0]), data->total_expense, minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[1]), data->total_income, minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[2]), data->total_expense + data->total_income, minor);

}

static void statistic_compute(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gint tmpfor, tmpkind;
gint from, to;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint n_result, i, id;
GDate *date1, *date2;
gdouble *tmp_income, *tmp_expense;
gdouble exprate, incrate, balrate;

	DB( g_print("(statistic) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpkind = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));


	DB( g_print(" for=%d,kind=%d\n", tmpfor, tmpkind) );



	/* do nothing if no operation */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	//get our min max date
	from = data->filter->mindate;
	to   = data->filter->maxdate;

	/* count number or results */
	switch(tmpfor)
	{
		case STAT_CATEGORY:
			n_result = da_cat_get_max_key() + 1;
			break;
		case STAT_SUBCATEGORY:
			n_result = da_cat_get_max_key() + 1;
			break;
		case STAT_PAYEE:
			n_result = da_pay_get_max_key() + 1;
			break;
		case STAT_TAG:
			n_result = da_tag_length();
			break;
		case STAT_MONTH:
			date1 = g_date_new_julian(from);
			date2 = g_date_new_julian(to);
			n_result = ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1) + 1;
			g_date_free(date2);
			g_date_free(date1);
			break;
		case STAT_YEAR:
			date1 = g_date_new_julian(from);
			date2 = g_date_new_julian(to);
			n_result = g_date_get_year(date2) - g_date_get_year(date1) + 1;
			g_date_free(date2);
			g_date_free(date1);
			break;
		default:
			n_result = 0;
	}

	DB( g_print(" %s :: n_result=%d\n", CYA_STATSELECT[tmpfor], n_result) );

	/* allocate some memory */
	tmp_expense = g_malloc0((n_result+2) * sizeof(gdouble));
	tmp_income  = g_malloc0((n_result+2) * sizeof(gdouble));

	data->total_expense = 0.0;
	data->total_income  = 0.0;

	if(tmp_expense && tmp_income)
	{
		/* compute the results */
		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope = list->data;

			//filter here
			//if( !(ope->flags & OF_REMIND) && ope->date >= from && ope->date <= to)

			//debug
			DB( g_print("** testing '%s', cat=%d==> %d\n", ope->wording, ope->category, filter_test(data->filter, ope)) );


			if(filter_test(data->filter, ope) == 1)
			{
			guint32 pos = 0;

				if( tmpfor != STAT_TAG )
				{
					switch(tmpfor)
					{
						case STAT_CATEGORY:
							{
							Category *catentry = da_cat_get(ope->category);
								if(catentry)
									pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
							}
							break;
						case STAT_SUBCATEGORY:
							pos = ope->category;
							break;
						case STAT_PAYEE:
							pos = ope->payee;
							break;
						case STAT_MONTH:
							pos = DateInPer(from, ope->date);
							break;
						case STAT_YEAR:
							pos = DateInYear(from, ope->date);
							break;
					}

					if(pos >= 0)
					{// add the amount
						if(ope->amount > 0)
						{
							tmp_income[pos] += ope->amount;
							data->total_income  += ope->amount;
						}
						else
						{
							tmp_expense[pos] += ope->amount;
							//data->total_expense += ABS(ope->amount);
							data->total_expense += ope->amount;
						}
					}
				}
				else
				/* the TAG process is particular */
				{
					if(ope->tags != NULL)
					{
					guint32 *tptr = ope->tags;
					
						while(*tptr)
						{
							pos = *tptr - 1;
	
							DB( g_print(" -> storing tag %d %.2f\n", pos, ope->amount) ); 
	
							if(ope->amount > 0)
							{
								tmp_income[pos] += ope->amount;
							}
							else
							{
								tmp_expense[pos] += ope->amount;
							}							
							tptr++;
						} 	

						data->total_income  += ope->amount;
						data->total_expense += ope->amount;

					}
				}



				// fix total according to selection
				//if(tmpkind==0 && !tmp_expense[pos]) { data->total_income  -= ope->amount; }
				//if(tmpkind==1 && !tmp_income[pos] ) { data->total_expense -= ope->amount; }


			}
			list = g_list_next(list);
		}

		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

		/* insert into the treeview */
		for(i=0, id=0; i<n_result; i++)
		{
		gchar *name, *fullcatname;
		gchar buffer[64];
		GDate *date;

			name = NULL;
			fullcatname = NULL;

			DB( g_print("try to insert item %d\n", i) );


			/* filter empty results */
			if(tmpfor == STAT_CATEGORY || tmpfor == STAT_SUBCATEGORY || tmpfor == STAT_PAYEE || tmpfor == STAT_TAG)
			{
				if( tmpkind == 1 && !tmp_expense[i] ) continue;
				if( tmpkind == 2 && !tmp_income[i] ) continue;
				if( !tmp_expense[i] && !tmp_income[i] ) continue;
			}

			/* get the result name */
			switch(tmpfor)
			{
				case STAT_CATEGORY:
					{
					Category *entry = da_cat_get(i);

						if(entry != NULL)
							name = entry->key == 0 ? _("(none)") : entry->name;
					}
					break;

				case STAT_SUBCATEGORY:
					{
					Category *entry = da_cat_get(i);
						if(entry != NULL)
						{
							if(entry->flags & GF_SUB)
							{
							Category *parent = da_cat_get(entry->parent);

								fullcatname = g_strdup_printf("%s : %s", parent->name, entry->name);
								name = fullcatname;
							}
							else
								name = entry->key == 0 ? _("(none)") : entry->name;
						}
					}
					break;

				case STAT_PAYEE:
					{
					Payee *entry = da_pay_get(i);
						if(entry != NULL)
							name = entry->key == 0 ? _("(none)") : entry->name;
					}
					break;

				case STAT_TAG:
					{
					Tag *entry = da_tag_get(i+1);
						name = entry->name;
					}
					break;

				case STAT_MONTH:
					date = g_date_new_julian(from);
					g_date_add_months(date, i);
					//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
					g_snprintf(buffer, 63, "%d-%s", g_date_get_year(date), _(CYA_MONTHS[g_date_get_month(date)]));
					g_date_free(date);
					name = buffer;
					break;

				case STAT_YEAR:
					date = g_date_new_julian(from);
					g_date_add_years(date, i);
					g_snprintf(buffer, 63, "%d", g_date_get_year(date));
					g_date_free(date);
					name = buffer;
					break;
			}

			DB( g_print(" inserting %2d, '%s', %9.2f %9.2f %9.2f\n", i, name, tmp_expense[i], tmp_income[i], tmp_expense[i] + tmp_income[i]) );

			//compute rate
			exprate = 0.0;
			incrate = 0.0;
			balrate = 0.0;

			if( data->total_expense )
				exprate = (ABS(tmp_expense[i]) * 100 / data->total_expense);
			
			if( data->total_income )
				incrate = (tmp_income[i] * 100 / data->total_income);
				
			if( (data->total_expense + data->total_income) )
				balrate = (ABS(tmp_expense[i]) + tmp_income[i]) * 100 / (data->total_expense + data->total_income);

	    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_STAT_POS, id++,
				LST_STAT_KEY, i,
				LST_STAT_NAME, name,
				LST_STAT_EXPENSE, tmp_expense[i],
				LST_STAT_INCOME, tmp_income[i],
				LST_STAT_BALANCE, tmp_expense[i] + tmp_income[i],
				LST_STAT_EXPRATE, exprate,
				LST_STAT_INCRATE, incrate,
				LST_STAT_BALRATE, balrate,
				-1);

			g_free(fullcatname);
		}

		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);
	}

	/* free our memory */
	g_free(tmp_expense);
	g_free(tmp_income);

  /* update info range text */
/*
	{
	gchar buffer1[128];
	gchar buffer2[128];
	GDate *date;
	gchar *info;

		date = g_date_new_julian(data->filter->mindate);
		g_date_strftime (buffer1, 128-1, "%x", date);
		g_date_set_julian(date, data->filter->maxdate);
		g_date_strftime (buffer2, 128-1, "%x", date);
		g_date_free(date);

		info = g_strdup_printf("%s -> %s", buffer1, buffer2);

		gtk_label_set_text(GTK_LABEL(data->TX_info), info);


		g_free(info);
	}
*/
	statistic_update_total(widget,NULL);

	statistic_update(widget, user_data);

}





/*
** update sensitivity
*/
static void statistic_sensitive(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gboolean active;
gboolean sensitive;
gint page, view;

	DB( g_print("(statistic) sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	active = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	sensitive = page == 0 ? active : FALSE;
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_DETAIL], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail"), sensitive);

	view = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

	sensitive = page == 0 ? FALSE : TRUE;
	//gtk_widget_set_sensitive(data->CY_view, sensitive);
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_LEGEND], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Legend"), sensitive);


}

static void statistic_update_detail(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(data->detail)
	{
	GtkTreeSelection *treeselection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	guint key;

		treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_report));

		if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
		{
			gtk_tree_model_get(model, &iter, LST_STAT_KEY, &key, -1);

			DB( g_print(" - active is %d\n", key) );

			statistic_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
		}



		gtk_widget_show(data->GR_detail);
	}
	else
		gtk_widget_hide(data->GR_detail);
}




static void statistic_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->detail ^= 1;

	DB( printf("(stats) toggledetail to %ld\n", data->detail) );

	statistic_update_detail(widget, user_data);

}

/*
** change the chart legend visibility
*/
static void statistic_toggle_legend(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
//gint active;

	DB( g_print("(statistic) legend\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->legend ^= 1;

	//active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_legend));

	gtk_chart_show_legend(GTK_CHART(data->RE_bar), data->legend);
	gtk_chart_show_legend(GTK_CHART(data->RE_pie), data->legend);

}

/*
** change the chart rate columns visibility
*/
static void statistic_toggle_rate(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
GtkTreeViewColumn *column;

	DB( g_print("(statistic) toggle rate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->rate ^= 1;

	column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 2);
	gtk_tree_view_column_set_visible(column, data->rate);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 4);
	gtk_tree_view_column_set_visible(column, data->rate);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 6);
	gtk_tree_view_column_set_visible(column, data->rate);


}

static void statistic_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct statistic_data *data;
gboolean minor;

	DB( g_print("(statistic) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	statistic_update_total(widget,NULL);

	//wallet_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_show_minor(GTK_CHART(data->RE_bar), minor);
	gtk_chart_show_minor(GTK_CHART(data->RE_pie), minor);

}


static void statistic_busy(GtkWidget *widget, gboolean state)
{
struct statistic_data *data;
GtkWidget *window;
GdkCursor *cursor;

	DB( g_printf("(statistic) busy\n") );

	window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
	data = g_object_get_data(G_OBJECT(window), "inst_data");

	// should busy ?
	if(state == TRUE)
	{
		cursor = gdk_cursor_new(GDK_WATCH);
		gdk_window_set_cursor(GTK_WIDGET(window)->window, cursor);
		gdk_cursor_unref(cursor);
		
		//gtk_grab_add(data->busy_popup);
		
		gtk_widget_set_sensitive(window, FALSE);
		gtk_action_group_set_sensitive(data->actions, FALSE);

		  /* make sure chnages is up */
		  while (gtk_events_pending ())
		    gtk_main_iteration ();
	}
	// unbusy
	else
	{
		gtk_widget_set_sensitive(window, TRUE);
		gtk_action_group_set_sensitive(data->actions, TRUE);	
		
		gdk_window_set_cursor(GTK_WIDGET(window)->window, NULL);
		//gtk_grab_remove(data->busy_popup);
	}
}

/*
**
*/
static void statistic_setup(struct statistic_data *data)
{
	DB( g_print("(statistic) setup\n") );

	data->filter = da_filter_malloc();

	DB( g_print(" filter is %x\n", data->filter) );

	data->detail = PREFS->stat_showdetail;
	data->legend = 1;
	data->rate = PREFS->stat_showrate^1;

	statistic_toggle_rate(data->window, NULL);

	filter_reset(data->filter);
	
	/* 3.4 : make int transfert out of stats */
	data->filter->option[FILTER_PAYMODE] = 1;
	data->filter->paymode[PAYMODE_PERSTRANSFERT] = FALSE;

	/* if ope get date bounds */
	if(g_list_length(GLOBALS->ope_list) > 0)
	{
	GList *list;

		//get our min max date
		da_operation_sort(GLOBALS->ope_list);
		list = g_list_first(GLOBALS->ope_list);
		data->filter->mindate = ((Operation *)list->data)->date;
		list = g_list_last(GLOBALS->ope_list);
		data->filter->maxdate   = ((Operation *)list->data)->date;

		/*  */
		GDate *date;
		gdouble min,max;

		date = g_date_new_julian(data->filter->mindate);
		min = g_date_get_year(date);
		g_date_set_julian(date, data->filter->maxdate);
		max = g_date_get_year(date);
		g_date_free(date);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->NB_year), min, max);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), max);
		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

		data->filter->range = 0;
		data->filter->month = 0;
		data->filter->year = max;

	}


}



static void statistic_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key = -1;

	DB( g_print("(statistic) selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_STAT_KEY, &key, -1);

	}

	DB( g_print(" - active is %d\n", key) );

	statistic_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	statistic_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


/*
**
*/
static gboolean statistic_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct statistic_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("(statistic) dispose\n") );

	da_filter_free(data->filter);

	g_free(data);

	//store position and size
	wg = &PREFS->sta_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );



	//enable define windows
	GLOBALS->define_off--;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

	return FALSE;
}

// the window creation
GtkWidget *create_statistic_window(void)
{
struct statistic_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table, *alignment, *vbar, *entry;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct statistic_data));
	if(!data) return NULL;

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Statistics Report"));

	//set the window icon
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "report_stats.svg");


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
	label = make_label(_("_For:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_STATSELECT);
	data->CY_for = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_for, 1, 2, row, row+1);

	row++;
	label = make_label(_("_View:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_KIND2);
	data->CY_view = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("By _amount"));
	data->CM_byamount = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Minor currency"));
	data->CM_minor = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

/*
	row++;
	widget = gtk_check_button_new_with_mnemonic ("Legend");
	data->CM_legend = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);
*/
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

	//ui manager
	actions = gtk_action_group_new ("Account");

	//as we use gettext
   	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	// data to action callbacks is set here (data)
	gtk_action_group_add_actions (actions, entries, n_entries, data);

	/* set which action should have priority in the toolbar */
	action = gtk_action_group_get_action(actions, "List");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Bar");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Pie");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Detail");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Filter");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Refresh");
	g_object_set(action, "is_important", TRUE, NULL);


	ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building UI failed: %s", error->message);
		g_error_free (error);
	}

	data->ui = ui;
	data->actions = actions;

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos + minor
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);

/*
	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
*/
	entry = gtk_label_new(NULL);
	data->TX_total[2] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Balance:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[1] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Income:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[0] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Expense:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	notebook = gtk_notebook_new();
	data->GR_result = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

	//page: list
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, NULL);

	widget = gtk_scrolled_window_new (NULL, NULL);
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (widget), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	treeview = create_list_statistic();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = widget;
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (widget), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	treeview = create_list_operation(PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);

    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//page: 2d bar
	widget = gtk_chart_new(CHART_BAR_TYPE);
	data->RE_bar = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//page: 2d pie
	widget = gtk_chart_new(CHART_PIE_TYPE);
	data->RE_pie = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);


	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor), GLOBALS->minor);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_byamount), PREFS->stat_byamount);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_view), 1);

	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail))), "minor", (gpointer)data->CM_minor);

	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (statistic_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (statistic_toggle_minor), NULL);

    data->handler_id[HID_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (statistic_date_change), (gpointer)data);
    data->handler_id[HID_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (statistic_date_change), (gpointer)data);

	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (statistic_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (statistic_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (statistic_range_change), NULL);

    g_signal_connect (data->CY_for, "changed", G_CALLBACK (statistic_compute), (gpointer)data);
    data->handler_id[HID_VIEW] = g_signal_connect (data->CY_view, "changed", G_CALLBACK (statistic_compute), (gpointer)data);

	g_signal_connect (data->CM_byamount, "toggled", G_CALLBACK (statistic_update), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (statistic_selection), NULL);


	//setup, init and show window
	statistic_setup(data);

	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

	//setup, init and show window
	wg = &PREFS->sta_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);

	statistic_busy(window, TRUE);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	//gtk_widget_hide(data->GR_detail);



	statistic_sensitive(window, NULL);
	statistic_update_detail(window, NULL);

	DB( g_print("range: %d\n", PREFS->filter_range) );

	if( PREFS->filter_range != 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
		statistic_compute(window, NULL);

	statistic_busy(window, FALSE);


	return window;
}

/*
** ============================================================================
*/

static void statistic_rate_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
   {
     GtkWidget *widget;

     widget = g_object_get_data(G_OBJECT(model), "minor");

	//todo g_assert here and null test

     gdouble  tmp;
     gchar   buf[128];
     gboolean minor;

     //todo
	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &tmp, -1);

	if(tmp != 0.0)
	{
		g_snprintf(buf, sizeof(buf), "(%.2f)", tmp);
		g_object_set(renderer, "text", buf, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);

}


static void statistic_amount_cell_data_function (GtkTreeViewColumn *col,
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
	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &value, -1);

	if( value )
	{
		widget = g_object_get_data(G_OBJECT(model), "minor");
		if(GTK_IS_TOGGLE_BUTTON(widget))
		{
			minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
		}
		else
			minor = 0;

		mystrfmon(buf, 127, value, minor);

		color = (value > 0) ? PREFS->color_inc : PREFS->color_exp;

		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);
	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}

}


static GtkTreeViewColumn *amount_list_statistic_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, statistic_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *rate_list_statistic_column(gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "%");
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", id);
	gtk_tree_view_column_set_cell_data_func(column, renderer, statistic_rate_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);

	//gtk_tree_view_column_set_visible(column, FALSE);

	return column;
}

/*
** create our statistic list
*/
static GtkWidget *create_list_statistic(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_STAT,
		G_TYPE_INT,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);

	/* column: Name */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Result"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, ope_result_cell_data_function, NULL, NULL);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_STAT_NAME);
	//gtk_tree_view_column_set_sort_column_id (column, LST_STAT_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Expense */
	column = amount_list_statistic_column(_("Expense"), LST_STAT_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = rate_list_statistic_column(LST_STAT_EXPRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = amount_list_statistic_column(_("Income"), LST_STAT_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = rate_list_statistic_column(LST_STAT_INCRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Balance */
	column = amount_list_statistic_column(_("Balance"), LST_STAT_BALANCE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = rate_list_statistic_column(LST_STAT_BALRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_STAT_POS    , stat_list_compare_func, GINT_TO_POINTER(LST_STAT_POS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_STAT_EXPENSE, stat_list_compare_func, GINT_TO_POINTER(LST_STAT_EXPENSE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_STAT_INCOME , stat_list_compare_func, GINT_TO_POINTER(LST_STAT_INCOME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_STAT_BALANCE, stat_list_compare_func, GINT_TO_POINTER(LST_STAT_BALANCE), NULL);


	return(view);
}

static gint stat_list_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint sortcol = GPOINTER_TO_INT(userdata);
gint ret = 0;
gint pos1, pos2;
gdouble val1, val2;

	gtk_tree_model_get(model, a,
		LST_STAT_POS, &pos1,
		sortcol, &val1,
		-1);
	gtk_tree_model_get(model, b,
		LST_STAT_POS, &pos2,
		sortcol, &val2,
		-1);
/*
	if(pos1 == -1) return(1);
	if(pos2 == -1) return(-1);
*/

	if(sortcol == LST_STAT_POS)
		ret = pos2 - pos1;
	else
		ret = (ABS(val1) - ABS(val2)) > 0 ? 1 : -1;

	//DB( g_print(" sort %d=%d or %.2f=%.2f :: %d\n", pos1,pos2, val1, val2, ret) );

    return ret;
  }

