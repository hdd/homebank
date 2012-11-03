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


static struct { 
  gchar *filename;
  gchar *stock_id;
} stock_icons[] = {
  { PIXMAPS_DIR "/account.svg"             , "hb-stock-account" },
  { PIXMAPS_DIR "/payee.svg"               , "hb-stock-payee" },
  { PIXMAPS_DIR "/category.svg"            , "hb-stock-category" },
  { PIXMAPS_DIR "/archive.svg"             , "hb-stock-archive" },
  { PIXMAPS_DIR "/budget.svg"              , "hb-stock-budget" },

  { PIXMAPS_DIR "/filter.svg"              , "hb-stock-filter" },

  { PIXMAPS_DIR "/ope_show.svg"            , "hb-stock-ope-show" },
  { PIXMAPS_DIR "/ope_add.svg"             , "hb-stock-ope-add" },
  { PIXMAPS_DIR "/ope_herit.svg"           , "hb-stock-ope-herit" },
  { PIXMAPS_DIR "/ope_edit.svg"            , "hb-stock-ope-edit" },
  { PIXMAPS_DIR "/ope_delete.svg"          , "hb-stock-ope-delete" },
  { PIXMAPS_DIR "/ope_valid.svg"           , "hb-stock-ope-valid" },

  { PIXMAPS_DIR "/report_stats.svg"        , "hb-stock-rep-stats" },
  { PIXMAPS_DIR "/report_budget.svg"       , "hb-stock-rep-budget" },
  { PIXMAPS_DIR "/report_overdrawn.svg"    , "hb-stock-rep-over" },
  { PIXMAPS_DIR "/report_car.svg"          , "hb-stock-rep-car" },

  { PIXMAPS_DIR "/view_list.svg"           , "hb-stock-view-list" },
  { PIXMAPS_DIR "/view_bar.svg"            , "hb-stock-view-bar" },
  { PIXMAPS_DIR "/view_pie.svg"            , "hb-stock-view-pie" },
  { PIXMAPS_DIR "/view_line.svg"           , "hb-stock-view-line" },

  { PIXMAPS_DIR "/legend.svg"             , "hb-stock-legend" },
  { PIXMAPS_DIR "/rate.svg"               , "hb-stock-rate" },
  { PIXMAPS_DIR "/refresh.svg"            , "hb-stock-refresh" },
  
  
};
 
static gint n_stock_icons = G_N_ELEMENTS (stock_icons);

#define MARKUP_STRING "<span size='small'>%s</span>"


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
		title = _("Import from csv");
		button = GTK_STOCK_OPEN;
	}
	else
	{
		title = _("Export as csv");
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

	/*
	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
	    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);
	}
	else
	{
		if(GLOBALS->wallet_is_new == TRUE)
		{
		    // the user just created a new document
		    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);
		    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), "untitled.xhb");
		}
		else
		{
		    // the user edited an existing document
		    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), GLOBALS->filename);
		}
	}
	*/

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		DB( g_printf("- filename: %s\n", filename) );

		//store filename
		*storage_ptr = filename;

		DB( g_printf("- filename: %s\n", GLOBALS->filename) );


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
gboolean retval;

	DB( g_printf("(wallet) alien file chooser\n") );

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_wallet);

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
/* Preferences */

void homebank_init_prefs(void)
{

	DB( g_print("\n(homebank) pref init\n") );

	g_free(PREFS->path_wallet);
	PREFS->path_wallet = g_strdup_printf("%s", g_get_home_dir ());
	//PREFS->path_wallet = g_strdup(DEFAULT_PATH_WALLET);

	//PREFS->image_size = 24;

	PREFS->color_exp  = DEFAULT_EXP_COLOR;
	PREFS->color_inc  = DEFAULT_INC_COLOR;
	PREFS->color_warn = DEFAULT_WARN_COLOR;

	PREFS->runwizard = TRUE;

	DB( g_print(" + #%x\n", PREFS->color_exp) );
	DB( g_print(" + #%x\n", PREFS->color_inc) );
	DB( g_print(" + #%x\n", PREFS->color_warn) );

	g_free(PREFS->path_navigator);
	PREFS->path_navigator = g_strdup(DEFAULT_PATH_NAVIGATOR);

	g_free(PREFS->date_format);
	PREFS->date_format = g_strdup(DEFAULT_FORMAT_DATE);

	PREFS->num_nbdecimal = 2;
	PREFS->num_separator = TRUE;

	PREFS->filter_range = 7;


	//todo: add intelligence here
	PREFS->euro_active  = FALSE;
	
	
	PREFS->euro_country = 0;
	PREFS->euro_value   = 1.0;
	PREFS->euro_nbdec   = 2;
	PREFS->euro_thsep   = TRUE;
	PREFS->euro_symbol	= g_strdup("??");

	PREFS->stat_byamount   = FALSE;
	PREFS->stat_showdetail = FALSE;
	PREFS->budg_showdetail = FALSE;

	PREFS->chart_legend = FALSE;

	homebank_pref_createformat();


	/*
	#if MYDEBUG == 1
	struct lconv *lc = localeconv();

	gchar buf[55];
	gchar buf2[55];
	hb_strfmon(buf, 55, 666.98654, FALSE);
	DB( g_print("test hb_strfmon '%s'\n", buf) );
	hb_strfmon(buf2, 55, 666.98654, TRUE);
	DB( g_print("test hb_strfmon '%s'\n\n", buf2) );

	DB( g_print("mon_decimal_point '%s'\n", lc->mon_decimal_point) );
	DB( g_print("mon_thousands_sep '%s'\n", lc->mon_thousands_sep) );

	DB( g_print("frac_digits '%d'\n", (gint)lc->frac_digits) );

	DB( g_print("currency_symbol '%s'\n", lc->currency_symbol) );
	#endif
	*/
}

/*
** create the format string for monetary strfmon (major/minor)
*/
void homebank_pref_createformat(void)
{

	DB( g_print("\n(homebank) pref create format\n") );

	if(PREFS->num_separator == FALSE)
		g_snprintf(GLOBALS->fmt_maj_number, 15, "%%^.%dn", PREFS->num_nbdecimal);
	else
		g_snprintf(GLOBALS->fmt_maj_number, 15, "%%.%dn", PREFS->num_nbdecimal);

	DB( g_print("+ major is: '%s'\n", GLOBALS->fmt_maj_number) );


	if(PREFS->euro_thsep == FALSE)
		g_snprintf(GLOBALS->fmt_min_number, 15, "%%!^.%dn %s", PREFS->euro_nbdec, PREFS->euro_symbol);
	else
		g_snprintf(GLOBALS->fmt_min_number, 15, "%%!.%dn %s", PREFS->euro_nbdec, PREFS->euro_symbol);

	DB( g_print("+ minor is: '%s'\n", GLOBALS->fmt_min_number) );

}

/*
** load preference from homedir/.homebank
*/
gboolean homebank_pref_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
GError *error = NULL;

	DB( g_print("\n(homebank) pref load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{
		filename = g_strdup_printf("%s/.homebank/preferences", g_get_home_dir ());
		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{

			group = "General";
			//gchar *version = g_key_file_get_string (keyfile, group, "Version", NULL);

			if(g_key_file_has_key(keyfile, group, "BarStyle", NULL))
				PREFS->toolbar_style = g_key_file_get_integer (keyfile, group, "BarStyle", NULL);
			//if(g_key_file_has_key(keyfile, group, "BarImageSize", NULL))
			//	PREFS->image_size = g_key_file_get_integer (keyfile, group, "BarImageSize", NULL);

			if(g_key_file_has_key(keyfile, group, "ColorExp", NULL))
				PREFS->color_exp = g_key_file_get_integer (keyfile, group, "ColorExp", NULL);
			if(g_key_file_has_key(keyfile, group, "ColorInc", NULL))
				PREFS->color_inc = g_key_file_get_integer (keyfile, group, "ColorInc", NULL);
			if(g_key_file_has_key(keyfile, group, "ColorWarn", NULL))
				PREFS->color_warn = g_key_file_get_integer (keyfile, group, "ColorWarn", NULL);

			#if DOWIZARD == 1
			if(g_key_file_has_key(keyfile, group, "RunWizard", NULL))
				PREFS->runwizard = g_key_file_get_boolean (keyfile, group, "RunWizard", NULL);
			#endif

			if(g_key_file_has_key(keyfile, group, "WalletPath", NULL))
			{
				g_free(PREFS->path_wallet);
				PREFS->path_wallet = g_strdup(g_key_file_get_string  (keyfile, group, "WalletPath", NULL));
			}

			if(g_key_file_has_key(keyfile, group, "NavigatorPath", NULL))
			{
				g_free(PREFS->path_navigator);
				PREFS->path_navigator = g_strdup(g_key_file_get_string  (keyfile, group, "NavigatorPath", NULL));
			}

			group = "Format";
			PREFS->num_nbdecimal = g_key_file_get_integer (keyfile, group, "NumNbDec", NULL);
			PREFS->num_separator = g_key_file_get_boolean (keyfile, group, "NumSep", NULL);
			PREFS->british_unit  = g_key_file_get_boolean (keyfile, group, "UKUnits", NULL);

			group = "Filter";
			PREFS->filter_range = g_key_file_get_integer (keyfile, group, "DefRange", NULL);

		//euro options
			group = "Euro";
			if(g_key_file_has_key(keyfile, group, "Active", NULL))
				PREFS->euro_active = g_key_file_get_boolean (keyfile, group, "Active", NULL);

			PREFS->euro_country = g_key_file_get_integer (keyfile, group, "Country", NULL);

			gchar *ratestr = g_key_file_get_string (keyfile, group, "ChangeRate", NULL);
			if(ratestr != NULL) PREFS->euro_value = g_ascii_strtod(ratestr, NULL);
			PREFS->euro_nbdec = g_key_file_get_integer (keyfile, group, "NBDec", NULL);
			PREFS->euro_thsep = g_key_file_get_boolean (keyfile, group, "Sep", NULL);

			gchar *tmpstr = g_key_file_get_string  (keyfile, group, "Symbol", &error);


			if (error)
		    {
		      g_warning ("error: %s\n", error->message);
		      g_error_free(error);
		      error = NULL;
		    }

			PREFS->euro_symbol = g_locale_to_utf8(tmpstr, -1, NULL, NULL, NULL);

		//report options
			group = "Report";
			if(g_key_file_has_key(keyfile, group, "StatByAmount", NULL))
				PREFS->stat_byamount   = g_key_file_get_boolean (keyfile, group, "StatByAmount", NULL);
			if(g_key_file_has_key(keyfile, group, "StatDetail", NULL))
				PREFS->stat_showdetail = g_key_file_get_boolean (keyfile, group, "StatDetail", NULL);
			if(g_key_file_has_key(keyfile, group, "BudgDetail", NULL))
				PREFS->budg_showdetail = g_key_file_get_boolean (keyfile, group, "BudgDetail", NULL);

			//group = "Chart";
			//PREFS->chart_legend = g_key_file_get_boolean (keyfile, group, "Legend", NULL);


			//g_key_file_set_string  (keyfile, group, "", PREFS->);
			//g_key_file_set_boolean (keyfile, group, "", PREFS->);
			//g_key_file_set_integer (keyfile, group, "", PREFS->);


			#if MYDEBUG == 1
			gsize length;
			gchar *contents = g_key_file_to_data (keyfile, &length, NULL);
			//DB( g_print(" keyfile:\n%s\n len=%d\n", contents, length) );
			g_free(contents);

			//g_print(" + active: %d\n", PREFS->euro_active);

			g_print(" + rate  : %f\n", PREFS->euro_value);
			g_print(" + nbdec : %d\n", PREFS->euro_nbdec);
			g_print(" + sepa  : %d\n", PREFS->euro_thsep);
			g_print(" + symbol: %s\n", PREFS->euro_symbol);

			#endif


		}
		g_free(filename);
		g_key_file_free (keyfile);
	}

	homebank_pref_createformat();

	return retval;
}


/*
** save preference to homedir/.homebank
*/
gboolean homebank_pref_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
guint length;

	DB( g_print("\n(homebank) pref save\n") );

	keyfile = g_key_file_new();
	if(keyfile )
	{
		group = "General";
		g_key_file_set_string  (keyfile, group, "Version", PREF_VERSION);

		g_key_file_set_integer (keyfile, group, "BarStyle", PREFS->toolbar_style);
		//g_key_file_set_integer (keyfile, group, "BarImageSize", PREFS->image_size);

		g_key_file_set_integer (keyfile, group, "ColorExp", PREFS->color_exp);
		g_key_file_set_integer (keyfile, group, "ColorInc", PREFS->color_inc);
		g_key_file_set_integer (keyfile, group, "ColorWarn", PREFS->color_warn);

		#if DOWIZARD == 1
		g_key_file_set_boolean (keyfile, group, "RunWizard", PREFS->runwizard);
		#endif

		g_key_file_set_string  (keyfile, group, "WalletPath", PREFS->path_wallet);
		g_key_file_set_string  (keyfile, group, "NavigatorPath", PREFS->path_navigator);

		group = "Format";
		g_key_file_set_integer (keyfile, group, "NumNbDec", PREFS->num_nbdecimal);
		g_key_file_set_boolean (keyfile, group, "NumSep", PREFS->num_separator);
		g_key_file_set_boolean (keyfile, group, "UKUnits", PREFS->british_unit);

		group = "Filter";
		g_key_file_set_integer (keyfile, group, "DefRange", PREFS->filter_range);

	//euro options
		group = "Euro";
		g_key_file_set_boolean (keyfile, group, "Active", PREFS->euro_active);
		g_key_file_set_integer (keyfile, group, "Country", PREFS->euro_country);
		gchar ratestr[64];
		g_ascii_dtostr(ratestr, 63, PREFS->euro_value);
		g_key_file_set_string (keyfile, group, "ChangeRate", ratestr);
		g_key_file_set_integer (keyfile, group, "NBDec", PREFS->euro_nbdec);
		g_key_file_set_boolean (keyfile, group, "Sep", PREFS->euro_thsep);
		g_key_file_set_string  (keyfile, group, "Symbol", PREFS->euro_symbol);

	//report options
		group = "Report";
		g_key_file_set_boolean (keyfile, group, "StatByAmount", PREFS->stat_byamount);
		g_key_file_set_boolean (keyfile, group, "StatDetail", PREFS->stat_showdetail);
		g_key_file_set_boolean (keyfile, group, "BudgDetail", PREFS->budg_showdetail);

		//group = "Chart";
		//g_key_file_set_boolean (keyfile, group, "Legend", PREFS->chart_legend);

		//g_key_file_set_string  (keyfile, group, "", PREFS->);
		//g_key_file_set_boolean (keyfile, group, "", PREFS->);
		//g_key_file_set_integer (keyfile, group, "", PREFS->);

		gchar *contents = g_key_file_to_data (keyfile, &length, NULL);

		//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

		filename = g_strdup_printf("%s/.homebank/preferences", g_get_home_dir ());
		g_file_set_contents(filename, contents, length, NULL);
		g_free(filename);

		g_free(contents);
		g_key_file_free (keyfile);
	}

	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = */
/* lastopenedfiles

/*
** load lastopenedfiles from homedir/.homebank
*/
gboolean homebank_lastopenedfiles_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
GError *error = NULL;

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
				GLOBALS->lastfilename = g_strdup(g_key_file_get_string  (keyfile, group, "LastOpenedFile", NULL));
				DB( g_print("lastfile loaded: %s\n", GLOBALS->lastfilename ) );
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





/*
** application cleanup: icons, GList, memory
*/
void homebank_cleanup()
{

	DB( g_print("\n(homebank) cleanup\n") );

	//debug
	//homebank_pref_save();

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
		g_free(PREFS->date_format);
		g_free(PREFS->path_wallet);
		g_free(PREFS->path_navigator);
		g_free(PREFS->euro_symbol );
		g_free(PREFS);
	}

	if(GLOBALS)
	{
		g_free(GLOBALS->lastfilename);
		g_free(GLOBALS->filename);
		g_free(GLOBALS);
	}
}


static void homebank_register_stock_icons()
{
   GtkIconFactory *icon_factory;
   GtkIconSet *icon_set; 
   GtkIconSource *icon_source;
   gint i;

   icon_factory = gtk_icon_factory_new ();
   
   for (i = 0; i < n_stock_icons; i++) 
    {
      icon_set = gtk_icon_set_new ();
      icon_source = gtk_icon_source_new ();
      gtk_icon_source_set_filename (icon_source, stock_icons[i].filename);
      gtk_icon_set_add_source (icon_set, icon_source);
      gtk_icon_source_free (icon_source);
      gtk_icon_factory_add (icon_factory, stock_icons[i].stock_id, icon_set);
      gtk_icon_set_unref (icon_set);
    }

   gtk_icon_factory_add_default (icon_factory); 

   g_object_unref (icon_factory);

}

/*
** application setup: icons, GList, memory
*/
gboolean homebank_setup()
{
GDate *date;

	DB( g_print("\n(homebank) setup\n") );

	GLOBALS = NULL;
	PREFS   = NULL;

	/* allocate our global datas */
	DB( g_print(" -> allocate datas\n") );

	GLOBALS = g_malloc0(sizeof(struct HomeBank));
	if(!GLOBALS) return FALSE;

	PREFS   = g_malloc0(sizeof(struct Preferences));
	if(!PREFS) return FALSE;

	/* today's date */
	date = g_date_new();
	g_date_set_time(date, time(NULL));
	GLOBALS->today = g_date_get_julian(date);
	g_date_free(date);

	/* filename */
	GLOBALS->filename = NULL;

	/* initialize our GList */
	GLOBALS->acc_list = NULL;
	GLOBALS->pay_list = NULL;
	GLOBALS->cat_list = NULL;
	GLOBALS->arc_list = NULL;
	GLOBALS->ope_list = NULL;

	DB( g_print(" -> load pixbufs for list & widget\n") );
	load_list_pixbuf();
	load_paymode_icons();
	load_pref_icons();

	DB( g_print(" -> register icons\n") );
	homebank_register_stock_icons();

	//debug
	DB( g_print(" -> init prefs\n") );
	homebank_init_prefs();

	// check homedir for .homebank dir
	gchar *homedir;
	gboolean exists;

	homedir = g_strdup_printf("%s/.homebank", g_get_home_dir ());
	exists = g_file_test(homedir, G_FILE_TEST_IS_DIR);
	if(!exists)
	{
		DB( g_print(" creating %s\n", homedir) );
		g_mkdir(homedir, 0755);
	}
	else
	{
		/* load last opened file */
		DB( g_print(" -> get last opened file\n") );
	
		homebank_lastopenedfiles_load();

		DB( g_print(" -> get prefs\n") );

		homebank_pref_load();
	}
	g_free(homedir);


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

	DB( g_print(" => end\n") );

	return TRUE;
}




int main (int argc, char *argv[])
{
GtkWidget *mainwin;
GtkWidget *splash, *vbox, *version;
  gchar *ver_string, *markup;

	DB( g_print("\n--------------------------------\nhomebank starting...\n" ) );

  /* Initialize i18n support */
#ifdef ENABLE_NLS
     setlocale (LC_ALL, "");
     bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
     bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
     textdomain (GETTEXT_PACKAGE);
#endif

	/* Initialize the widget set */
	gtk_init (&argc, &argv);

	/* splash window */
	splash = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_decorated(GTK_WINDOW(splashwin),FALSE);

	gtk_window_set_type_hint (GTK_WINDOW (splash), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	//gtk_window_set_skip_taskbar_hint (GTK_WINDOW (splash), TRUE);

	gtk_window_set_title (GTK_WINDOW (splash), "HomeBank");
	gtk_window_set_position (GTK_WINDOW (splash), GTK_WIN_POS_CENTER);


	GtkWidget *image = gtk_image_new_from_file(PIXMAPS_DIR "/splash.png");

	vbox = gtk_vbox_new (FALSE, 3);

  ver_string = g_strdup_printf(_("Version: HomeBank-%s"), VERSION);

  version = gtk_label_new(NULL);
  markup = g_markup_printf_escaped(MARKUP_STRING, ver_string);
  gtk_label_set_markup(GTK_LABEL(version), markup);
  g_free(markup);
  g_free(ver_string);

	gtk_container_add (GTK_CONTAINER (splash), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), version, FALSE, FALSE, 0);


	
  gtk_window_set_auto_startup_notification (FALSE);
  gtk_widget_show_all (splash);
  gtk_window_set_auto_startup_notification (TRUE);

  /* make sure splash is up */
  while (gtk_events_pending ())
    gtk_main_iteration ();



	/* setup homebank */
	if( homebank_setup() )
	{

		//DB( g_print(" gchar=%d gshort=%d gint=%d glong=%d double=%d memalign=%d\n", sizeof(gchar), sizeof(gshort), sizeof(gint), sizeof(glong), sizeof(double), G_MEM_ALIGN ) );

		DB( g_print("** creating window\n" ) );

		gtk_window_set_default_icon_from_file(PIXMAPS_DIR "/homebank.svg", NULL);

		/* Create the main window */
		mainwin = (GtkWidget *)create_wallet_window (NULL);
		//mainwin = NULL;

		gtk_widget_destroy(splash);

		if(mainwin)
		{
			DB( g_print("** start gtk_main()\n" ) );

			/* Enter the main event loop, and wait for user interaction */
			gtk_main ();

			DB( g_print("** end gtk_main()\n" ) );
		}

	}

	/* The user lost interest */
    homebank_cleanup();

	return 0;
}
