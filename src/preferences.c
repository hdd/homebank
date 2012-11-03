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


void homebank_pref_init_monetary_win32(void)
{
	DB( g_print("(preferences) monetary win32\n") );

	//todo: to be really set by a win32 specialist from the registry...
	
	PREFS->base_cur.prefix_symbol = NULL;
	PREFS->base_cur.suffix_symbol = NULL;
	PREFS->base_cur.decimal_char  = NULL;
	PREFS->base_cur.grouping_char = NULL;
	PREFS->base_cur.frac_digits   = 2;
	
}

void homebank_pref_init_monetary_unix(void)
{
struct lconv *lc = localeconv();

	DB( g_print("(preferences) monetary unix\n") );

	DB( g_print("mon_decimal_point '%s'\n", lc->mon_decimal_point) );
	DB( g_print("mon_thousands_sep '%s'\n", lc->mon_thousands_sep) );

	DB( g_print("frac_digits '%d'\n", (gint)lc->frac_digits) );

	DB( g_print("currency_symbol '%s'\n", lc->currency_symbol) );

	DB( g_print("p_cs_precedes '%d'\n", lc->p_cs_precedes) );

	DB( g_print("n_cs_precedes '%d'\n", lc->n_cs_precedes) );


	
	if( lc->p_cs_precedes || lc->n_cs_precedes )
	{
		PREFS->base_cur.prefix_symbol = g_strdup(lc->currency_symbol);
		DB( g_print("locale mon cs is a prefix\n") );
	}
	else
		PREFS->base_cur.suffix_symbol = g_strdup(lc->currency_symbol);

	PREFS->base_cur.decimal_char  = g_strdup(lc->mon_decimal_point);	
	PREFS->base_cur.grouping_char = g_strdup(lc->mon_thousands_sep);	
	PREFS->base_cur.frac_digits   = lc->frac_digits;
	

}

void homebank_pref_free(void)
{
	DB( g_print("(preferences) free\n") );


	g_free(PREFS->date_format);

	g_free(PREFS->path_wallet);
	g_free(PREFS->path_import);
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

	DB( g_print("(preferences) pref init\n") );

	homebank_pref_free();

	PREFS->date_format = g_strdup(DEFAULT_FORMAT_DATE);

	PREFS->path_wallet = g_strdup_printf("%s", g_get_home_dir ());
	PREFS->path_import = g_strdup_printf("%s", g_get_home_dir ());
	//PREFS->path_navigator = g_strdup(DEFAULT_PATH_NAVIGATOR);


	PREFS->color_exp  = DEFAULT_EXP_COLOR;
	PREFS->color_inc  = DEFAULT_INC_COLOR;
	PREFS->color_warn = DEFAULT_WARN_COLOR;

	//PREFS->runwizard = TRUE;

	DB( g_print(" + #%x\n", PREFS->color_exp) );
	DB( g_print(" + #%x\n", PREFS->color_inc) );
	DB( g_print(" + #%x\n", PREFS->color_warn) );


#ifdef G_OS_WIN32
	homebank_pref_init_monetary_win32();
#else
	homebank_pref_init_monetary_unix();
#endif


	PREFS->lst_ope_columns[COL_OPE_STATUS] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_DATE] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_INFO] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_PAYEE] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_WORDING] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_AMOUNT] = FALSE;
	PREFS->lst_ope_columns[COL_OPE_EXPENSE] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_INCOME] = TRUE;
	PREFS->lst_ope_columns[COL_OPE_CATEGORY] = TRUE;

	//PREFS->base_cur.nbdecimal = 2;
	//PREFS->base_cur.separator = TRUE;

	PREFS->filter_range = 7;


	//todo: add intelligence here
	PREFS->euro_active  = FALSE;
	
	//todo add intelligence here

	PREFS->minor_cur.suffix_symbol = g_strdup("??");
	PREFS->minor_cur.decimal_char  = g_strdup(".");	
	PREFS->minor_cur.grouping_char = g_strdup(" ");
	PREFS->minor_cur.frac_digits   = 2;
		
	
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

	DB( g_print("(preferences) pref create format\n") );

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
	/* new fmt */
	g_snprintf(PREFS->base_cur.format , 8-1, "%%.%df", PREFS->base_cur.frac_digits);
	g_snprintf(PREFS->minor_cur.format, 8-1, "%%.%df", PREFS->minor_cur.frac_digits);

}

/*
** load preference from homedir/.homebank
*/

static homebank_pref_get_boolean(
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

static homebank_pref_get_integer(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gint *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_integer(key_file, group_name, key, NULL);
	}
}

static homebank_pref_get_short(
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

static homebank_pref_get_string(
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
			DB( g_print(" storage was not null, freeing\n") );
			
			g_free(*storage);
		}
		
		*storage = NULL;
		
		string = g_key_file_get_string(key_file, group_name, key, NULL);
		if( string != "" )
		{
			*storage = g_strdup(string);
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

	DB( g_print("(preferences) pref load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{
		filename = g_strdup_printf("%s/.homebank/preferences", g_get_home_dir ());
		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{

			group = "General";

				gchar *version = g_key_file_get_string (keyfile, group, "Version", NULL);
				DB( g_print(" version: %s\n", version) );

				homebank_pref_get_short(keyfile, group, "BarStyle" , &PREFS->toolbar_style);
				homebank_pref_get_integer(keyfile, group, "ColorExp" , &PREFS->color_exp);
				homebank_pref_get_integer(keyfile, group, "ColorInc" , &PREFS->color_inc);
				homebank_pref_get_integer(keyfile, group, "ColorWarn", &PREFS->color_warn);

				#if DOWIZARD == 1
				homebank_pref_get_boolean(keyfile, group, "RunWizard", &PREFS->runwizard);
				#endif
				
				homebank_pref_get_string(keyfile, group, "WalletPath", &PREFS->path_wallet);
				homebank_pref_get_string(keyfile, group, "ImportPath", &PREFS->path_import);
				//homebank_pref_get_string(keyfile, group, "NavigatorPath", &PREFS->path_navigator);

				
				if( g_key_file_has_key(keyfile, group, "ColumnsOpe", NULL) )
				{
				gboolean okay = TRUE;
				gint *pos, *d, i;
				gsize length;

					pos = g_key_file_get_boolean_list(keyfile, group, "ColumnsOpe",
						&length, &error);

					if( length == NUM_COL_OPE)
						memcpy(PREFS->lst_ope_columns, pos, length*sizeof(gboolean));

					g_free(pos);
				}

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
				}

				homebank_pref_get_boolean(keyfile, group, "UKUnits", &PREFS->british_unit);
				

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
				}			

			//PREFS->euro_symbol = g_locale_to_utf8(tmpstr, -1, NULL, NULL, NULL);

		//report options
			group = "Report";

				homebank_pref_get_boolean(keyfile, group, "StatByAmount", &PREFS->stat_byamount);
				homebank_pref_get_boolean(keyfile, group, "StatDetail", &PREFS->stat_showdetail);
				homebank_pref_get_boolean(keyfile, group, "StatRate", &PREFS->stat_showrate);
				homebank_pref_get_boolean(keyfile, group, "BudgDetail", &PREFS->budg_showdetail);
		

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

	//homebank_pref_createformat();

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

	DB( g_print("(preferences) pref save\n") );

	keyfile = g_key_file_new();
	if(keyfile )
	{
		group = "General";
		g_key_file_set_string  (keyfile, group, "Version", PREF_VERSION);

		g_key_file_set_integer (keyfile, group, "BarStyle", PREFS->toolbar_style);
		//g_key_file_set_integer (keyfile, group, "BarImageSize", PREFS->image_size);

		g_key_file_set_integer (keyfile, group, "ColorExp" , PREFS->color_exp);
		g_key_file_set_integer (keyfile, group, "ColorInc" , PREFS->color_inc);
		g_key_file_set_integer (keyfile, group, "ColorWarn", PREFS->color_warn);

		#if DOWIZARD == 1
		g_key_file_set_boolean (keyfile, group, "RunWizard", PREFS->runwizard);
		#endif

		g_key_file_set_string  (keyfile, group, "WalletPath"   , PREFS->path_wallet);
		g_key_file_set_string  (keyfile, group, "ImportPath"   , PREFS->path_import);
		//g_key_file_set_string  (keyfile, group, "NavigatorPath", PREFS->path_navigator);

		g_key_file_set_boolean_list(keyfile, group, "ColumnsOpe", PREFS->lst_ope_columns, NUM_COL_OPE);

		group = "Format";
		g_key_file_set_string  (keyfile, group, "DateFmt"   , PREFS->date_format);

		g_key_file_set_string  (keyfile, group, "PreSymbol" , PREFS->base_cur.prefix_symbol);
		g_key_file_set_string  (keyfile, group, "SufSymbol" , PREFS->base_cur.suffix_symbol);
		g_key_file_set_string  (keyfile, group, "DecChar"   , PREFS->base_cur.decimal_char);
		g_key_file_set_string  (keyfile, group, "GroupChar" , PREFS->base_cur.grouping_char);
		g_key_file_set_integer (keyfile, group, "FracDigits", PREFS->base_cur.frac_digits);

		g_key_file_set_boolean (keyfile, group, "UKUnits" , PREFS->british_unit);

		group = "Filter";
		g_key_file_set_integer (keyfile, group, "DefRange", PREFS->filter_range);

	//euro options
		group = "Euro";
		g_key_file_set_boolean (keyfile, group, "Active" , PREFS->euro_active);
		g_key_file_set_integer (keyfile, group, "Country", PREFS->euro_country);
		gchar ratestr[64];
		g_ascii_dtostr(ratestr, 63, PREFS->euro_value);
		g_key_file_set_string  (keyfile, group, "ChangeRate", ratestr);
		g_key_file_set_string  (keyfile, group, "PreSymbol" , PREFS->minor_cur.prefix_symbol);
		g_key_file_set_string  (keyfile, group, "SufSymbol" , PREFS->minor_cur.suffix_symbol);
		g_key_file_set_string  (keyfile, group, "DecChar"   , PREFS->minor_cur.decimal_char);
		g_key_file_set_string  (keyfile, group, "GroupChar" , PREFS->minor_cur.grouping_char);
		g_key_file_set_integer (keyfile, group, "FracDigits", PREFS->minor_cur.frac_digits);

	//report options
		group = "Report";
		g_key_file_set_boolean (keyfile, group, "StatByAmount", PREFS->stat_byamount);
		g_key_file_set_boolean (keyfile, group, "StatDetail"  , PREFS->stat_showdetail);
		g_key_file_set_boolean (keyfile, group, "StatRate"    , PREFS->stat_showrate);
		g_key_file_set_boolean (keyfile, group, "BudgDetail"  , PREFS->budg_showdetail);

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

