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
#include "dsp_wallet.h"

#include "dsp_account.h"
#include "xml.h"

#include "def_wallet.h"
#include "ui_account.h"
#include "ui_payee.h"
#include "ui_category.h"
#include "ui_assign.h"
#include "ui-assist-start.h"
#include "def_archive.h"
#include "def_budget.h"
#include "def_pref.h"
#include "def_operation.h"
#include "hb_transaction.h"



#include "list_account.h"
#include "list_upcoming.h"

#include "rep_stats.h"
#include "rep_time.h"
#include "rep_budget.h"
#include "rep_balance.h"
#include "rep_vehicle.h"

#include "import.h"
#include "imp_qif.h"

//#define HOMEBANK_URL_HELP           "http://homebank.free.fr/help/"
#define HOMEBANK_URL_HELP           "index.html"
#define HOMEBANK_URL_HELP_ONLINE    "https://launchpad.net/homebank/+addquestion"
#define HOMEBANK_URL_HELP_TRANSLATE "https://launchpad.net/homebank/+translations"
#define HOMEBANK_URL_HELP_PROBLEM   "https://launchpad.net/homebank/+filebug"


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
extern gchar *homebank_pixmaps_dir;





/* our functions prototype */
static void wallet_action_new(void);
static void wallet_action_open(void);
static void wallet_action_save(void);
static void wallet_action_saveas(void);
static void wallet_action_revert(void);
static void wallet_action_defwallet(void);
static void wallet_action_close(void);
static void wallet_action_quit(void);

static void wallet_action_defaccount(void);
static void wallet_action_defpayee(void);
static void wallet_action_defcategory(void);
static void wallet_action_defarchive(void);
static void wallet_action_defbudget(void);
static void wallet_action_defassign(void);
static void wallet_action_preferences(void);

static void wallet_action_toggle_toolbar(GtkToggleAction *action);
static void wallet_action_toggle_statusbar(GtkToggleAction *action);
static void wallet_action_toggle_upcoming(GtkToggleAction *action);
static void wallet_action_toggle_minor(GtkToggleAction *action);

static void wallet_action_showoperations(void);
static void wallet_action_addoperations(void);
static void wallet_action_checkautomated(void);

static void wallet_action_statistic(void);
static void wallet_action_trendtime(void);
static void wallet_action_budget(void);
static void wallet_action_balance(void);
static void wallet_action_carcost(void);

static void wallet_action_import(void);
static void wallet_action_export(void);

static void wallet_action_help(void);
void wallet_action_help_welcome(void);
static void wallet_action_help_online(void);
static void wallet_action_help_translate(void);
static void wallet_action_help_problem(void);
static void wallet_about(void);

static GtkWidget *
create_recent_chooser_menu (GtkRecentManager *manager);

void wallet_open(GtkWidget *widget, gpointer user_data);

void wallet_save(GtkWidget *widget, gpointer user_data);
void wallet_revert(GtkWidget *widget, gpointer user_data);
void wallet_action(GtkWidget *widget, gpointer user_data);
void wallet_toggle_minor(GtkWidget *widget, gpointer user_data);
void wallet_clear(GtkWidget *widget, gpointer user_data);

void wallet_busy(GtkWidget *widget, gboolean state);

gboolean wallet_check_change(GtkWidget *widget, gpointer user_data);

void wallet_refresh_upcoming(GtkWidget *widget, gpointer user_data);

void wallet_update(GtkWidget *widget, gpointer user_data);
void wallet_check_automated(GtkWidget *widget, gpointer user_data);
void wallet_addoperations(GtkWidget *widget, gpointer user_data);

void wallet_recent_add (struct wallet_data *data, const gchar *path);

extern gchar *CYA_UNIT[];


static GtkActionEntry entries[] = {

  /* name, stock id, label */

  { "FileMenu"     , NULL, N_("_File") },
  { "EditMenu"     , NULL, N_("_Edit") },
  { "ViewMenu"     , NULL, N_("_View") },
  { "ManageMenu"   , NULL, N_("_Manage") },
  { "OperationMenu", NULL, N_("_Transactions") },
  { "ReportMenu"   , NULL, N_("_Reports")  },
  { "HelpMenu"     , NULL, N_("_Help") },

//  { "Import"       , NULL, N_("Import") },
  { "Export"       , NULL, N_("Export to") },
	/* name, stock id, label, accelerator, tooltip */

  /* FileMenu */
  { "New"        , GTK_STOCK_NEW            , N_("_New")          , NULL, N_("Create a new wallet"),    G_CALLBACK (wallet_action_new) },
  { "Open"       , GTK_STOCK_OPEN           , N_("_Open...")      , NULL, N_("Open a wallet"),    G_CALLBACK (wallet_action_open) },
  { "Save"       , GTK_STOCK_SAVE           , N_("_Save")         , NULL, N_("Save the current wallet"),    G_CALLBACK (wallet_action_save) },
  { "SaveAs"     , GTK_STOCK_SAVE_AS        , N_("Save As...")    , "<shift><control>S", N_("Save the current wallet with a different name"),    G_CALLBACK (wallet_action_saveas) },
  { "Revert"     , GTK_STOCK_REVERT_TO_SAVED, N_("Revert")        , NULL, N_("Revert to a saved version of this file"),    G_CALLBACK (wallet_action_revert) },
  { "FileImport" , "hb-file-import"         , N_("Import QIF/OFX/CSV...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (wallet_action_import) },

  { "Properties" , GTK_STOCK_PROPERTIES     , N_("_Properties..."), NULL, N_("Configure wallet"),    G_CALLBACK (wallet_action_defwallet) },
  { "Close"      , GTK_STOCK_CLOSE          , N_("_Close")        , NULL, N_("Close the current wallet"),    G_CALLBACK (wallet_action_close) },
  { "Quit"       , GTK_STOCK_QUIT           , N_("_Quit")         , NULL, N_("Quit homebank"),    G_CALLBACK (wallet_action_quit) },

  /* Export Menu */
  { "ExportQIF"  , "hb-file-export"         , N_("QIF Format...") , NULL, N_("Open the export to QIF assistant"),    G_CALLBACK (wallet_action_export) },

  /* EditMenu */
  { "Preferences", GTK_STOCK_PREFERENCES, N_("Preferences..."), NULL,    N_("Configure homebank"),    G_CALLBACK (wallet_action_preferences) },

  /* ManageMenu */
  { "Account"    , "hb-account"   , N_("Acc_ounts...")  , NULL,    N_("Configure the accounts"), G_CALLBACK (wallet_action_defaccount) },
  { "Payee"      , "hb-payee"     , N_("_Payees...")    , NULL,    N_("Configure the payees"),    G_CALLBACK (wallet_action_defpayee) },
  { "Category"   , "hb-category"  , N_("Categories...") , NULL,    N_("Configure the categories"),    G_CALLBACK (wallet_action_defcategory) },
  { "Archive"    , "hb-archive"   , N_("Arc_hives...")  , NULL,    N_("Configure the archives"),    G_CALLBACK (wallet_action_defarchive) },
  { "Budget"     , "hb-budget"    , N_("Budget...")     , NULL,    N_("Configure the budget"),    G_CALLBACK (wallet_action_defbudget) },
  { "Assign"     , "hb-assign"    , N_("Assignments..."), NULL,    N_("Configure the automatic assignments"),    G_CALLBACK (wallet_action_defassign) },

  /* OperationMenu */
  { "ShowOpe"    , HB_STOCK_OPE_SHOW, N_("Show...")           , NULL,    N_("Shows selected account transactions"),    G_CALLBACK (wallet_action_showoperations) },
  { "AddOpe"     , HB_STOCK_OPE_ADD , N_("Add...")            , NULL,    N_("Add transaction"),    G_CALLBACK (wallet_action_addoperations) },
  { "Automated"  , NULL             , N_("Check automated..."), NULL,    N_("Insert pending automated transactions"),    G_CALLBACK (wallet_action_checkautomated) },

  /* ReportMenu */
  { "RStatistics" , HB_STOCK_REP_STATS , N_("_Statistics...") , NULL,    N_("Open the Statistics report"),    G_CALLBACK (wallet_action_statistic) },
  { "RTrendTime"   , HB_STOCK_REP_TIME , N_("_Trend Time...") , NULL,    N_("Open the Trend Time report"),    G_CALLBACK (wallet_action_trendtime) },
  { "RBudget"    , HB_STOCK_REP_BUDGET, N_("B_udget...")     , NULL,    N_("Open the Budget report"),    G_CALLBACK (wallet_action_budget) },
  { "RBalance"  , HB_STOCK_REP_BALANCE, N_("Balance...")  , NULL,    N_("Open the Balance report"),    G_CALLBACK (wallet_action_balance) },
  { "RCarcost"    , HB_STOCK_REP_CAR   , N_("_Car cost...")   , NULL,    N_("Open the Car cost report"),    G_CALLBACK (wallet_action_carcost) },

  /* HelpMenu */
  { "Contents"   , GTK_STOCK_HELP    , N_("_Contents")                    , "F1", N_("Documentation about HomeBank"), G_CALLBACK (wallet_action_help) },
  { "Welcome"    , NULL              , N_("Show welcome dialog...")       , NULL, NULL                              , G_CALLBACK (wallet_action_help_welcome) },
  { "Online"     , "lpi-help"        , N_("Get Help Online...")           , NULL, N_("Connect to the LaunchPad website for online help"), G_CALLBACK (wallet_action_help_online) },
  { "Translate"  , "lpi-translate"   , N_("Translate this Application..."), NULL, N_("Connect to the LaunchPad website to help translate this application"), G_CALLBACK (wallet_action_help_translate) },
  { "Problem"    , "lpi-bug"         , N_("Report a Problem...")          , NULL, N_("Connect to the LaunchPad website to help fix problems"), G_CALLBACK (wallet_action_help_problem) },

  { "About"      , GTK_STOCK_ABOUT      , N_("_About")     , NULL, N_("About HomeBank")      ,G_CALLBACK (wallet_about) },

};
static guint n_entries = G_N_ELEMENTS (entries);


static GtkToggleActionEntry toggle_entries[] = {
/*  name         , stockid, label, accelerator, tooltip, callback, is_active */
  { "Toolbar"    , NULL                 , N_("_Toolbar")  , NULL,    NULL,    G_CALLBACK (wallet_action_toggle_toolbar), TRUE },
  { "Statusbar"  , NULL                 , N_("_Statusbar"), NULL,    NULL,    G_CALLBACK (wallet_action_toggle_statusbar), TRUE },
  { "Upcoming"   , NULL                 , N_("_Upcoming") , NULL,    NULL,    G_CALLBACK (wallet_action_toggle_upcoming), TRUE },


  { "AsMinor"    , NULL                 , N_("Minor currency"), "<control>M",    NULL,    G_CALLBACK (wallet_action_toggle_minor), FALSE },
};

static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);


static const gchar *ui_info = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
//"      <menu action='Import'>"
"        <menuitem action='FileImport'/>"
//"      </menu>"
"      <separator/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='Revert'/>"
"      <separator/>"

"      <menu action='Export'>"
"        <menuitem action='ExportQIF'/>"
//"        <menuitem action='ExportOFX'/>"
"      </menu>"

"      <separator/>"
"      <menuitem action='Properties'/>"
"      <separator/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"

"    <menu action='EditMenu'>"
"      <menuitem action='Preferences'/>"
"    </menu>"

"    <menu action='ViewMenu'>"
"      <menuitem action='Toolbar'/>"
"      <menuitem action='Statusbar'/>"
"      <separator/>"
"      <menuitem action='Upcoming'/>"
"      <separator/>"
"      <menuitem action='AsMinor'/>"
"    </menu>"

"    <menu action='ManageMenu'>"
"      <menuitem action='Account'/>"
"      <menuitem action='Payee'/>"
"      <menuitem action='Category'/>"
"      <menuitem action='Assign'/>"
"      <menuitem action='Archive'/>"
"      <menuitem action='Budget'/>"
"      <separator/>"
"    </menu>"

"    <menu action='OperationMenu'>"
"      <menuitem action='ShowOpe'/>"
"      <separator/>"
"      <menuitem action='AddOpe'/>"
"      <menuitem action='Automated'/>"
"    </menu>"

"    <menu action='ReportMenu'>"
"      <menuitem action='RStatistics'/>"
"      <menuitem action='RTrendTime'/>"
"      <menuitem action='RBalance'/>"
"      <menuitem action='RBudget'/>"
"      <menuitem action='RCarcost'/>"
"    </menu>"

"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <separator/>"
"      <menuitem action='Welcome'/>"
"      <separator/>"
"      <menuitem action='Online'/>"
"      <menuitem action='Translate'/>"
"      <menuitem action='Problem'/>"
"      <separator/>"
"      <menuitem action='About'/>"
"    </menu>"

"  </menubar>"

"  <toolbar  name='ToolBar'>"
"    <toolitem action='New'/>"
"    <toolitem action='Save'/>"

"      <separator/>"

"    <toolitem action='Account'/>"
"    <toolitem action='Payee'/>"
"    <toolitem action='Category'/>"
"    <toolitem action='Assign'/>"
"    <toolitem action='Archive'/>"
"    <toolitem action='Budget'/>"

"      <separator/>"

"    <toolitem action='ShowOpe'/>"
"    <toolitem action='AddOpe'/>"

"      <separator/>"

"    <toolitem action='RStatistics'/>"
"    <toolitem action='RTrendTime'/>"
"    <toolitem action='RBalance'/>"
"    <toolitem action='RBudget'/>"
"    <toolitem action='RCarcost'/>"

"  </toolbar>"

"</ui>";

/*
static void
activate_action (void)
{
gchar *name;

	name = (gchar *)gtk_action_get_name (action);
	g_message ("Action \"%s\" activated", name);

}*/

/* wallet action functions -------------------- */
static void wallet_action_new(void)
{
GtkWidget *widget = GLOBALS->mainwindow;

	if( wallet_check_change(widget,NULL) == TRUE )
	{
		//clear all, and init GLOBALS->filename to default
		wallet_clear(widget, GINT_TO_POINTER(TRUE)); // GPOINTER_TO_INT(
		wallet_update(widget, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
	}
}

static void wallet_action_open(void)
{
	wallet_open(GLOBALS->mainwindow, NULL);
}

static void wallet_action_save(void)
{
	wallet_save(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
}

static void wallet_action_saveas(void)
{
	wallet_save(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
}

static void wallet_action_revert(void)
{
	wallet_revert(GLOBALS->mainwindow, NULL);
}

static void wallet_action_close(void)
{
GtkWidget *widget = GLOBALS->mainwindow;

	if( wallet_check_change(widget,NULL) == TRUE )
	{
		//clear all, and init GLOBALS->filename to default
		wallet_clear(widget, GINT_TO_POINTER(TRUE));
		wallet_update(widget, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
	}

}


static void wallet_action_quit(void)
{
gboolean result;

	//gtk_widget_destroy(GLOBALS->mainwindow);

	g_signal_emit_by_name(GLOBALS->mainwindow, "delete-event", NULL, &result);

	//gtk_main_quit();
}




static void wallet_action_defwallet(void)
{
	create_defwallet_window();
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}

static void wallet_action_defaccount(void)
{
	ui_acc_manage_dialog();

	//our global list has changed, so update the treeview
	wallet_populate_listview(GLOBALS->mainwindow, NULL);

	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
}

static void wallet_action_defpayee(void)
{
	ui_pay_manage_dialog();
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}

static void wallet_action_defcategory(void)
{
	ui_cat_manage_dialog();
	wallet_refresh_upcoming(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}

static void wallet_action_defarchive(void)
{
struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");
GtkTreeModel *model;

	data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	// upcoming list have direct pointer to the arc (which may have chnaged)
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	create_defarchive_window();

	wallet_refresh_upcoming(GLOBALS->mainwindow, NULL);

	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void wallet_action_defbudget(void)
{
	create_defbudget_window();
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void wallet_action_defassign(void)
{

	ui_asg_manage_dialog();

	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void wallet_action_preferences(void)
{
	defpref_dialog_new();
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

/* display action */

static void wallet_action_toggle_toolbar(GtkToggleAction *action)
{
//struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_toolbar = gtk_toggle_action_get_active(action);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void wallet_action_toggle_statusbar(GtkToggleAction *action)
{
//struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_statusbar = gtk_toggle_action_get_active(action);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void wallet_action_toggle_upcoming(GtkToggleAction *action)
{
//struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_upcoming = gtk_toggle_action_get_active(action);
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void wallet_action_toggle_minor(GtkToggleAction *action)
{
struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	GLOBALS->minor = gtk_toggle_action_get_active(action);

	//gtk_widget_queue_draw(data->LV_acc);
	
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_upc));
}

static void wallet_action_showoperations(void)
{
struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");
GtkWidget *window;

	//todo:change this
	if( data->acc )
	{
		if( data->acc->window == NULL )
		{
			window = create_account_window(data->acc->key, data->acc);
			account_init_window(window, NULL);
		}	
		else
		{
			if(GTK_IS_WINDOW(data->acc->window))
				gtk_window_present(data->acc->window);
		
		}
	}
}


static void wallet_action_addoperations(void)
{
	wallet_addoperations(GLOBALS->mainwindow, NULL);
}

static void wallet_action_checkautomated(void)
{
	wallet_check_automated(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
}

static void wallet_action_statistic(void)
{
	create_statistic_window();
}

static void wallet_action_trendtime(void)
{
	create_trendtime_window();
}

static void wallet_action_budget(void)
{
	repbudget_window_new();
}

static void wallet_action_balance(void)
{
	repbalance_window_new();
}

static void wallet_action_carcost(void)
{
	repcost_window_new();
}

static void wallet_action_import(void)
{
	create_import_window();
	

}

static void wallet_action_export(void)
{
	test_qif_export();
	
	
	//create_import_window();
}

static void wallet_action_help(void)
{
gboolean retval;
gchar *link;

    link = g_build_filename("file:///", homebank_app_get_help_dir(), HOMEBANK_URL_HELP, NULL );

	retval = homebank_util_url_show (link);

    g_free(link);
}

static void
activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data)
{
gboolean retval;
	
	retval = homebank_util_url_show (link);

}

static void
wallet_action_help_welcome1 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 1);
}

static void
wallet_action_help_welcome2 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 2);
}

static void
wallet_action_help_welcome3 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 3);
}

void wallet_action_help_welcome(void)
{
GtkWidget *dialog;
GtkWidget *mainvbox, *widget, *label;
	
	dialog = gtk_dialog_new_with_buttons (_("Welcome to HomeBank"),
			GTK_WINDOW(GLOBALS->mainwindow),
			0,
			GTK_STOCK_CLOSE,
			GTK_RESPONSE_ACCEPT,
			NULL);

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), mainvbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

	label = make_label (_("HomeBank"), 0, 0);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

	label = make_label (_("Free, easy, personal accounting for everyone."), 0, 0);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

		widget = gtk_hseparator_new();
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), widget, FALSE, FALSE, 0);

	mainvbox = gtk_vbox_new (FALSE, HB_MAINBOX_SPACING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

	label = make_label (_("What do you want to do:"), 0, 0);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

	widget = gtk_button_new_with_mnemonic(_("Create a _new wallet"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (wallet_action_help_welcome1), dialog); 
	
	widget = gtk_button_new_with_mnemonic(_("_Open an existing wallet"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (wallet_action_help_welcome2), dialog); 
	
	widget = gtk_button_new_with_mnemonic(_("Open the _example file"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (wallet_action_help_welcome3), dialog); 
	
	//connect all our signals
	g_signal_connect (dialog, "destroy", G_CALLBACK (gtk_widget_destroyed), &dialog);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	// cleanup and destroy
	gtk_widget_destroy (dialog);

	// do appropriate action
	switch(result)
	{
		case 1:
			wallet_action_new();
			break;
		case 2:
			wallet_action_open();
			break;
		case 3:
			g_free(GLOBALS->filename);
			GLOBALS->filename = g_build_filename(homebank_app_get_datas_dir(), "example.xhb", NULL);
			wallet_open_internal(GLOBALS->mainwindow, NULL);
			break;
	}

}



static void wallet_action_help_online(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_ONLINE;

	retval = homebank_util_url_show (link);

}

static void wallet_action_help_translate(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_TRANSLATE;
	
	retval = homebank_util_url_show (link);

}

static void wallet_action_help_problem(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_PROBLEM;
	
	retval = homebank_util_url_show (link);

}


static void wallet_about(void)
{
gchar *pathfilename;
GdkPixbuf *pixbuf;


  static const gchar *artists[] = {
    "Maxime DOYEN",
    "Nathan M. Willard (some icons)",
    NULL
  };

  static const gchar *authors[] = {
    "Lead developer:\n" \
    "Maxime DOYEN",
    "\nContributor:\n" \
    "Gaetan LORIDANT (Maths formulas for charts)\n",
    NULL
  };

/*
  const gchar *documenters[] = {
    "Maxime DOYEN",
    NULL
  };
*/

	static const gchar license[] =
		"This program is free software; you can redistribute it and/or modify\n"
		  "it under the terms of the GNU General Public License as\n"
		  "published by the Free Software Foundation; either version 2 of the\n"
		  "License, or (at your option) any later version.\n\n"
		  "This program is distributed in the hope that it will be useful,\n"
		  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		  "GNU General Public License for more details.\n\n"
		  "You should have received a copy of the GNU General Public License\n"
		  "along with this program; if not, write to the Free Software\n"
		  "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, "
		  "MA 02110-1301, USA.";
		  
	static const gchar *copyright = "Copyright \xc2\xa9 1995-2011 - Maxime DOYEN";


	gtk_about_dialog_set_url_hook (activate_url, NULL, NULL);

	pathfilename = g_build_filename(homebank_app_get_images_dir(), "splash.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file(pathfilename, NULL);
	g_free(pathfilename);

 	gtk_show_about_dialog(GTK_WINDOW(GLOBALS->mainwindow),
		"name", g_get_application_name (),
		"logo-icon-name", "homebank",
		"logo"      , pixbuf,
		"artists"	, artists,
		"authors"	, authors,
	//	"translator-credits"	, "trans",
		"comments"	, _("Free, easy, personal accounting for everyone."),
		"license"	, license,
		"copyright"	, copyright,
		"version"	, PACKAGE_VERSION,
		"website"	, "http://homebank.free.fr",
		"website-label", "Visit the HomeBank website",
        NULL);





}


/* wallet functions -------------------- */



/*
** request the user to save last change
*/
gboolean wallet_check_change(GtkWidget *widget, gpointer user_data)
{
gboolean retval = TRUE;
GtkWidget *dialog = NULL;


  	if(GLOBALS->change)
	{
	gint result;

		dialog = gtk_message_dialog_new
		(
			GTK_WINDOW(GLOBALS->mainwindow),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			//GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE,
			_("Do you want to save the changes\nin the current file ?")
		);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
			_("If you do not save, some changes will be\ndefinitively lost: %d."),
			GLOBALS->change
			);

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    _("Do _not save"), 0,
		    GTK_STOCK_CANCEL, 1,
			GTK_STOCK_SAVE, 2,
			NULL);

		gtk_dialog_set_default_response(GTK_DIALOG( dialog ), 2);
		
	  result = gtk_dialog_run( GTK_DIALOG( dialog ) );
	  gtk_widget_destroy( dialog );

	  if(result == 1)
	  	retval = FALSE;

	  if(result == 2)
	  {
		DB( g_printf(" + should quick save %s\n", GLOBALS->filename) );
		homebank_save_xml(GLOBALS->filename);
		}




	}
	return retval;
}

/*
**
*/
static void wallet_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	wallet_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(UF_SENSITIVE));
}


static void wallet_close_openbooks(void)
{
GList *list;
	
	DB( g_printf("(wallet) close openbooks\n") );

	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if(item->window)
			gtk_widget_destroy(GTK_WIDGET(item->window));

			
		list = g_list_next(list);
	}
	g_list_free(list);

}



/*
**
*/
void wallet_clear(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
gboolean file_clear = GPOINTER_TO_INT(user_data);

	DB( g_printf("(wallet) clear\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	//init default path
	//filename
	if(file_clear == TRUE)
	{
		g_free(GLOBALS->filename);
		GLOBALS->filename = g_build_filename(PREFS->path_wallet, "untitled.xhb", NULL);
		GLOBALS->wallet_is_new = TRUE;
	}
	else
	{
		GLOBALS->wallet_is_new = FALSE;
	}

	GLOBALS->exists_old = FALSE;

	//csv



	//get today's date

	// clear & set base structure
	//memset(&data->base, 0, sizeof(struct Base));
	//data->base.id = FILE_ID;
	//strcpy(data->base.name, "(Nobody)");

	g_free(GLOBALS->title);
	GLOBALS->title = g_strdup(_("(Nobody)"));

/*
	if(GLOBALS->prefs.euro_Enable)
	{
		//data->base.okeuro = 0xff;
		GLOBALS->prefs.euro_Primary = 1;
	}
*/

	data->bank = 0;
	data->today = 0;
	data->future = 0;

	/* close any open accoutn window */
	wallet_close_openbooks();

	
	// clear Lists
	DB( g_printf(" -> h_acc\n") );
	da_acc_destroy();
	da_acc_new();

	DB( g_printf(" -> h_pay\n") );
	da_pay_destroy();
	da_pay_new();

	DB( g_printf(" -> h_cat\n") );
	da_cat_destroy();
	da_cat_new();

	DB( g_printf(" -> h_tag\n") );
	da_tag_destroy();
	da_tag_new();

	DB( g_printf(" -> h_rul\n") );
	da_asg_destroy();
	da_asg_new();

	da_archive_destroy(GLOBALS->arc_list);
	GLOBALS->arc_list = NULL;

	da_operation_destroy(GLOBALS->ope_list);
	GLOBALS->ope_list = NULL;

	//memos completion
	g_hash_table_remove_all(GLOBALS->h_memo);

	//clear TreeView
	gtk_tree_store_clear(GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc))));


	GLOBALS->change = 0;

	/* open the new wallet assistant */
	if(file_clear == TRUE)
	{
		ui_start_assistant();
	}
	
}


/*
** add some operations directly
*/
void wallet_addoperations(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkWidget *window;
gint result = 1;
guint32 date;
gint account = 1, count;

	DB( g_printf("(wallet) add operations\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* init the operation */
	date = GLOBALS->today;
	if(data->acc)
		account = data->acc->key;

	window = create_defoperation_window(GTK_WINDOW(data->window), NULL, OPERATION_EDIT_ADD, account);
	count = 0;
	while(result == GTK_RESPONSE_ADD)
	{
	Operation *ope;

		/* fill in the operation */
		ope = da_operation_malloc();
		ope->date    = date;
		ope->account = account;

		defoperation_set_operation(window, ope);

		result = gtk_dialog_run (GTK_DIALOG (window));

		DB( g_printf(" -> dialog result is %d\n", result) );

		if(result == GTK_RESPONSE_ADD || result == GTK_RESPONSE_ACCEPT)
		{
			defoperation_get(window, NULL);
			operation_add(ope, NULL, ope->account);


			DB( g_printf(" -> added 1 operation to %d\n", ope->account) );

			count++;
		}

		da_operation_free(ope);
		ope = NULL;

	}


	defoperation_dispose(window, NULL);
	gtk_widget_destroy (window);

	/* todo optimize this */
	if(count > 0)
	{
		GLOBALS->change += count;
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
	}
}


void wallet_refresh_upcoming(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gdouble total = 0;
	
	DB( g_printf("(wallet) refresh upcoming\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *arc = list->data;

		if((arc->flags & OF_AUTO) && arc->account > 0)
		{
		guint decay;

			decay = arc->nextdate - GLOBALS->today;		
			DB( g_printf(" populate: %s : %d\n", arc->wording, decay) );
		
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				  LST_DSPUPC_DATAS, arc,
			      LST_DSPUPC_WORDING, arc->wording,
			      LST_DSPUPC_AMOUNT, arc->amount,
				  LST_DSPUPC_REMAINING, decay,
				  -1);

			total += arc->amount;
		}
		list = g_list_next(list);
	}

	// insert total
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		  LST_DSPUPC_DATAS, NULL,
	      LST_DSPUPC_WORDING, _("<b>Total</b>"),
	      LST_DSPUPC_AMOUNT, total,
	      LST_DSPUPC_REMAINING, G_MAXINT32,
		  -1);

	
}

/*
** called after load, importamiga, on demand
*/
void wallet_check_automated(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GList *list;
gint count;
gint usermode = GPOINTER_TO_INT(user_data);

	DB( g_printf("(wallet) check automated\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	wallet_busy(widget, TRUE);

	count = 0;
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *arc = list->data;

		//debug
		//arc->arc_Flags |= OF_AUTO;
		//arc->arc_Every = 1;
		//arc->arc_Unit = 2;

		if((arc->flags & OF_AUTO) && arc->account > 0)
		{
		guint32 maxdate;

			/*#if MYDEBUG == 1
			gchar buffer1[128]; GDate *date;
			date = g_date_new_julian(arc->nextdate);
			g_date_strftime (buffer1, 128-1, "%x", date);
			g_date_free(date);
			g_print("  -> '%s' - every %d %s - next %s limit %d\n", arc->wording, arc->every, CYA_UNIT[arc->unit], buffer1, arc->limit);
			#endif*/


			maxdate = GLOBALS->today + GLOBALS->auto_nbdays;
			if(arc->nextdate <= maxdate)
			{
			guint32 mydate = arc->nextdate;

				while(mydate <= maxdate)
				{
				Operation ope;
				Account *acc;

				/*#if MYDEBUG == 1
					gchar buffer1[128]; GDate *date;
					date = g_date_new_julian(mydate);
					g_date_strftime (buffer1, 128-1, "%x", date);
					g_date_free(date);
					g_printf("  -> adding '%s' on %s\n", arc->wording, buffer1);
				#endif*/

					/* fill in the operation */
					memset(&ope, 0, sizeof(ope));
					ope.date		= mydate;
					ope.amount		= arc->amount;
					ope.account		= arc->account;
					ope.paymode		= arc->paymode;
					ope.flags		= arc->flags | OF_ADDED;
					ope.payee		= arc->payee;
					ope.category	= arc->category;
					ope.dst_account	= arc->dst_account;
					ope.wording		= g_strdup(arc->wording);
					ope.info		= NULL;

					/* todo: fill in cheque number */

					operation_add(&ope, NULL, 0);
					GLOBALS->change++;
					count++;

					/* todo: update acc flags */
					acc = da_acc_get(arc->account);
					if(acc)
						acc->flags |= AF_ADDED;
					if(arc->paymode == PAYMODE_INTXFER)
					{
						acc = da_acc_get(arc->dst_account);
						if(acc)
							acc->flags |= AF_ADDED;
					}

					/* compute next occurence */
					switch(arc->unit)
					{
						case AUTO_UNIT_DAY:
							mydate += arc->every;
							break;
						case AUTO_UNIT_WEEK:
							mydate += (7*arc->every);
							break;
						case AUTO_UNIT_MONTH:
						{
						GDate *date = g_date_new_julian(mydate);
							g_date_add_months(date, (gint)arc->every);
							mydate = g_date_get_julian(date);
							g_date_free(date);
							}
							break;
						case AUTO_UNIT_YEAR:
							mydate += (365*arc->every);
							break;
					}

					/* check limit, update and maybe break */
					if(arc->flags & OF_LIMIT)
					{
						arc->limit--;
						if(arc->limit <= 0)
						{
							arc->flags ^= (OF_LIMIT | OF_AUTO);
							goto nextarchive;
						}
					}



				}

				/* store next occurence */
				arc->nextdate = mydate;

			}


		}
nextarchive:

		list = g_list_next(list);
		

	}

	// refresh account listview
	
	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	//gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

	//force inform in case of insertion
	/*if(usermode == FALSE && count > 0)
		usermode = TRUE;
	*/

	wallet_busy(widget, FALSE);


	//inform the user
	if(usermode == TRUE)
	{
	gchar *txt;

		if(count == 0)
			txt = _("No transaction to insert");
		else
			txt = _("transaction inserted: %d");

		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Check automated transactions result"),
			txt,
			count);
	}



}





/*
**
*/
void wallet_revert(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkWidget *dialog;
gchar *basename;
gint result;

	DB( g_printf("(wallet) revert\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

		basename = g_path_get_basename(GLOBALS->filename);
		dialog = gtk_message_dialog_new
		(
			GTK_WINDOW(GLOBALS->mainwindow),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			_("Revert to the previously saved file of '%s'?"),
			basename
		);
		g_free(basename);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
			_("- Changes made to the wallet will be permanently lost\n"
			"- Wallet will be restored to the last save (.xhb~)")
			);

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    GTK_STOCK_CANCEL, 0,
			GTK_STOCK_REVERT_TO_SAVED, 1,
			NULL);

	  result = gtk_dialog_run( GTK_DIALOG( dialog ) );
	  gtk_widget_destroy( dialog );

	if( result == 1)
	{
		DB( g_printf(" - should revert\n") );

		g_free(GLOBALS->filename);
		GLOBALS->filename = GLOBALS->oldfilename;
		GLOBALS->oldfilename = NULL;
		wallet_open_internal(widget, NULL);
	}

}

/*
**
*/
void wallet_open(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;

	DB( g_printf("(wallet) open\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( wallet_check_change(widget,NULL) == TRUE )
	{
		if(homebank_file_chooser(GTK_FILE_CHOOSER_ACTION_OPEN) == TRUE)
		{
			wallet_open_internal(widget, NULL);
			
			
		}
	}
}

/*
 *	open the file stored in GLOBALS->filename
 */
void wallet_open_internal(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
gchar *basename;
gint r;

	DB( g_printf("(wallet) open internal\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( GLOBALS->filename != NULL )
	{
		homebank_alienfile_recognize(GLOBALS->filename);

		//DB( g_print("file type is %d\n", type) );
		
		wallet_clear(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
		GLOBALS->wallet_is_new = FALSE;


		g_free(GLOBALS->oldfilename);
		GLOBALS->oldfilename = NULL;
		GLOBALS->exists_old = FALSE;
		basename = g_path_get_basename(GLOBALS->filename);
		if( g_str_has_suffix(basename, ".xhb") )
		{
			g_free(GLOBALS->oldfilename);
			GLOBALS->oldfilename = homebank_get_filename_with_extension(GLOBALS->filename, "xhb~");
			GLOBALS->exists_old = g_file_test(GLOBALS->oldfilename, G_FILE_TEST_EXISTS);
			
		}
		g_free(basename);

		DB( g_printf(" filename: %s\n", GLOBALS->filename) );
		DB( g_printf(" oldfilename: %s, exists: %d\n", GLOBALS->oldfilename, GLOBALS->exists_old) );

		r = homebank_load_xml(GLOBALS->filename);
		if( r == XML_OK )
		{
			
			wallet_populate_listview(GLOBALS->mainwindow, NULL);
			wallet_check_automated(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
			wallet_refresh_upcoming(GLOBALS->mainwindow, NULL);
			wallet_compute_balances(GLOBALS->mainwindow, NULL);
			wallet_recent_add(data, GLOBALS->filename);
			
			homebank_lastopenedfiles_save();
		}
		else
		{
		gchar *msg = _("The file %s is not a valid HomeBank file.");

			switch(r)
			{
				case XML_IO_ERROR:
					msg = _("I/O error for file %s.");
					break;
			}

			
			homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
				_("File error"),
				msg,
				GLOBALS->filename
				);

			wallet_clear(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
		
		}

		
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_VISUAL+UF_BALANCE));
	}
	//safe

}

/*
**
*/
void wallet_save(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
gboolean saveas = GPOINTER_TO_INT(user_data);
gint r = XML_UNSET;

	DB( g_printf("(wallet) save\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( GLOBALS->wallet_is_new == TRUE )
		saveas = 1;

	if(saveas == 1)
	{
		if(homebank_file_chooser(GTK_FILE_CHOOSER_ACTION_SAVE) == TRUE)
		{
			DB( g_printf(" + should save as %s\n", GLOBALS->filename) );
			homebank_backup_current_file(GLOBALS->filename);
			homebank_file_ensure_xhb();
			r = homebank_save_xml(GLOBALS->filename);
			GLOBALS->wallet_is_new = FALSE;

		}
	}
	else
	{
		DB( g_printf(" + should quick save %s\n", GLOBALS->filename) );
		homebank_backup_current_file(GLOBALS->filename);
		homebank_file_ensure_xhb();
		r = homebank_save_xml(GLOBALS->filename);
	}

	
	if(r == XML_OK)
	{
		wallet_recent_add(data, GLOBALS->filename);

		GLOBALS->change = 0;
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_VISUAL));
	}
	else
	{
	gchar *msg = _("I/O error for file %s.");

		homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
			_("File error"),
			msg,
			GLOBALS->filename
			);

	}
	

}




/*
**
*/
void wallet_populate_listview(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkTreeModel *model;
GtkTreeIter  iter1, child_iter;
GList *list;
Account *acc;

	DB( g_printf("(wallet) populate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));

	gtk_tree_store_clear (GTK_TREE_STORE(model));

	//1: Accounts
	gtk_tree_store_append (GTK_TREE_STORE(model), &iter1, NULL);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iter1,
			  LST_DSPACC_DATATYPE, DSPACC_TYPE_HEADER,
			  LST_DSPACC_NAME, _("<b>Accounts</b>"),
			  -1);

	//insert all glist item into treeview
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
		acc = list->data;

		if( !(acc->flags & AF_CLOSED))		//do not display closed accounts
		{
			DB( g_printf(" -> insert %d:%s\n", acc->key, acc->name) );

			gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter1);
			gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
				  LST_DSPACC_DATAS, acc,
				  LST_DSPACC_DATATYPE, DSPACC_TYPE_NORMAL,
				  -1);
		}

		list = g_list_next(list);
	}
	g_list_free(list);

	// insert the total line
	gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter1);
	gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
			  LST_DSPACC_DATATYPE, DSPACC_TYPE_SUBTOTAL,
			  LST_DSPACC_NAME, _("<b>Total</b>"),
			  -1);
	



	gtk_tree_view_expand_all(GTK_TREE_VIEW(data->LV_acc));

}


typedef struct
{
	gdouble bank, today, future;
} tmp_balances;

void wallet_compute_balances(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkTreeModel *model;
GtkTreeIter  iter, child_iter;
tmp_balances *array;
gboolean valid;
gint length, i;

	DB( g_printf("(wallet) compute balances\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	/* allocate tmp_balances memory */
	length = da_acc_get_max_key();
	array = g_malloc0((length+1) * sizeof(tmp_balances));

	//debug
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));

	if( model == NULL )
		return;

	if(array)
	{
		data->bank = 0;
		data->today = 0;
		data->future = 0;

		GList *list;

		/* parse all operations */
		DB( g_printf(" parse all ope\n") );

		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Operation *ope;

			ope = list->data;

			//g_printf("parsing %x %d %.2f\n", ope, ope->ope_Date, ope->ope_Amount);

			if(!(ope->flags & OF_REMIND))
			{
				i = ope->account;

				if( i <= length )	// just in case bug 239939
				{
				/* balances */
					array[i].future += ope->amount;

					if(ope->date <= GLOBALS->today)
						array[i].today += ope->amount;

					if(ope->flags & OF_VALID)
						array[i].bank += ope->amount;
				}
				
				// catch orphans transactions and remove them
				else
				{
					GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, ope);
					da_operation_free(ope);
		
				}
				
				
			}
			list = g_list_next(list);
		}

		/* set initial amount for each account */
	    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		if( valid )
		{
			valid = gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child_iter, &iter);
		    while (valid)
		    {
			Account *acc;

				gtk_tree_model_get(GTK_TREE_MODEL(model), &child_iter, LST_DSPACC_DATAS, &acc, -1);

				if( acc != NULL)
				{
					i = acc->key;

					array[i].bank += acc->initial;
					array[i].today += acc->initial;
					array[i].future += acc->initial;


					gtk_tree_store_set(GTK_TREE_STORE(model), &child_iter,
						LST_DSPACC_BANK, array[i].bank,
						LST_DSPACC_TODAY, array[i].today,
						LST_DSPACC_FUTURE, array[i].future,
						-1);

					data->bank += array[i].bank;
					data->today += array[i].today;
					data->future += array[i].future;
				}
				else
				{
				 /* set the group */
		 			gtk_tree_store_set(GTK_TREE_STORE(model), &child_iter,
						LST_DSPACC_BANK, data->bank,
						LST_DSPACC_TODAY, data->today,
						LST_DSPACC_FUTURE, data->future,
						-1);
				}

		       /* Make iter point to the next row in the list store */
		       valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child_iter);
		    }
		 
		 
		 
		 }
	}

	g_free(array);
}


void wallet_update(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
gint flags;

	DB( g_printf("(wallet) refresh_display\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	flags = GPOINTER_TO_INT(user_data);

	/* set window title */
	if(flags & UF_TITLE)
	{
	gchar *basename;
	gchar *changed;

		DB( printf(" +  1: wintitle %x\n", (gint)data->wintitle) );

		basename = g_path_get_basename(GLOBALS->filename);

		DB( printf(" global changes: %d\n", GLOBALS->change) );

		g_free(data->wintitle);
		
		changed = (GLOBALS->change > 0) ? "*" : "";

		data->wintitle = g_strdup_printf("%s%s %s - " PROGNAME, changed, basename, GLOBALS->title);

	    gtk_window_set_title (GTK_WINDOW (gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), data->wintitle);

		g_free(basename);
	}

	/* update disabled things */
	if(flags & UF_SENSITIVE)
	{
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	GtkTreePath		*path;
	gboolean	active,sensitive;

		DB( printf(" +  2: disabled, opelist count\n") );

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));

		active = gtk_tree_selection_get_selected(selection, &model, &iter);
		if(active)
		{
		Account *acc;
		gint depth;

			path = gtk_tree_model_get_path(model, &iter);
			depth =	gtk_tree_path_get_depth(path);

			if( depth > 1 )
			{
				DB( printf(" depth is %d\n", depth) );

				gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPACC_DATAS, &acc, -1);
				data->acc = acc;

			}
			else
				active = FALSE;
		}
		else
		{
			//ensure data->acc will not be null
			data->acc = da_acc_get(1);
		}


		// no change: disable save
		DB( printf(" changes %d - new %d\n", GLOBALS->change, GLOBALS->wallet_is_new) );
		
		
		sensitive = (GLOBALS->change != 0 ) ? TRUE : FALSE;
		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/SaveAs"), sensitive);
		//if(sensitive == TRUE && GLOBALS->wallet_is_new == TRUE) sensitive = FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Save"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Revert"), GLOBALS->exists_old);


	// define off ?
		sensitive = GLOBALS->define_off == 0 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Account"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Payee"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Category"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Budget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/EditMenu/Preferences"), sensitive);

	// empty account list: disable Import, Archives, Edit, Filter, Add, Statistics, Overdrawn, Car Cost
		sensitive = da_acc_length() > 0 ? TRUE : FALSE;

		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data-data->manager, "/MenuBar/FileMenu/Import"), sensitive);

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Close"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Archive"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/OperationMenu/AddOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/OperationMenu/ShowOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RStatistics"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RTrendTime"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RBudget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RBalance"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RCarcost"), sensitive);

	// empty category list: disable Budget & Budget report
		sensitive = da_cat_length() > 1 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Budget"), sensitive);

	// empty archive list: disable Automated check
		sensitive = g_list_length(GLOBALS->arc_list) > 0 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/OperationMenu/Automated"), sensitive);

	// no active account: disable Edit, Over
		sensitive = (active == TRUE ) ? TRUE : FALSE;
		if(data->acc && data->acc->window != NULL)
			sensitive = FALSE;
		
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/OperationMenu/ShowOpe"), sensitive);

	}

	/* update toolbar, list, statusbar */
	if(flags & UF_VISUAL)
	{
		DB( printf(" +  8: visual\n") );
	
		if(PREFS->toolbar_style == 0)
			gtk_toolbar_unset_style(GTK_TOOLBAR(data->toolbar));
		else
			gtk_toolbar_set_style(GTK_TOOLBAR(data->toolbar), PREFS->toolbar_style-1);
	
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (data->LV_acc), PREFS->rules_hint);
		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

		homebank_pref_createformat();
		homebank_pref_init_measurement_units();
		
		DB( printf(" toolbar %d\n", PREFS->wal_toolbar) );
		if(PREFS->wal_toolbar)
			gtk_widget_show(GTK_WIDGET(data->toolbar));
		else
			gtk_widget_hide(GTK_WIDGET(data->toolbar));

		DB( printf(" statusbar %d\n", PREFS->wal_statusbar) );
		if(PREFS->wal_statusbar)
			gtk_widget_show(GTK_WIDGET(data->statusbar));
		else
			gtk_widget_hide(GTK_WIDGET(data->statusbar));

		DB( printf(" upcoming %d\n", PREFS->wal_upcoming) );
		if(PREFS->wal_upcoming)
			gtk_widget_show(GTK_WIDGET(data->GR_upc));
		else
			gtk_widget_hide(GTK_WIDGET(data->GR_upc));
				
		DB( printf(" minor %d\n", PREFS->euro_active) );
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/AsMinor"), PREFS->euro_active);
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{

		DB( printf(" +  4: balances\n") );

		//minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

		/*
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->bank, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->today, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->future, minor);
		*/
	}



}

void wallet_busy(GtkWidget *widget, gboolean state)
{
struct wallet_data *data;
GtkWidget *window;
GdkCursor *cursor;

	DB( g_printf("(wallet) busy: %d\n", state) );

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


static void
  wallet_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
  {
    GtkTreeModel *model;
    GtkTreeIter   iter;

    DB( g_print ("(wallet) A row has been double-clicked!\n") );

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
	Account *acc;

       gtk_tree_model_get(model, &iter, LST_DSPACC_DATAS, &acc, -1);

		if( acc != NULL )
		{

       DB( g_print ("Double-clicked row contains name %s\n", acc->name) );

		wallet_action_showoperations();

       //g_free(name);
    	}
    }
  }

/*
**
*/
static gboolean wallet_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct wallet_data *data = user_data;
struct WinGeometry *wg;
gboolean retval = FALSE;

	DB( g_printf("(wallet) dispose\n") );

	//store position and size
	wg = &PREFS->wal_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

 	PREFS->wal_vpaned = gtk_paned_get_position(GTK_PANED(data->vpaned));

	DB( g_print(" -> paned position is %d\n", PREFS->wal_vpaned) );	

	//todo
	if(wallet_check_change(widget, NULL) == FALSE)
	{
		retval = TRUE;
	}
	else
	{
		DB( g_printf(" free wintitle %x\n", (gint)data->wintitle) );

		g_free(data->wintitle);
		g_free(user_data);
		gtk_main_quit();
	}



	//delete-event TRUE abort/FALSE destroy
	return retval;
}







static void
wallet_menu_item_selected_cb (GtkItem *item, struct wallet_data *data)
{
	GtkAction *action;
	gchar *tooltip;

	action = GTK_ACTION (g_object_get_data (G_OBJECT (item), "action-for-proxy"));

	g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);

	if (tooltip != NULL)
		gtk_statusbar_push (GTK_STATUSBAR (data->statusbar),
				    data->statusbar_menu_context_id, tooltip);

	g_free (tooltip);
}

static void
wallet_menu_item_deselected_cb (GtkItem *item, struct wallet_data *data)
{
	gtk_statusbar_pop (GTK_STATUSBAR (data->statusbar),
			   data->statusbar_menu_context_id);
}

static void
wallet_ui_connect_proxy_cb (GtkUIManager *ui,
		         GtkAction    *action,
		         GtkWidget    *proxy,
		         struct wallet_data *data)
{	
	if (GTK_IS_MENU_ITEM (proxy))
	{	
		g_object_set_data (G_OBJECT (proxy), "action-for-proxy", action);
	
		g_signal_connect(G_OBJECT(proxy), "select", 
				 G_CALLBACK (wallet_menu_item_selected_cb), data);
		g_signal_connect(G_OBJECT(proxy), "deselect", 
				 G_CALLBACK (wallet_menu_item_deselected_cb), data);
	}
}

static void
wallet_ui_disconnect_proxy_cb (GtkUIManager *manager,
                     	    GtkAction *action,
                            GtkWidget *proxy,
                            struct wallet_data *data)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_handlers_disconnect_by_func
                	(proxy, G_CALLBACK (wallet_menu_item_selected_cb), data);
		g_signal_handlers_disconnect_by_func
			(proxy, G_CALLBACK (wallet_menu_item_deselected_cb), data);
	}
}




static void
wallet_recent_chooser_item_activated_cb (GtkRecentChooser *chooser, struct wallet_data *data)
{
	gchar *uri, *path;
	GError *error = NULL;

	uri = gtk_recent_chooser_get_current_uri (chooser);

	path = g_filename_from_uri (uri, NULL, NULL);
	if (error)
	{
		g_warning ("Could not convert uri \"%s\" to a local path: %s", uri, error->message);
		g_error_free (error);
		return;
	}

	if( wallet_check_change(data->window, NULL) == TRUE )
	{

		//todo: FixMe
		/*
		if (! load)
		{
			gpw_recent_remove (gpw, path);
		}
		*/


		g_free(GLOBALS->filename);
		GLOBALS->filename = path;	
		wallet_open_internal(data->window, NULL);
	}
	else
	{
		g_free (path);
	}
	g_free (uri);
}


static void
wallet_window_screen_changed_cb (GtkWidget *widget,
			      GdkScreen *old_screen,
			      struct wallet_data *data)
{

	DB( g_printf("wallet_window_screen_changed_cb\n") );


	data->recent_manager = gtk_recent_manager_get_default ();

	gtk_menu_detach (GTK_MENU (data->recent_menu));
	g_object_unref (G_OBJECT (data->recent_menu));
	
	data->recent_menu = create_recent_chooser_menu (data->recent_manager);

	g_signal_connect (data->recent_menu,
			  "item-activated",
			  G_CALLBACK (wallet_recent_chooser_item_activated_cb),
			  data);
			  
	//menu_item = gtk_ui_manager_get_widget (data->manager, "/MenuBar/FileMenu/OpenRecent");
	//gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), data->recent_menu);
}


void
wallet_recent_add (struct wallet_data *data, const gchar *path)
{
	GtkRecentData *recent_data;
	gchar *uri;
	GError *error = NULL;

	uri = g_filename_to_uri (path, NULL, &error);
	if (error)
	{	
		g_warning ("Could not convert uri \"%s\" to a local path: %s", uri, error->message);
		g_error_free (error);
		return;
	}

	recent_data = g_slice_new (GtkRecentData);

	recent_data->display_name   = NULL;
	recent_data->description    = NULL;
	recent_data->mime_type      = "application/x-homebank";
	recent_data->app_name       = (gchar *) g_get_application_name ();
	recent_data->app_exec       = g_strjoin (" ", g_get_prgname (), "%u", NULL);
	recent_data->groups         = NULL;
	recent_data->is_private     = FALSE;

	if (!gtk_recent_manager_add_full (data->recent_manager,
				          uri,
				          recent_data))
	{
      		g_warning ("Unable to add '%s' to the list of recently used documents", uri);
	}

	g_free (uri);
	g_free (recent_data->app_exec);
	g_slice_free (GtkRecentData, recent_data);

}





enum
{
	TARGET_URI_LIST
};

static GtkTargetEntry drop_types[] =
{
	{"text/uri-list", 0, TARGET_URI_LIST}
};

static void
drag_data_received (GtkWidget *widget,
			GdkDragContext *context,
			gint x, gint y,
			GtkSelectionData *selection_data,
			guint info, guint time, GtkWindow *window)
{
	gchar **uris, **str;
	gchar *data;
	gint filetype;

	if (info != TARGET_URI_LIST)
		return;

	/* On MS-Windows, it looks like `selection_data->data' is not NULL terminated. */
	data = g_new (gchar, selection_data->length + 1);
	memcpy (data, selection_data->data, selection_data->length);
	data[selection_data->length] = 0;

	uris = g_uri_list_extract_uris (data);

	str = uris;
	//for (str = uris; *str; str++)
	if( *str )
	{
		GError *error = NULL;
		gchar *path = g_filename_from_uri (*str, NULL, &error);

		if (path)
		{
			filetype = homebank_alienfile_recognize(path);

			DB( g_print(" dragged %s, type is %d\n", path, filetype ) );

			if( filetype == FILETYPE_HOMEBANK)
			{
				g_free(GLOBALS->filename);
				GLOBALS->filename = g_strdup(path);
				wallet_open_internal(GTK_WIDGET(window), NULL);
			}
			else
			{
				//todo: future here to implement import for other filetype

				homebank_message_dialog(GTK_WINDOW(window), GTK_MESSAGE_ERROR,
					_("File error"),
					_("The file %s is not a valid HomeBank file."),
					path
					);

				
			}
			
		}
		else
		{
			g_warning ("Could not convert uri to local path: %s", error->message); 

			g_error_free (error);
		}
		g_free (path);
	}
	g_strfreev (uris);
}








static GtkWidget *
create_recent_chooser_menu (GtkRecentManager *manager)
{
GtkWidget *toolbar_recent_menu;
GtkRecentFilter *filter;

	toolbar_recent_menu = gtk_recent_chooser_menu_new_for_manager (manager);

	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (toolbar_recent_menu),
					FALSE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (toolbar_recent_menu), 
					GTK_RECENT_SORT_MRU);
	//todo: add a user pref for this
	gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER (toolbar_recent_menu),
					5);


	//gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (toolbar_recent_menu), FALSE);

	//gtk_recent_chooser_menu_set_show_numbers (GTK_RECENT_CHOOSER_MENU (toolbar_recent_menu), TRUE);

	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_application (filter, g_get_application_name());
	gtk_recent_chooser_set_filter (GTK_RECENT_CHOOSER (toolbar_recent_menu), filter);

	return toolbar_recent_menu;
}


static void
create_menu_bar_and_toolbar(struct wallet_data *data, GtkWidget *mainvbox)
{
GtkUIManager *manager;
GtkActionGroup *action_group;
GtkAction *action;
GError *error = NULL;
	
	manager = gtk_ui_manager_new ();
	data->manager = manager;

	gtk_window_add_accel_group (GTK_WINDOW (data->window),
				gtk_ui_manager_get_accel_group(manager));

	action_group = gtk_action_group_new ("Wallet");
	gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
			entries,
			n_entries,
			NULL);

	gtk_action_group_add_toggle_actions (action_group,
			toggle_entries,
			n_toggle_entries,
			NULL);

	gtk_ui_manager_insert_action_group (data->manager, action_group, 0);
	g_object_unref (action_group);
	data->actions = action_group;

	/* set short labels to use in the toolbar */
	action = gtk_action_group_get_action(action_group, "Open");
	g_object_set(action, "short_label", _("Open"), NULL);
	
	action = gtk_action_group_get_action(action_group, "Save");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(action_group, "Account");
	g_object_set(action, "short_label", _("Account"), NULL);

	action = gtk_action_group_get_action(action_group, "Payee");
	g_object_set(action, "short_label", _("Payee"), NULL);

	action = gtk_action_group_get_action(action_group, "Category");
	g_object_set(action, "short_label", _("Category"), NULL);

	action = gtk_action_group_get_action(action_group, "Archive");
	//TRANSLATORS: an archive is stored transaction buffers (kind of bookmark to prefill manual insertion)
	g_object_set(action, "short_label", _("Archive"), NULL);

	action = gtk_action_group_get_action(action_group, "Budget");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(action_group, "ShowOpe");
	g_object_set(action, "short_label", _("Show"), NULL);

	action = gtk_action_group_get_action(action_group, "AddOpe");
	g_object_set(action, "is_important", TRUE, "short_label", _("Add"), NULL);

	action = gtk_action_group_get_action(action_group, "RStatistics");
	g_object_set(action, "short_label", _("Statistics"), NULL);

	action = gtk_action_group_get_action(action_group, "RBudget");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(action_group, "RBalance");
	g_object_set(action, "short_label", _("Balance"), NULL);

	action = gtk_action_group_get_action(action_group, "RCarcost");
	g_object_set(action, "short_label", _("Carcost"), NULL);

	/* now load the UI definition */
	gtk_ui_manager_add_ui_from_string (data->manager, ui_info, -1, &error);
	if (error != NULL)
	{
		g_message ("Building menus failed: %s", error->message);
		g_error_free (error);
	}
	
	/* show tooltips in the statusbar */
	g_signal_connect(G_OBJECT(data->manager), 
	         "connect-proxy",
			 G_CALLBACK (wallet_ui_connect_proxy_cb),
	         data);
	g_signal_connect(G_OBJECT(data->manager),
	         "disconnect-proxy",
			 G_CALLBACK (wallet_ui_disconnect_proxy_cb),
	         data);


	data->menubar = gtk_ui_manager_get_widget (manager, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (mainvbox), 
			    data->menubar, 
			    FALSE, 
			    FALSE, 
			    0);

	data->toolbar = gtk_ui_manager_get_widget (manager, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (mainvbox), 
			    data->toolbar, 
			    FALSE, 
			    FALSE, 
			    0);

	
	/* recent files menu */



	data->recent_manager = gtk_recent_manager_get_default ();

	data->recent_menu = create_recent_chooser_menu (data->recent_manager);

	g_signal_connect (data->recent_menu,
			  "item-activated",
			  G_CALLBACK (wallet_recent_chooser_item_activated_cb),
			  data);

/*			  
	widget = gtk_ui_manager_get_widget (data->manager, "/MenuBar/FileMenu/OpenRecent");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), data->recent_menu);		  
*/

	/* testing */
		/* add the custom Open button to the toolbar */
	GtkToolItem *open_button = gtk_menu_tool_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (open_button),
				       data->recent_menu);

	gtk_tool_item_set_tooltip_text (open_button, _("Open a file"));
	gtk_menu_tool_button_set_arrow_tooltip_text (GTK_MENU_TOOL_BUTTON (open_button),
						     _("Open a recently used file"));


	action = gtk_action_group_get_action (data->actions, "Open");
	g_object_set (action,
		      "short_label", _("Open"),
		      NULL);
	//gtk_action_connect_proxy (action, GTK_WIDGET (open_button));
	gtk_activatable_set_related_action (GTK_ACTIVATABLE (open_button), action);

	gtk_toolbar_insert (GTK_TOOLBAR (data->toolbar),
			    open_button,
			    1);
	/* end testing */

}




/*
 *
 */
static void
create_statusbar (struct wallet_data *data, GtkWidget *mainvbox)
{
	data->statusbar = gtk_statusbar_new ();

	data->statusbar_menu_context_id =
		gtk_statusbar_get_context_id (GTK_STATUSBAR (data->statusbar), "menu");
	data->statusbar_actions_context_id =
		gtk_statusbar_get_context_id (GTK_STATUSBAR (data->statusbar), "actions");

	gtk_box_pack_end (GTK_BOX (mainvbox),
			  data->statusbar,
			  FALSE, 
			  TRUE, 
			  0);

}


/*
** the window creation
*/
GtkWidget *create_wallet_window(GtkWidget *do_widget)
{
struct wallet_data *data;
GtkWidget *mainvbox, *vbox, *paned;
GtkWidget *label;
GtkWidget *treeview, *sw;
GtkWidget *window;
GtkWidget *align;
GtkAction *action;

	DB( g_print("(wallet) create window\n") );

	data = g_malloc0(sizeof(struct wallet_data));
	if(!data) return NULL;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

		//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(wallet) new window=%x, inst_data=%0x\n", (gint)window, (gint)data) );

	// this is our mainwindow, so store it to GLOBALS data
	data->window = window;
	GLOBALS->mainwindow = window;


	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	/* Add menubar and toolbar */
	DB( g_printf("create menu\n") );

	create_menu_bar_and_toolbar (data, mainvbox);

	/* Add status bar */
	DB( g_printf("create statusbar\n") );
	create_statusbar (data, mainvbox);

	/* Add the main area */
	vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), HB_MAINBOX_SPACING);
    gtk_box_pack_start (GTK_BOX (mainvbox), vbox, TRUE, TRUE, 0);

	paned = gtk_vpaned_new();
	data->vpaned = paned;
    gtk_box_pack_start (GTK_BOX (vbox), paned, TRUE, TRUE, 0);


	/* accounts */
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_paned_pack1 (GTK_PANED(paned), vbox, FALSE, FALSE);

		label = make_label(NULL, 0.0, 0.0);
		gtk_label_set_markup (GTK_LABEL(label), _("<b>Accounts summary</b>"));
	    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

		align = gtk_alignment_new(0, 0, 1.0, 1.0);
		// top, bottom, left, right
		gtk_alignment_set_padding (GTK_ALIGNMENT(align), HB_BOX_SPACING/2, HB_BOX_SPACING, HB_BOX_SPACING, 0);
		gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);


		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (align), sw);
		//gtk_container_set_border_width (GTK_CONTAINER(sw), 5);

		// create tree view
		treeview = (GtkWidget *)create_list_account();
		data->LV_acc = treeview;
		gtk_container_add (GTK_CONTAINER (sw), treeview);


	/* bill book :: échéancier */
		vbox = gtk_vbox_new (FALSE, 0);
		data->GR_upc = vbox;
		gtk_paned_pack2 (GTK_PANED(paned), vbox, FALSE, FALSE);

		label = make_label(NULL, 0.0, 0.0);
		gtk_label_set_markup (GTK_LABEL(label), _("<b>Upcoming automated transactions</b>"));
	    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

		align = gtk_alignment_new(0, 0, 1.0, 1.0);
		// top, bottom, left, right
		gtk_alignment_set_padding (GTK_ALIGNMENT(align), HB_BOX_SPACING/2, HB_BOX_SPACING, HB_BOX_SPACING, 0);
		gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (align), sw);

		treeview = (GtkWidget *)create_list_upcoming();
		data->LV_upc = treeview;
		gtk_container_add (GTK_CONTAINER (sw), treeview);




	//todo: move this elsewhere
	DB( g_printf("init actions\n") );


	if(PREFS->wal_vpaned > 0)
		gtk_paned_set_position(GTK_PANED(data->vpaned), PREFS->wal_vpaned);

	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/Toolbar");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_toolbar);
	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/Statusbar");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_statusbar);
	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/Upcoming");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_upcoming);

	/* Drag and drop support, set targets to NULL because we add the
	   default uri_targets below */

	/* support for opening a file by dragging onto the project window */
	gtk_drag_dest_set (GTK_WIDGET (window),
			   GTK_DEST_DEFAULT_ALL,
			   drop_types,
	           G_N_ELEMENTS (drop_types),
			   GDK_ACTION_COPY);

	g_signal_connect (G_OBJECT (window), "drag-data-received",
			  G_CALLBACK (drag_data_received), window);



	//connect all our signals
	DB( g_printf("connect signals\n") );


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), "changed", G_CALLBACK (wallet_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_acc), "row-activated", G_CALLBACK (wallet_onRowActivated), GINT_TO_POINTER(2));

	/* GtkWindow events */
    g_signal_connect (window, "delete-event", G_CALLBACK (wallet_dispose), (gpointer)data);


	g_signal_connect (window, "screen-changed",
			  G_CALLBACK (wallet_window_screen_changed_cb),
			  data);

  return window;
}


