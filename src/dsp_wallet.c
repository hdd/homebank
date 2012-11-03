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

#include "dsp_wallet.h"
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
#define AUTOLOAD 0
#define DODEFWIZARD 0
#define DOIMPORTWIZARD 1

/* our functions prototype */
static void wallet_action_new(void);
static void wallet_action_open(void);
static void wallet_action_save(void);
static void wallet_action_saveas(void);
static void wallet_action_defwallet(void);
static void wallet_action_close(void);
static void wallet_action_quit(void);

static void wallet_action_defaccount(void);
static void wallet_action_defpayee(void);
static void wallet_action_defcategory(void);
static void wallet_action_defarchive(void);
static void wallet_action_defbudget(void);
static void wallet_action_preferences(void);

static void wallet_action_showoperations(void);
static void wallet_action_addoperations(void);
static void wallet_action_checkautomated(void);

static void wallet_action_statistic(void);
static void wallet_action_budget(void);
static void wallet_action_overdrawn(void);
static void wallet_action_carcost(void);

static void wallet_action_importamiga(void);

#if DOIMPORTWIZARD == 1
static void wallet_action_import(void);
#endif

static void wallet_action_help(void);
static void wallet_about(void);


void wallet_open(GtkWidget *widget, gpointer user_data);
void wallet_save(GtkWidget *widget, gpointer user_data);
void wallet_populate_listview(GtkWidget *view);
void wallet_action(GtkWidget *widget, gpointer user_data);
void wallet_toggle_minor(GtkWidget *widget, gpointer user_data);
void wallet_clear(GtkWidget *widget, gpointer user_data);

void wallet_update(GtkWidget *widget, gpointer user_data);
void wallet_check_automated(GtkWidget *widget, gpointer user_data);
void wallet_addoperations(GtkWidget *widget, gpointer user_data);
void wallet_import_amiga(GtkWidget *widget, gpointer user_data);

struct wallet_data
{

	gchar	*wintitle;
	GtkWidget	*TB_bar;
	GtkWidget	*BT_help;
	GtkWidget	*GR_info;
	GtkWidget	*GR_minor;
	GtkWidget	*CM_minor;
	GtkWidget	*TX_balance[3];
	GtkWidget	*GO_gauge;

	GtkWidget	*LV_acc;

	gdouble	bank, today, future;

	//struct	Base base;

	Account *acc;
	gint	accnum;

	GtkUIManager	*ui;

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

extern gchar *CYA_UNIT[];


static GtkActionEntry entries[] = {

  /* name, stock id, label */

  { "FileMenu"     , NULL, N_("_File") },
  { "EditMenu"     , NULL, N_("_Edit") },
  { "OperationMenu", NULL, N_("_Operations") },
  { "ReportMenu"   , NULL, N_("_Reports")  },
  { "ToolMenu"     , NULL, N_("_Tools") },
  { "HelpMenu"     , NULL, N_("_Help") },

	/* name, stock id, label, accelerator, tooltip */

  { "New"        , GTK_STOCK_NEW        , N_("_New")          , NULL,    N_("Clear all"),    G_CALLBACK (wallet_action_new) },
  { "Open"       , GTK_STOCK_OPEN       , N_("_Open...")      , NULL,    N_("Open a wallet"),    G_CALLBACK (wallet_action_open) },
  { "Save"       , GTK_STOCK_SAVE       , N_("_Save")         , NULL,    N_("Save this wallet"),    G_CALLBACK (wallet_action_save) },
  { "SaveAs"     , GTK_STOCK_SAVE_AS    , N_("Save As...")    , NULL,    NULL,    G_CALLBACK (wallet_action_saveas) },
  { "Properties" , GTK_STOCK_PROPERTIES , N_("_Properties..."), NULL,    NULL,    G_CALLBACK (wallet_action_defwallet) },

#if DOIMPORTWIZARD == 1
  { "Import", NULL,    N_("Import..."), NULL,    N_("tip"),    G_CALLBACK (wallet_action_import) },
#endif

  { "Close"      , GTK_STOCK_CLOSE      , N_("_Close")        , NULL,    NULL,    G_CALLBACK (wallet_action_close) },
  { "Quit"       , GTK_STOCK_QUIT       , N_("_Quit")         , NULL,    NULL,    G_CALLBACK (wallet_action_quit) },

  { "Account"    , "hb-stock-account"   , N_("Acc_ounts...")  , NULL,    N_("Edit the accounts"), G_CALLBACK (wallet_action_defaccount) },
  { "Payee"      , "hb-stock-payee"     , N_("_Payees...")    , NULL,    N_("Edit the payees"),    G_CALLBACK (wallet_action_defpayee) },
  { "Category"   , "hb-stock-category"  , N_("Categories...") , NULL,    N_("Edit the categories"),    G_CALLBACK (wallet_action_defcategory) },
  { "Archive"    , "hb-stock-archive"   , N_("Arc_hives...")  , NULL,    N_("Edit the archives"),    G_CALLBACK (wallet_action_defarchive) },
  { "Budget"     , "hb-stock-budget"    , N_("Budget...")     , NULL,    N_("Edit the budget"),    G_CALLBACK (wallet_action_defbudget) },
  { "Preferences", GTK_STOCK_PREFERENCES, N_("Preferences..."), NULL,    NULL,    G_CALLBACK (wallet_action_preferences) },

  { "ShowOpe"    , "hb-stock-ope-show"  , N_("Show...")           , NULL,    N_("Show operations"),    G_CALLBACK (wallet_action_showoperations) },
  { "AddOpe"     , "hb-stock-ope-add"   , N_("Add...")            , NULL,    N_("Add operation(s)"),    G_CALLBACK (wallet_action_addoperations) },
  { "Automated"  , NULL                 , N_("Check automated..."), NULL,    N_("tip"),    G_CALLBACK (wallet_action_checkautomated) },

  { "Statistics" , "hb-stock-rep-stats" , N_("_Statistics..."), NULL,    N_("Statistics"),    G_CALLBACK (wallet_action_statistic) },
  { "BudgetR"    , "hb-stock-rep-budget", N_("B_udget...")    , NULL,    N_("Budget"),    G_CALLBACK (wallet_action_budget) },
  { "Overdrawn"  , "hb-stock-rep-over"  , N_("Ove_rdrawn...") , NULL,    N_("Overdrawn"),    G_CALLBACK (wallet_action_overdrawn) },
  { "Carcost"    , "hb-stock-rep-car"   , N_("_Carcost...")   , NULL,    N_("Car cost"),    G_CALLBACK (wallet_action_carcost) },

  { "ImportAmiga", NULL,    N_("Import Amiga..."), NULL,    N_("Import an Amiga 3.0 file"),    G_CALLBACK (wallet_action_importamiga) },

  { "Contents"   , GTK_STOCK_HELP       , N_("_Contents...")  , "F1",    NULL,    G_CALLBACK (wallet_action_help) },
  { "About"      , GTK_STOCK_ABOUT      , N_("_About...")     , NULL,    NULL,    G_CALLBACK (wallet_about) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <separator/>"
"      <menuitem action='Open'/>"
"      <separator/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <separator/>"

#if DOIMPORTWIZARD == 1
"      <menuitem action='Import'/>"
"      <separator/>"
#endif

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

"    <menu action='OperationMenu'>"
"      <menuitem action='ShowOpe'/>"
"      <separator/>"
"      <menuitem action='AddOpe'/>"
"      <menuitem action='Automated'/>"
"    </menu>"

"    <menu action='ReportMenu'>"
"      <menuitem action='Statistics'/>"
"      <menuitem action='BudgetR'/>"
"      <menuitem action='Overdrawn'/>"
"      <separator/>"
"      <menuitem action='Carcost'/>"
"    </menu>"

"    <menu action='ToolMenu'>"
"      <menuitem action='ImportAmiga'/>"
"    </menu>"

"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
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
"    <toolitem action='Archive'/>"
"    <toolitem action='Budget'/>"

"      <separator/>"

"    <toolitem action='ShowOpe'/>"
"    <toolitem action='AddOpe'/>"

"      <separator/>"

"    <toolitem action='Statistics'/>"
"    <toolitem action='BudgetR'/>"
"    <toolitem action='Overdrawn'/>"
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
		wallet_clear(widget, GINT_TO_POINTER(TRUE));
		wallet_update(widget, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);

		// run the assistant
		#if DODEFWIZARD == 1
			if(PREFS->runwizard == TRUE)
				create_defwizard_window();
		#else
			//add an empty account
			/*
			Account *item;
			item = da_account_malloc();
			item->name = g_strdup_printf( _("(account %d)") , 0);
			GLOBALS->acc_list = g_list_append(GLOBALS->acc_list, item);
			*/
			wallet_action_defaccount();
		#endif
		
	}

}

static void wallet_action_open(void)
{
	wallet_open(GLOBALS->mainwindow, NULL);
}

static void wallet_action_save(void)
{
	wallet_save(GLOBALS->mainwindow, (gpointer)FALSE);
}

static void wallet_action_saveas(void)
{
	wallet_save(GLOBALS->mainwindow, (gpointer)TRUE);
}

static void wallet_action_close(void)
{
GtkWidget *widget = GLOBALS->mainwindow;

	if( wallet_check_change(widget,NULL) == TRUE )
	{
		//clear all, and init GLOBALS->filename to default
		wallet_clear(widget, GINT_TO_POINTER(TRUE));
		wallet_update(widget, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
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
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);
}

static void wallet_action_defaccount(void)
{
struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	create_editaccount_window();

	//our global acc_listchanged, so update the treeview
	wallet_populate_listview(data->LV_acc);

	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
}

static void wallet_action_defpayee(void)
{
	create_defpayee_window();
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);
}

static void wallet_action_defcategory(void)
{
	create_defcategory_window();
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);
}

static void wallet_action_defarchive(void)
{
	create_defarchive_window();
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);
}


static void wallet_action_defbudget(void)
{
	create_defbudget_window();
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);
}

static void wallet_action_preferences(void)
{
	defpref_dialog_new();
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_VISUAL);
}


static void wallet_action_showoperations(void)
{
struct wallet_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	create_account_window(data->accnum, data->acc);
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

#if DOIMPORTWIZARD == 1
static void wallet_action_import(void)
{
	create_wizimport_window();
}
#endif

static void wallet_action_help(void)
{
gint res;
gchar *path;

	DB( g_print("help\n") );

//todo; when local doc
//	path = g_strdup_printf("%s %s/en/index.html", PREFS->path_navigator, HELP_DIR);

	path = g_strdup_printf("%s http://homebank.free.fr/help/ &", PREFS->path_navigator);


	DB( g_print("%s\n", path) );

	res = system(path);
	if(res == -1)
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Web navigator problem"),
			_("Please check the preferences.")
			);

	g_free(path);
}

static void
activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data)
{
gint res;
gchar *path;

  DB( g_print ("show url %s\n", link) );

  path = g_strdup_printf("%s %s &", PREFS->path_navigator, link);

	res = system(path);
	if(res == -1)
		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Web navigator problem"),
			_("Please check the preferences.")
			);

  g_free(path);
}

static void wallet_about(void)
{
GdkPixbuf *logo;
GtkWindow *about;

	logo = gdk_pixbuf_new_from_file_at_size ( PIXMAPS_DIR "/homebank.svg", 128, 128, NULL);

  const gchar *artists[] = {
    "Maxime DOYEN",
    "Nathan M. Willard (some icons)",
    NULL
  };

  const gchar *authors[] = {
    "Lead developer:\n" \
    "Maxime DOYEN",
    "\nContributor:\n" \
    "Gaetan LORIDANT (Maths formulas for charts)\n",
    NULL
  };

  const gchar *documenters[] = {
    "Maxime DOYEN",
    NULL
  };

  const gchar *copyright = "Copyright © 1995-2006 Maxime DOYEN";
  const gchar *comments = N_("Free easy personal accounting for all !");
  const gchar *website = "http://homebank.free.fr";


	gtk_about_dialog_set_url_hook (activate_url, NULL, NULL);
 	gtk_show_about_dialog(
 		GTK_WINDOW(GLOBALS->mainwindow),
		"artists"	, artists,
		"authors"	, authors,
		"comments"	, _(comments),
		"copyright"	, copyright,
	//	"documenters", documenters,
		"license"	,	"This program is released under the GNUGeneral Public License.\n" \
						"Please visit http://www.gnu.org/copyleft/gpl.html for details.",
		"logo"		, logo,
		"name"		, PROGNAME,
	//	"translator-credits"	, "trans",

		"version"	, PROGVERSION,
		"website"	, website,
        NULL);

    g_object_unref (logo);

}


/* wallet functions -------------------- */

/*
**
*/
void wallet_populate_listview(GtkWidget *view)
{
GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
GtkTreeIter  iter;
GList *list;
Account *acc;

	DB( g_printf("(wallet) populate\n") );


	gtk_list_store_clear (GTK_LIST_STORE(model));

	//insert all glist item into treeview
	list = g_list_first(GLOBALS->acc_list);
	while (list != NULL)
	{
		acc = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);

     	 DB( g_printf(" populate: %s\n", acc->name) );

     	 gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  LST_DSPACC_DATAS, acc,
			  -1);

		list = g_list_next(list);
	}

}

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
void wallet_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;

	DB( g_printf("(wallet) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	wallet_update(data->LV_acc, (gpointer)UF_BALANCE);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
}

/*
**
*/
void wallet_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	wallet_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), (gpointer)UF_SENSITIVE);
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
		GLOBALS->filename = g_strdup_printf("%s/untitled.xhb", PREFS->path_wallet);
		GLOBALS->wallet_is_new = TRUE;
	}
	else
	{
		GLOBALS->wallet_is_new = FALSE;
	}

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
	//data->change = 0;
	//data->accnum = 0;

	// clear GList
	da_account_destroy(GLOBALS->acc_list);
	GLOBALS->acc_list = NULL;

	da_payee_destroy(GLOBALS->pay_list);
	GLOBALS->pay_list = NULL;
	GLOBALS->pay_list = g_list_append(GLOBALS->pay_list, da_payee_malloc());

	da_category_destroy(GLOBALS->cat_list);
	GLOBALS->cat_list = NULL;
	GLOBALS->cat_list = g_list_append(GLOBALS->cat_list, da_category_malloc());

	da_archive_destroy(GLOBALS->arc_list);
	GLOBALS->arc_list = NULL;

	da_operation_destroy(GLOBALS->ope_list);
	GLOBALS->ope_list = NULL;


	//clear TreeView
	gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))));


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
	account = data->accnum;

	window = create_defoperation_window(NULL, OPERATION_EDIT_ADD, data->accnum);
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

		if(result != GTK_RESPONSE_REJECT)
		{
			defoperation_get(window, NULL);
			operation_add(ope, NULL, ope->account);


			DB( g_printf(" -> added 1 operation to %d\n", ope->account) );

			count++;
		}

		da_operation_free(ope);
		ope = NULL;

		DB( g_printf(" -> result %d\n", result) );
	}


	defoperation_dispose(window, NULL);
	gtk_widget_destroy (window);

	/* todo optimize this */
	if(count > 0)
	{
		GLOBALS->change += count;
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
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

	count = 0;
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *arc = list->data;

		//debug
		//arc->arc_Flags |= OF_AUTO;
		//arc->arc_Every = 1;
		//arc->arc_Unit = 2;

		if(arc->flags & OF_AUTO)
		{
		guint32 maxdate;

			#ifdef MYDEBUG
			gchar buffer1[128]; GDate *date;
			date = g_date_new_julian(arc->nextdate);
			g_date_strftime (buffer1, 128-1, "%x", date);
			g_date_free(date);
			g_print("  -> '%s' - every %d %s - next %s limit %d\n", arc->wording, arc->every, CYA_UNIT[arc->unit], buffer1, arc->limit);
			#endif


			maxdate = GLOBALS->today + GLOBALS->auto_nbdays;
			if(arc->nextdate <= maxdate)
			{
			guint32 mydate = arc->nextdate;

				while(mydate <= maxdate)
				{
				Operation ope;
				Account *acc;

				#if MYDEBUG == 1
					gchar buffer1[128]; GDate *date;
					date = g_date_new_julian(mydate);
					g_date_strftime (buffer1, 128-1, "%x", date);
					g_date_free(date);
					g_printf("  -> adding '%s' on %s\n", arc->wording, buffer1);
				#endif

					/* fill in the operation */
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
					count++;

					/* todo: update acc flags */
					acc = g_list_nth_data(GLOBALS->acc_list, arc->account);
					acc->flags |= AF_ADDED;
					if(arc->paymode == PAYMODE_PERSTRANSFERT)
					{
						acc = g_list_nth_data(GLOBALS->acc_list, arc->dst_account);
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

	//inform the user
	if(usermode == TRUE)
	{
	gchar *txt;

		if(count == 0)
			txt = _("No operation to insert");
		else
			txt = _("%d operations inserted");

		homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Check automated operations result"),
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

			wallet_populate_listview(data->LV_acc);
			wallet_check_automated(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
			wallet_compute_balances(GLOBALS->mainwindow, NULL);
			wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
		}
	}
}

/*
** open a file chooser dialog and store filename to GLOBALS if OK
*/
gboolean homebank_file_chooser(GtkFileChooserAction action)
{
GtkWidget *chooser;
GtkFileFilter *filter;
gchar *title;
gchar *button;
gboolean retval;

	DB( g_printf("(wallet) file chooser %d\n", action) );

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
		title = "Open a wallet";
		button = GTK_STOCK_OPEN;
	}
	else
	{
		title = "Save wallet as";
		button = GTK_STOCK_SAVE;
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "HB Wallet");
	gtk_file_filter_add_pattern (filter, "*.xhb");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "All");
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
	    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);
	}
	else
	{
		if(GLOBALS->wallet_is_new == TRUE)
		{
		    /* the user just created a new document */
		    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);
		    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), "untitled.xhb");
		}
		else
		{
		    /* the user edited an existing document */
		    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), GLOBALS->filename);
		}
	}

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		DB( g_printf("- filename: %s\n", filename) );

		g_free(GLOBALS->filename);
		GLOBALS->filename = filename;

		DB( g_printf("- filename: %s\n", GLOBALS->filename) );


		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
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
			//clear all, but not the GLOBALS->filename
			wallet_clear(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));

			GLOBALS->wallet_is_new = FALSE;

			DB( g_printf("- filename: %s\n", GLOBALS->filename) );

			homebank_load_xml(GLOBALS->filename);

			wallet_populate_listview(data->LV_acc);
			wallet_check_automated(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
			wallet_compute_balances(GLOBALS->mainwindow, NULL);
			wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
		}
	}
}

/*
**
*/
void wallet_save(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
gboolean saveas = (gboolean)user_data;

	DB( g_printf("(wallet) save\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(saveas == 1)
	{
		if(homebank_file_chooser(GTK_FILE_CHOOSER_ACTION_SAVE) == TRUE)
		{
			DB( g_printf(" + should save as %s\n", GLOBALS->filename) );
			homebank_save_xml(GLOBALS->filename);
		}
	}
	else
	{
		DB( g_printf(" + should quick save %s\n", GLOBALS->filename) );
		homebank_save_xml(GLOBALS->filename);
	}

	GLOBALS->change = 0;
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE);

}

typedef struct
{
	gdouble bank, today, future;
} tmp_balances;

void wallet_compute_balances(GtkWidget *widget, gpointer user_data)
{
struct wallet_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
tmp_balances *array;
gboolean valid;
gint length, i;

	DB( g_printf("(wallet) compute balances\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	/* allocate tmp_balances memory */
	length = g_list_length(GLOBALS->acc_list);
	array = g_malloc0(length * sizeof(tmp_balances));

	//debug
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));

    g_return_if_fail ( model != NULL );


	if(array)
	{
		data->bank = 0;
		data->today = 0;
		data->future = 0;

		GList *list;

		/* parse all operation */
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


			/* balances */
				array[i].future += ope->amount;

				if(ope->date <= GLOBALS->today)
					array[i].today += ope->amount;

				if(ope->flags & OF_VALID)
					array[i].bank += ope->amount;

			/* account dates */
			/*	DoMethod(data->LV_acc, MUIM_List_GetEntry, ope->ope_Account, &al);
				if(ope->ope_Date < al->al_MinDate) al->al_MinDate = ope->ope_Date;
				if(ope->ope_Date > al->al_MaxDate) al->al_MaxDate = ope->ope_Date;
			*/

			/* wallet dates */
			/*
				if(ope->ope_Date < data->mindate) data->mindate = ope->ope_Date;
				if(ope->ope_Date > data->maxdate) data->maxdate = ope->ope_Date;
			*/


			}
			list = g_list_next(list);
		}

		/* set initial amount for each account */
	    i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	    while (valid)
	    {
		Account *acc;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPACC_DATAS, &acc, -1);

			array[i].bank += acc->initial;
			array[i].today += acc->initial;
			array[i].future += acc->initial;

			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				LST_DSPACC_BANK, array[i].bank,
				LST_DSPACC_TODAY, array[i].today,
				LST_DSPACC_FUTURE, array[i].future,
				-1);

			data->bank += array[i].bank;
			data->today += array[i].today;
			data->future += array[i].future;

	       /* Make iter point to the next row in the list store */
	       i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
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

	flags = (gint)user_data;

	/* set window title */
	if(flags & UF_TITLE)
	{
	gchar *basename;

		DB( printf(" +  1: wintitle %x\n", (gint)data->wintitle) );

		basename = g_path_get_basename(GLOBALS->filename);

		g_free(data->wintitle);
		if(!(GLOBALS->change))
			data->wintitle = g_strdup_printf(PROGNAME " - %s (%s)", GLOBALS->title, basename);
		else
			data->wintitle = g_strdup_printf(PROGNAME " - %s (*%s)", GLOBALS->title, basename);

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
		gint *indices;

			path = gtk_tree_model_get_path(model, &iter);
			indices = gtk_tree_path_get_indices(path);

			data->accnum = indices[0];

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPACC_DATAS, &acc, -1);
			data->acc = acc;

			DB( printf(" active is %d\n", indices[0]) );

		}

		// no change: disable save
		sensitive = (GLOBALS->change != 0 ) ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/SaveAs"), sensitive);
		if(sensitive == TRUE && GLOBALS->wallet_is_new == TRUE) sensitive = FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Save"), sensitive);

	// define off ?
		sensitive = GLOBALS->define_off == 0 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Account"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Payee"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Category"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Budget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Preferences"), sensitive);

	// empty account list: disable Import, Archives, Edit, Filter, Add, Statistics, Overdrawn, Car Cost
		sensitive = g_list_length(GLOBALS->acc_list) > 0 ? TRUE : FALSE;

#if DOIMPORTWIZARD == 1
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Import"), sensitive);
#endif
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/FileMenu/Close"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Archive"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/AddOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/ShowOpe"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Statistics"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/BudgetR"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Overdrawn"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/ReportMenu/Carcost"), sensitive);

	// empty category list: disable Budget & Budget report
		sensitive = g_list_length(GLOBALS->cat_list) > 1 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/EditMenu/Budget"), sensitive);

	// empty archive list: disable Automated check
		sensitive = g_list_length(GLOBALS->arc_list) > 1 ? TRUE : FALSE;

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/Automated"), sensitive);

	// no active account: disable Edit, Over
		sensitive = (active == TRUE ) ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/OperationMenu/ShowOpe"), sensitive);

	}

	/* update toolbar & list */
	if(flags & UF_VISUAL)
	{
	guint i;
	
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
		if( PREFS->euro_active )
			gtk_widget_show(data->CM_minor);
		else
			gtk_widget_hide(data->CM_minor);
		
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{
	gboolean minor;

		DB( printf(" +  4: balances\n") );

		minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->bank, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->today, minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->future, minor);
	}



}


void
  wallet_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
  {
    GtkTreeModel *model;
    GtkTreeIter   iter;

    DB( g_print ("A row has been double-clicked!\n") );

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
		Account *acc;

       gtk_tree_model_get(model, &iter, LST_DSPACC_DATAS, &acc, -1);

       DB( g_print ("Double-clicked row contains name %s\n", acc->name) );

		wallet_action_showoperations();

       //g_free(name);
    }
  }




/*
**
*/
gboolean wallet_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct wallet_data *data = user_data;
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
	//delete-event TRUE abort/FALSE destroy
	return retval;
}



// the window creation
GtkWidget *create_wallet_window(GtkWidget *do_widget)
{
struct wallet_data *data;
GtkWidget *vbox,*hbox;
GtkWidget *label, *entry;
GtkWidget *sw;
GtkWidget *treeview;
GtkWidget *toolbar1, *statusbar;
GtkWidget *window, *check_button, *vbar;
GtkUIManager *ui;
GtkActionGroup *actions;
GError *error = NULL;

	data = g_malloc0(sizeof(struct wallet_data));
	if(!data) return NULL;

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //gtk_window_set_title (GTK_WINDOW (window), "HomeBank v0.1");

	// this is our mainwindow, so store it to GLOABLS data
	GLOBALS->mainwindow = window;



	//gtk_window_set_icon_from_file(GTK_WINDOW (WI_wallet), "./pixmaps/.png", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_printf("(wallet) new window=%x, inst_data=%0x\n", (gint)window, (gint)data) );

	//set the window icon
	//gtk_window_set_icon_from_file(GTK_WINDOW (window), PIXMAPS_DIR "/homebank-icon.png", NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	data->wintitle = NULL;

	//store our base to globals
	//GLOBALS->base = &data->base;
	//DB( g_printf("(dspwallet) base %x wintitle=%x\n", GLOBALS->base, data->wintitle) );



#if UI == 1
	//start test uimanager

		actions = gtk_action_group_new ("Wallet");

      	//as we use gettext
      	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

    	DB( g_print("\033[34mdomain set to '%s'\033[0m\n", GETTEXT_PACKAGE) );

		DB( g_print("add actions: %x\n", (gint)actions) );
		gtk_action_group_add_actions (actions, entries, n_entries, NULL);

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
	  error = NULL;
	}

	data->ui = ui;
      gtk_box_pack_start (GTK_BOX (vbox),
			  gtk_ui_manager_get_widget (ui, "/MenuBar"),
			  FALSE, FALSE, 0);

	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);


#endif
  	//end test uimanager

	/*
	toolbar1 = create_wallet_toolbar(data);
    gtk_box_pack_start (GTK_BOX (vbox), toolbar1, FALSE, FALSE, 0);
	data->TB_bar = toolbar1;
	*/

	//total
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);

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
      gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	//gtk_container_set_border_width (GTK_CONTAINER(sw), 5);

	// create tree view
	treeview = (GtkWidget *)create_list_account();
	data->LV_acc = treeview;
	gtk_container_add (GTK_CONTAINER (sw), treeview);

	//status bar
	statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))), "minor", (gpointer)data->CM_minor);

	//connect all our signals
    g_signal_connect (window, "delete-event", G_CALLBACK (wallet_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (wallet_toggle_minor), NULL);

	//g_signal_connect (GTK_TREE_VIEW(treeview), "cursor-changed", G_CALLBACK (wallet_update), (gpointer)2);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (wallet_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(treeview), "row-activated", G_CALLBACK (wallet_onRowActivated), (gpointer)2);


	//setup, init and show window
	gtk_window_resize(GTK_WINDOW(window), 680, 250);

    gtk_widget_show_all (window);

	//clear all, and init GLOBALS->filename to default name
	wallet_clear(treeview, GINT_TO_POINTER(TRUE));

	if( GLOBALS->lastfilename != NULL )
	{
		GLOBALS->wallet_is_new = FALSE;
		homebank_load_xml(GLOBALS->lastfilename);
		g_free(GLOBALS->filename);
		GLOBALS->filename = g_strdup(GLOBALS->lastfilename);
	}

	//todo debug
#if AUTOLOAD == 1
	//import_from_amiga("./wallets/amiga/Example_v3.hb");
	//import_from_amiga("./wallets/Maxime.hb");
	//homebank_load_xml("/home/max/dev/homebank/data/example.xhb");

	// run the assistant
	#if DODEFWIZARD == 1
	create_defwizard_window();
	#endif

#endif

	wallet_populate_listview(data->LV_acc);
	//wallet_check_automated(GLOBALS->mainwindow, NULL);
	wallet_compute_balances(GLOBALS->mainwindow, NULL);
	wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_VISUAL);


  return window;
}

