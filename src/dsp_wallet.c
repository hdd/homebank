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
#include "dsp_wallet.h"

#include "dsp_account.h"
#include "xml.h"

#include "def_wallet.h"
#include "ui_account.h"
#include "ui_payee.h"
#include "ui_category.h"
#include "def_archive.h"
#include "def_budget.h"
#include "def_pref.h"
#include "def_operation.h"

#include "list_account.h"
#include "list_upcoming.h"

#include "rep_stats.h"
#include "rep_budget.h"
#include "rep_over.h"
#include "rep_car.h"

#include "import.h"
#include "imp_amiga.h"
#include "imp_qif.h"

//#define HOMEBANK_URL_HELP           "http://homebank.free.fr/help/"
#define HOMEBANK_URL_HELP           "index.html"
#define HOMEBANK_URL_HELP_ONLINE    "https://answers.launchpad.net/homebank/+addquestion"
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



struct wallet_data
{
	GtkWidget	*window;

	gchar	*wintitle;
	GtkWidget	*TB_bar;
	GtkWidget	*BT_help;
	GtkWidget	*GR_info;
	GtkWidget	*GR_minor;
	GtkWidget	*GO_gauge;

	GtkWidget	*LV_acc;
	GtkWidget	*LV_upc;

	GtkWidget *statusbar; /* A pointer to the status bar. */
	guint statusbar_menu_context_id; /* The context id of the menu bar */
	guint statusbar_actions_context_id; /* The context id of actions messages */

	gdouble	bank, today, future;

	//struct	Base base;

	Account *acc;

	gint	busy;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkRecentManager *recent_manager;
	GtkWidget *recent_menu;

	/*
	UBYTE	accnum;
	UBYTE	pad0;
	struct	Account *acc;

	ULONG	mindate, maxdate;
	ULONG	change;
	ULONG	keyvalue;
	UBYTE	title[140];
	UBYTE	Filename[108];
	UBYTE	csvpath[108];
	*/
};

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
static void wallet_action_preferences(void);

static void wallet_action_toggle_minor(GtkToggleAction *action);

static void wallet_action_showoperations(void);
static void wallet_action_addoperations(void);
static void wallet_action_checkautomated(void);

static void wallet_action_statistic(void);
static void wallet_action_budget(void);
static void wallet_action_overdrawn(void);
static void wallet_action_carcost(void);

static void wallet_action_importamiga(void);

static void wallet_action_import(void);
static void wallet_action_export(void);

static void wallet_action_help(void);
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
void wallet_import_amiga(GtkWidget *widget, gpointer user_data);

void wallet_recent_add (struct wallet_data *data, const gchar *path);

extern gchar *CYA_UNIT[];


static GtkActionEntry entries[] = {

  /* name, stock id, label */

  { "FileMenu"     , NULL, N_("_File") },
  { "EditMenu"     , NULL, N_("_Edit") },
  { "ViewMenu"     , NULL, N_("_View") },
  { "OperationMenu", NULL, N_("_Transactions") },
  { "ReportMenu"   , NULL, N_("_Reports")  },
  { "ToolsMenu"    , NULL, N_("T_ools") },
  { "HelpMenu"     , NULL, N_("_Help") },

  { "OpenRecent"   , NULL, N_("Open _Recent") },
  { "Export"       , NULL, N_("Export to") },
	/* name, stock id, label, accelerator, tooltip */

  /* FileMenu */
  { "New"        , GTK_STOCK_NEW            , N_("_New")          , NULL, N_("Create a new wallet"),    G_CALLBACK (wallet_action_new) },
  { "Open"       , GTK_STOCK_OPEN           , N_("_Open...")      , NULL, N_("Open a wallet"),    G_CALLBACK (wallet_action_open) },
  { "Save"       , GTK_STOCK_SAVE           , N_("_Save")         , NULL, N_("Save the current wallet"),    G_CALLBACK (wallet_action_save) },
  { "SaveAs"     , GTK_STOCK_SAVE_AS        , N_("Save As...")    , NULL, N_("Save the current wallet with a different name"),    G_CALLBACK (wallet_action_saveas) },
  { "Revert"     , GTK_STOCK_REVERT_TO_SAVED, N_("Revert")        , NULL, N_("Revert to a saved version of this file"),    G_CALLBACK (wallet_action_revert) },
  { "Import"     , NULL                     , N_("Import QIF/OFX/CSV...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (wallet_action_import) },

  { "Properties" , GTK_STOCK_PROPERTIES     , N_("_Properties..."), NULL, N_("Configure wallet"),    G_CALLBACK (wallet_action_defwallet) },
  { "Close"      , GTK_STOCK_CLOSE          , N_("_Close")        , NULL, N_("Close the current wallet"),    G_CALLBACK (wallet_action_close) },
  { "Quit"       , GTK_STOCK_QUIT           , N_("_Quit")         , NULL, N_("Quit homebank"),    G_CALLBACK (wallet_action_quit) },

  /* Export Menu */
  { "ExportQIF"  , NULL                     , N_("QIF Format...") , NULL, N_("Open the export to QIF assistant"),    G_CALLBACK (wallet_action_export) },

  /* EditMenu */
  { "Account"    , "hb-stock-account"   , N_("Acc_ounts...")  , NULL,    N_("Configure the accounts"), G_CALLBACK (wallet_action_defaccount) },
  { "Payee"      , "hb-stock-payee"     , N_("_Payees...")    , NULL,    N_("Configure the payees"),    G_CALLBACK (wallet_action_defpayee) },
  { "Category"   , "hb-stock-category"  , N_("Categories...") , NULL,    N_("Configure the categories"),    G_CALLBACK (wallet_action_defcategory) },
  { "Archive"    , "hb-stock-archive"   , N_("Arc_hives...")  , NULL,    N_("Configure the archives"),    G_CALLBACK (wallet_action_defarchive) },
  { "Budget"     , "hb-stock-budget"    , N_("Budget...")     , NULL,    N_("Configure the budget"),    G_CALLBACK (wallet_action_defbudget) },
  { "Preferences", GTK_STOCK_PREFERENCES, N_("Preferences..."), NULL,    N_("Configure homebank"),    G_CALLBACK (wallet_action_preferences) },

  /* OperationMenu */
  { "ShowOpe"    , "hb-stock-ope-show"  , N_("Show...")           , NULL,    N_("Shows selected account transactions"),    G_CALLBACK (wallet_action_showoperations) },
  { "AddOpe"     , "hb-stock-ope-add"   , N_("Add...")            , NULL,    N_("Add transaction"),    G_CALLBACK (wallet_action_addoperations) },
  { "Automated"  , NULL                 , N_("Check automated..."), NULL,    N_("Insert pending automated transactions"),    G_CALLBACK (wallet_action_checkautomated) },

  /* ReportMenu */
  { "Statistics" , "hb-stock-rep-stats" , N_("_Statistics..."), NULL,    N_("Open the Statistics report"),    G_CALLBACK (wallet_action_statistic) },
  { "BudgetR"    , "hb-stock-rep-budget", N_("B_udget...")    , NULL,    N_("Open the Budget report"),    G_CALLBACK (wallet_action_budget) },
  { "Overdrawn"  , "hb-stock-rep-over"  , N_("Ove_rdrawn...") , NULL,    N_("Open the Overdrawn report"),    G_CALLBACK (wallet_action_overdrawn) },
  { "Carcost"    , "hb-stock-rep-car"   , N_("_Car cost...")   , NULL,    N_("Open the Car cost report"),    G_CALLBACK (wallet_action_carcost) },

  /* ToolsMenu */
  { "ImportAmiga", NULL                     , N_("Import Amiga..."), NULL, N_("Import a file in Amiga 3.0 format"),    G_CALLBACK (wallet_action_importamiga) },

  /* HelpMenu */
  { "Contents"   , GTK_STOCK_HELP       , N_("_Contents")  , "F1", N_("Documentation about HomeBank"), G_CALLBACK (wallet_action_help) },
  { "Online"     , "hb-lpi-help"        , N_("Get Help Online...")  , NULL, N_("Connect to the LaunchPad website for online help"), G_CALLBACK (wallet_action_help_online) },
  { "Translate"  , "hb-lpi-translate"   , N_("Translate this Application...")  , NULL, N_("Connect to the LaunchPad website to help translate this application"), G_CALLBACK (wallet_action_help_translate) },
  { "Problem"    , "hb-lpi-bug"         , N_("Report a Problem...")  , NULL, N_("Connect to the LaunchPad website to help fix problems"), G_CALLBACK (wallet_action_help_problem) },

  { "About"      , GTK_STOCK_ABOUT      , N_("_About")     , NULL, N_("About HomeBank")      ,G_CALLBACK (wallet_about) },

};
static guint n_entries = G_N_ELEMENTS (entries);


static GtkToggleActionEntry toggle_entries[] = {
/*  name         , stockid, label, accelerator, tooltip, callback, is_active */
  { "AsMinor"    , NULL                 , N_("Minor currency"), "<control>M",    NULL,    G_CALLBACK (wallet_action_toggle_minor), FALSE },
};

static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);


static const gchar *ui_info = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='OpenRecent'/>"
"      <separator/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='Revert'/>"
"      <separator/>"
"      <menuitem action='Import'/>"

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
"      <menuitem action='Account'/>"
"      <menuitem action='Payee'/>"
"      <menuitem action='Category'/>"
"      <separator/>"
"      <menuitem action='Archive'/>"
"      <menuitem action='Budget'/>"
"      <separator/>"
"      <menuitem action='Preferences'/>"
"    </menu>"

"    <menu action='ViewMenu'>"
"      <menuitem action='AsMinor'/>"
"    </menu>"

"    <menu action='OperationMenu'>"
"      <menuitem action='ShowOpe'/>"
"      <separator/>"
"      <menuitem action='AddOpe'/>"
"      <menuitem action='Automated'/>"
"    </menu>"

"    <menu action='ReportMenu'>"
"      <menuitem action='Statistics'/>"
"      <menuitem action='Overdrawn'/>"
"      <separator/>"
"      <menuitem action='BudgetR'/>"
"      <menuitem action='Carcost'/>"
"    </menu>"

"    <menu action='ToolsMenu'>"
"      <menuitem action='ImportAmiga'/>"
"    </menu>"

"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
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
"    <toolitem action='Open'/>"
"    <toolitem action='Save'/>"

"      <separator/>"

"    <toolitem action='Account'/>"
"    <toolitem action='Payee'/>"
"    <toolitem action='Category'/>"

"      <separator/>"

"    <toolitem action='Archive'/>"
"    <toolitem action='Budget'/>"

"      <separator/>"

"    <toolitem action='ShowOpe'/>"
"    <toolitem action='AddOpe'/>"

"      <separator/>"

"    <toolitem action='Statistics'/>"
"    <toolitem action='Overdrawn'/>"

"      <separator/>"

"    <toolitem action='BudgetR'/>"
"    <toolitem action='Carcost'/>"

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

static void wallet_action_preferences(void)
{
	defpref_dialog_new();
	wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

/* display action */

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

static void wallet_action_budget(void)
{
	repbudget_window_new();
}

static void wallet_action_overdrawn(void)
{
	repover_window_new();
}

static void wallet_action_carcost(void)
{
	repcar_window_new();
}

static void wallet_action_importamiga(void)
{
	wallet_import_amiga(GLOBALS->mainwindow, NULL);
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
	if (!retval)
	{
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("No suitable web browser executable could be found."),
			_("Could not display the URL '%s'"),
			link
			);
	}

    g_free(link);
}

static void
activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data)
{
gboolean retval;
	
	retval = homebank_util_url_show (link);
	
	if (!retval)
	{
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("No suitable web browser application could be found."),
			_("Could not display the URL '%s'"),
			link
			);
	}
}

static void wallet_action_help_online(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_ONLINE;

	retval = homebank_util_url_show (link);
	if (!retval)
	{
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("No suitable web browser application could be found."),
			_("Could not display the URL '%s'"),
			link
			);
	}
}

static void wallet_action_help_translate(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_TRANSLATE;
	
	retval = homebank_util_url_show (link);
	if (!retval)
	{
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("No suitable web browser executable could be found."),
			_("Could not display the URL '%s'"),
			link
			);
	}
}

static void wallet_action_help_problem(void)
{
gboolean retval;
const gchar *link = HOMEBANK_URL_HELP_PROBLEM;
	
	retval = homebank_util_url_show (link);
	if (!retval)
	{
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("No suitable web browser executable could be found."),
			_("Could not display the URL '%s'"),
			link
			);
	}
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
		  
	static const gchar *copyright = "Copyright \xc2\xa9 1995-2008 - Maxime DOYEN";


	gtk_about_dialog_set_url_hook (activate_url, NULL, NULL);

	pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), "splash.png", NULL);
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
			_("If you do not save, %d changes will be\ndefinitively lost."),
			GLOBALS->change
			);

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    _("Do _not save"), 0,
		    GTK_STOCK_CANCEL, 1,
			GTK_STOCK_SAVE, 2,
			NULL);

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
gint account, count;

	DB( g_printf("(wallet) add operations\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* init the operation */
	date = GLOBALS->today;
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
				  LST_DSPUPC_REMAINING, decay,
				  -1);
		}
		list = g_list_next(list);
	}

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
					acc->flags |= AF_ADDED;
					if(arc->paymode == PAYMODE_PERSTRANSFERT)
					{
						acc = da_acc_get(arc->dst_account);
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
			txt = _("%d transactions inserted");

		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Check automated transactions result"),
			txt,
			count);
	}



}




/*
**
*/
void wallet_import_amiga(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;

	DB( g_printf("(wallet) import amiga\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( wallet_check_change(widget,NULL) == TRUE )
	{
		if(homebank_alienfile_chooser(_("Import Amiga file")) == TRUE)
		{
			//clear all, but not the GLOBALS->filename
			wallet_clear(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));

			GLOBALS->wallet_is_new = TRUE;

			DB( g_printf("- filename: %s\n", GLOBALS->filename) );

			import_from_amiga(GLOBALS->filename);

			wallet_populate_listview(GLOBALS->mainwindow, NULL);
			wallet_check_automated(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
			wallet_refresh_upcoming(GLOBALS->mainwindow, NULL);
			wallet_compute_balances(GLOBALS->mainwindow, NULL);
			wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
		}
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
		
			homebank_message_dialog(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
				_("File error"),
				_("The file %s is not a valid HomeBank file."),
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
gboolean update = FALSE;

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
			homebank_save_xml(GLOBALS->filename);
			GLOBALS->wallet_is_new = FALSE;
			update = TRUE;
		}
	}
	else
	{
		DB( g_printf(" + should quick save %s\n", GLOBALS->filename) );
		homebank_backup_current_file(GLOBALS->filename);
		homebank_file_ensure_xhb();
		homebank_save_xml(GLOBALS->filename);
		update = TRUE;
	}

	if( update == TRUE )
	{
		wallet_recent_add(data, GLOBALS->filename);

		GLOBALS->change = 0;
		wallet_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
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
			  LST_DSPACC_DATAS, NULL,
			  LST_DSPACC_NAME, "<b>Accounts</b>",
			  -1);

/*	
	//2: Investments
	gtk_tree_store_append (GTK_TREE_STORE(model), &iter2, NULL);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iter2,
			  LST_DSPACC_DATAS, NULL,
			  LST_DSPACC_NAME, "<b>Investments</b>",
			  -1);
*/

	//insert all glist item into treeview
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
		acc = list->data;

		//do not display closed accounts
		if( !(acc->flags & AF_CLOSED))
		{

			//todo: for stock account
			/*
				if( acc->type == ACC_TYPE_STOCKS )
					gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter2);
				else
			*/

			DB( g_printf(" -> insert %d:%s\n", acc->key, acc->name) );

			gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter1);
			gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
				  LST_DSPACC_DATAS, acc,
				  -1);
		}

		list = g_list_next(list);
	}
	g_list_free(list);

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

		       /* Make iter point to the next row in the list store */
		       valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child_iter);
		    }
		 
		 /* set the group */
 			gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				LST_DSPACC_BANK, data->bank,
				LST_DSPACC_TODAY, data->today,
				LST_DSPACC_FUTURE, data->future,
				-1);

		 
		 
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

		data->wintitle = g_strdup_printf("%s%s %s : " PROGNAME, changed, basename, GLOBALS->title);

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
		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/SaveAs"), sensitive);
		//if(sensitive == TRUE && GLOBALS->wallet_is_new == TRUE) sensitive = FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Save"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Revert"), GLOBALS->exists_old);


	// define off ?
		sensitive = GLOBALS->define_off == 0 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Account"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Payee"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Category"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Budget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Preferences"), sensitive);

	// empty account list: disable Import, Archives, Edit, Filter, Add, Statistics, Overdrawn, Car Cost
		sensitive = da_acc_length() > 0 ? TRUE : FALSE;

		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Import"), sensitive);

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Close"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Archive"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/AddOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/ShowOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Statistics"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/BudgetR"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Overdrawn"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Carcost"), sensitive);

	// empty category list: disable Budget & Budget report
		sensitive = da_cat_length() > 1 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Budget"), sensitive);

	// empty archive list: disable Automated check
		sensitive = g_list_length(GLOBALS->arc_list) > 0 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Automated"), sensitive);

	// no active account: disable Edit, Over
		sensitive = (active == TRUE ) ? TRUE : FALSE;
		if(data->acc && data->acc->window != NULL)
			sensitive = FALSE;
		
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/ShowOpe"), sensitive);

	}

	/* update toolbar & list */
	if(flags & UF_VISUAL)
	{
		DB( printf(" +  8: visual\n") );
	
		if(PREFS->toolbar_style == 0)
			gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
		else
			gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);
	
		//gtk_toolbar_set_icon_size(GTK_TOOLBAR(data->TB_bar), GTK_ICON_SIZE_LARGE_TOOLBAR);

		//todo: update image size :: don't work
		/*
		for(i=0;i<MAX_ACTION_WALLET;i++)
		{
		GtkWidget *image, *newimage;
		GdkPixbuf *pixbuf, *newpixbuf;
		
			if( GTK_IS_TOOL_BUTTON(data->TB_buttons[i] ))
			{
				image = gtk_tool_button_get_icon_widget(data->TB_buttons[i]);
				
				
				g_print( 
				
				newpixbuf = gdk_pixbuf_new_from_file_at_size(toolbar_wallet[i].image, PREFS->image_size, PREFS->image_size, NULL);
				newimage = gtk_image_new_from_pixbuf(newpixbuf);
				gtk_tool_button_set_icon_widget(data->TB_buttons[i], image);
				
				gtk_widget_destroy(image);
			}	
		}
		*/

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

		homebank_pref_createformat();
		
		//minor ?
		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ViewMenu/AsMinor"), PREFS->euro_active);
		gtk_action_set_visible(gtk_ui_manager_get_action(data->ui, "/MenuBar/ViewMenu"), PREFS->euro_active);
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

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_printf(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

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

	//store position and size
	wg = &PREFS->wal_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	
	DB( g_printf(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );


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

/*
 *
 */
static GtkWidget *
wallet_construct_menu(struct wallet_data *data)
{
GError *error = NULL;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;

	actions = gtk_action_group_new ("Wallet");
	data->actions = actions;
	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (actions,
			entries,
			n_entries,
			NULL);

	/* set which action should have priority in the toolbar & also the short label (without ...) */
	action = gtk_action_group_get_action(actions, "Open");
	g_object_set(action, "is_important", TRUE, "short_label", _("Open"), NULL);
	
	action = gtk_action_group_get_action(actions, "Save");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Account");
	g_object_set(action, "short_label", _("Account"), NULL);

	action = gtk_action_group_get_action(actions, "Payee");
	g_object_set(action, "short_label", _("Payee"), NULL);

	action = gtk_action_group_get_action(actions, "Category");
	g_object_set(action, "short_label", _("Category"), NULL);

	action = gtk_action_group_get_action(actions, "Archive");
	g_object_set(action, "short_label", _("Archive"), NULL);

	action = gtk_action_group_get_action(actions, "Budget");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(actions, "ShowOpe");
	g_object_set(action, "short_label", _("Show"), NULL);

	action = gtk_action_group_get_action(actions, "AddOpe");
	g_object_set(action, "is_important", TRUE, "short_label", _("Add"), NULL);

	action = gtk_action_group_get_action(actions, "Statistics");
	g_object_set(action, "short_label", _("Statistics"), NULL);

	action = gtk_action_group_get_action(actions, "BudgetR");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(actions, "Overdrawn");
	g_object_set(action, "short_label", _("Overdrawn"), NULL);

	action = gtk_action_group_get_action(actions, "Carcost");
	g_object_set(action, "short_label", _("Carcost"), NULL);

	gtk_action_group_add_toggle_actions (actions,
			toggle_entries,
			n_toggle_entries,
			NULL);

	ui = gtk_ui_manager_new ();
	data->ui = ui;


	g_signal_connect(G_OBJECT(data->ui), "connect-proxy",
			 G_CALLBACK (wallet_ui_connect_proxy_cb), data);
	g_signal_connect(G_OBJECT(data->ui), "disconnect-proxy",
			 G_CALLBACK (wallet_ui_disconnect_proxy_cb), data);


	gtk_ui_manager_insert_action_group (data->ui, actions, 0);

	gtk_window_add_accel_group (GTK_WINDOW (data->window),
			gtk_ui_manager_get_accel_group(data->ui));


	if (!gtk_ui_manager_add_ui_from_string (data->ui, ui_info, -1, &error))
	{
		g_message ("Building menus failed: %s", error->message);
		g_error_free (error);
	}

	return gtk_ui_manager_get_widget (data->ui, "/MenuBar");
}

/*
 *
 */
/*
static GtkWidget *
wallet_create_acccount_view (struct wallet_data *data)
{
GtkWidget *account_view;




}
*/





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
	GtkWidget *menu_item;

	DB( g_printf("wallet_window_screen_changed_cb\n") );


	data->recent_manager = gtk_recent_manager_get_default ();

	gtk_menu_detach (GTK_MENU (data->recent_menu));
	g_object_unref (G_OBJECT (data->recent_menu));
	
	data->recent_menu = create_recent_chooser_menu (data->recent_manager);

	g_signal_connect (data->recent_menu,
			  "item-activated",
			  G_CALLBACK (wallet_recent_chooser_item_activated_cb),
			  data);
			  
	menu_item = gtk_ui_manager_get_widget (data->ui, "/MenuBar/FileMenu/OpenRecent");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), data->recent_menu);
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

/*
static void
wallet_recent_remove (struct wallet_data *data, const gchar *path)
{
	gchar *uri;
	GError *error = NULL;

	uri = g_filename_to_uri (path, NULL, &error);
	if (error)
	{	
		g_warning ("Could not convert uri \"%s\" to a local path: %s", uri, error->message);
		g_error_free (error);
		return;
	}
	
	gtk_recent_manager_remove_item (data->recent_manager, uri, &error);
	if (error)
	{
		g_warning ("Could not remove recent-files uri \"%s\": %s", uri, error->message);	
		g_error_free (error);
	}
	
	g_free (uri);
}
*/

static GtkWidget *
create_recent_chooser_menu (GtkRecentManager *manager)
{
	GtkWidget *recent_menu;
	GtkRecentFilter *filter;

	recent_menu = gtk_recent_chooser_menu_new_for_manager (manager);

	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), FALSE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu), GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER (recent_menu), 5);
	gtk_recent_chooser_menu_set_show_numbers (GTK_RECENT_CHOOSER_MENU (recent_menu), TRUE);

	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_application (filter, g_get_application_name());
	gtk_recent_chooser_set_filter (GTK_RECENT_CHOOSER (recent_menu), filter);

	return recent_menu;
}

/*
 *
 */
static GtkWidget *
wallet_construct_statusbar (struct wallet_data *data)
{
GtkWidget *statusbar;

	statusbar = gtk_statusbar_new ();
	data->statusbar = statusbar;
	data->statusbar_menu_context_id =
		gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "menu");
	data->statusbar_actions_context_id =
		gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "actions");

	return statusbar;
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
			DB( g_print("should open %s\n", path ) );
			g_free(GLOBALS->filename);
			GLOBALS->filename = g_strdup(path);
			wallet_open_internal(GTK_WIDGET(window), NULL);
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



/*
** the window creation
*/
GtkWidget *create_wallet_window(GtkWidget *do_widget)
{
struct wallet_data *data;
GtkWidget *vbox, *box;
GtkWidget *label;
GtkWidget *treeview, *sw;
GtkWidget *window, *widget;
GtkWidget *menubar;
GtkWidget *toolbar;
GtkWidget *statusbar;

	DB( g_print("(wallet) create window\n") );

	data = g_malloc0(sizeof(struct wallet_data));
	if(!data) return NULL;

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// this is our mainwindow, so store it to GLOBALS data
	data->window = window;
	GLOBALS->mainwindow = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(wallet) new window=%x, inst_data=%0x\n", (gint)window, (gint)data) );

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	//todo move this ?
	data->wintitle = NULL;

	menubar = wallet_construct_menu(data);
	toolbar = gtk_ui_manager_get_widget (data->ui, "/ToolBar");
	//account_view = wallet_create_acccount_view(data);

	statusbar = wallet_construct_statusbar (data);

	data->TB_bar = toolbar;	


	gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);



	//list
	box = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER(box), HB_MAINBOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), box, TRUE, TRUE, 0);

	/* accounts */
		label = make_label(NULL, 0.0, 0.0);
		gtk_label_set_markup (GTK_LABEL(label), _("<b>Accounts summary</b>"));
	    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, HB_BOX_SPACING);

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (box), sw, TRUE, TRUE, 0);
		//gtk_container_set_border_width (GTK_CONTAINER(sw), 5);

		// create tree view
		treeview = (GtkWidget *)create_list_account();
		data->LV_acc = treeview;
		gtk_container_add (GTK_CONTAINER (sw), treeview);

	/* bill book :: échéancier */
		label = make_label(NULL, 0.0, 0.0);
		gtk_label_set_markup (GTK_LABEL(label), _("<b>Upcoming automated transactions</b>"));
	    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, HB_BOX_SPACING);

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (box), sw, TRUE, TRUE, 0);

		treeview = (GtkWidget *)create_list_upcoming();
		data->LV_upc = treeview;
		gtk_container_add (GTK_CONTAINER (sw), treeview);


	//status bar
	gtk_box_pack_end (GTK_BOX (vbox), statusbar, FALSE, TRUE, 0);

	/* recent files */	
	data->recent_manager = gtk_recent_manager_get_default ();

	data->recent_menu = create_recent_chooser_menu (data->recent_manager);

	g_signal_connect (data->recent_menu,
			  "item-activated",
			  G_CALLBACK (wallet_recent_chooser_item_activated_cb),
			  data);
			  
	widget = gtk_ui_manager_get_widget (data->ui, "/MenuBar/FileMenu/OpenRecent");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), data->recent_menu);		  

	/* support for opening a file by dragging onto the project window */
	gtk_drag_dest_set (GTK_WIDGET (window),
			   GTK_DEST_DEFAULT_ALL,
			   drop_types, G_N_ELEMENTS (drop_types),
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect (G_OBJECT (window), "drag-data-received",
			  G_CALLBACK (drag_data_received), window);



	//connect all our signals

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), "changed", G_CALLBACK (wallet_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_acc), "row-activated", G_CALLBACK (wallet_onRowActivated), GINT_TO_POINTER(2));

	/* GtkWindow events */
    g_signal_connect (window, "delete-event", G_CALLBACK (wallet_dispose), (gpointer)data);


	g_signal_connect (window, "screen-changed",
			  G_CALLBACK (wallet_window_screen_changed_cb),
			  data);

  return window;
}


