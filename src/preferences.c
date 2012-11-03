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
#include "preferences.h"
#include "def_pref.h"

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


static void homebank_pref_init_monetary(void)
{
	DB( g_print("\n** (preferences) monetary\n") );


#ifdef G_OS_UNIX

struct lconv *lc = localeconv();

	DB( g_print("\n** (preferences) monetary unix\n") );

	DB( g_print("mon_decimal_point is utf8: %d\n", g_utf8_validate(lc->mon_decimal_point, -1, NULL)) );
	DB( g_print("mon_decimal_point '%s'\n", lc->mon_decimal_point) );
	DB( g_print("mon_decimal_point %x %x %x %x\n", lc->mon_decimal_point[0], lc->mon_decimal_point[1], lc->mon_decimal_point[2], lc->mon_decimal_point[3]) );

	DB( g_print("mon_thousands_sep is utf8: %d\n", g_utf8_validate(lc->mon_thousands_sep, -1, NULL)) );
	DB( g_print("mon_thousands_sep '%s'\n", lc->mon_thousands_sep) );
	DB( g_print("mon_thousands_sep %x %x %x %x\n", lc->mon_thousands_sep[0], lc->mon_thousands_sep[1], lc->mon_thousands_sep[2], lc->mon_thousands_sep[3]) );


	DB( g_print("frac_digits '%d'\n", (gint)lc->frac_digits) );

	DB( g_print("currency_symbol '%s'\n", lc->currency_symbol) );

	DB( g_print("p_cs_precedes '%d'\n", lc->p_cs_precedes) );

	DB( g_print("n_cs_precedes '%d'\n", lc->n_cs_precedes) );

	/* ok assign */

	
	if( lc->p_cs_precedes || lc->n_cs_precedes )
	{
		PREFS->base_cur.prefix_symbol = g_strdup(lc->currency_symbol);
		PREFS->base_cur.suffix_symbol = NULL; //g_strdup("");
		DB( g_print("locale mon cs is a prefix\n") );
	}
	else
	{
		PREFS->base_cur.prefix_symbol = NULL; //g_strdup("");
		PREFS->base_cur.suffix_symbol = g_strdup(lc->currency_symbol);
	}

	PREFS->base_cur.decimal_char  = g_strdup(lc->mon_decimal_point);	
	
	PREFS->base_cur.grouping_char = g_strdup(lc->mon_thousands_sep);	
	
	//todo:fix
	//PREFS->base_cur.grouping_char = g_locale_to_utf8(lc->mon_thousands_sep, -1, NULL, NULL, NULL);
	//PREFS->base_cur.grouping_char = g_convert (lc->mon_thousands_sep, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
	
	DB( g_print(" -> grouping_char: '%s'\n", PREFS->base_cur.grouping_char) );
	
	PREFS->base_cur.frac_digits   = lc->frac_digits;

	//fix 378992/421228
	if( PREFS->base_cur.frac_digits > MAX_FRAC_DIGIT )
		PREFS->base_cur.frac_digits = MAX_FRAC_DIGIT;


#else
	#ifdef G_OS_WIN32
	//todo: to be really set by a win32 specialist from the registry...
	
	PREFS->base_cur.prefix_symbol = NULL; //g_strdup("");
	PREFS->base_cur.suffix_symbol = NULL; //g_strdup("");
	PREFS->base_cur.decimal_char  = g_strdup(".");
	PREFS->base_cur.grouping_char = NULL; //g_strdup("");
	PREFS->base_cur.frac_digits   = 2;
	#else
	PREFS->base_cur.prefix_symbol = NULL; //g_strdup("");
	PREFS->base_cur.suffix_symbol = NULL; //g_strdup("");
	PREFS->base_cur.decimal_char  = g_strdup(".");
	PREFS->base_cur.grouping_char = NULL; //g_strdup("");
	PREFS->base_cur.frac_digits   = 2;
	#endif
#endif
	
}



static void homebank_pref_init_wingeometry(struct WinGeometry *wg, gint l, gint t, gint w, gint h)
{
	wg->l = l;
	wg->t = t;
	wg->w = w;
	wg->h = h;
}


void homebank_pref_init_measurement_units(void)
{

	if(PREFS->imperial_unit == TRUE)
	{
		////TRANSLATORS: carcost for distance (Miles)
		PREFS->car_unit_dist = "%d m.";

		////TRANSLATORS: carcost for volume (Galons)
		PREFS->car_unit_vol  = "%.2f gal";
	
		////TRANSLATORS: carcost label for '100 miles'
		PREFS->car_unit_100  = "100 miles";
	}
	else
	{
		////TRANSLATORS: carcost for distance (Kilometer)
		PREFS->car_unit_dist = "%d km";

		////TRANSLATORS: carcost for volume (Liters)
		PREFS->car_unit_vol  = "%.2f L";
	
		////TRANSLATORS: carcost label for '100 km'
		PREFS->car_unit_100  = "100 Km";
	}

}

void homebank_pref_free(void)
{
	DB( g_print("\n** (preferences) free\n") );


	g_free(PREFS->date_format);

	g_free(PREFS->color_exp);
	g_free(PREFS->color_inc);
	g_free(PREFS->color_warn);

	g_free(PREFS->path_wallet);
	g_free(PREFS->path_import);
	g_free(PREFS->path_export);
	//g_free(PREFS->path_navigator);

	g_free(PREFS->base_cur.prefix_symbol);
	g_free(PREFS->base_cur.suffix_symbol);
	g_free(PREFS->base_cur.decimal_char);	
	g_free(PREFS->base_cur.grouping_char);	

	g_free(PREFS->minor_cur.prefix_symbol);
	g_free(PREFS->minor_cur.suffix_symbol);
	g_free(PREFS->minor_cur.decimal_char);	
	g_free(PREFS->minor_cur.grouping_char);	

	memset(PREFS, 0, sizeof(struct Preferences));
}


void homebank_pref_setdefault(void)
{
gint i;

	DB( g_print("\n** (preferences) pref init\n") );

	homebank_pref_free();

	PREFS->date_format = g_strdup(DEFAULT_FORMAT_DATE);

	PREFS->path_wallet = g_strdup_printf("%s", g_get_home_dir ());
	PREFS->path_import = g_strdup_printf("%s", g_get_home_dir ());
	PREFS->path_export = g_strdup_printf("%s", g_get_home_dir ());
	//PREFS->path_navigator = g_strdup(DEFAULT_PATH_NAVIGATOR);

	PREFS->loadlast = TRUE;
	PREFS->showsplash = TRUE;
	PREFS->heritdate = FALSE;

	PREFS->toolbar_style = 0;
	PREFS->custom_colors = TRUE;
	PREFS->color_exp  = g_strdup(DEFAULT_EXP_COLOR);
	PREFS->color_inc  = g_strdup(DEFAULT_INC_COLOR);
	PREFS->color_warn = g_strdup(DEFAULT_WARN_COLOR);
	PREFS->rules_hint = FALSE;

	/* windows position/size */
	homebank_pref_init_wingeometry(&PREFS->wal_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->acc_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->sta_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->tme_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->ove_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->bud_wg, 0, 0, 640, 480);
	homebank_pref_init_wingeometry(&PREFS->car_wg, 0, 0, 640, 480);

	PREFS->acc_wg.l = 20;
	PREFS->acc_wg.t = 20;
	PREFS->acc_wg.w = 640;
	PREFS->acc_wg.h = 480;

	homebank_pref_init_monetary();

	PREFS->wal_toolbar = TRUE;
	PREFS->wal_statusbar = TRUE;
	PREFS->wal_upcoming = TRUE;
	PREFS->wal_vpaned = 0;


	i = 0;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_STATUS;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_DATE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INFO;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_PAYEE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_WORDING;
	PREFS->lst_ope_columns[i++] = -LST_DSPOPE_AMOUNT;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_EXPENSE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INCOME;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_CATEGORY;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_TAGS;


	PREFS->lst_ope_sort_id    = LST_DSPOPE_DATE;
	PREFS->lst_ope_sort_order = GTK_SORT_ASCENDING;


	//PREFS->base_cur.nbdecimal = 2;
	//PREFS->base_cur.separator = TRUE;

	PREFS->filter_range = 7;


	//todo: add intelligence here
	PREFS->euro_active  = FALSE;
	
	PREFS->euro_country = 0;
	PREFS->euro_value   = 1.0;
	//PREFS->euro_nbdec   = 2;
	//PREFS->euro_thsep   = TRUE;
	//PREFS->euro_symbol	= g_strdup("??");

	PREFS->stat_byamount   = FALSE;
	PREFS->stat_showdetail = FALSE;
	PREFS->stat_showrate   = FALSE;
	PREFS->budg_showdetail = FALSE;

	PREFS->chart_legend = FALSE;




}

/*
** create the format string for monetary strfmon (major/minor)
*/
void homebank_pref_createformat(void)
{
struct Currency *cur;
gchar *ptr;

	DB( g_print("\n** (preferences) pref create format\n") );

/*
	if(PREFS->base_cur.grouping_char != NULL)
		g_snprintf(GLOBALS->fmt_maj_number, 15, "%%^.%dn", PREFS->base_cur.frac_digits);
	else
		g_snprintf(GLOBALS->fmt_maj_number, 15, "%%.%dn", PREFS->base_cur.frac_digits);

	DB( g_print("+ major is: '%s'\n", GLOBALS->fmt_maj_number) );


	if(PREFS->minor_cur.grouping_char != NULL)
		g_snprintf(GLOBALS->fmt_min_number, 15, "%s %%!^.%dn %s",
			PREFS->minor_cur.prefix_symbol,
			PREFS->minor_cur.frac_digits,
			PREFS->minor_cur.suffix_symbol
			);
	else
		g_snprintf(GLOBALS->fmt_min_number, 15, "%s %%!.%dn %s",
			PREFS->minor_cur.prefix_symbol,
			PREFS->minor_cur.frac_digits,
			PREFS->minor_cur.suffix_symbol
			);

	DB( g_print("+ minor is: '%s'\n", GLOBALS->fmt_min_number) );
*/
	/* base format */
	cur = &PREFS->base_cur;

	g_snprintf(cur->format , 8-1, "%%.%df", cur->frac_digits);

	ptr = cur->monfmt;
	if(cur->prefix_symbol != NULL)
	{
		ptr = g_stpcpy(ptr, cur->prefix_symbol);
		ptr = g_stpcpy(ptr, " ");
	}
	ptr = g_stpcpy(ptr, "%s");
	if(cur->suffix_symbol != NULL)
	{
		ptr = g_stpcpy(ptr, " ");
		ptr = g_stpcpy(ptr, cur->suffix_symbol);
	}

	cur = &PREFS->minor_cur;

	g_snprintf(cur->format , 8-1, "%%.%df", cur->frac_digits);

	ptr = cur->monfmt;
	if(cur->prefix_symbol != NULL)
	{
		ptr = g_stpcpy(ptr, cur->prefix_symbol);
		ptr = g_stpcpy(ptr, " ");
	}
	ptr = g_stpcpy(ptr, "%s");
	if(cur->suffix_symbol != NULL)
	{
		ptr = g_stpcpy(ptr, " ");
		ptr = g_stpcpy(ptr, cur->suffix_symbol);
	}

}

/*
** load preference from homedir/.homebank (HB_DATA_PATH)
*/
static void homebank_pref_get_wingeometry(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	struct WinGeometry *storage)
{
	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
	gint *wg;
	gsize length;
	
		wg = g_key_file_get_integer_list(key_file, group_name, key, &length, NULL);
		memcpy(storage, wg, 4*sizeof(gint));
		g_free(wg);
	}
}




static void homebank_pref_get_boolean(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gboolean *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_boolean(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_integer(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gint *storage)
{

	DB( g_print(" search %s in %s\n", key, group_name) );


	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_integer(key_file, group_name, key, NULL);
		
		DB( g_print(" store integer %d for %s at %x\n", *storage, key, *storage) );
	}
}

static void homebank_pref_get_guint32(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	guint32 *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_integer(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_short(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gshort *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = (gshort)g_key_file_get_integer(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_string(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gchar **storage)
{
gchar *string;

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		/* free any previous string */
		if( *storage != NULL )
		{
			//DB( g_print(" storage was not null, freeing\n") );
			
			g_free(*storage);
		}
		
		*storage = NULL;
		
		string = g_key_file_get_string(key_file, group_name, key, NULL);
		if( string != NULL )
		{
			*storage = g_strdup(string);
			
			//DB( g_print(" store '%s' for %s at %x\n", string, key, *storage) );
		}
	}

/*
	if (error)
    {
      g_warning ("error: %s\n", error->message);
      g_error_free(error);
      error = NULL;
    }
*/


}


gboolean homebank_pref_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
GError *error = NULL;

	DB( g_print("\n** (preferences) pref load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{
		filename = g_build_filename(homebank_app_get_config_dir(), "preferences", NULL );

		DB( g_print(" filename: %s\n", filename) );


		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{

			group = "General";

				gchar *version = g_key_file_get_string (keyfile, group, "Version", NULL);
				DB( g_print(" version: %s\n", version) );

				homebank_pref_get_short(keyfile, group, "BarStyle" , &PREFS->toolbar_style);

				if(!strcmp("0.2", version))	// retrieve old settings
				{				
				guint32 color;
					
					homebank_pref_get_guint32(keyfile, group, "ColorExp" , &color);
					g_free(PREFS->color_exp);
					PREFS->color_exp = g_strdup_printf("#%06x", color);
				
					homebank_pref_get_guint32(keyfile, group, "ColorInc" , &color);
					g_free(PREFS->color_inc);
					PREFS->color_inc = g_strdup_printf("#%06x", color);

					homebank_pref_get_guint32(keyfile, group, "ColorWarn", &color);
					g_free(PREFS->color_warn);
					PREFS->color_warn = g_strdup_printf("#%06x", color);
				}
				else
				{
					homebank_pref_get_boolean(keyfile, group, "CustomColors", &PREFS->custom_colors);
					
					homebank_pref_get_string(keyfile, group, "ColorExp" , &PREFS->color_exp);
					homebank_pref_get_string(keyfile, group, "ColorInc" , &PREFS->color_inc);
					homebank_pref_get_string(keyfile, group, "ColorWarn", &PREFS->color_warn);

					homebank_pref_get_boolean(keyfile, group, "RulesHint", &PREFS->rules_hint);
				}

				DB( g_print(" color exp: %s\n", PREFS->color_exp) );
				DB( g_print(" color inc: %s\n", PREFS->color_inc) );
				DB( g_print(" color wrn: %s\n", PREFS->color_warn) );


				homebank_pref_get_string(keyfile, group, "WalletPath", &PREFS->path_wallet);
				homebank_pref_get_string(keyfile, group, "ImportPath", &PREFS->path_import);
				homebank_pref_get_string(keyfile, group, "ExportPath", &PREFS->path_export);

				homebank_pref_get_boolean(keyfile, group, "LoadLast", &PREFS->loadlast);
				homebank_pref_get_boolean(keyfile, group, "ShowSplash", &PREFS->showsplash);
				homebank_pref_get_boolean(keyfile, group, "HeritDate", &PREFS->heritdate);
				
				if( g_key_file_has_key(keyfile, group, "ColumnsOpe", NULL) )
				{
				gboolean *bsrc;
				gint *src, i;
				gsize length;

					if(!strcmp("0.2", version))	//retrieve old 0.1 or 0.2 visibility boolean
					{
						bsrc = g_key_file_get_boolean_list(keyfile, group, "ColumnsOpe", &length, &error);
						if( length == NUM_LST_DSPOPE-1 )
						{
							//and convert
							for(i=0; i<NUM_LST_DSPOPE-1 ; i++)
							{
								PREFS->lst_ope_columns[i] = (bsrc[i] == TRUE) ? i+1 : -(i+1);
							}
						}
						g_free(bsrc);
					}
					else
					{
						src = g_key_file_get_integer_list(keyfile, group, "ColumnsOpe", &length, &error);
						if( length == NUM_LST_DSPOPE-1 )
							memcpy(PREFS->lst_ope_columns, src, length*sizeof(gint));
						g_free(src);
					}

				}
				
				homebank_pref_get_integer(keyfile, group, "OpeSortId", &PREFS->lst_ope_sort_id);
				homebank_pref_get_integer(keyfile, group, "OpeSortOrder", &PREFS->lst_ope_sort_order);

			    DB( g_print("set sort to %d %d\n", PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order) );


			group = "Windows";

				homebank_pref_get_wingeometry(keyfile, group, "Wal", &PREFS->wal_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Acc", &PREFS->acc_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Sta", &PREFS->sta_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Tme", &PREFS->tme_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Ove", &PREFS->ove_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Bud", &PREFS->bud_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Car", &PREFS->car_wg);

				homebank_pref_get_integer(keyfile, group, "WalVPaned", &PREFS->wal_vpaned);
				homebank_pref_get_boolean(keyfile, group, "WalToolbar", &PREFS->wal_toolbar);
				homebank_pref_get_boolean(keyfile, group, "WalStatusbar", &PREFS->wal_statusbar);
				homebank_pref_get_boolean(keyfile, group, "WalUpcoming", &PREFS->wal_upcoming);


			group = "Format";

				homebank_pref_get_string(keyfile, group, "DateFmt", &PREFS->date_format);
			
				if(!strcmp("0.1", version))
				{
					//retrieve old 0.1 preferences
					homebank_pref_get_short(keyfile, group, "NumNbDec", &PREFS->base_cur.frac_digits);
					//PREFS->base_cur.separator = g_key_file_get_boolean (keyfile, group, "NumSep", NULL);
				}
				else
				{
					homebank_pref_get_string(keyfile, group, "PreSymbol", &PREFS->base_cur.prefix_symbol);
					homebank_pref_get_string(keyfile, group, "SufSymbol", &PREFS->base_cur.suffix_symbol);
					homebank_pref_get_string(keyfile, group, "DecChar"  , &PREFS->base_cur.decimal_char);
					homebank_pref_get_string(keyfile, group, "GroupChar", &PREFS->base_cur.grouping_char);
					homebank_pref_get_short(keyfile, group, "FracDigits", &PREFS->base_cur.frac_digits);

					//fix 378992/421228
					if( PREFS->base_cur.frac_digits > MAX_FRAC_DIGIT )
						PREFS->base_cur.frac_digits = MAX_FRAC_DIGIT;
				}

				homebank_pref_get_boolean(keyfile, group, "UKUnits", &PREFS->imperial_unit);
				

			group = "Filter";

				homebank_pref_get_integer(keyfile, group, "DefRange", &PREFS->filter_range);


			group = "Euro";

				homebank_pref_get_boolean(keyfile, group, "Active", &PREFS->euro_active);
				homebank_pref_get_integer(keyfile, group, "Country", &PREFS->euro_country);

				gchar *ratestr = g_key_file_get_string (keyfile, group, "ChangeRate", NULL);
				if(ratestr != NULL) PREFS->euro_value = g_ascii_strtod(ratestr, NULL);
				
				if(!strcmp("0.1", version))
				{
					homebank_pref_get_string(keyfile, group, "Symbol", &PREFS->minor_cur.suffix_symbol);
					PREFS->minor_cur.frac_digits = g_key_file_get_integer (keyfile, group, "NBDec", NULL);

					//PREFS->euro_nbdec = g_key_file_get_integer (keyfile, group, "NBDec", NULL);
					//PREFS->euro_thsep = g_key_file_get_boolean (keyfile, group, "Sep", NULL);
					//gchar *tmpstr = g_key_file_get_string  (keyfile, group, "Symbol", &error);
				}
				else
				{
					homebank_pref_get_string(keyfile, group, "PreSymbol", &PREFS->minor_cur.prefix_symbol);
					homebank_pref_get_string(keyfile, group, "SufSymbol", &PREFS->minor_cur.suffix_symbol);
					homebank_pref_get_string(keyfile, group, "DecChar"  , &PREFS->minor_cur.decimal_char);
					homebank_pref_get_string(keyfile, group, "GroupChar", &PREFS->minor_cur.grouping_char);
					homebank_pref_get_short(keyfile, group, "FracDigits", &PREFS->minor_cur.frac_digits);

					//fix 378992/421228
					if( PREFS->minor_cur.frac_digits > MAX_FRAC_DIGIT )
						PREFS->minor_cur.frac_digits = MAX_FRAC_DIGIT;

				}			

			//PREFS->euro_symbol = g_locale_to_utf8(tmpstr, -1, NULL, NULL, NULL);

			group = "Report";

				homebank_pref_get_boolean(keyfile, group, "StatByAmount", &PREFS->stat_byamount);
				homebank_pref_get_boolean(keyfile, group, "StatDetail", &PREFS->stat_showdetail);
				homebank_pref_get_boolean(keyfile, group, "StatRate", &PREFS->stat_showrate);
				homebank_pref_get_boolean(keyfile, group, "BudgDetail", &PREFS->budg_showdetail);

			group = "Exchange";
				homebank_pref_get_integer(keyfile, group, "OfxMemo", &PREFS->dtex_ofxmemo);


			//group = "Chart";
			//PREFS->chart_legend = g_key_file_get_boolean (keyfile, group, "Legend", NULL);


			/*
			#if MYDEBUG == 1
			gsize length;
			gchar *contents = g_key_file_to_data (keyfile, &length, NULL);
			g_print(" keyfile:\n%s\n len=%d\n", contents, length);
			g_free(contents);
			#endif
			*/

		}
		g_free(filename);
		g_key_file_free (keyfile);
	}

	return retval;
}

static void homebank_pref_set_string(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gchar *string)
{

	DB( g_print(" -> homebank_pref_set_string :: group='%s' key='%s' value='%s'\n", group_name, key, string) );

	if( string != NULL && *string != '\0')
		g_key_file_set_string  (key_file, group_name, key, string);
	else
		g_key_file_set_string  (key_file, group_name, key, "");
		
}


/*
** save preference to homedir/.homebank (HB_DATA_PATH)
*/
gboolean homebank_pref_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
gsize length;

	DB( g_print("\n** (preferences) pref save\n") );

	keyfile = g_key_file_new();
	if(keyfile )
	{

		DB( g_print(" -> ** general\n") );


		group = "General";
		homebank_pref_set_string  (keyfile, group, "Version", PREF_VERSION);

		g_key_file_set_integer (keyfile, group, "BarStyle", PREFS->toolbar_style);
		//g_key_file_set_integer (keyfile, group, "BarImageSize", PREFS->image_size);

		g_key_file_set_boolean (keyfile, group, "CustomColors", PREFS->custom_colors);
		g_key_file_set_string (keyfile, group, "ColorExp" , PREFS->color_exp);
		g_key_file_set_string (keyfile, group, "ColorInc" , PREFS->color_inc);
		g_key_file_set_string (keyfile, group, "ColorWarn", PREFS->color_warn);

		g_key_file_set_boolean (keyfile, group, "RulesHint", PREFS->rules_hint);


		homebank_pref_set_string  (keyfile, group, "WalletPath"   , PREFS->path_wallet);
		homebank_pref_set_string  (keyfile, group, "ImportPath"   , PREFS->path_import);
		homebank_pref_set_string  (keyfile, group, "ExportPath"   , PREFS->path_export);
		//g_key_file_set_string  (keyfile, group, "NavigatorPath", PREFS->path_navigator);

		g_key_file_set_boolean (keyfile, group, "LoadLast", PREFS->loadlast);
		g_key_file_set_boolean (keyfile, group, "ShowSplash", PREFS->showsplash);
		g_key_file_set_boolean (keyfile, group, "HeritDate", PREFS->heritdate);

		g_key_file_set_integer_list(keyfile, group, "ColumnsOpe", PREFS->lst_ope_columns, NUM_LST_DSPOPE-1);
		g_key_file_set_integer     (keyfile, group, "OpeSortId" , PREFS->lst_ope_sort_id);
		g_key_file_set_integer     (keyfile, group, "OpeSortOrder" , PREFS->lst_ope_sort_order);

		// added v3.4
		DB( g_print(" -> ** windows\n") );

		group = "Windows";
		g_key_file_set_integer_list(keyfile, group, "Wal", (gint *)&PREFS->wal_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Acc", (gint *)&PREFS->acc_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Sta", (gint *)&PREFS->sta_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Tme", (gint *)&PREFS->tme_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Ove", (gint *)&PREFS->ove_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Bud", (gint *)&PREFS->bud_wg, 4);
		g_key_file_set_integer_list(keyfile, group, "Car", (gint *)&PREFS->car_wg, 4);

		g_key_file_set_integer (keyfile, group, "WalVPaned" , PREFS->wal_vpaned);
		g_key_file_set_boolean (keyfile, group, "WalToolbar", PREFS->wal_toolbar);
		g_key_file_set_boolean (keyfile, group, "WalStatusbar", PREFS->wal_statusbar);
		g_key_file_set_boolean (keyfile, group, "WalUpcoming", PREFS->wal_upcoming);


		DB( g_print(" -> ** format\n") );

		group = "Format";
		homebank_pref_set_string  (keyfile, group, "DateFmt"   , PREFS->date_format);

		homebank_pref_set_string  (keyfile, group, "PreSymbol" , PREFS->base_cur.prefix_symbol);
		homebank_pref_set_string  (keyfile, group, "SufSymbol" , PREFS->base_cur.suffix_symbol);
		homebank_pref_set_string  (keyfile, group, "DecChar"   , PREFS->base_cur.decimal_char);
		homebank_pref_set_string  (keyfile, group, "GroupChar" , PREFS->base_cur.grouping_char);
		g_key_file_set_integer (keyfile, group, "FracDigits", PREFS->base_cur.frac_digits);

		g_key_file_set_boolean (keyfile, group, "UKUnits" , PREFS->imperial_unit);

		DB( g_print(" -> ** filter\n") );

		group = "Filter";
		g_key_file_set_integer (keyfile, group, "DefRange", PREFS->filter_range);

		DB( g_print(" -> ** euro\n") );

	//euro options
		group = "Euro";
		g_key_file_set_boolean (keyfile, group, "Active" , PREFS->euro_active);
		if( PREFS->euro_active )
		{	
			g_key_file_set_integer (keyfile, group, "Country", PREFS->euro_country);
			gchar ratestr[64];
			g_ascii_dtostr(ratestr, 63, PREFS->euro_value);
			homebank_pref_set_string  (keyfile, group, "ChangeRate", ratestr);
			homebank_pref_set_string  (keyfile, group, "PreSymbol" , PREFS->minor_cur.prefix_symbol);
			homebank_pref_set_string  (keyfile, group, "SufSymbol" , PREFS->minor_cur.suffix_symbol);
			homebank_pref_set_string  (keyfile, group, "DecChar"   , PREFS->minor_cur.decimal_char);
			homebank_pref_set_string  (keyfile, group, "GroupChar" , PREFS->minor_cur.grouping_char);
			g_key_file_set_integer (keyfile, group, "FracDigits", PREFS->minor_cur.frac_digits);
		}

	//report options
		DB( g_print(" -> ** report\n") );

		group = "Report";
		g_key_file_set_boolean (keyfile, group, "StatByAmount", PREFS->stat_byamount);
		g_key_file_set_boolean (keyfile, group, "StatDetail"  , PREFS->stat_showdetail);
		g_key_file_set_boolean (keyfile, group, "StatRate"    , PREFS->stat_showrate);
		g_key_file_set_boolean (keyfile, group, "BudgDetail"  , PREFS->budg_showdetail);


		group = "Exchange";
		g_key_file_set_integer (keyfile, group, "OfxMemo", PREFS->dtex_ofxmemo);

		//group = "Chart";
		//g_key_file_set_boolean (keyfile, group, "Legend", PREFS->chart_legend);

		//g_key_file_set_string  (keyfile, group, "", PREFS->);
		//g_key_file_set_boolean (keyfile, group, "", PREFS->);
		//g_key_file_set_integer (keyfile, group, "", PREFS->);

		DB( g_print(" -> ** g_key_file_to_data\n") );

		gchar *contents = g_key_file_to_data (keyfile, &length, NULL);

		//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

		filename = g_build_filename(homebank_app_get_config_dir(), "preferences", NULL );

		DB( g_print(" -> filename: %s\n", filename) );

		g_file_set_contents(filename, contents, length, NULL);

		DB( g_print(" -> contents: %s\n", contents) );

		DB( g_print(" -> freeing filename\n") );
		g_free(filename);

		DB( g_print(" -> freeing buffer\n") );

		g_free(contents);

		DB( g_print(" -> freeing keyfile\n") );

		g_key_file_free (keyfile);
	}

	return retval;
}

