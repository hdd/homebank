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

#include "da_transaction.h"
#include "hb_transaction.h"
#include "dsp_account.h"
#include "list_operation.h"
#include "def_archive.h"
#include "def_filter.h"
#include "def_operation.h"
#include "dsp_wallet.h"
#include "ui_payee.h"
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

//debug
#define UI 1

enum
{
	ACTION_ACCOUNT_ADD,
	ACTION_ACCOUNT_INHERIT,
	ACTION_ACCOUNT_EDIT,
	ACTION_ACCOUNT_VALIDATE,
	ACTION_ACCOUNT_REMOVE,
	ACTION_ACCOUNT_FILTER,
	ACTION_ACCOUNT_CLOSE,
	MAX_ACTION_ACCOUNT
};

enum {
	HID_MONTH,
	HID_YEAR,
	HID_RANGE,
	MAX_HID
};
struct account_data
{
	gchar	*wintitle;
	GtkWidget	*window;
	GtkWidget	*TB_bar;
	GtkWidget	*BT_help;

	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*CY_range;


	gint	busy;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkWidget	*GR_info;
	GtkWidget	*CM_minor;
	GtkWidget	*TX_info;
	GtkWidget	*TX_balance[3];
	GtkWidget	*GO_gauge;

	GtkWidget	*LV_ope;
	GtkWidget	*statusbar;

	double	bank, today, future;

	Operation cur_ope;
	Operation bak_ope;
	gint accnum;
	Account *acc;

	/* status counters */
	gint	hidden, total;

	Filter		*filter;

	gulong		handler_id[MAX_HID];

	//gint change;	/* change shouldbe done directly */

};


extern gchar *CYA_RANGE[];
extern gchar *CYA_SELECT[];




/* our functions prototype */
static void account_action_close(GtkAction *action, gpointer user_data);

static void account_action_editfilter(GtkAction *action, gpointer user_data);

static void account_action_add(GtkAction *action, gpointer user_data);
static void account_action_inherit(GtkAction *action, gpointer user_data);
static void account_action_edit(GtkAction *action, gpointer user_data);
static void account_action_validate(GtkAction *action, gpointer user_data);
static void account_action_remove(GtkAction *action, gpointer user_data);
static void account_action_createarchive(GtkAction *action, gpointer user_data);

static void account_action_exportcsv(GtkAction *action, gpointer user_data);

static void account_action_assign(GtkAction *action, gpointer user_data);
static void account_populate(GtkWidget *view);
static void account_action(GtkWidget *widget, gpointer user_data);
static void account_toggle(GtkWidget *widget, gpointer user_data);
static void account_update(GtkWidget *widget, gpointer user_data);

static void account_export_csv(GtkWidget *widget, gpointer user_data);

static void account_make_archive(GtkWidget *widget, gpointer user_data);
static void account_period_change(GtkWidget *widget, gpointer user_data);
static void account_range_change(GtkWidget *widget, gpointer user_data);
static void account_add(GtkWidget *widget, gpointer user_data);
static void account_populate(GtkWidget *view);
static void validate_selected_foreach_func (GtkTreeModel  *model, GtkTreePath   *path, GtkTreeIter   *iter, gpointer       userdata);
Operation *get_active_operation(GtkTreeView *treeview);
static void account_action(GtkWidget *widget, gpointer user_data);
static void account_toggle(GtkWidget *widget, gpointer user_data);
static void account_selection(GtkTreeSelection *treeselection, gpointer user_data);
static void account_update(GtkWidget *widget, gpointer user_data);
static void account_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);

static GtkActionEntry entries[] = {

  /* name, stock id, label */
  { "AccountMenu"  , NULL, N_("_Account") },
  { "OperationMenu", NULL, N_("Transacti_on") },
  { "ActionsMenu"  , NULL, N_("_Actions") },
  { "ToolsMenu"    , NULL, N_("_Tools") },

  { "Close"        , GTK_STOCK_CLOSE  , N_("_Close")        , NULL, N_("Close the current account"),    G_CALLBACK (account_action_close) },

	/* name, stock id, label, accelerator, tooltip */
  { "Filter"       , "hb-filter"      , N_("_Filter..."), NULL,    N_("Open the list filter"), G_CALLBACK (account_action_editfilter) },

  { "Add"          , HB_STOCK_OPE_ADD    , N_("_Add..."), NULL,    N_("Add a new transaction"), G_CALLBACK (account_action_add) },
  { "Inherit"      , HB_STOCK_OPE_HERIT  , N_("_Inherit..."), NULL, N_("Inherit from the active transaction"), G_CALLBACK (account_action_inherit) },
  { "Edit"         , HB_STOCK_OPE_EDIT   , N_("_Edit..."), NULL, N_("Edit the active transaction"),  G_CALLBACK (account_action_edit) },
  { "Validate"     , "hb-ope-valid"      , N_("(In)_Validate"), NULL,    N_("Validate the active transactions"), G_CALLBACK (account_action_validate) },
  { "Remove"       , HB_STOCK_OPE_DELETE , N_("_Remove..."), NULL,    N_("Remove the active transactions"), G_CALLBACK (account_action_remove) },
  { "MakeArchive"  , NULL                , N_("Make an archive..."), NULL,    NULL, G_CALLBACK (account_action_createarchive) },

  { "Assign"       , "hb-assign-run"     , N_("Auto. Assignments"), NULL,    N_("Run auto assignments"), G_CALLBACK (account_action_assign) },
  { "Export"       , "hb-file-export"    , N_("Export CSV..."), NULL,    N_("Export as CSV"), G_CALLBACK (account_action_exportcsv) },

};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <menubar name='MenuBar'>"

"    <menu action='AccountMenu'>"
"      <menuitem action='Close'/>"
"    </menu>"

"    <menu action='OperationMenu'>"
"      <menuitem action='Add'/>"
"      <menuitem action='Inherit'/>"
"      <menuitem action='Edit'/>"
"      <menuitem action='Remove'/>"
"        <separator/>"
"      <menuitem action='Validate'/>"
"        <separator/>"
"      <menuitem action='MakeArchive'/>"
"    </menu>"

"    <menu action='ActionsMenu'>"
"      <menuitem action='Assign'/>"
"        <separator/>"
"      <menuitem action='Export'/>"
"    </menu>"

"    <menu action='ToolsMenu'>"
"      <menuitem action='Filter'/>"
"        <separator/>"
"    </menu>"
"  </menubar>"

"  <toolbar  name='ToolBar'>"
"    <toolitem action='Add'/>"
"    <toolitem action='Inherit'/>"
"    <toolitem action='Edit'/>"
"    <toolitem action='Remove'/>"
"      <separator/>"
"    <toolitem action='Validate'/>"
"      <separator/>"
"    <toolitem action='Filter'/>"
"      <separator/>"
"    <toolitem action='Assign'/>"
"    <toolitem action='Export'/>"
"  </toolbar>"
"</ui>";


/* account action functions -------------------- */
static void account_action_close(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	DB( g_print("action close\n") );

	DB( g_print("window %x\n", data->window) );

	gtk_widget_destroy (GTK_WIDGET (data->window));
	
	//g_signal_emit_by_name(data->window, "delete-event", NULL, &result);

}


static void account_action_editfilter(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_FILTER));
}

static void account_action_add(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_ADD));
}

static void account_action_inherit(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_INHERIT));
}

static void account_action_edit(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_EDIT));
}

static void account_action_validate(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_VALIDATE));
}

static void account_action_remove(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_REMOVE));
}



static void account_action_createarchive(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_make_archive(data->window, NULL);
}




static void account_action_exportcsv(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_export_csv(data->window, NULL);
}



static void account_action_assign(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;
gint count;
gboolean usermode = TRUE;


	count = transaction_auto_assign(GLOBALS->ope_list, data->accnum);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_ope));
	GLOBALS->change += count;
	
	//inform the user
	if(usermode == TRUE)
	{
	gchar *txt;

		if(count == 0)
			txt = _("No transaction changed");
		else
			txt = _("%d transactions auto assigned");

		homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_INFO,
			_("Auto assigment result"),
			txt,
			count);
	}
	
}




/* these 5 functions are independant from account window */

/* account functions -------------------- */






static void account_export_csv(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
gchar *filename = NULL;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GIOChannel *io;

	DB( g_print("(account) export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
	{

		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{


			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));

			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Operation *ope;
			gchar *outstr;
			GDate *date;
			gchar datebuf[256];
			gchar *info, *payeename, *categoryname;
			Payee *payee;
			Category *category;
			char amountbuf[G_ASCII_DTOSTR_BUF_SIZE];

				gtk_tree_model_get (model, &iter,
						LST_DSPOPE_DATAS, &ope,
						-1);

			//date
				date = g_date_new_julian (ope->date);
				g_sprintf(datebuf, "%02d/%02d/%04d", 
					g_date_get_day(date),
					g_date_get_month(date),
					g_date_get_year(date)
					);
				g_date_free(date);

				info = ope->info;
				if(info == NULL) info = "";
				payee = da_pay_get(ope->payee);
				payeename = (payee->name == NULL) ? "" : payee->name;
				category = da_cat_get(ope->category);
				categoryname = (category->name == NULL) ? "" : da_cat_get_fullname(category);


				g_ascii_dtostr (amountbuf, sizeof (amountbuf), ope->amount);

				DB( g_print("amount = %f '%s'\n", ope->amount, amountbuf) );


				outstr = g_strdup_printf("%s;%d;%s;%s;%s;%s;%s\n", datebuf, ope->paymode, info, payeename, ope->wording, amountbuf, categoryname);

				DB( g_print("%s", outstr) );

				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				g_free(outstr);

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

		g_free( filename );

	}

}



/*
** make an archive with the active operation
*/
static void account_make_archive(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;

	DB( g_print("(account) make archive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

GtkWidget *p_dialog = NULL;
GtkTreeModel *model;
GList *selection, *list;
gint result, count;

	count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)));

	if( count > 0 )
	{
		  p_dialog = gtk_message_dialog_new
		  (
		    NULL,
		    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		    GTK_MESSAGE_WARNING,
			GTK_BUTTONS_YES_NO,
			_("Do you want to create an Archive with\neach of the selected transaction ?")
		  );

	/*
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
			_("%d archives will be created"),
			GLOBALS->change
			);
	*/

		result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
		gtk_widget_destroy( p_dialog );


		if(result == GTK_RESPONSE_YES)
		{

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
			selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)), &model);

			list = g_list_first(selection);
			while(list != NULL)
			{
			Archive *item;
			Operation *ope;
			GtkTreeIter iter;

				gtk_tree_model_get_iter(model, &iter, list->data);
				gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);

				DB( g_printf(" create archive %s %.2f\n", ope->wording, ope->amount) );

				item = da_archive_malloc();

				//fill it
				item->amount		= ope->amount;
				item->account		= ope->account;
				item->dst_account	= ope->dst_account;
				item->paymode		= ope->paymode;
				item->flags			= ope->flags  & (OF_INCOME);
				item->payee			= ope->payee;
				item->category		= ope->category;
				if(ope->wording != NULL)
					item->wording 		= g_strdup(ope->wording);
				else
					item->wording 		= g_strdup(_("(new archive)"));

				GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, item);
				GLOBALS->change++;

				list = g_list_next(list);
			}

			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}
	}
}




static void account_period_change(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
gint month, year;

	DB( g_print("(account) period change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	month = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_month));
	year = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_year));

	if(month != 0)
		get_period_minmax(month-1, year, &data->filter->mindate, &data->filter->maxdate);
	else
		get_period_minmax(0, year, &data->filter->mindate, &data->filter->maxdate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), 0);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);

	account_populate(data->LV_ope);
}

static void account_range_change(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(account) range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

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

		g_date_free(date);

		account_populate(data->LV_ope);
	}
}

/*
*	update balance with data->cur_ope amount for the active account (data->accnum)
*	call operation_add with data->cur_ope, data->accnum
*/
static void account_add(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
Operation *ope;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(account) add (data=%08x)\n", data) );

	ope = &data->cur_ope;
	if(ope->account == data->accnum)
	{
		DB( g_printf(" -> update balance %.2f\n", ope->amount) );

		//update our account balance
		if(!(ope->flags & OF_REMIND))
		{
			data->future += ope->amount;
			
			if(ope->date <= GLOBALS->today)
				data->today += ope->amount;

			if(ope->flags & OF_VALID)
				data->bank += ope->amount;

		}
	}

	operation_add(ope, data->LV_ope, data->accnum);

}



static void account_populate(GtkWidget *view)
{
struct account_data *data;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(view, GTK_TYPE_WINDOW)), "inst_data");


	DB( g_printf("(account) populate\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	//... insert a couple of thousand rows ...

	data->bank = 0;
	data->today = 0;
	data->future = 0;

	/* init our acc */
	{
	Account *acc;

		acc = da_acc_get(data->accnum);
		if(acc)
		{
			data->bank = acc->initial;
			data->today = acc->initial;
			data->future = acc->initial;
		}
	}

	data->hidden = 0;

	//insert all glist item into treeview
	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *ope;

		ope = list->data;
		if(ope->account == data->accnum)
		{

			if(!(ope->flags & OF_REMIND))
			{

		/* balances */
			data->future += ope->amount;

			if(ope->date <= GLOBALS->today)
				data->today += ope->amount;

			if(ope->flags & OF_VALID)
				data->bank += ope->amount;
			}

			/* then filter on date */
			if(filter_test(data->filter, ope) == 1)
			{
				/* append to our treeview */
		    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

	     		//g_printf(" populate: %s\n", ope->ope_Word);

	     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_DSPOPE_DATAS, ope,
					-1);

			}
			else
			{
				/* later */
				data->hidden++;

			}
					
		}


		list = g_list_next(list);
	}


  gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */

  g_object_unref(model);

  /* update info range text */
	{
	gchar buffer1[128];
	gchar buffer2[128];
	GDate *date;
	gchar *info;

		date = g_date_new_julian(data->filter->mindate);
		g_date_strftime (buffer1, 128-1, PREFS->date_format, date);
		g_date_set_julian(date, data->filter->maxdate);
		g_date_strftime (buffer2, 128-1, PREFS->date_format, date);
		g_date_free(date);

		info = g_strdup_printf("<small>%s - %s</small>", buffer1, buffer2);

		gtk_label_set_markup(GTK_LABEL(data->TX_info), info);


		g_free(info);
	}

	account_update(view, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));

}


static void validate_selected_foreach_func (GtkTreeModel  *model, GtkTreePath   *path, GtkTreeIter   *iter, gpointer       userdata)
{
struct account_data *data = userdata;
Operation *entry;
gint *indices;

	//path = gtk_tree_model_get_path(model, &iter);
	indices = gtk_tree_path_get_indices(path);

	DB( printf(" active is %d\n", indices[0]) );

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &entry, -1);


	if(!(entry->flags & OF_REMIND))
	{
		DB( printf("[account] line - toggle\n") );

		if(entry->flags & OF_VALID)
			data->bank -= entry->amount;
		else
			data->bank += entry->amount;
			entry->flags ^= OF_VALID;
			entry->flags |= OF_CHANGED;
	}

	/* deal with transfer to be validated also */
	/* #492755
	if( entry->paymode == PAYMODE_INTXFER )
	{
	Operation *ct;
		
		ct = operation_get_child_transfer(entry);
		if(ct != NULL)
		{
			ct->flags ^= OF_VALID;
			ct->flags |= OF_CHANGED;
		}
	}*/


}


Operation *get_active_operation(GtkTreeView *treeview)
{
GtkTreeModel *model;
GList *list;
Operation *ope;

	ope = NULL;

	model = gtk_tree_view_get_model(treeview);
	list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

	if(list != NULL)
	{
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);

	return ope;
}


static void remove_active_operation(GtkTreeView *treeview)
{
GtkTreeModel *model;
GList *list;

	model = gtk_tree_view_get_model(treeview);
	list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

	if(list != NULL)
	{
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
		
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);


}



static void account_action(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
gint action = (gint)user_data;
gboolean result;

	DB( g_printf("(account) action\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	DB( g_printf(" action=%d\n", action) );

	switch(action)
	{
		//add
		case ACTION_ACCOUNT_ADD:
		{
		GtkWidget *window;
		gint result = 1;
		guint32 date;
		gint account;

			DB( g_printf("(operation) add multiple\n") );

			/* init the operation */
			date = GLOBALS->today;
			account = data->accnum;

			DB( g_printf(" -> ope=%8x\n", &data->cur_ope) );

			window = create_defoperation_window(GTK_WINDOW(data->window), &data->cur_ope, OPERATION_EDIT_ADD, data->accnum);
			result = GTK_RESPONSE_ADD;
			while(result == GTK_RESPONSE_ADD)
			{
				/* fill in the operation */
				memset(&data->cur_ope, 0, sizeof(Operation));
				data->cur_ope.date    = date;
				data->cur_ope.account = data->accnum;

				defoperation_set_operation(window, &data->cur_ope);

				result = gtk_dialog_run (GTK_DIALOG (window));

				if(result == GTK_RESPONSE_ADD || result == GTK_RESPONSE_ACCEPT)
				{

					defoperation_get(window, NULL);

					account_add(widget, NULL);
					account_update(widget, GINT_TO_POINTER(UF_BALANCE));

					data->acc->flags |= AF_ADDED;
					GLOBALS->change++;

				}

				//keep these values for next
				date = data->cur_ope.date;
				account = data->cur_ope.account;

				//DB( g_printf(" -> result %d\n", result) );
			}
			defoperation_dispose(window, NULL);
		
			gtk_widget_destroy (window);
		}
		break;

		//inherit
		case ACTION_ACCOUNT_INHERIT:
	    {
		Operation *ope;
		GtkWidget *window;

			DB( g_printf("(operation) inherit multiple\n") );

			ope = get_active_operation(GTK_TREE_VIEW(data->LV_ope));
			if(ope)
			{
				window = create_defoperation_window(GTK_WINDOW(data->window), &data->cur_ope, OPERATION_EDIT_INHERIT, data->accnum);
				result = GTK_RESPONSE_ADD;
				while(result == GTK_RESPONSE_ADD)
				{
					/* fill in the operation */

					memcpy(&data->cur_ope, ope, sizeof(Operation));
					data->cur_ope.wording = g_strdup(ope->wording);
					data->cur_ope.info    = g_strdup(ope->info);

					//fix: 318733
					if( PREFS->heritdate == FALSE )
					{
						data->cur_ope.date = GLOBALS->today;
					}

					defoperation_set_operation(window, &data->cur_ope);

					result = gtk_dialog_run (GTK_DIALOG (window));

					if(result == GTK_RESPONSE_ADD || result == GTK_RESPONSE_ACCEPT)
					{
						defoperation_get(window, NULL);

						account_add(widget, NULL);
						account_update(widget, GINT_TO_POINTER(UF_BALANCE));

						data->acc->flags |= AF_ADDED;
						GLOBALS->change++;
					}

				}

				defoperation_dispose(window, NULL);
				gtk_widget_destroy (window);

			}
		}
		break;

		//edit
		case ACTION_ACCOUNT_EDIT:
	    {
		Operation *ope, *oldope;
		GtkWidget *window;

			ope = get_active_operation(GTK_TREE_VIEW(data->LV_ope));
			if(ope)
			{
				memcpy(&data->cur_ope, ope, sizeof(Operation));

				oldope = &data->cur_ope;
				window = create_defoperation_window(GTK_WINDOW(data->window), ope, OPERATION_EDIT_MODIFY, data->accnum);

				defoperation_set_operation(window, ope);

				result = gtk_dialog_run (GTK_DIALOG (window));

				if(result == GTK_RESPONSE_ACCEPT)
				{
					//sub the old amount to balances
					if(!(ope->flags & OF_REMIND))
					{
						data->future -= oldope->amount;
						if(ope->date <= GLOBALS->today)
							data->today -= oldope->amount;
						if(ope->flags & OF_VALID)
							data->bank -= oldope->amount;
					}
					//delete any child transfer
					if( ope->paymode == PAYMODE_INTXFER )
					{
						operation_delete_child_transfer(ope);
					}

					
					defoperation_get(window, NULL);


					//add our new amount to balances if ope->account == this account
					if( ope->account == data->accnum )
					{					
						if(!(ope->flags & OF_REMIND))
						{
							data->future += ope->amount;
							if(ope->date <= GLOBALS->today)
								data->today += ope->amount;
							if(ope->flags & OF_VALID)
								data->bank += ope->amount;
						}
					}
					else
					{
						//remove from the display
						remove_active_operation(GTK_TREE_VIEW(data->LV_ope));
				
					}


					if( ope->paymode == PAYMODE_INTXFER )
					{
					Operation *ct;
						
						ct = operation_get_child_transfer(ope);
						if(ct == NULL)
						{
							//todo:sould create the transeftr also
							operation_add_transfer(ope, data->LV_ope);
						}
					}

					account_update(widget, GINT_TO_POINTER(UF_BALANCE));

					data->acc->flags |= AF_CHANGED;
					GLOBALS->change++;

				}

				defoperation_dispose(window, NULL);
				gtk_widget_destroy (window);

			}

		}
		break;


		//delete
		case ACTION_ACCOUNT_REMOVE:
		{
		GtkWidget *p_dialog = NULL;
		GtkTreeModel *model;
		GList *selection, *list;
		gint result, count;

			DB( g_printf(" delete\n") );

			count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)));

			  p_dialog = gtk_message_dialog_new
			  (
			    NULL,
			    GTK_DIALOG_MODAL,
			    GTK_MESSAGE_WARNING,
				GTK_BUTTONS_YES_NO,
				_("Do you want to delete\neach of the selected transaction ?")
			  );

			/*
			gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
				_("%d operations will be definitively lost.\n"),
				GLOBALS->change
				);
			*/


			result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
			gtk_widget_destroy( p_dialog );


			if(result == GTK_RESPONSE_YES)
			{

				model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
				selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)), &model);

				g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
				gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), NULL); /* Detach model from view */
				

				DB( g_printf(" delete %d line\n", g_list_length(selection)) );

  
				list = g_list_last(selection);
				while(list != NULL)
				{
				Operation *entry;
				GtkTreeIter iter;

					gtk_tree_model_get_iter(model, &iter, list->data);
					gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &entry, -1);

					DB( g_printf(" delete %s %.2f\n", entry->wording, entry->amount) );

					if(!(entry->flags & OF_REMIND))
					{
						DB( printf("[account] line - sub\n") );

						data->future += entry->amount;
						if(entry->date <= GLOBALS->today)
							data->today += entry->amount;
						if(entry->flags & OF_VALID)
							data->bank += entry->amount;
					}

					/* v3.4: also remove child transfer */
					if( entry->paymode == PAYMODE_INTXFER )
					{
						operation_delete_child_transfer( entry );
					}

					gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
					GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, entry);
					da_operation_free(entry);

					GLOBALS->change++;


					list = g_list_previous(list);
				}

				g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
				g_list_free(selection);

			  	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), model); /* Re-attach model to view */
			  	g_object_unref(model);
				
				account_update(widget, GINT_TO_POINTER(UF_BALANCE));

				data->acc->flags |= AF_CHANGED;

			}
		}
		break;

		//validate
		case ACTION_ACCOUNT_VALIDATE:
		{
			GtkTreeSelection *selection;

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
			gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)validate_selected_foreach_func, data);

			DB( g_printf(" validate\n") );

			gtk_widget_queue_draw (data->LV_ope);
			//gtk_widget_queue_resize (data->LV_acc);


			account_update(widget, GINT_TO_POINTER(UF_BALANCE));

			data->acc->flags |= AF_CHANGED;
			GLOBALS->change++;

		}

			break;

		case ACTION_ACCOUNT_FILTER:
		{

			if(create_deffilter_window(data->filter, FALSE) != GTK_RESPONSE_REJECT)
			{
				account_populate(data->LV_ope);
				account_update(data->LV_ope, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));
			}

		}
		break;

		//close
		case ACTION_ACCOUNT_CLOSE:
		{
			DB( g_printf(" close\n") );

			//g_signal_emit_by_name(data->window, "delete-event");


		}
		break;
	}

}



static void account_toggle(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;

	DB( g_printf("(account) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	account_update(data->LV_ope, GINT_TO_POINTER(UF_BALANCE));
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_ope));
}

static void account_selection(GtkTreeSelection *treeselection, gpointer user_data)
{

	DB( g_printf("****\n(account) selection changed cb\n") );


	account_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(UF_SENSITIVE));

}


static void account_update(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
GtkTreeSelection *selection;
gint flags;
gint count = 0;

	DB( g_printf("****\n(account) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	flags = (gint)user_data;

		GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


	/* set window title */
	if(flags & UF_TITLE)
	{
		DB( printf(" +  1: wintitle\n") );

	}

	/* update disabled things */
	if(flags & UF_SENSITIVE)
	{
	gboolean	sensitive;

		//DB( printf("----------\n") );

		//DB( printf(" +  2: disabled, opelist count\n") );

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
		count = gtk_tree_selection_count_selected_rows(selection);
		DB( printf(" count = %d\n", count) );


	/*
		if (active = gtk_tree_selection_get_selected(selection, &model, &iter))
		{
		gint *indices;

			path = gtk_tree_model_get_path(model, &iter);
			indices = gtk_tree_path_get_indices(path);

			data->accnum = indices[0];

			DB( printf(" active is %d, sel=%d\n", indices[0], active) );
		}
		*/

		sensitive = (count > 0 ) ? TRUE : FALSE;
		// no selection: disable validate, remove
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Validate"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Remove"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/MakeArchive"), sensitive);

		// multiple: disable inherit, edit
		sensitive = (count != 1 ) ? FALSE : TRUE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Inherit"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Edit"), sensitive);

	}

	/* update toolbar & list */
	if(flags & UF_VISUAL)
	{
		DB( printf(" +  8: visual\n") );

		if(PREFS->toolbar_style == 0)
			gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
		else
			gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

		//minor ?
		if( PREFS->euro_active )
		{
			gtk_widget_show(data->CM_minor);
		}
		else
		{
			gtk_widget_hide(data->CM_minor);
		}			
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{

		DB( printf(" +  4: balances\n") );


		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->bank, GLOBALS->minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->today, GLOBALS->minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->future, GLOBALS->minor);
	}

	/* update statusbar */
	DB( printf(" +  statusbar\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
	count = gtk_tree_selection_count_selected_rows(selection);
	DB( printf(" count = %d\n", count) );

	/* if more than one ope selected, we make a sum to display to the user */
	gdouble opeexp = 0.0;
	gdouble opeinc = 0.0;
	gchar buf1[64];
	gchar buf2[64];
	gchar buf3[64];
	
	
	if( count > 1 )
	{
	GList *list, *tmplist;
	GtkTreeModel *model;
	GtkTreeIter iter;
	
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
	
		list = gtk_tree_selection_get_selected_rows(selection, &model);
	
		tmplist = g_list_first(list);
		while (tmplist != NULL)
		{
		Operation *item;

			gtk_tree_model_get_iter(model, &iter, tmplist->data);
			gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &item, -1);
		
			if( item->flags & OF_INCOME )
				opeinc += item->amount;
			else
				opeexp += item->amount;

			DB( printf(" ++ %s, %.2f\n", item->wording, item->amount ) );

			tmplist = g_list_next(tmplist);
		}	
		g_list_free(list);

		DB( printf(" %f - %f = %f\n", opeinc, opeexp, opeinc + opeexp) );
		
		mystrfmon(buf1, 64-1, opeinc, GLOBALS->minor);
		mystrfmon(buf2, 64-1, -opeexp, GLOBALS->minor);
		mystrfmon(buf3, 64-1, opeinc + opeexp, GLOBALS->minor);
	
	}

	gchar *msg;

	gtk_statusbar_pop (GTK_STATUSBAR(data->statusbar), 0); /* clear any previous message, underflow is allowed */

	if( count <= 1 )
		msg = g_strdup_printf (_("%d transactions selected, %d hidden"), count, data->hidden);
	else
		//TRANSLATORS: detail of the 3 %s which are some amount of selected transaction, 1=total 2=income, 3=expense
		msg = g_strdup_printf (_("%d transactions selected, %d hidden :: %s ( %s - %s)"), count, data->hidden, buf3, buf1, buf2);

	gtk_statusbar_push (GTK_STATUSBAR(data->statusbar), 0, msg);
	g_free (msg);

	DB( printf(" ok\n") );
}



void account_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
struct account_data *data;
GtkTreeModel *model;
GtkTreeIter iter;
gint col_id, count;
GList *selection, *list;
Operation *ope;
gchar *tagstr, *txt;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	col_id = gtk_tree_view_column_get_sort_column_id (col);

	count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(treeview));

	model = gtk_tree_view_get_model(treeview);

	//get operation double clicked to initiate the widget
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);


    DB( g_print ("%d rows been double-clicked on column=%d! ope=%s\n", count, col_id, ope->wording) );

	if( count == 1)
	{
		account_action(GTK_WIDGET(treeview), GINT_TO_POINTER(ACTION_ACCOUNT_EDIT));
	}
	else
	if(col_id >= LST_DSPOPE_DATE )
	{
	GtkWidget *parentwindow, *window, *mainvbox, *widget1, *widget2;

		parentwindow = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);

			window = gtk_dialog_new_with_buttons (NULL,
						    GTK_WINDOW (parentwindow),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);

		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		widget1 = widget2 = NULL;

		switch( col_id )
		{
			case LST_DSPOPE_DATE:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify date..."));
				widget1 = gtk_dateentry_new();
				gtk_dateentry_set_date(GTK_DATE_ENTRY(widget1), (guint)ope->date);
				break;
			case LST_DSPOPE_INFO:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify info..."));
				widget1 = make_paymode(NULL);
				widget2 = make_string(NULL);
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget1), ope->paymode);
				gtk_entry_set_text(GTK_ENTRY(widget2), ope->info);
				break;
			case LST_DSPOPE_PAYEE:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify payee..."));
				widget1 = ui_pay_comboboxentry_new(NULL);
				ui_pay_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(widget1), GLOBALS->h_pay);
				ui_pay_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(widget1), ope->payee);
				break;
			case LST_DSPOPE_WORDING:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify description..."));
				widget1 = make_string(NULL);
				gtk_entry_set_text(GTK_ENTRY(widget1), ope->wording);
				break;
			case LST_DSPOPE_EXPENSE:
			case LST_DSPOPE_INCOME:
			case LST_DSPOPE_AMOUNT:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify amount..."));
				widget1 = make_amount(NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), ope->amount);
				break;
			case LST_DSPOPE_CATEGORY:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify category..."));
				widget1 = ui_cat_comboboxentry_new(FALSE);
				ui_cat_comboboxentry_populate(GTK_COMBO_BOX_ENTRY(widget1), GLOBALS->h_cat);
				ui_cat_comboboxentry_set_active(GTK_COMBO_BOX_ENTRY(widget1), ope->category);
				break;
			case LST_DSPOPE_TAGS:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify tags..."));
				widget1 = make_string(NULL);

				tagstr = transaction_get_tagstring(ope);

				txt = (tagstr != NULL) ? tagstr : "";
				gtk_entry_set_text(GTK_ENTRY(widget1), txt);
				g_free(tagstr);

				break;
		}

		if(widget1 != NULL) gtk_box_pack_start (GTK_BOX (mainvbox), widget1, TRUE, TRUE, 0);
		if(widget2 != NULL) gtk_box_pack_start (GTK_BOX (mainvbox), widget2, TRUE, TRUE, 0);

		gtk_widget_show_all(mainvbox);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
			selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

			list = g_list_first(selection);
			while(list != NULL)
			{
			GtkTreeIter iter;
			const gchar *txt;

				gtk_tree_model_get_iter(model, &iter, list->data);
				gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);

				DB( g_printf(" modifying %s %.2f\n", ope->wording, ope->amount) );

				switch( col_id )
				{
					case LST_DSPOPE_DATE:
						ope->date = gtk_dateentry_get_date(GTK_DATE_ENTRY(widget1));
						break;
					case LST_DSPOPE_INFO:
						ope->paymode = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
						txt = gtk_entry_get_text(GTK_ENTRY(widget2));
						if (txt && *txt)
						{
							g_free(ope->info);
							ope->info = g_strdup(txt);
						}
						break;
					case LST_DSPOPE_PAYEE:
						ope->payee = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(widget1));
						DB( g_print(" -> payee: '%d'\n", ope->payee) );
						break;
					case LST_DSPOPE_WORDING:
						txt = gtk_entry_get_text(GTK_ENTRY(widget1));
						if (txt && *txt)
						{
							g_free(ope->wording);
							ope->wording = g_strdup(txt);
						}
						break;
					case LST_DSPOPE_EXPENSE:
					case LST_DSPOPE_INCOME:
					case LST_DSPOPE_AMOUNT:
						ope->flags &= (~OF_INCOME);
						ope->amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget1));
						if(ope->amount > 0) ope->flags |= OF_INCOME;
						break;
					case LST_DSPOPE_CATEGORY:
						ope->category = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX_ENTRY(widget1));
						//bad .... ope->category = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
						
						DB( g_print(" -> category: '%d'\n", ope->category) );
						break;
					case LST_DSPOPE_TAGS:
						txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget1));
						if (txt && *txt)
						{
							DB( g_print(" -> tags: '%s'\n", txt) );
		
							transaction_set_tags(ope, txt);
						}
					
						break;

				}

				GLOBALS->change++;

				list = g_list_next(list);
			}

			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}

		// cleanup and destroy
		gtk_widget_destroy (window);

	}
}

void account_busy(GtkWidget *widget, gboolean state)
{
struct account_data *data;
GtkWidget *window;
GdkCursor *cursor;

	DB( g_printf("(account) busy %d\n", state) );

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
	
		  // make sure changes is up
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
static gint listview_context_cb (GtkWidget *widget, GdkEventButton *event, GtkWidget *menu)
{

	if (event->button == 3)
	{

		
		if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
			(gint) event->x, (gint) event->y, &path, NULL, NULL, NULL))
		{
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);
			gtk_tree_path_free (path);
		}
		
		


        gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL,
                        event->button, event->time);

        // On indique à l'appelant que l'on a géré cet événement.

        return TRUE;
    }

    // On indique à l'appelant que l'on n'a pas géré cet événement.

    return FALSE;
}
*/


/*
** populate the account window
*/
void account_init_window(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
GDate *date;
guint year;
GList *list;
Operation *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(account) init window\n") );

	account_busy(widget, TRUE);

	GLOBALS->ope_list = da_operation_sort(GLOBALS->ope_list);

	if(g_list_length(GLOBALS->ope_list) > 1)
	{
		DB( g_printf(" -> get date bounds\n") );

		//get our min max date
		list = g_list_first(GLOBALS->ope_list);
		item = list->data;
		data->filter->mindate = item->date;
		list = g_list_last(GLOBALS->ope_list);
		item = list->data;
		data->filter->maxdate = item->date;
		
	}
	else
	{
		data->filter->mindate = GLOBALS->today;
		data->filter->maxdate = GLOBALS->today;
	}
	
	date = g_date_new_julian(data->filter->maxdate);
	year = g_date_get_year(date);
	g_date_free(date);

	g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), year);
	g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);

	DB( g_print(" mindate=%d, maxdate=%d %x\n", data->filter->mindate,data->filter->maxdate) );

	DB( g_print(" -> call update visual\n") );
	account_update(widget, GINT_TO_POINTER(UF_VISUAL));

	DB( g_print(" -> set range or populate+update sensitive+balance\n") );
	if( PREFS->filter_range != 0 )
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
	{
		account_populate(widget);
		account_update(widget, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));
	}

	account_busy(widget, FALSE);



}

/*
**
*/
static gboolean account_getgeometry(GtkWidget         *widget,
                                                        GdkEventConfigure *event,
                                                        gpointer           user_data)
{
//struct account_data *data = user_data;
struct WinGeometry *wg;

	DB( g_printf("(account) get geometry\n") );

	//store position and size
	wg = &PREFS->acc_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );



	return FALSE;
}

/*
**
*/
static gboolean account_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
//struct account_data *data = user_data;


	DB( g_printf("(account) delete-event\n") );


	return FALSE;
}

/* Another callback */
static void account_destroy( GtkWidget *widget,
                     gpointer   user_data )
{
struct account_data *data;

	data = g_object_get_data(G_OBJECT(widget), "inst_data");

 
	DB( g_print ("(account) destroy event occurred\n") );
 
 
 
	//enable define windows
	GLOBALS->define_off--;

	/* unset operation edit mutex */
	if(data->acc)
		data->acc->window = NULL;

	/* free title and filter */
	DB( g_printf(" user_data=%08x to be free\n", user_data) );
	g_free(data->wintitle);
	
	
	da_filter_free(data->filter);

	g_free(data);



	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));

}


// the window creation
GtkWidget *create_account_window(gint accnum, Account *acc)
{
struct account_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *vbox, *hbox, *statusbar;
GtkWidget *treeview, *check_button, *vbar, *label, *entry, *sw;
//GtkWidget *menu, *menu_items;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct account_data));
	if(!data) return NULL;

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//debug
	data->wintitle = NULL;
	data->accnum = accnum;
	data->acc = acc;

	/* set operation edit mutex */
	if(data->acc)
		data->acc->window = GTK_WINDOW(window);

	//g_free(data->wintitle);
	data->wintitle = g_strdup_printf("HomeBank - %s", data->acc->name);
	gtk_window_set_title (GTK_WINDOW (window), data->wintitle);

	// connect our dispose function
    g_signal_connect (window, "delete_event",
		G_CALLBACK (account_dispose), (gpointer)data);

	// connect our dispose function
    g_signal_connect (window, "destroy",
		G_CALLBACK (account_destroy), (gpointer)data);

	// connect our dispose function
    g_signal_connect (window, "configure-event",
		G_CALLBACK (account_getgeometry), (gpointer)data);




	//gtk_window_set_icon_from_file(GTK_WINDOW (WI_account), "./pixmaps/.png", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(account) new window=%08lx, inst_data=%08lx\n", window, data) );

	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_show.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_SHOW );

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

#if UI == 1
	//start test uimanager

		actions = gtk_action_group_new ("Account");

      	//as we use gettext
      	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);


		DB( g_print("add actions: %x user_data: %x\n", (gint)actions, data) );
		gtk_action_group_add_actions (actions, entries, n_entries, data);

		/* set which action should have priority in the toolbar */
		action = gtk_action_group_get_action(actions, "Add");
		g_object_set(action, "is_important", TRUE, "short_label", _("Add"), NULL);

		action = gtk_action_group_get_action(actions, "Inherit");
		g_object_set(action, "is_important", TRUE, "short_label", _("Inherit"), NULL);

		action = gtk_action_group_get_action(actions, "Edit");
		g_object_set(action, "is_important", TRUE, "short_label", _("Edit"), NULL);

		action = gtk_action_group_get_action(actions, "Filter");
		g_object_set(action, "is_important", TRUE, "short_label", _("Filter"), NULL);

		action = gtk_action_group_get_action(actions, "Validate");
		g_object_set(action, "is_important", TRUE, "short_label", _("Validate"), NULL);

		action = gtk_action_group_get_action(actions, "Remove");
		g_object_set(action, "is_important", TRUE, "short_label", _("Remove"), NULL);


		ui = gtk_ui_manager_new ();

		DB( g_print("insert action group:\n") );
		gtk_ui_manager_insert_action_group (ui, actions, 0);

      GtkAccelGroup *ag = gtk_ui_manager_get_accel_group (ui);

      DB( g_print("add_accel_group actions=%x, ui=%x, ag=%x\n", (gint)actions, (gint)ui, (gint)ag) );

      gtk_window_add_accel_group (GTK_WINDOW (window), ag);

		DB( g_print("add ui from string:\n") );
	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
	  g_message ("building menus failed: %s", error->message);
	  g_error_free (error);
	}

	data->ui = ui;
	data->actions = actions;

	//menubar
    gtk_box_pack_start (GTK_BOX (mainvbox),
			  gtk_ui_manager_get_widget (ui, "/MenuBar"),
			  FALSE, FALSE, 0);

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (mainvbox), data->TB_bar, FALSE, FALSE, 0);

#endif
  	//end test uimanager

	vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (mainvbox), vbox, TRUE, TRUE, 0);


	// fast date filter
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = make_label(_("_Range:"), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_range = make_cycle(label, CYA_RANGE);
    gtk_box_pack_start (GTK_BOX (hbox), data->CY_range, FALSE, FALSE, 0);

	label = make_label(_("_Month:"), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_month = make_cycle(label, CYA_SELECT);
    gtk_box_pack_start (GTK_BOX (hbox), data->CY_month, FALSE, FALSE, 0);

	label = make_label(_("_Year:"), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->NB_year = make_year(label);
    gtk_box_pack_start (GTK_BOX (hbox), data->NB_year, FALSE, FALSE, 0);

	// info + total
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

	//vbar = gtk_vseparator_new();
	//gtk_box_pack_start (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_balance[2] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Future:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_balance[1] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Today:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_balance[0] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Bank:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_end (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

	//TRANSLATORS: this is for Euro specific users, a toggle to display in 'Minor' currency
	check_button = gtk_check_button_new_with_mnemonic (_("Minor"));
	data->CM_minor = check_button;
	gtk_box_pack_end (GTK_BOX (hbox), check_button, FALSE, FALSE, 0);


	//list
      sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_ALWAYS);
      gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, HB_BOX_SPACING);

	//gtk_container_set_border_width (GTK_CONTAINER(sw), HB_BOX_SPACING);

	// create tree view
	treeview = (GtkWidget *)create_list_operation(TRN_LIST_TYPE_BOOK, PREFS->lst_ope_columns);
	data->LV_ope = treeview;
	gtk_container_add (GTK_CONTAINER (sw), treeview);

	//status bar
	statusbar = gtk_statusbar_new ();
	data->statusbar = statusbar;
    gtk_box_pack_start (GTK_BOX (mainvbox), statusbar, FALSE, FALSE, 0);

	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);


	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope))), "minor", data->CM_minor);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (account_toggle), NULL);

	data->handler_id[HID_MONTH] = g_signal_connect (data->CY_month, "changed", G_CALLBACK (account_period_change), NULL);
	data->handler_id[HID_YEAR]  = g_signal_connect (data->NB_year, "value-changed", G_CALLBACK (account_period_change), NULL);

	data->handler_id[HID_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (account_range_change), NULL);


	//todo: test context menu
/*
	menu = gtk_menu_new();

//	menu_items = gtk_ui_manager_get_widget (ui, "/MenuBar/OperationMenu/Add");
	
	menu_items = gtk_menu_item_new_with_label ("test");
	gtk_widget_show(menu_items);
	gtk_menu_shell_append (GTK_MENU (menu), menu_items);

	
	
	
	//todo: debug test
	g_signal_connect (treeview, "button-press-event", G_CALLBACK (listview_context_cb), 
		// todo: here is not a GtkMenu but GtkImageMenuItem...
		menu
		//gtk_ui_manager_get_widget (ui, "/MenuBar")
	);
	*/


	//g_signal_connect (GTK_TREE_VIEW(treeview), "cursor-changed", G_CALLBACK (account_update), (gpointer)2);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (account_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(treeview), "row-activated", G_CALLBACK (account_onRowActivated), GINT_TO_POINTER(2));


	//setup, init and show window
	wg = &PREFS->acc_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);

  /* make sure splash is up */
  while (gtk_events_pending ())
    gtk_main_iteration ();

	/* setup to moove later */
	data->filter = da_filter_malloc();
	DB( g_printf(" filter ok %x\n", (gint)data->filter) );
	filter_reset(data->filter);


	return window;
}
