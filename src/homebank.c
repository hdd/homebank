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
#include "def_pref.h"

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#define APPLICATION_NAME (_("HomeBank"))

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
struct HomeBank *GLOBALS;
struct Preferences *PREFS;


/* installation paths */
static gchar *pixmaps_dir  = NULL;
static gchar *locale_dir   = NULL;
static gchar *help_dir     = NULL;


static struct { 
  gchar *filename;
  gchar *stock_id;
} stock_icons[] = {
  { "homebank.svg"            , "hb-stock-homebank" },

  { "account.svg"             , "hb-stock-account" },
  { "payee.svg"               , "hb-stock-payee" },
  { "category.svg"            , "hb-stock-category" },
  { "archive.svg"             , "hb-stock-archive" },
  { "budget.svg"              , "hb-stock-budget" },

  { "filter.svg"              , "hb-stock-filter" },

  { "ope_show.svg"            , "hb-stock-ope-show" },
  { "ope_add.svg"             , "hb-stock-ope-add" },
  { "ope_herit.svg"           , "hb-stock-ope-herit" },
  { "ope_edit.svg"            , "hb-stock-ope-edit" },
  { "ope_delete.svg"          , "hb-stock-ope-delete" },
  { "ope_valid.svg"           , "hb-stock-ope-valid" },

  { "report_stats.svg"        , "hb-stock-rep-stats" },
  { "report_budget.svg"       , "hb-stock-rep-budget" },
  { "report_overdrawn.svg"    , "hb-stock-rep-over" },
  { "report_car.svg"          , "hb-stock-rep-car" },

  { "view_list.svg"           , "hb-stock-view-list" },
  { "view_bar.svg"            , "hb-stock-view-bar" },
  { "view_pie.svg"            , "hb-stock-view-pie" },
  { "view_line.svg"           , "hb-stock-view-line" },

  { "legend.svg"             , "hb-stock-legend" },
  { "rate.svg"               , "hb-stock-rate" },
  { "refresh.svg"            , "hb-stock-refresh" },

  { "lpi-help.png"           , "hb-lpi-help" },
  { "lpi-translate.svg"      , "hb-lpi-translate" },
  { "lpi-bug.svg"            , "hb-lpi-bug" },

};
 
static gint n_stock_icons = G_N_ELEMENTS (stock_icons);

#define MARKUP_STRING "<span size='small'>%s</span>"


/* Application arguments */
static gboolean arg_version = FALSE;
static gchar **files = NULL;

static GOptionEntry option_entries[] = 
{
	{ "version", '\0', 0, G_OPTION_ARG_NONE, &arg_version,
	  N_("Output version information and exit"), NULL },
	  
	{ G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &files, 
	  NULL, N_("[FILE]") },
	  
	{ NULL }
};


/* = = = = = = = = = = = = = = = = = = = = */
/* Message dialog */

gint homebank_question_dialog(GtkWindow *parent, gchar *title, gchar *message_format, ...)
{
GtkWidget *dialog;
gchar* msg = NULL;
va_list args;
gint result;

	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  GTK_MESSAGE_QUESTION,
	                                  GTK_BUTTONS_YES_NO,
	                                  "%s", 
	                                  title
	                                  );

  if (message_format)
    {
      va_start (args, message_format);
      msg = g_strdup_vprintf (message_format, args);
      va_end (args);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);

      g_free (msg);
    }

	result = gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_destroy (dialog);

	return result;
}

/*
** open a info/error dialog for user information purpose
*/
void homebank_message_dialog(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...)
{
GtkWidget *dialog;
gchar* msg = NULL;
va_list args;


	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                  GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  type,
	                                  GTK_BUTTONS_CLOSE,
	                                  "%s", 
	                                  title
	                                  );

  if (message_format)
    {
      va_start (args, message_format);
      msg = g_strdup_vprintf (message_format, args);
      va_end (args);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);

      g_free (msg);
    }

	 gtk_dialog_run (GTK_DIALOG (dialog));
	 gtk_widget_destroy (dialog);
}

/*
** 
*/
gboolean homebank_chooser_open_qif(GtkWindow *parent, gchar **storage_ptr)
{
GtkWidget *chooser;
GtkFileFilter *filter;
gboolean retval;

	DB( g_printf("(homebank) chooser open qif\n") );

	chooser = gtk_file_chooser_dialog_new (
					_("Export as QIF"),
					GTK_WINDOW(GLOBALS->mainwindow),
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);

	//todo chnage this ?	
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_export);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("QIF files"));
	gtk_file_filter_add_pattern (filter, "*.qif");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		*storage_ptr = filename;

		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}



/*
** open a file chooser dialog and store filename to GLOBALS if OK
*/
gboolean homebank_csv_file_chooser(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr)
{
GtkWidget *chooser;
GtkFileFilter *filter;
gchar *title;
gchar *button;
gboolean retval;
gchar *path;

	DB( g_printf("(hombank) csvfile chooser %d\n", action) );

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
		title = _("Import from CSV");
		button = GTK_STOCK_OPEN;
		path = PREFS->path_import;
	}
	else
	{
		title = _("Export as CSV");
		button = GTK_STOCK_SAVE;
		path = PREFS->path_export;
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(parent),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), path);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.csv");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		DB( g_printf("- filename: %s\n", filename) );

		//store filename
		//g_free(*storage_ptr);
		*storage_ptr = filename;

		//DB( g_printf("- filename: %s\n", GLOBALS->filename) );


		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
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
		title = _("Open homebank file");
		button = GTK_STOCK_OPEN;
	}
	else
	{
		title = _("Save homebank file as");
		button = GTK_STOCK_SAVE;
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("HomeBank files"));
	gtk_file_filter_add_pattern (filter, "*.xhb");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
	    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);
	}
	else
	{
		/* save */
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
gboolean homebank_folder_chooser(GtkWindow *parent, gchar *title, gchar **storage_ptr)
{
GtkWidget *chooser;
gboolean retval;

	DB( g_printf("(wallet) folder chooser\n") );

	chooser = gtk_file_chooser_dialog_new (title,
					parent,
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	DB( g_printf(" - set folder %s\n", *storage_ptr) );

	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), *storage_ptr);

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

		//nb: filename must be freed with g_free
	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		DB( g_printf("- folder %s\n", filename) );

		g_free(*storage_ptr);
		*storage_ptr = filename;

		DB( g_printf("- folder stored: %s\n", *storage_ptr) );


		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}




/*
** open an alien file chooser dialog and store filename to GLOBALS if OK
*/
gboolean homebank_alienfile_chooser(gchar *title)
{
GtkWidget *chooser;
GtkFileFilter *filter;
gboolean retval;

	DB( g_printf("(homebank) alien file chooser\n") );

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_import);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Amiga files"));
	gtk_file_filter_add_pattern (filter, "*.hb");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

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

/* = = = = = = = = = = = = = = = = = = = = */
/* file backup */
gchar *homebank_get_filename_with_extension(gchar *path, gchar *extension)
{
gchar *basename;
gchar *dirname;
gchar *filename;
gchar *newname;
gchar **str_array;

	basename = g_path_get_basename(path);
	dirname  = g_path_get_dirname (path);

	str_array = g_strsplit(basename, ".", 0);

	filename = g_strdup_printf("%s.%s", str_array[0], extension);	

	newname = g_build_filename(dirname, filename, NULL);

	g_strfreev(str_array);
	g_free(basename);
	g_free(dirname);
	g_free(filename);

	return newname;
}

gchar *homebank_get_filename_without_extension(gchar *path)
{
gchar *basename;
gchar *newname;
gchar **str_array;

	basename = g_path_get_basename(path);

	str_array = g_strsplit(basename, ".", 0);

	newname = g_strdup(str_array[0]);	

	g_strfreev(str_array);
	g_free(basename);

	return newname;
}



/*
** ensure the filename ends with '.xhb'
*/
void homebank_file_ensure_xhb(void)
{
gchar *basename;
gchar *dirname;

	basename = g_path_get_basename(GLOBALS->filename);
	dirname  = g_path_get_dirname (GLOBALS->filename);

	DB( g_printf("() ensure .xhb for %s\n", basename) );

	if( !(g_str_has_suffix(basename, ".xhb")))
	{
	gchar **str_array;
	gchar *filename;
	gchar *newname;

		str_array = g_strsplit(basename, ".", 0);
		filename = g_strdup_printf("%s.xhb", str_array[0]);	
		g_strfreev(str_array);
		newname = g_build_filename(dirname, filename, NULL);
		g_free(filename);

		g_free(GLOBALS->filename);
		GLOBALS->filename = newname;
	}

	DB( g_printf("() out: %s\n", GLOBALS->filename) );

	g_free(basename);
	g_free(dirname);
}




void homebank_backup_current_file(gchar *pathname)
{
gchar *basename;
gchar *dirname;
gchar *filename;
gchar *newname;
gchar **str_array;
int retval;

	basename = g_path_get_basename(pathname);
	dirname  = g_path_get_dirname (pathname);

	if( g_str_has_suffix(basename, ".xhb") )
	{
		str_array = g_strsplit(basename, ".", 0);
		filename = g_strdup_printf("%s.xhb~", str_array[0]);	
		newname = g_build_filename(dirname, filename, NULL);
		g_free(filename);

		if( g_file_test(newname, G_FILE_TEST_EXISTS) )
		{
			DB( g_print("remove existing: %s\n", newname) );
			g_remove(newname);
		}

		DB( g_print("rename %s => %s\n", pathname, newname) );

		retval = g_rename(pathname, newname);
		
		DB( g_print("retval %d\n", retval) );
		
		g_strfreev(str_array);
		g_free(newname);
	}
	g_free(basename);
	g_free(dirname);
}

/* = = = = = = = = = = = = = = = = = = = = */
/* url open */


#ifdef G_OS_WIN32
#define SW_NORMAL 1

static gboolean
homebank_util_url_show_win32 (const gchar *url)
{
gint retval;

	/* win32 API call */
	retval = ShellExecuteA (NULL, "open", url, NULL, NULL, SW_NORMAL);
	
	if (retval <= 32)
		return FALSE;

	return TRUE;
}

#else

/* pilfered from Beast - birnetutils.cc */
static gboolean
homebank_util_url_show_unix (const gchar *url)
{

	static struct {
	const gchar   *prg, *arg1, *prefix, *postfix;
	gboolean       asyncronous; /* start asyncronously and check exit code to catch launch errors */
	volatile gboolean disabled;
	} browsers[] = {

	/* configurable, working browser launchers */
	{ "gnome-open",             NULL,           "", "", 0 }, /* opens in background, correct exit_code */
	{ "exo-open",               NULL,           "", "", 0 }, /* opens in background, correct exit_code */

	/* non-configurable working browser launchers */
	{ "kfmclient",              "openURL",      "", "", 0 }, /* opens in background, correct exit_code */
	{ "gnome-moz-remote",       "--newwin",     "", "", 0 }, /* opens in background, correct exit_code */

#if 0   /* broken/unpredictable browser launchers */
	{ "browser-config",         NULL,            "", "", 0 }, /* opens in background (+ sleep 5), broken exit_code (always 0) */
	{ "xdg-open",               NULL,            "", "", 0 }, /* opens in foreground (first browser) or background, correct exit_code */
	{ "sensible-browser",       NULL,            "", "", 0 }, /* opens in foreground (first browser) or background, correct exit_code */
	{ "htmlview",               NULL,            "", "", 0 }, /* opens in foreground (first browser) or background, correct exit_code */
#endif

	/* direct browser invocation */
	{ "x-www-browser",          NULL,           "", "", 1 }, /* opens in foreground, browser alias */
	{ "firefox",                NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "mozilla-firefox",        NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "mozilla",                NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "konqueror",              NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "opera",                  "-newwindow",   "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "epiphany",               NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */	
	{ "galeon",                 NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "amaya",                  NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */
	{ "dillo",                  NULL,           "", "", 1 }, /* opens in foreground, correct exit_code */

	};
  
	guint i;
	for (i = 0; i < G_N_ELEMENTS (browsers); i++)

		if (!browsers[i].disabled)
		{
		        gchar *args[128] = { 0, };
		        guint n = 0;
		        gchar *string;
		        gchar fallback_error[64] = "Ok";
		        gboolean success;

		        args[n++] = (gchar*) browsers[i].prg;
		        
		        if (browsers[i].arg1)
		        	args[n++] = (gchar*) browsers[i].arg1;
		        
		        string = g_strconcat (browsers[i].prefix, url, browsers[i].postfix, NULL);
		        args[n] = string;
        
		        if (!browsers[i].asyncronous) /* start syncronously and check exit code */
			{
				gint exit_status = -1;
				success = g_spawn_sync (NULL, /* cwd */
		                                        args,
		                                        NULL, /* envp */
		                                        G_SPAWN_SEARCH_PATH,
		                                        NULL, /* child_setup() */
		                                        NULL, /* user_data */
		                                        NULL, /* standard_output */
		                                        NULL, /* standard_error */
		                                        &exit_status,
		                                        NULL);
		                success = success && !exit_status;
		            
				if (exit_status)
					g_snprintf (fallback_error, sizeof (fallback_error), "exitcode: %u", exit_status);

			}
		        else
			{
				success = g_spawn_async (NULL, /* cwd */
							 args,
							 NULL, /* envp */
							 G_SPAWN_SEARCH_PATH,
							 NULL, /* child_setup() */
							 NULL, /* user_data */
							 NULL, /* child_pid */
							 NULL);
			}
			
			g_free (string);
			if (success)
				return TRUE;
			browsers[i].disabled = TRUE;
	}
	
	/* reset all disabled states if no browser could be found */
	for (i = 0; i < G_N_ELEMENTS (browsers); i++)
		browsers[i].disabled = FALSE;
		     
	return FALSE;	
}

#endif

gboolean
homebank_util_url_show (const gchar *url)
{

	if(url == NULL)
		return FALSE;


#ifdef G_OS_WIN32
	return homebank_util_url_show_win32 (url);
#else
	return homebank_util_url_show_unix (url);
#endif
}


/* = = = = = = = = = = = = = = = = = = = = */
/* lastopenedfiles */

/*
** load lastopenedfiles from homedir/.homebank
*/
gboolean homebank_lastopenedfiles_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;

	DB( g_print("\n(homebank) lastopenedfiles load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{
		filename = g_build_filename(g_get_home_dir (), HB_DATA_PATH, "lastopenedfiles", NULL );
		//filename = g_strdup_printf("%s/.homebank/lastopenedfiles", g_get_home_dir ());

		DB( g_print(" -> filename: %s\n", filename) );

		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{
			group = "HomeBank";

            DB( g_print(" -> load keyfile ok\n") );

			if(g_key_file_has_key(keyfile, group, "LastOpenedFile", NULL))
			{
                DB( g_print(" -> keyfile has key ok\n") );


				GLOBALS->filename = g_strdup(g_key_file_get_string  (keyfile, group, "LastOpenedFile", NULL));
				DB( g_print("lastfile loaded: %s\n", GLOBALS->filename ) );
				retval = TRUE;
			}
		}
		g_free(filename);
		g_key_file_free (keyfile);
	}

	DB( g_print(" -> return: %d\n", retval) );


	return retval;
}


/*
** save lastopenedfiles to homedir/.homebank (HB_DATA_PATH)
*/
gboolean homebank_lastopenedfiles_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
guint length;

	DB( g_print("\n(homebank) lastopenedfiles save\n") );

	if( GLOBALS->filename != NULL )
	{

		keyfile = g_key_file_new();
		if(keyfile )
		{
			group = "HomeBank";
			g_key_file_set_string  (keyfile, group, "LastOpenedFile", GLOBALS->filename);

			gchar *contents = g_key_file_to_data (keyfile, &length, NULL);

			//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

			//filename = g_strdup_printf("%s/" HB_DATA_PATH "/lastopenedfiles", g_get_home_dir ());
			filename = g_build_filename(g_get_home_dir (), HB_DATA_PATH, "lastopenedfiles", NULL );
			g_file_set_contents(filename, contents, length, NULL);
			g_free(filename);

			g_free(contents);
			g_key_file_free (keyfile);
		}
	}

	return retval;
}



/* = = = = = = = = = = = = = = = = = = = = */
/* Main homebank */



static void free_list_pixbuf(void)
{
guint i;

	for(i=0;i<NUM_LST_PIXBUF;i++)
	{
		if(GLOBALS->lst_pixbuf[i] != NULL)
			g_object_unref(GLOBALS->lst_pixbuf[i]);
	}
}

static void load_list_pixbuf(void)
{
gchar *filename;
guint i;

	DB( g_print("\n(homebank) load_list_pixbuf\n") );

    //GLOBALS->lst_pixbuf[LST_PIXBUF_ADD]    = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_new.svg", NULL);
    //GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT]   = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_edit.png", NULL);;
    //GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND] = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_remind.png", NULL);
    //GLOBALS->lst_pixbuf[LST_PIXBUF_VALID]  = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_vali.png", NULL);
	//GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO]   = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_auto.svg", NULL);

	//todo: rework that
	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_new.svg", NULL);
    GLOBALS->lst_pixbuf[LST_PIXBUF_ADD]     = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);

	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_auto.svg", NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO]    = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);


	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_edit.svg", NULL);
    GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT]    = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);


	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_remind.svg", NULL);
    GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND]  = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);


	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_vali.svg", NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_VALID]   = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);


	filename = g_build_filename(homebank_app_get_pixmaps_dir(), "/lst_warning.svg", NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_WARNING] = gdk_pixbuf_new_from_file_at_size(filename, 16, 16, NULL);
	g_free(filename);


	GLOBALS->lst_pixbuf_maxwidth = 0;
	for(i=0;i<NUM_LST_PIXBUF;i++)
	{
		/*
		if( GLOBALS->lst_pixbuf[i] == NULL )
		{



		}
		*/

		if( GLOBALS->lst_pixbuf[i] != NULL )
			GLOBALS->lst_pixbuf_maxwidth = MAX(GLOBALS->lst_pixbuf_maxwidth, gdk_pixbuf_get_width(GLOBALS->lst_pixbuf[i]) );

	}

	DB( g_print(" -> pixbuf list maxwidth: %d\n", GLOBALS->lst_pixbuf_maxwidth) );

}

static void homebank_register_stock_icons()
{
   GtkIconFactory *icon_factory;
   GtkIconSet *icon_set;
   GtkIconSource *icon_source;
   gchar *filename;
   gint i;

   icon_factory = gtk_icon_factory_new ();

   for (i = 0; i < n_stock_icons; i++)
    {
      icon_set = gtk_icon_set_new ();
      icon_source = gtk_icon_source_new ();

      //todo: win32
      filename = g_build_filename(homebank_app_get_pixmaps_dir(), stock_icons[i].filename, NULL);

      gtk_icon_source_set_filename (icon_source, filename);
      gtk_icon_set_add_source (icon_set, icon_source);
      gtk_icon_source_free (icon_source);
      gtk_icon_factory_add (icon_factory, stock_icons[i].stock_id, icon_set);
      gtk_icon_set_unref (icon_set);
       g_free(filename);
   }

   gtk_icon_factory_add_default (icon_factory);

   g_object_unref (icon_factory);

}

void homebank_window_set_icon_from_file(GtkWindow *window, gchar *filename)
{
gchar *pathfilename;

	pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), filename, NULL);
	gtk_window_set_icon_from_file(GTK_WINDOW (window), pathfilename, NULL);
	g_free(pathfilename);
}




const gchar *
homebank_app_get_pixmaps_dir (void)
{
	return pixmaps_dir;
}

const gchar *
homebank_app_get_locale_dir (void)
{
	return locale_dir;
}

const gchar *
homebank_app_get_help_dir (void)
{
	return help_dir;
}

/* build package paths at runtime */
static void
build_package_paths (void)
{
#ifdef G_OS_WIN32
	gchar *prefix;
	
	prefix = g_win32_get_package_installation_directory (NULL, NULL);
	pixmaps_dir  = g_build_filename (prefix, "share", PACKAGE, "pixmaps", NULL);
	locale_dir   = g_build_filename (prefix, "share", "locale", NULL);
	help_dir     = g_build_filename (prefix, "share", PACKAGE, "help", NULL);
	g_free (prefix);
#else
	pixmaps_dir  = g_strdup (HOMEBANK_PIXMAPSDIR);
	locale_dir   = g_strdup (HOMEBANK_LOCALEDIR);
	help_dir     = g_strdup (HOMEBANK_HELPDIR);
#endif

	DB( g_print("pixmaps_dir: %s\n", pixmaps_dir) );
	DB( g_print("locale_dir : %s\n", locale_dir) );
	DB( g_print("help_dir   : %s\n", help_dir) );

}













/*
 * check/create user home directory for .homebank (HB_DATA_PATH) directory
 */
static void homebank_check_app_dir()
{
gchar *homedir;
gboolean exists;

	DB( g_print("homebank_check_app_dir()\n") );

	//homedir = g_strdup_printf("%s/" HB_DATA_PATH, g_get_home_dir ());
	homedir = g_build_filename(g_get_home_dir (), HB_DATA_PATH, NULL );
	exists = g_file_test(homedir, G_FILE_TEST_IS_DIR);
	if(!exists)
		g_mkdir(homedir, 0755);

	/* security */
	g_chmod(homedir, 0700);
	g_free(homedir);
}


/*
** application cleanup: icons, GList, memory
*/
static void homebank_cleanup()
{

	DB( g_print("\n(homebank) cleanup\n") );

	//v3.4 save windows size/position
	homebank_pref_save();

	free_list_pixbuf();
	free_paymode_icons();
	free_pref_icons();


	/* free our GList */
	da_operation_destroy(GLOBALS->ope_list);
	da_archive_destroy(GLOBALS->arc_list);

	g_hash_table_destroy(GLOBALS->h_memo);

	da_tag_destroy();
	da_cat_destroy();
	da_pay_destroy();	
	da_acc_destroy();

	/* free our global datas */
	if( PREFS  )
	{
		homebank_pref_free();
		g_free(PREFS);
	}

	if(GLOBALS)
	{
		g_free(GLOBALS->oldfilename);
		g_free(GLOBALS->filename);
		g_free(GLOBALS);
	}

	g_free (pixmaps_dir);	
	g_free (locale_dir);
	g_free (help_dir);

}



/*
** application setup: icons, GList, memory
*/
static gboolean homebank_setup()
{
GDate *date;

	DB( g_print("(homebank) setup\n") );

	/* allocate our global datas */
	GLOBALS = g_malloc0(sizeof(struct HomeBank));
	if(!GLOBALS) return FALSE;

	PREFS   = g_malloc0(sizeof(struct Preferences));
	if(!PREFS) return FALSE;

	da_acc_new();
	da_pay_new();
	da_cat_new();
	da_tag_new();

	GLOBALS->h_memo = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, NULL);

	homebank_register_stock_icons();

	load_list_pixbuf();
	load_paymode_icons();
	load_pref_icons();


	// check homedir for .homebank dir
	homebank_check_app_dir();

	//debug
	//DB( g_print(" -> init prefs\n") );
	homebank_pref_setdefault();
	homebank_pref_load();
	homebank_pref_createformat();

	/* today's date */
	date = g_date_new();
	g_date_set_time_t(date, time(NULL));
	GLOBALS->today = g_date_get_julian(date);
	g_date_free(date);

	/* default filename */
	GLOBALS->filename = g_build_filename(PREFS->path_wallet, "untitled.xhb", NULL);
	GLOBALS->wallet_is_new = TRUE;

	//fix: v4.0
	GLOBALS->title = g_strdup(_("(Nobody)"));

	//debug
	#if MYDEBUG == 1
	/*
	g_print("user_name: %s\n", g_get_user_name ());
	g_print("real_name: %s\n", g_get_real_name ());
	g_print("user_cache_dir: %s\n", g_get_user_cache_dir());
	g_print("user_data_dir: %s\n", g_get_user_data_dir ());
	g_print("user_config_dir: %s\n", g_get_user_config_dir ());
	//g_print("system_data_dirs: %s\n", g_get_system_data_dirs ());
	//g_print("system_config_dirs: %s\n", g_get_system_config_dirs ());

	g_print("home_dir: %s\n", g_get_home_dir ());
	g_print("tmp_dir: %s\n", g_get_tmp_dir ());
	g_print("current_dir: %s\n", g_get_current_dir ());
	*/

	#endif

	return TRUE;
}


/* = = = = = = = = = = = = = = = = = = = = */
/* Main homebank */

static GtkWidget *
homebank_construct_splash()
{
GtkWidget *window;
GtkWidget *frame, *vbox, *image;
//gchar *ver_string, *markup, *version;
gchar *pathfilename;

	window = gtk_window_new(GTK_WINDOW_POPUP);	//TOPLEVEL DONT WORK
	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);

	gtk_window_set_title (GTK_WINDOW (window), "HomeBank");
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), "splash.png", NULL);
	image = gtk_image_new_from_file((const gchar *)pathfilename);
	g_free(pathfilename);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (window), frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	/*
	ver_string = g_strdup_printf(_("Version: HomeBank-%s"), VERSION);

	version = gtk_label_new(NULL);
	markup = g_markup_printf_escaped(MARKUP_STRING, ver_string);
	gtk_label_set_markup(GTK_LABEL(version), markup);
	g_free(markup);
	g_free(ver_string);
	*/

	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	//gtk_box_pack_start (GTK_BOX (vbox), version, FALSE, FALSE, 0);

	return window;
}





int
main (int argc, char *argv[])
{
GOptionContext *option_context;
GOptionGroup *option_group;
GError *error = NULL;
GtkWidget *mainwin;
GtkWidget *splash;
gchar *pathfilename;
gboolean openlast;

	DB( g_print("\n--------------------------------\nhomebank starting...\n" ) );

	build_package_paths();

  /* Initialize i18n support */
#ifdef ENABLE_NLS

     DB( g_print(" -> enable nls\n" ) );

     setlocale (LC_ALL, "");

     bindtextdomain (GETTEXT_PACKAGE, locale_dir);

     bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
     textdomain (GETTEXT_PACKAGE);

#endif


	/* Set up option groups */
	option_context = g_option_context_new (NULL);
	
	//g_option_context_set_summary (option_context, _(""));

	option_group = g_option_group_new ("homebank",
					   N_("HomeBank options"),
					   N_("HomeBank options"),
					   NULL, NULL);
	g_option_group_add_entries (option_group, option_entries);
	g_option_context_set_main_group (option_context, option_group);
	g_option_group_set_translation_domain (option_group, GETTEXT_PACKAGE);

	/* Add Gtk option group */
	g_option_context_add_group (option_context, gtk_get_option_group (FALSE));

	/* Parse command line */
	if (!g_option_context_parse (option_context, &argc, &argv, &error))
	{
		g_option_context_free (option_context);
		
		if (error)
		{
			g_print ("%s\n", error->message);
			g_error_free (error);
		}
		else
			g_print ("An unknown error occurred\n");

		return -1;
	}
	
	g_option_context_free (option_context);
	option_context = NULL;

	if (arg_version != FALSE)
	{
		/* Print version information and exit */
		g_print ("%s\n", PACKAGE " " VERSION);
		return 0;
	}

	/* Pass NULL here since we parsed the gtk+ args already...
	 * from this point all we need a DISPLAY variable to be set.
	 */
	gtk_init (NULL, NULL);

	//todo: check gtk version here ?

	g_set_application_name (APPLICATION_NAME);



	splash = homebank_construct_splash();

  gtk_window_set_auto_startup_notification (FALSE);
  gtk_widget_show_all (splash);
  gtk_window_set_auto_startup_notification (TRUE);

  /* make sure splash is up */
  while (gtk_events_pending ())
    gtk_main_iteration ();
    

	if( homebank_setup() )
	{

		pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), "homebank.svg", NULL);
		gtk_window_set_default_icon_from_file(pathfilename, NULL);
		g_free(pathfilename);

		DB( g_print(" -> creating window\n" ) );


		mainwin = (GtkWidget *)create_wallet_window (NULL);

		if(mainwin)
		{
		struct WinGeometry *wg;
		
			//setup, init and show window
			wg = &PREFS->wal_wg;
			gtk_window_move(GTK_WINDOW(mainwin), wg->l, wg->t);
			gtk_window_resize(GTK_WINDOW(mainwin), wg->w, wg->h);

			//todo: pause on splash
			//g_usleep( G_USEC_PER_SEC * 1 );
			gtk_widget_hide(splash);

		    gtk_widget_show_all (mainwin);

#if HB_UNSTABLE == 1
			homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_WARNING,
				"This is an alpha/beta release of HomeBank",
				"DO NOT USE it with some important files.\n"
				"This kind of release is for TESTING ONLY.\n"
				"It may be buggy, crash, or loose your datas.\n\n"
				"Please report bugs/questions/suggestions\n"
				"directly by email to: homebank@free.fr\n\n"
				"PLEASE DO NOT USE LaunchPad for alpha/beta.\n",
				NULL
				);
#endif

			while (gtk_events_pending ()) /* make sure splash is gone */
				gtk_main_iteration ();


			DB( g_print(" -> open last file ?\n" ) );

			// load a file ?
			/* load 1st file specified on commandline */
			openlast = PREFS->loadlast;
			if (files != NULL)
			{
				if (g_file_test (files[0], G_FILE_TEST_EXISTS) != FALSE)
				{
					DB( g_print(" -> should load %s\n", files[0] ) );
					g_free(GLOBALS->filename);
					GLOBALS->filename = g_strdup(files[0]);
					wallet_open_internal(mainwin, NULL);
					openlast = FALSE;
				}
				else
				{
					g_warning (_("Unable to open '%s', the file does not exist.\n"), files[0]);
					
				}
				g_strfreev (files);
			}
			
			
			DB( g_print(" -> GLOBALS->filename: '%s'\n", GLOBALS->filename ) );
			
			if( openlast )
			{
				if( homebank_lastopenedfiles_load() == TRUE )
					wallet_open_internal(mainwin, NULL);
			}

			/* update the mainwin display */
			wallet_update(mainwin, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));

		DB( g_print(" -> gtk_main()\n" ) );

			gtk_main ();
		}

	}

	gtk_widget_destroy(splash);

    homebank_cleanup();

	return 0;
}

#ifdef G_OS_WIN32
/* In case we build this as a windowed application */

#ifdef __GNUC__
#define _stdcall  __attribute__((stdcall))
#endif

int _stdcall
WinMain (struct HINSTANCE__ *hInstance,
	 struct HINSTANCE__ *hPrevInstance,
	 char               *lpszCmdLine,
	 int                 nCmdShow)
{
	return main (__argc, __argv);
}
#endif

