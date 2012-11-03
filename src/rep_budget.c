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
#include "dsp_wallet.h"
#include "rep_budget.h"
#include "list_operation.h"

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

enum {
	HID_MINDATE,
	HID_MAXDATE,
	HID_MONTH,
	HID_YEAR,
	HID_RANGE,
	MAX_HID
};

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


struct repbudget_data
{
	GtkWidget	*window;
	GtkUIManager	*ui;

	GtkWidget	*TB_bar;

	GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*CY_for;
	GtkWidget	*CY_view;
	GtkWidget	*LV_report;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*TX_total[3];

	GtkWidget	*RE_bar;
	GtkWidget	*RE_pie;

	GtkWidget	*GR_detail;
	GtkWidget	*LV_detail;

	guint32		mindate, maxdate;

	gdouble		total_spent;
	gdouble		total_budget;

	gboolean	detail;
	gboolean	legend;

	gulong		handler_id[MAX_HID];
};

/* prototypes */
static void repbudget_action_viewlist(GtkAction *action, gpointer user_data);
static void repbudget_action_viewbar(GtkAction *action, gpointer user_data);
static void repbudget_action_detail(GtkAction *action, gpointer user_data);
static void repbudget_action_legend(GtkAction *action, gpointer user_data);
static void repbudget_action_refresh(GtkAction *action, gpointer user_data);


gchar *CYA_BUDGETSELECT[] = { N_("Spent & Budget"), N_("Spent"), N_("Budget"), N_("Decay"), NULL };

gchar *CYA_KIND[] = { N_("Expense"), N_("Income"), NULL };

extern gchar *CYA_RANGE[];
extern gchar *CYA_SELECT[];

static GtkActionEntry entries[] = {
  { "List"    , "hb-stock-view-list" , N_("List")  , NULL,    N_("View results as list"), G_CALLBACK (repbudget_action_viewlist) },
  { "Bar"     , "hb-stock-view-bar"  , N_("Bar")   , NULL,    N_("View results as bars"), G_CALLBACK (repbudget_action_viewbar) },

  { "Detail"  , "hb-stock-ope-show"  , N_("Detail"), NULL,    N_("Toggle detail"), G_CALLBACK (repbudget_action_detail) },
  { "Legend"  , "hb-stock-legend"    , N_("Legend"), NULL,    N_("Toggle legend"), G_CALLBACK (repbudget_action_legend) },

  { "Refresh" , "hb-stock-refresh"   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (repbudget_action_refresh) },
};

static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Bar'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"    <toolitem action='Legend'/>"
"      <separator/>"
"    <toolitem action='Refresh'/>"
"  </toolbar>"
"</ui>";

/* list stat */
enum
{
	LST_BUDGET_POS,
	LST_BUDGET_KEY,
	LST_BUDGET_NAME,
	LST_BUDGET_SPENT,
	LST_BUDGET_BUDGET,
	LST_BUDGET_DECAY,
	NUM_LST_BUDGET
};

/* prototypes */

static void repbudget_date_change(GtkWidget *widget, gpointer user_data);
static void repbudget_period_change(GtkWidget *widget, gpointer user_data);
static void repbudget_range_change(GtkWidget *widget, gpointer user_data);
static void repbudget_toggle_detail(GtkWidget *widget, gpointer user_data);
static void repbudget_detail(GtkWidget *widget, gpointer user_data);
static void repbudget_compute(GtkWidget *widget, gpointer user_data);
static void repbudget_update_total(GtkWidget *widget, gpointer user_data);
static void repbudget_sensitive(GtkWidget *widget, gpointer user_data);
static void repbudget_toggle_legend(GtkWidget *widget, gpointer user_data);
static void repbudget_toggle(GtkWidget *widget, gpointer user_data);
static GtkWidget *create_list_budget(void);
static void repbudget_update_detail(GtkWidget *widget, gpointer user_data);

/* action functions -------------------- */
static void repbudget_action_viewlist(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	repbudget_sensitive(data->window, NULL);
}

static void repbudget_action_viewbar(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	repbudget_sensitive(data->window, NULL);
}

static void repbudget_action_detail(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_toggle_detail(data->window, NULL);
}

static void repbudget_action_legend(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_toggle_legend(data->window, NULL);
}

static void repbudget_action_refresh(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_compute(data->window, NULL);
}



/* ======================== */


static gint getmonth(guint date)
{
GDate *date1;
gint month;

	date1 = g_date_new_julian(date);
	month = g_date_get_month(date1);

	#if MYDEBUG == 1
		gchar buffer1[128];
		g_date_strftime (buffer1, 128-1, "%x", date1);
		g_print("  date is '%s'\n", buffer1);

		g_print(" month is %d\n", month);

	#endif

	g_date_free(date1);

	return(month);
}

static gint countmonth(guint32 mindate, guint32 maxdate)
{
GDate *date1, *date2;
gint nbmonth;

	date1 = g_date_new_julian(mindate);
	date2 = g_date_new_julian(maxdate);

	nbmonth = 0;
	while(g_date_compare(date1, date2) < 0)
	{
		nbmonth++;
		g_date_add_months(date1, 1);
	}

	g_date_free(date2);
	g_date_free(date1);

	return(nbmonth);
}


static void repbudget_date_change(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	DB( g_print("(repbudget) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	repbudget_compute(widget, NULL);

}

static void repbudget_period_change(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gint month, year;

	DB( g_print("(repbudget) period change\n") );

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

	repbudget_compute(widget, NULL);

}

static void repbudget_range_change(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(repbudget) range change\n") );

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

		repbudget_compute(widget, NULL);
	}
}

static void repbudget_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(repbudget) toggle detail\n") );

	data->detail ^= 1;

	repbudget_update_detail(widget, user_data);

}		
		
static void repbudget_update_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(repbudget) toggle detail\n") );

	if(data->detail)
	{
	GtkTreeSelection *treeselection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	guint key;

		treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_report));

		if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
		{
			gtk_tree_model_get(model, &iter, LST_BUDGET_KEY, &key, -1);

			//DB( g_print(" - active is %d\n", pos) );

			repbudget_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
		}



		gtk_widget_show(data->GR_detail);
	}
	else
		gtk_widget_hide(data->GR_detail);
}


static void repbudget_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
guint active = GPOINTER_TO_INT(user_data);
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(repbudget) detail\n") );

	if(data->detail)
	{
		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		/* fill in the model */
		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope = list->data;

			//DB( g_print(" get %s\n", ope->ope_Word) );


			//filter here
			if( !(ope->flags & OF_REMIND) && ope->date >= data->mindate && ope->date <= data->maxdate)
			{
			gint pos = 0;

				pos = ope->category;

				//insert
				if( pos == active )
				{

			    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						LST_DSPOPE_DATAS, ope,
						-1);
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


static void repbudget_compute(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

gint tmpfor, tmpview;
gint mindate, maxdate;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint n_result, id, column;
gdouble *tmp_spent, *tmp_budget;
gint nbmonth = 1;

	DB( g_print("(repbudget) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor   = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpview  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

	mindate = data->mindate;
	maxdate = data->maxdate;

	DB( g_print(" kind=%d,view=%d\n", tmpfor, tmpview) );


	/* do nothing if no operation */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	nbmonth = countmonth(mindate, maxdate);
	DB( g_print(" date: min=%d max=%d nbmonth=%d\n", mindate, maxdate, nbmonth) );

	n_result = da_cat_get_max_key();
	DB( g_print(" nbcat=%d\n", n_result) );

	/* allocate some memory */
	tmp_spent  = g_malloc0((n_result+1) * sizeof(gdouble));
	tmp_budget = g_malloc0((n_result+1) * sizeof(gdouble));

	if(tmp_spent && tmp_budget)
	{
	guint i = 0;
		/* compute the results */
		data->total_spent = 0.0;
		data->total_budget = 0.0;

		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope = list->data;
		Account *acc;

			DB( g_print("%d, get ope: %s :: acc=%d\n", i, ope->wording, ope->account) );

			acc = da_acc_get(ope->account);
			if(acc != NULL)
			{
				if((acc->flags & AF_CLOSED)) goto next;
				if(!(acc->flags & AF_BUDGET)) goto next;

				if( !(ope->flags & OF_REMIND) && ope->date >= mindate && ope->date <= maxdate)
				{
				gint pos;

					pos = ope->category;
					if( pos >= 0)
						tmp_spent[pos] += ope->amount;
				}
			}	

		next:
			list = g_list_next(list);
			i++;
		}

		DB( g_print("clear and detach model\n") );

		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

		/* insert into the treeview */
		for(i=0, id=0; i<n_result; i++)
		{
		gchar *name, *fullcatname;
		Category *entry;
		//gchar buffer[128];

			fullcatname = NULL;

			entry = da_cat_get(i);
			if( entry == NULL)
				continue;

			//debug
			#if MYDEBUG == 1
			gint k;

			g_print("--------\n");

			g_print("+ %s", entry->name);
			for(k=0;k<13;k++)
				g_print( "%d[%.2f] ", k, entry->budget[k]);
			g_print("\n");
	
			#endif

			if(entry->flags & GF_SUB)
			{
			Category *parent = da_cat_get( entry->parent);

				fullcatname = g_strdup_printf("%s:%s", parent->name, entry->name);
				name = fullcatname;
			}
			else
				name = entry->name;

			// display expense or income
			if( tmpfor !=  ((entry->flags & GF_INCOME) ? 1 : 0)) continue;

			if(name == NULL) name  = "(None)";

			// same value each month ?
			if(!(entry->flags & GF_CUSTOM))
			{
				DB( g_print(" cat %s -> monthly %.2f\n", name, entry->budget[0]) );
				tmp_budget[i] = entry->budget[0]*nbmonth;
			}
			//otherwise	sum each month from mindate month		
			else
			{
			gint month = getmonth(mindate);
			gint j;

				DB( g_print(" cat '%s' -> custom month=%ld nbmonth=%ld\n", name, month, nbmonth) );

				for(j=0;j<nbmonth;j++)
				{
					DB( g_print(" %d, cat %s -> custom j=%ld month=%ld budg=%.2f\n", j, name, month, tmp_budget[i]) );

					tmp_budget[i] += entry->budget[month];
					month++;
					if(month > 12) month = 1;
				}

				
			}


			//g_print(" inserting %i, %s\n", i, name);

			DB( g_print(" inserting %i, %s, %.2f %.2f %.2f\n", i, name, tmp_spent[i], tmp_budget[i], tmp_spent[i] - tmp_budget[i]) );

			if(tmp_budget[i])
			{
		    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_BUDGET_POS, id++,
					LST_BUDGET_KEY, i,
					LST_BUDGET_NAME, name,
					LST_BUDGET_SPENT, tmp_spent[i],
					LST_BUDGET_BUDGET, tmp_budget[i],
					LST_BUDGET_DECAY, ABS(tmp_budget[i]) - ABS(tmp_spent[i]),
					-1);

				data->total_spent  += tmp_spent[i];
				data->total_budget += tmp_budget[i];
			}

			g_free(fullcatname);
		}

		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);

		

		repbudget_update_total(widget, NULL);

		column = LST_BUDGET_SPENT+tmpview-1;

		/* update bar chart */
		DB( g_print(" set bar to %d\n\n", column) );

		if( tmpview == 0 )
			gtk_chart_set_dualdatas(GTK_CHART(data->RE_bar), model, LST_BUDGET_SPENT, LST_BUDGET_BUDGET);
		else
			gtk_chart_set_datas(GTK_CHART(data->RE_bar), model, column);

		gtk_chart_set_title(GTK_CHART(data->RE_bar), _(CYA_BUDGETSELECT[tmpfor]));

		
		
	}


	//DB( g_print(" inserting %i, %f %f\n", i, total_expense, total_income) );

	/* free our memory */
	g_free(tmp_spent);
	g_free(tmp_budget);

  /* update info range text */
/*
	{
	gchar buffer1[128];
	gchar buffer2[128];
	GDate *date;
	gchar *info;

		date = g_date_new_julian(data->mindate);
		g_date_strftime (buffer1, 128-1, "%x", date);
		g_date_set_julian(date, data->maxdate);
		g_date_strftime (buffer2, 128-1, "%x", date);
		g_date_free(date);

		info = g_strdup_printf("%s -> %s", buffer1, buffer2);

		gtk_label_set_text(GTK_LABEL(data->TX_info), info);


		g_free(info);
	}
*/


}


static void repbudget_update_total(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gboolean minor;

	DB( g_print("(repbudget) update total\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	hb_label_set_colvalue(GTK_LABEL(data->TX_total[0]), data->total_spent, minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[1]), data->total_budget, minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[2]), ABS(data->total_budget) - ABS(data->total_spent), minor);

}


/*
** update sensitivity
*/
static void repbudget_sensitive(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gboolean active;
gboolean sensitive;
gint page;

	DB( g_print("(repbudget) sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	active = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	sensitive = page == 0 ? active : FALSE;
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_DETAIL], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail"), sensitive);

	sensitive = page == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CY_view, sensitive);
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_LEGEND], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Legend"), sensitive);

}



/*
** change the chart legend visibility
*/
static void repbudget_toggle_legend(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	DB( g_print("(repbudget) legend\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_legend));
	data->legend ^= 1;

	gtk_chart_show_legend(GTK_CHART(data->RE_bar), data->legend);

}

static void repbudget_toggle(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gboolean minor;

	DB( g_print("(repbudget) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repbudget_update_total(widget, NULL);

	//wallet_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_show_minor(GTK_CHART(data->RE_bar), minor);

}



static void repbudget_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key;

	DB( g_print("(repbudget) selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_BUDGET_KEY, &key, -1);

		DB( g_print(" - active is %d\n", key) );

		repbudget_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	}

	repbudget_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}
/*
**
*/
static gboolean repbudget_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repbudget_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("(repbudget) start dispose\n") );

	g_free(data);

	//store position and size
	wg = &PREFS->bud_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

	DB( g_print("(repbudget) end dispose\n") );

	return FALSE;
}


// the window creation
GtkWidget *repbudget_window_new(void)
{
struct repbudget_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table, *alignment, *vbar, *entry;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GError *error = NULL;

	data = g_malloc0(sizeof(struct repbudget_data));
	if(!data) return NULL;

	DB( g_print("(repbudget) new\n") );

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Budget report"));

	//set the window icon
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "report_budget.svg");


	//window contents
	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//control part
	table = gtk_table_new (9, 2, FALSE);
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
	label = make_label(_("_Kind:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_KIND);
	data->CY_for = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_for, 1, 2, row, row+1);

	row++;
	label = make_label(_("_View:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_BUDGETSELECT);
	data->CY_view = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_view, 1, 2, row, row+1);

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

	//ui manager
	actions = gtk_action_group_new ("Account");

	//as we use gettext
   	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	// data to action callbacks is set here (data)
	gtk_action_group_add_actions (actions, entries, n_entries, data);

	ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building UI failed: %s", error->message);
		g_error_free (error);
	}

	data->ui = ui;	

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos
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
	label = gtk_label_new(_("Decay:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[1] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Budget:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[0] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Spent:"));
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
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	treeview = create_list_budget();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = widget;
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	treeview = create_list_operation(PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);

    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//page: 2d bar
	widget = gtk_chart_new(CHART_BAR_TYPE);
	data->RE_bar = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);


	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail))), "minor", (gpointer)data->CM_minor);

	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repbudget_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repbudget_toggle), NULL);
	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (repbudget_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (repbudget_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repbudget_range_change), NULL);

    g_signal_connect (data->CY_view, "changed", G_CALLBACK (repbudget_compute), (gpointer)data);
    g_signal_connect (data->CY_for, "changed", G_CALLBACK (repbudget_compute), (gpointer)data);

    data->handler_id[HID_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repbudget_date_change), (gpointer)data);
    data->handler_id[HID_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repbudget_date_change), (gpointer)data);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (repbudget_selection), NULL);

	//setup, init and show window
	//repbudget_setup(data);

	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);


	//setup, init and show window
	wg = &PREFS->bud_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	//debug
	//get our min max date


	data->detail = PREFS->budg_showdetail;
	data->legend = 1;

	if(g_list_length(GLOBALS->ope_list) > 0)
	{
	GList *list;
	GDate *date;
	guint year;

		da_operation_sort(GLOBALS->ope_list);
		list = g_list_first(GLOBALS->ope_list);
		data->mindate = ((Operation *)list->data)->date;
		list = g_list_last(GLOBALS->ope_list);
		data->maxdate   = ((Operation *)list->data)->date;

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_MINDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->mindate);
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_MINDATE]);

		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_MAXDATE]);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->maxdate);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_MAXDATE]);

		date = g_date_new_julian(data->maxdate);
		year = g_date_get_year(date);
		g_date_free(date);

		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), year);
		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

	}

	gtk_widget_show_all (window);


	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);


	//gtk_widget_hide(data->GR_detail);

	repbudget_sensitive(window, NULL);
	repbudget_update_detail(window, NULL);

	if( PREFS->filter_range != 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
		repbudget_compute(window, NULL);

	return(window);
}

/*
** ============================================================================
*/
static void budget_amount_cell_data_function (GtkTreeViewColumn *col,
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

static GtkTreeViewColumn *amount_list_budget_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, budget_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

/*
** create our statistic list
*/
static GtkWidget *create_list_budget(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_BUDGET,
		G_TYPE_INT,
		G_TYPE_INT,
		G_TYPE_STRING,
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
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, ope_result_cell_data_function, NULL, NULL);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_BUDGET_NAME);
	//gtk_tree_view_column_set_sort_column_id (column, LST_STAT_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Expense */
	column = amount_list_budget_column(_("Spent"), LST_BUDGET_SPENT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = amount_list_budget_column(_("Budget"), LST_BUDGET_BUDGET);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Balance */
	column = amount_list_budget_column(_("Decay"), LST_BUDGET_DECAY);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* sort */
/*
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_POS   , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_POS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_SPENT , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_SPENT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_BUDGET, stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_BUDGET), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_DECAY , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_DECAY), NULL);
*/

	return(view);
}

