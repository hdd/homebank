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
#include "rep_over.h"
#include "ui_account.h"
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
	MAX_HID
};

struct repover_data
{
	GtkWidget	*window;

	gint	busy;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkWidget	*TB_bar;
	
	GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*LV_report;
	GtkWidget	*PO_acc;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*RE_line;

	guint32		mindate, maxdate;

	gdouble		minimum;
	guint		nbover, nbope;

	gulong		handler_id[MAX_HID];

};

/* prototypes */
static void repover_action_viewlist(GtkAction *action, gpointer user_data);
static void repover_action_viewline(GtkAction *action, gpointer user_data);
static void repover_action_refresh(GtkAction *action, gpointer user_data);

//todo amiga/linux
//prev
//next

static GtkActionEntry entries[] = {
  { "List"    , "hb-stock-view-list" , N_("List")   , NULL,   N_("View results as list"), G_CALLBACK (repover_action_viewlist) },
  { "Line"    , "hb-stock-view-line" , N_("Line")   , NULL,   N_("View results as lines"), G_CALLBACK (repover_action_viewline) },

  { "Refresh" , "hb-stock-refresh"   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (repover_action_refresh) },
};

static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Line'/>"
"      <separator/>"
"    <toolitem action='Refresh'/>"
"  </toolbar>"
"</ui>";

/* list stat */
enum
{
	LST_OVER_OVER,
	LST_OVER_DATE,
	LST_OVER_DATESTR,
	LST_OVER_WORDING,
	LST_OVER_EXPENSE,
	LST_OVER_INCOME,
	LST_OVER_BALANCE,
	NUM_LST_OVER
};

extern gchar *CYA_RANGE[];
extern gchar *CYA_SELECT[];

/* prototypes */
static void repover_date_change(GtkWidget *widget, gpointer user_data);
static void repover_period_change(GtkWidget *widget, gpointer user_data);
static void repover_range_change(GtkWidget *widget, gpointer user_data);
static void repover_update_info(GtkWidget *widget, gpointer user_data);
static void repover_toggle_minor(GtkWidget *widget, gpointer user_data);
static void repover_compute(GtkWidget *widget, gpointer user_data);
static void repover_setup(struct repover_data *data);
static gboolean repover_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static GtkWidget *create_list_repover(void);

static void repover_busy(GtkWidget *widget, gboolean state);


/* action functions -------------------- */
static void repover_action_viewlist(GtkAction *action, gpointer user_data)
{
struct repover_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	//repover_sensitive(data->window, NULL);
}

static void repover_action_viewline(GtkAction *action, gpointer user_data)
{
struct repover_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	//repover_sensitive(data->window, NULL);
}

static void repover_action_refresh(GtkAction *action, gpointer user_data)
{
struct repover_data *data = user_data;

	repover_compute(data->window, NULL);
}



/* ======================== */






static void repover_date_change(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;

	DB( g_print("(repover) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	repover_compute(widget, NULL);

}


static void repover_period_change(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;
gint month, year;

	DB( g_print("(repover) period change\n") );

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

	repover_compute(widget, NULL);

}

static void repover_range_change(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(repover) range change\n") );

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

		repover_compute(widget, NULL);
	}
}






static void repover_update_info(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;
gchar *info;
gchar   buf[128];

	DB( g_print("(repover) update info\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	mystrfmon(buf, 127, data->minimum, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor)) );

	info = g_strdup_printf(_("%d/%d under %s"), data->nbover, data->nbope, buf);
	gtk_label_set_text(GTK_LABEL(data->TX_info), info);
	g_free(info);
}




static void repover_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;
gboolean minor;

	DB( g_print("(repover) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repover_update_info(widget,NULL);

	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_show_minor(GTK_CHART(data->RE_line), minor);
}





static void repover_compute(GtkWidget *widget, gpointer user_data)
{
struct repover_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gdouble balance;
gint acckey;
Account *acc;
guint32 lastdate;

	DB( g_print("(repover) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* do nothing if no operation */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	// get the account key
	acckey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX_ENTRY(data->PO_acc));
	data->nbope = 0;
	data->nbover = 0;

	DB( g_print(" acc key = %d\n", acckey) );


		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */


	// get the initial balance
	acc = da_acc_get(acckey);
	if( acc != NULL )
	{
		data->minimum = acc->minimum;
		balance = acc->initial;


		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope = list->data;

			if( (ope->account == acckey) && !(ope->flags & OF_REMIND) )
			{
			gdouble expense = ope->amount < 0 ? ope->amount : 0;
			gdouble income  = ope->amount > 0 ? ope->amount : 0;

				balance += ope->amount;

				if( (ope->date >= data->mindate) && (ope->date <= data->maxdate) )
				{
				gboolean is_over;
				GDate *date;
				gchar buf[256];
				//guint32 decay;

					is_over = balance < acc->minimum ? TRUE : FALSE;

					//todo
					/*
					decay = ope->date - lastdate;
					if(data->nbope > 0 && decay > 1)
					{
						while(decay > 1)
						{
							lastdate++;

							date = g_date_new_julian (lastdate);
							g_date_strftime (buf, 256-1, PREFS->date_format, date);
							g_date_free(date);
						
							gtk_list_store_append (GTK_LIST_STORE(model), &iter);
							gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								LST_OVER_OVER, is_over,
								LST_OVER_DATE, lastdate,
								LST_OVER_DATESTR, buf,
								LST_OVER_BALANCE, balance,
								-1);
					
						
						
							decay--;						
						}
					}
					*/

					date = g_date_new_julian (ope->date);
					g_date_strftime (buf, 256-1, PREFS->date_format, date);
					g_date_free(date);


		/* column 0: pos (gint) */
		/* not used: column 1: key (gint) */
		/* column 2: name (gchar) */
		/* column x: values (double) */

			    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			    	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						LST_OVER_OVER, is_over,
						LST_OVER_DATE, ope->date,
						LST_OVER_DATESTR, buf,
						LST_OVER_WORDING, ope->wording,
						LST_OVER_EXPENSE, expense,
						LST_OVER_INCOME, income,
						LST_OVER_BALANCE, balance,
						-1);

					if(is_over == TRUE)
						data->nbover++;
				
					data->nbope++;
					lastdate = ope->date;
				}
			}
			list = g_list_next(list);
		}


		repover_update_info(widget, NULL);


		gtk_chart_show_legend(GTK_CHART(data->RE_line), FALSE);
		gtk_chart_show_xval(GTK_CHART(data->RE_line), TRUE);
		gtk_chart_set_overdrawn(GTK_CHART(data->RE_line), acc->minimum);
		gtk_chart_show_overdrawn(GTK_CHART(data->RE_line), TRUE);
		
		


	}

	/* Re-attach model to view */
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
	g_object_unref(model);

		/* update bar chart */
		//DB( g_print(" set bar to %d\n\n", LST_STAT_EXPENSE+tmpkind) );
		gtk_chart_set_datas(GTK_CHART(data->RE_line), model, LST_OVER_BALANCE);
		//gtk_chart_set_line_datas(GTK_CHART(data->RE_line), model, LST_OVER_BALANCE, LST_OVER_DATE);


}


static void repover_busy(GtkWidget *widget, gboolean state)
{
struct repover_data *data;
GtkWidget *window;
GdkCursor *cursor;

	DB( g_printf("(repover) busy\n") );

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
static void repover_setup(struct repover_data *data)
{
	DB( g_print("(repover) setup\n") );

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

	ui_acc_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(data->PO_acc), GLOBALS->h_acc);
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(data->PO_acc), 1);

}


/*
**
*/
static gboolean repover_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repover_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("(repover) dispose\n") );

	g_free(data);

	//store position and size
	wg = &PREFS->ove_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

	return FALSE;
}


// the window creation
GtkWidget *repover_window_new(void)
{
struct repover_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table, *alignment;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GError *error = NULL;

	data = g_malloc0(sizeof(struct repover_data));
	if(!data) return NULL;

	DB( g_print("(repover) new\n") );

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Overdrawn report"));

	//set the window icon
	homebank_window_set_icon_from_file(GTK_WINDOW (window), "report_overdrawn.svg");




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
	label = make_label(_("A_ccount:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_widget_set_size_request (widget, 10, -1);
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
	data->actions = actions;	

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);


	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);


	notebook = gtk_notebook_new();
	data->GR_result = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

	//page: list
	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	treeview = create_list_repover();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//page: 2d lines
	widget = gtk_chart_new(CHART_LINE_TYPE);
	data->RE_line = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);




	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);



	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);





	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repover_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repover_toggle_minor), NULL);


    data->handler_id[HID_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repover_date_change), (gpointer)data);
    data->handler_id[HID_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repover_date_change), (gpointer)data);

	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (repover_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (repover_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repover_range_change), NULL);

//	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (statistic_selection), NULL);

	//setup, init and show window
	repover_setup(data);

	//let this here or the setup trigger a compute...
	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (repover_compute), NULL);


	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);


	//setup, init and show window
	wg = &PREFS->ove_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);

	repover_busy(window, TRUE);


	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	//repover_sensitive(window, NULL);
	//repover_compute(window, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);

	repover_busy(window, FALSE);


	return(window);
}

/*
** ============================================================================
*/


static void repover_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gchar *datestr;
gchar *markuptxt;
gboolean is_over;

	gtk_tree_model_get(model, iter,
		LST_OVER_DATESTR, &datestr,
		LST_OVER_OVER, &is_over,
		-1);

	if(is_over==TRUE)
	{
		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", PREFS->color_warn, datestr);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);
	}
	else
	g_object_set(renderer, "markup", datestr, NULL);

}

static void repover_text_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gchar *buf;
gchar *markuptxt;
gboolean is_over;

	gtk_tree_model_get(model, iter,
		LST_OVER_WORDING, &buf,
		LST_OVER_OVER, &is_over,
		-1);

	if(is_over==TRUE)
	{
		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", PREFS->color_warn, buf);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);
	}
	else
		g_object_set(renderer, "markup", buf, NULL);

}

static void repover_amount_cell_data_function (GtkTreeViewColumn *col,
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
gboolean is_over;

	//get datas
	gtk_tree_model_get(model, iter,
		LST_OVER_OVER, &is_over,
		GPOINTER_TO_INT(user_data), &value,
		-1);

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
		if(is_over==TRUE) color = PREFS->color_warn;

		markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, buf);

		//debug
		/*gboolean toto = g_utf8_validate(buf, -1, NULL);
		gboolean toto2 = g_utf8_validate(markuptxt, -1, NULL);
		g_print("DEBUG: '%s' (isutf8=%d) :: '%s' (isutf8=%d)\n", buf, toto, markuptxt, toto2);
		*/
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);
	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}

}

static GtkTreeViewColumn *amount_list_repover_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repover_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}


/*
** create our statistic list
*/
static GtkWidget *create_list_repover(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_OVER,
		G_TYPE_BOOLEAN,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);

	/* column debug over */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "debug over");
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_OVER_OVER);
*/

	/* column date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_OVER_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repover_date_cell_data_function, NULL, NULL);


	/* column wording */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Description"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_OVER_WORDING);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repover_text_cell_data_function, NULL, NULL);

	/* column: Expense */
	column = amount_list_repover_column(_("Expense"), LST_OVER_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = amount_list_repover_column(_("Income"), LST_OVER_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Balance */
	column = amount_list_repover_column(_("Balance"), LST_OVER_BALANCE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);



	return(view);
}
