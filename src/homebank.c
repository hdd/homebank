/* HomeBank -- Free easy personal accounting for all !
 * Copyright (C) 1995-2007 Maxime DOYEN
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

#ifdef G_OS_WIN32
#include <windows.h>

long __stdcall
ShellExecuteA (long        hwnd,
               const char* lpOperation,
               const char* lpFile,
               const char* lpParameters,
               const char* lpDirectory,
               int         nShowCmd);
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

gchar *homebank_pixmaps_dir = PIXMAPS_DIR;
gchar *homebank_locale_dir = LOCALE_DIR;;

static struct { 
  gchar *filename;
  gchar *stock_id;
} stock_icons[] = {
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
  { "lpi-translate.png"      , "hb-lpi-translate" },
  { "lpi-bug.png"            , "hb-lpi-bug" },

};
 
static gint n_stock_icons = G_N_ELEMENTS (stock_icons);

#define MARKUP_STRING "<span size='small'>%s</span>"

static gchar *list_pixbux_names[] =
{
	"lst_new.svg",
	"lst_edit.svg",
	"lst_remind.svg",
	"lst_vali.svg",
	"lst_auto.svg",
	"lst_warning.svg",
	NULL
};

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
	                                  title
	                                  );

  if (message_format)
    {
      va_start (args, message_format);
      msg = g_strdup_vprintf (message_format, args);
      va_end (args);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), msg);

      g_free (msg);
    }

	 gtk_dialog_run (GTK_DIALOG (dialog));
	 gtk_widget_destroy (dialog);
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

	DB( g_printf("(hombank) csvfile chooser %d\n", action) );

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
		title = _("Import from CSV");
		button = GTK_STOCK_OPEN;
	}
	else
	{
		title = _("Export as CSV");
		button = GTK_STOCK_SAVE;
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(parent),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.csv");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All"));
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

	DB( g_printf("(wallet) alien file chooser\n") );

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Amiga files"));
	gtk_file_filter_add_pattern (filter, "*.hb");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All"));
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
gchar *newname;
gchar **str_array;

	basename = g_path_get_basename(path);
	dirname  = g_path_get_dirname (path);

	str_array = g_strsplit(basename, ".", 0);

	newname = g_strdup_printf("%s/%s.%s", dirname, str_array[0], extension);	

	g_strfreev(str_array);
	g_free(basename);
	g_free(dirname);

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
	gchar *newname;

		str_array = g_strsplit(basename, ".", 0);
		newname = g_strdup_printf("%s/%s.xhb", dirname, str_array[0]);	
		g_strfreev(str_array);

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
gchar *newname;
gchar **str_array;
int retval;

	basename = g_path_get_basename(pathname);
	dirname  = g_path_get_dirname (pathname);

	if( g_str_has_suffix(basename, ".xhb") )
	{
		str_array = g_strsplit(basename, ".", 0);
		newname = g_strdup_printf("%s/%s.old", dirname, str_array[0]);	

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
	
	if (code <= 32)
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
	g_return_val_if_fail (url != NULL, FALSE);

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
		filename = g_strdup_printf("%s/.homebank/lastopenedfiles", g_get_home_dir ());
		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{
			group = "HomeBank";

			if(g_key_file_has_key(keyfile, group, "LastOpenedFile", NULL))
			{
				GLOBALS->filename = g_strdup(g_key_file_get_string  (keyfile, group, "LastOpenedFile", NULL));
				DB( g_print("lastfile loaded: %s\n", GLOBALS->filename ) );
			}
		}
		g_free(filename);
		g_key_file_free (keyfile);
	}

	return retval;
}


/*
** save lastopenedfiles to homedir/.homebank
*/
gboolean homebank_lastopenedfiles_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
guint length;

	DB( g_print("\n(homebank) lastopenedfiles save\n") );

	keyfile = g_key_file_new();
	if(keyfile )
	{
		group = "HomeBank";
		g_key_file_set_string  (keyfile, group, "LastOpenedFile", GLOBALS->filename);

		gchar *contents = g_key_file_to_data (keyfile, &length, NULL);

		//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

		filename = g_strdup_printf("%s/.homebank/lastopenedfiles", g_get_home_dir ());
		g_file_set_contents(filename, contents, length, NULL);
		g_free(filename);

		g_free(contents);
		g_key_file_free (keyfile);
	}

	return retval;
}



/* = = = = = = = = = = = = = = = = = = = = */
/* Main homebank */



void free_list_pixbuf(void)
{
guint i;

	for(i=0;i<NUM_LST_PIXBUF;i++)
	{
		if(GLOBALS->lst_pixbuf[i] != NULL)
			g_object_unref(GLOBALS->lst_pixbuf[i]);
	}
}

void load_list_pixbuf(void)
{
guint i;

    //GLOBALS->lst_pixbuf[LST_PIXBUF_ADD]    = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_new.svg", NULL);
    //GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT]   = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_edit.png", NULL);;
    //GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND] = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_remind.png", NULL);
    //GLOBALS->lst_pixbuf[LST_PIXBUF_VALID]  = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_vali.png", NULL);
	//GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO]   = gdk_pixbuf_new_from_file(PIXMAPS_DIR "/lst_auto.svg", NULL);

    GLOBALS->lst_pixbuf[LST_PIXBUF_ADD]     = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_new.svg", 16, 16, NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO]    = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_auto.svg", 16, 16, NULL);
    GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT]    = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_edit.svg", 16, 16, NULL);
    GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND]  = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_remind.svg", 16, 16, NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_VALID]   = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_vali.svg", 16, 16, NULL);
	GLOBALS->lst_pixbuf[LST_PIXBUF_WARNING] = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR "/lst_warning.svg", 16, 16, NULL);

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

	DB( g_print("pixbuf list maxwidth: %d\n", GLOBALS->lst_pixbuf_maxwidth) );

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
      filename = g_build_filename(homebank_pixmaps_dir, stock_icons[i].filename, NULL);
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

	pathfilename = g_build_filename(homebank_pixmaps_dir, filename, NULL);
	gtk_window_set_icon_from_file(GTK_WINDOW (window), pathfilename, NULL);
	g_free(pathfilename);
}

/*
 * check/create user home directory for .homebank directory
 */
void homebank_check_app_dir()
{
gchar *homedir;
gboolean exists;

	DB( g_print("homebank_check_app_dir()\n") );

	homedir = g_strdup_printf("%s/.homebank", g_get_home_dir ());
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
void homebank_cleanup()
{

	DB( g_print("\n(homebank) cleanup\n") );

	//v3.4 save windows size/position
	homebank_pref_save();

	homebank_lastopenedfiles_save();

	free_list_pixbuf();
	free_paymode_icons();
	free_pref_icons();


	/* free our GList */
	da_operation_destroy(GLOBALS->ope_list);
	da_archive_destroy(GLOBALS->arc_list);
	da_category_destroy(GLOBALS->cat_list);
	da_payee_destroy(GLOBALS->pay_list);
	da_account_destroy(GLOBALS->acc_list);

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

	#ifdef G_OS_WIN32
		g_free(homebank_pixmaps_dir);
		g_free(homebank_locale_dir);
	#endif

}



/*
** application setup: icons, GList, memory
*/
gboolean homebank_setup()
{
GDate *date;

	DB( g_print("(homebank) setup\n") );

	/* allocate our global datas */
	GLOBALS = g_malloc0(sizeof(struct HomeBank));
	if(!GLOBALS) return FALSE;

	PREFS   = g_malloc0(sizeof(struct Preferences));
	if(!PREFS) return FALSE;

	/* today's date */
	date = g_date_new();
	g_date_set_time(date, time(NULL));
	GLOBALS->today = g_date_get_julian(date);
	g_date_free(date);

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
GtkWidget *vbox, *version, *image;
gchar *ver_string, *markup, *pathfilename;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	//gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
	gtk_window_set_title (GTK_WINDOW (window), "HomeBank");
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	pathfilename = g_build_filename(homebank_pixmaps_dir, "splash.png", NULL);
	image = gtk_image_new_from_file(pathfilename);
	g_free(pathfilename);

	vbox = gtk_vbox_new (FALSE, 3);

	/*
	ver_string = g_strdup_printf(_("Version: HomeBank-%s"), VERSION);

	version = gtk_label_new(NULL);
	markup = g_markup_printf_escaped(MARKUP_STRING, ver_string);
	gtk_label_set_markup(GTK_LABEL(version), markup);
	g_free(markup);
	g_free(ver_string);
	*/

	gtk_container_add (GTK_CONTAINER (window), vbox);
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

  /* Initialize i18n support */
#ifdef ENABLE_NLS
     setlocale (LC_ALL, "");
     bindtextdomain (GETTEXT_PACKAGE, homebank_locale_dir);
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
	 * from this point on we need a DISPLAY variable to be set.
	 */
	gtk_init (NULL, NULL);

	//todo: check gtk version here ?

	g_set_application_name (APPLICATION_NAME);

	splash = homebank_construct_splash();
	
	gtk_window_set_auto_startup_notification (FALSE);
	gtk_widget_show_all (splash);
	gtk_window_set_auto_startup_notification (TRUE);
	
	while (gtk_events_pending ())	/* ensure splash is up */
		gtk_main_iteration ();

	if( homebank_setup() )
	{
		DB( g_print("** creating window\n" ) );

		pathfilename = g_build_filename(homebank_pixmaps_dir, "homebank.svg", NULL);
		gtk_window_set_default_icon_from_file(pathfilename, NULL);
		g_free(pathfilename);

		mainwin = (GtkWidget *)create_wallet_window (NULL);

		if(mainwin)
		{
		struct WinGeometry *wg;
		
			//setup, init and show window
			wg = &PREFS->wal_wg;
			gtk_window_move(GTK_WINDOW(mainwin), wg->l, wg->t);
			gtk_window_resize(GTK_WINDOW(mainwin), wg->w, wg->h);

			//pause
			g_usleep( G_USEC_PER_SEC * 1.5 );
			gtk_widget_hide(splash);

		    gtk_widget_show_all (mainwin);

			while (gtk_events_pending ()) /* make sure splash is gone */
				gtk_main_iteration ();


			// load a file ?
			/* load 1st file specified on commandline */
			openlast = TRUE;
			if (files != NULL)
			{
				if (g_file_test (files[0], G_FILE_TEST_EXISTS) != FALSE)
				{
					DB( g_print(" - should load %s\n", files[0] ) );
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
			
			if( openlast )
			{
				homebank_lastopenedfiles_load();
				wallet_open_internal(mainwin, NULL);
			}

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

