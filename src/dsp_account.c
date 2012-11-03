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

#include "dsp_account.h"

#include "list_operation.h"

#include "def_lists.h"
#include "def_filter.h"
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
extern struct Preferences *PREFS;

//debug
#define UI 1

/* our functions prototype */
void account_populate(GtkWidget *view);
void account_action(GtkWidget *widget, gpointer user_data);
void account_toggle(GtkWidget *widget, gpointer user_data);
void account_clear(GtkWidget *widget, gpointer user_data);
void account_updateacc(GtkWidget *widget, gpointer user_data);
void account_update(GtkWidget *widget, gpointer user_data);


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

	GtkWidget	*TX_info;

	GtkUIManager	*ui;

	GtkWidget	*GR_info;
	GtkWidget	*GR_minor;
	GtkWidget	*CM_minor;
	GtkWidget	*TX_balance[3];
	GtkWidget	*GO_gauge;

	GtkWidget	*LV_ope;
	GtkWidget	*statusbar;

	double	bank, today, future;

	Operation ope;
	gint accnum;
	Account *acc;

	/* status counters */
	gint	hidden, total;

	Filter		*filter;

	gulong		handler_id[MAX_HID];

	gint change;

};


extern gchar *CYA_RANGE[];
extern gchar *CYA_SELECT[];




/* our functions prototype */
static void account_action_editfilter(GtkAction *action, gpointer user_data);

static void account_action_add(GtkAction *action, gpointer user_data);
static void account_action_inherit(GtkAction *action, gpointer user_data);
static void account_action_edit(GtkAction *action, gpointer user_data);
static void account_action_validate(GtkAction *action, gpointer user_data);
static void account_action_remove(GtkAction *action, gpointer user_data);
static void account_action_createarchive(GtkAction *action, gpointer user_data);

static void account_action_importcsv(GtkAction *action, gpointer user_data);
static void account_action_exportcsv(GtkAction *action, gpointer user_data);

/* these 2 functions are independant from account window */
void operation_add_treeview(Operation *ope, GtkWidget *treeview, gint accnum);
void operation_add(Operation *ope, GtkWidget *treeview, gint accnum);

void account_import_csv(GtkWidget *widget, gpointer user_data);
void account_export_csv(GtkWidget *widget, gpointer user_data);
void account_make_archive(GtkWidget *widget, gpointer user_data);
void account_period_change(GtkWidget *widget, gpointer user_data);
void account_range_change(GtkWidget *widget, gpointer user_data);
void account_add(GtkWidget *widget, gpointer user_data);
void account_populate(GtkWidget *view);
void validate_selected_foreach_func (GtkTreeModel  *model, GtkTreePath   *path, GtkTreeIter   *iter, gpointer       userdata);
Operation *get_active_operation(GtkTreeView *treeview);
void account_action(GtkWidget *widget, gpointer user_data);
void account_toggle(GtkWidget *widget, gpointer user_data);
void account_selection(GtkTreeSelection *treeselection, gpointer user_data);
void account_update(GtkWidget *widget, gpointer user_data);
GtkWidget *create_account_toolbar(struct account_data *data);
void account_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);

static GtkActionEntry entries[] = {

  /* name, stock id, label */
  { "EditMenu"     , NULL, N_("_Edit") },
  { "OperationMenu", NULL, N_("_Operation") },
  { "ToolsMenu"    , NULL, N_("_Tools") },

	/* name, stock id, label, accelerator, tooltip */
  { "Filter"       , "hb-stock-filter", N_("_Filter..."), NULL,    N_("Open the list filter"), G_CALLBACK (account_action_editfilter) },

  { "Add"          , "hb-stock-ope-add", N_("_Add..."), NULL,    N_("Add a new operation"), G_CALLBACK (account_action_add) },
  { "Inherit"      , "hb-stock-ope-herit", N_("_Inherit..."), NULL, N_("Inherit from active operation"), G_CALLBACK (account_action_inherit) },
  { "Edit"         , "hb-stock-ope-edit", N_("_Edit..."), NULL, N_("Edit active operation"),  G_CALLBACK (account_action_edit) },
  { "Validate"     , "hb-stock-ope-valid", N_("(De)_Validate..."), NULL,    N_("Validate active operation(s)"), G_CALLBACK (account_action_validate) },
  { "Remove"       , "hb-stock-ope-delete", N_("_Remove..."), NULL,    N_("Remove active operation(s)"), G_CALLBACK (account_action_remove) },
  { "MakeArchive"  , NULL, N_("Make an archive..."), NULL,    NULL, G_CALLBACK (account_action_createarchive) },

  { "Import"       , NULL, N_("Import csv..."), NULL,    NULL, G_CALLBACK (account_action_importcsv) },
  { "Export"       , NULL, N_("Export csv..."), NULL,    NULL, G_CALLBACK (account_action_exportcsv) },

};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='EditMenu'>"
"      <menuitem action='Filter'/>"
"    </menu>"

"    <menu action='OperationMenu'>"
"      <menuitem action='Add'/>"
"      <menuitem action='Inherit'/>"
"      <menuitem action='Edit'/>"
"      <menuitem action='Validate'/>"
"      <menuitem action='Remove'/>"
"      <separator/>"
"      <menuitem action='MakeArchive'/>"
"    </menu>"

"    <menu action='ToolsMenu'>"
"      <menuitem action='Import'/>"
"      <menuitem action='Export'/>"
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

"  </toolbar>"


"</ui>";


/* account action functions -------------------- */
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




static void account_action_importcsv(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_import_csv(data->window, NULL);
}

static void account_action_exportcsv(GtkAction *action, gpointer user_data)
{
struct account_data *data = user_data;

	account_export_csv(data->window, NULL);
}





/* these 2 functions are independant from account window */
void operation_add_treeview(Operation *ope, GtkWidget *treeview, gint accnum)
{
GtkTreeModel *model;
GtkTreeIter  iter;

	DB( g_printf("(operation) operation add treeview\n") );

	if(ope->account == accnum)
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);

		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, ope,
				-1);
	}
}

void operation_add(Operation *ope, GtkWidget *treeview, gint accnum)
{
Operation *newope;
Account *acc;

	DB( g_printf("(operation) operation add\n") );

	//allocate a new entry and copy from our edited structure
	newope = da_operation_clone(ope);

	//init flag and keep remind state
	// already done in defoperation_get
	//ope->flags |= (OF_ADDED);
	//remind = (ope->flags & OF_REMIND) ? TRUE : FALSE;
	//ope->flags &= (~OF_REMIND);

	/* cheque number is already stored in defoperation_get */


	/* add normal operation */
	DB( g_printf(" + add normal\n") );

	/* update acc flags */
	acc = g_list_nth_data(GLOBALS->acc_list, ope->account);
	if(acc != NULL)
		acc->flags |= AF_ADDED;

	GLOBALS->ope_list = g_list_append(GLOBALS->ope_list, newope);
	if(treeview != NULL) operation_add_treeview(newope, treeview, accnum);

	/* prepare for remind or transfert */
	ope->amount = -ope->amount;
	ope->flags ^= (OF_INCOME);	//xor

	/* add transfert operation */
	if(ope->paymode == PAYMODE_PERSTRANSFERT)
	{
	gchar swap;

		DB( g_printf(" + add transfert\n") );

		ope->flags &= (~OF_REMIND);
		swap = ope->account;
		ope->account = ope->dst_account;
		ope->dst_account = swap;

		/* update acc flags */
		acc = g_list_nth_data(GLOBALS->acc_list, ope->account);
		acc->flags |= AF_ADDED;

		newope = da_operation_clone(ope);
		GLOBALS->ope_list = g_list_append(GLOBALS->ope_list, newope);
		if(treeview != NULL) operation_add_treeview(newope, treeview, accnum);
	}
}

/* account functions -------------------- */





void account_import_csv(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
gchar *filename;
GIOChannel *io;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};

	DB( g_print("(account) import csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		DB( g_print(" + will add to %d\n", data->accnum) );

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		gchar *tmpstr;
		gint io_stat;
		gboolean valid;
		gint count = 0;
		gint error = 0;

			for(;;)
			{
				io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
				if( io_stat == G_IO_STATUS_EOF)
					break;
				if( io_stat == G_IO_STATUS_NORMAL)
				{
					if( tmpstr != "")
					{
					gchar **str_array;
					Operation *newope = &data->ope;

						hb_string_strip_crlf(tmpstr);

						/* control validity here */
						valid = hb_string_csv_valid(tmpstr, 7, csvtype);
					
						 g_print("valid %d, '%s'\n", valid, tmpstr) ;
					
						if( !valid )
						{
							error++;
						}
						else
						{
							count++;
						
							str_array = g_strsplit (tmpstr, ";", 7);
							// date; paymode; info; payee, wording; amount; category

							DB( g_print(" read %s : %s : %s\n", str_array[0], str_array[1], str_array[2]) );

							/* fill in the operation */
							memset(&data->ope, 0, sizeof(Operation));

							newope->date		= hb_date_get_julian_parse(str_array[0]);	
							newope->paymode     = atoi(str_array[1]);
							newope->info        = g_strdup(str_array[2]);
							newope->payee       = da_payee_exists(GLOBALS->pay_list, str_array[3]);
							newope->wording     = g_strdup(str_array[4]);
							newope->amount      = g_ascii_strtod(str_array[5],NULL);
							newope->category    = da_category_exists(GLOBALS->cat_list, str_array[6]);
							newope->account     = data->accnum;
							newope->dst_account = data->accnum;

							newope->flags |= OF_ADDED;

							if( newope->amount > 0)
								newope->flags |= OF_INCOME;

							account_add(widget, NULL);

							g_strfreev (str_array);
						}
					}
					g_free(tmpstr);
				}

			}
			g_io_channel_unref (io);

			homebank_message_dialog(GTK_WINDOW(data->window), error > 0 ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
				_("Operation CSV import result"),
				_("%d operation(s) inserted\n%d error(s) in the file"),
				count, error);
				 
		}

		account_update(widget, (gpointer)UF_BALANCE);
		account_range_change(widget, NULL);

	}

}

void account_export_csv(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
gchar *filename;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GIOChannel *io;

	DB( g_print("(account) export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( homebank_csv_file_chooser(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename) == TRUE )
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
				payee = g_list_nth_data(GLOBALS->pay_list, ope->payee);
				payeename = (payee->name == NULL) ? "" : payee->name;
				category = g_list_nth_data(GLOBALS->cat_list, ope->category);
				categoryname = (category->name == NULL) ? "" : category->name;


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

	}

}

/*
** make an archive with the active operation
*/
void account_make_archive(GtkWidget *widget, gpointer user_data)
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
			_("Do you want to create an Archive with\neach of the selected Operation(s) ?")
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
				item->wording 		= g_strdup(ope->wording);

				GLOBALS->arc_list = g_list_insert_sorted(GLOBALS->arc_list, item, (GCompareFunc)defarchive_list_sort);
				data->change++;

				list = g_list_next(list);
			}

			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}
	}
}




void account_period_change(GtkWidget *widget, gpointer user_data)
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

void account_range_change(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
GList *list;
gint range, refdate;
GDate *date;

	DB( g_print("(account) range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(g_list_length(GLOBALS->ope_list) == 0) return;

	//get our min max date
	da_operation_sort(GLOBALS->ope_list);
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



void account_add(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
Operation *ope;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_printf("(account) add (data=%08x)\n", data) );

	ope = &data->ope;
	if(ope->account == data->accnum)
	{
		DB( g_printf(" -> update balance %.2f\n", ope->amount) );

		//update our account balance
		if(ope->flags & OF_VALID) data->bank += ope->amount;
		data->today += ope->amount;
		data->future += ope->amount;
	}

	operation_add(ope, data->LV_ope, data->accnum);

}



void account_populate(GtkWidget *view)
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

		acc = g_list_nth_data(GLOBALS->acc_list, data->accnum);
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

		/* balances */
			data->future += ope->amount;

			if(ope->date <= GLOBALS->today)
				data->today += ope->amount;

			if(ope->flags & OF_VALID)
				data->bank += ope->amount;

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
		g_date_strftime (buffer1, 128-1, "%x", date);
		g_date_set_julian(date, data->filter->maxdate);
		g_date_strftime (buffer2, 128-1, "%x", date);
		g_date_free(date);

		info = g_strdup_printf("%s -> %s", buffer1, buffer2);

		gtk_label_set_text(GTK_LABEL(data->TX_info), info);


		g_free(info);
	}

	account_update(view, (gpointer)UF_SENSITIVE+UF_BALANCE);

}


void validate_selected_foreach_func (GtkTreeModel  *model, GtkTreePath   *path, GtkTreeIter   *iter, gpointer       userdata)
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



void account_action(GtkWidget *widget, gpointer user_data)
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

			DB( g_printf(" -> ope=%8x\n", &data->ope) );

			window = create_defoperation_window(&data->ope, OPERATION_EDIT_ADD, data->accnum);
			while(result == GTK_RESPONSE_ADD)
			{
				/* fill in the operation */
				memset(&data->ope, 0, sizeof(Operation));
				data->ope.date    = date;
				data->ope.account = account;

				defoperation_set_operation(window, &data->ope);

				result = gtk_dialog_run (GTK_DIALOG (window));

				if(result != GTK_RESPONSE_REJECT)
				{

					defoperation_get(window, NULL);

					account_add(widget, NULL);
					account_update(widget, (gpointer)UF_BALANCE);

					data->acc->flags |= AF_ADDED;
					data->change++;

					DB( g_printf(" -> change=%d\n", data->change) );

				}

				date = data->ope.date;
				account = data->ope.account;

				DB( g_printf(" -> result %d\n", result) );
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

			ope = get_active_operation(GTK_TREE_VIEW(data->LV_ope));
			if(ope)
			{
				memcpy(&data->ope, ope, sizeof(Operation));
				data->ope.date    = GLOBALS->today;

				window = create_defoperation_window(&data->ope, OPERATION_EDIT_INHERIT, data->accnum);

				defoperation_set_operation(window, &data->ope);

				result = gtk_dialog_run (GTK_DIALOG (window));

				if(result == GTK_RESPONSE_ADD)
				{
					defoperation_get(window, NULL);

					account_add(widget, NULL);
					account_update(widget, (gpointer)UF_BALANCE);

					data->acc->flags |= AF_ADDED;
					data->change++;
					DB( g_printf(" -> change=%d\n", data->change) );
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
				memcpy(&data->ope, ope, sizeof(Operation));

				oldope = &data->ope;
				window = create_defoperation_window(ope, OPERATION_EDIT_MODIFY, data->accnum);

				defoperation_set_operation(window, ope);

				result = gtk_dialog_run (GTK_DIALOG (window));

				if(result == GTK_RESPONSE_ACCEPT)
				{
					defoperation_get(window, NULL);

					//sub the old amount to balances
					if(ope->flags & OF_VALID) data->bank -= oldope->amount;
					data->today -= oldope->amount;
					data->future -= oldope->amount;

					//add our new amount to balances
					if(ope->flags & OF_VALID) data->bank += ope->amount;
					data->today += ope->amount;
					data->future += ope->amount;

					account_update(widget, (gpointer)UF_BALANCE);

					data->acc->flags |= AF_CHANGED;
					data->change++;

					DB( g_printf(" -> change=%d\n", data->change) );
				}

				defoperation_dispose(window, NULL);
				gtk_widget_destroy (window);

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


			account_update(widget, (gpointer)UF_BALANCE);

			data->acc->flags |= AF_CHANGED;
			data->change++;

			DB( g_printf(" -> change=%d\n", data->change) );
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
				_("Do you want to delete\neach of the selected Operation(s) ?")
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
						if(entry->flags & OF_VALID) data->bank -= entry->amount;
						data->today -= entry->amount;
						data->future -= entry->amount;
					}

					gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
					GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, entry);

					data->change++;
					DB( g_printf(" -> change=%d\n", data->change) );


					list = g_list_previous(list);
				}

				g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
				g_list_free(selection);

				account_update(widget, (gpointer)UF_BALANCE);

				data->acc->flags |= AF_CHANGED;

			}
		}
		break;

		case ACTION_ACCOUNT_FILTER:
		{
			//debug
			//create_deffilter_window(data->filter, FALSE);

			if(create_deffilter_window(data->filter, FALSE) == GTK_RESPONSE_ACCEPT)
			{
				account_populate(data->LV_ope);
				account_update(data->LV_ope, (gpointer)UF_SENSITIVE+UF_BALANCE);
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



void account_toggle(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;

	DB( g_printf("(account) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	account_update(data->LV_ope, (gpointer)UF_BALANCE);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_ope));
}

void account_selection(GtkTreeSelection *treeselection, gpointer user_data)
{

	account_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), (gpointer)UF_SENSITIVE);

}


void account_update(GtkWidget *widget, gpointer user_data)
{
struct account_data *data;
GtkTreeSelection *selection;
gint flags;
gint count = 0;
gboolean minor;

	DB( g_printf("****\n(account) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	flags = (gint)user_data;

		minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


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
			gtk_widget_show(data->CM_minor);
		else
			gtk_widget_hide(data->CM_minor);
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{

		DB( printf(" +  4: balances\n") );


		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->bank, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->today, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->future, minor);
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
		
		hb_strfmonall(buf1, 64-1, opeinc, minor);
		hb_strfmonall(buf2, 64-1, -opeexp, minor);
		hb_strfmonall(buf3, 64-1, opeinc + opeexp, minor);
	
	}

	gchar *msg;

	gtk_statusbar_pop (GTK_STATUSBAR(data->statusbar), 0); /* clear any previous message, underflow is allowed */

	if( count <= 1 )
		msg = g_strdup_printf ("%d operation(s) selected, %d hidden", count, data->hidden);
	else
		msg = g_strdup_printf ("%d operation(s) selected, %d hidden :: %s ( %s - %s)", count, data->hidden, buf3, buf1, buf2);

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

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	col_id = gtk_tree_view_column_get_sort_column_id (col);

	count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(treeview));

	model = gtk_tree_view_get_model(treeview);

	//get operation double clicked to initiate the widget
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);


    DB( g_print ("%d row(s) been double-clicked on column=%d! ope=%s\n", count, col_id, ope->wording) );

	if( count == 1)
	{
		account_action(GTK_WIDGET(treeview), (gpointer)ACTION_ACCOUNT_EDIT);
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
				widget1 = make_poppayee(NULL);
				make_poppayee_populate(GTK_COMBO_BOX(widget1), GLOBALS->pay_list);
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget1), ope->payee);
				break;
			case LST_DSPOPE_WORDING:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify wording..."));
				widget1 = make_string(NULL);
				gtk_entry_set_text(GTK_ENTRY(widget1), ope->wording);
				break;
			case LST_DSPOPE_EXPENSE:
			case LST_DSPOPE_INCOME:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify amount..."));
				widget1 = make_amount(NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), ope->amount);
				break;
			case LST_DSPOPE_CATEGORY:
				gtk_window_set_title (GTK_WINDOW (window), _("Modify category..."));
				widget1 = make_popcategory(NULL);
				make_popcategory_populate(GTK_COMBO_BOX(widget1), GLOBALS->cat_list);
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget1), ope->category);
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
						ope->payee = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
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
						ope->flags &= (~OF_INCOME);
						ope->amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget1));
						if(ope->amount > 0) ope->flags |= OF_INCOME;
						break;
					case LST_DSPOPE_CATEGORY:
						ope->category = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
						break;
				}

				data->change++;

				list = g_list_next(list);
			}

			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}

		// cleanup and destroy
		gtk_widget_destroy (window);

	}
}




/*
**
*/
gboolean account_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct account_data *data = user_data;

	DB( g_printf("(account) dispose (delete-event)\n") );



	DB( g_printf(" -> add global change=%d\n", data->change) );
	//enable define windows
	GLOBALS->define_off--;
	GLOBALS->change += data->change;

	/* free title and filter */
	DB( g_printf(" user_data=%08x to be free\n", user_data) );
	g_free(data->wintitle);
	da_filter_free(data->filter);

	g_free(user_data);


	//propagate the event further

	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);

	return FALSE;
}

// the window creation
GtkWidget *create_account_window(gint accnum, Account *acc)
{
struct account_data *data;
GtkWidget *window, *mainvbox, *hbox, *hbox2, *statusbar;
GtkWidget *treeview, *check_button, *vbar, *label, *entry, *sw;
GtkUIManager *ui;
GtkActionGroup *actions;
GError *error = NULL;

	data = g_malloc0(sizeof(struct account_data));
	if(!data) return NULL;

	//disable define windows
	GLOBALS->define_off++;
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_SENSITIVE);


	//debug
	data->wintitle = NULL;
	data->accnum = accnum;
	data->acc = acc;

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//g_free(data->wintitle);
	data->wintitle = g_strdup_printf("HomeBank - %s", data->acc->name);
	gtk_window_set_title (GTK_WINDOW (window), data->wintitle);

	// connect our dispose function
    g_signal_connect (window, "delete-event",
		G_CALLBACK (account_dispose), (gpointer)data);


	//gtk_window_set_icon_from_file(GTK_WINDOW (WI_account), "./pixmaps/.png", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(account) new window=%08lx, inst_data=%08lx\n", window, data) );

	//set the window icon
	gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/ope_show.svg", NULL);

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

#if UI == 1
	//start test uimanager

		actions = gtk_action_group_new ("Account");

      	//as we use gettext
      	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);


		DB( g_print("add actions: %x user_data: %x\n", (gint)actions, data) );
		gtk_action_group_add_actions (actions, entries, n_entries, data);

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
      gtk_box_pack_start (GTK_BOX (mainvbox),
			  gtk_ui_manager_get_widget (ui, "/MenuBar"),
			  FALSE, FALSE, 0);




#endif
  	//end test uimanager


	hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, FALSE, FALSE, 0);

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (hbox), data->TB_bar, TRUE, TRUE, 0);

	// fast date filter
	hbox2 = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hbox2), HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);

	hbox = hbox2;
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
	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

	vbar = gtk_vseparator_new();
	gtk_box_pack_start (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);

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

	check_button = gtk_check_button_new_with_mnemonic (_("Minor"));
	data->CM_minor = check_button;
	gtk_box_pack_end (GTK_BOX (hbox), check_button, FALSE, FALSE, 0);


	//list
      sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					   GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				      GTK_POLICY_NEVER,
				      GTK_POLICY_ALWAYS);
      gtk_box_pack_start (GTK_BOX (mainvbox), sw, TRUE, TRUE, 0);

	//gtk_container_set_border_width (GTK_CONTAINER(sw), HB_BOX_SPACING);

	// create tree view
	treeview = (GtkWidget *)create_list_operation();
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



	//g_signal_connect (GTK_TREE_VIEW(treeview), "cursor-changed", G_CALLBACK (account_update), (gpointer)2);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (account_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(treeview), "row-activated", G_CALLBACK (account_onRowActivated), (gpointer)2);

	/* finish & show */
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (window);


	/* setup to moove later */

	data->filter = da_filter_malloc();
	DB( g_printf(" filter ok %x\n", (gint)data->filter) );
	filter_reset(data->filter);

	data->change = 0;

	DB( g_printf(" -> change=%d\n", data->change) );

	GLOBALS->ope_list = da_operation_sort(GLOBALS->ope_list);

	if(g_list_length(GLOBALS->ope_list) > 1)
	{
	GList *list;
	Operation *item;
	GDate *date;
	guint year;

		//get our min max date
		list = g_list_first(GLOBALS->ope_list);
		item = list->data;
		data->filter->mindate = item->date;
		list = g_list_last(GLOBALS->ope_list);
		item = list->data;
		data->filter->maxdate   = item->date;

		date = g_date_new_julian(data->filter->maxdate);
		year = g_date_get_year(date);
		g_date_free(date);

		g_signal_handler_block(data->NB_year, data->handler_id[HID_YEAR]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), year);
		g_signal_handler_unblock(data->NB_year, data->handler_id[HID_YEAR]);
	}

	DB( g_printf(" mindate=%d, maxdate=%d %x\n", data->filter->mindate,data->filter->maxdate) );

	account_update(treeview, (gpointer)UF_VISUAL);

	if( PREFS->filter_range != 0 )
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);
	else
	{
		account_populate(treeview);
		account_update(treeview, (gpointer)UF_SENSITIVE+UF_BALANCE);
	}

	return window;
}
