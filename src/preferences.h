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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#define PREF_VERSION	"0.2"

//#define DEFAULT_PATH_WALLET			"/home/max/dev/gnomebank/accounts"
//#define DEFAULT_PATH_NAVIGATOR		"mozilla"

//#define DEFAULT_FORMAT_DECIMAL		"%.2f"
#define DEFAULT_FORMAT_DATE			"%x"

#define OLD_EXP_COLOR		0xE88C00
#define OLD_INC_COLOR		0x00C800
#define OLD_WARN_COLOR		0XC80000

//Tango light
#define LIGHT_EXP_COLOR		0xfcaf3e	//Orange
#define LIGHT_INC_COLOR		0x8ae234	//Chameleon
#define LIGHT_WARN_COLOR	0Xef2929	//Scarlett Red

//Tango medium
#define MEDIUM_EXP_COLOR	0xf57900	//Orange
#define MEDIUM_INC_COLOR	0x73d216	//Chameleon
#define MEDIUM_WARN_COLOR	0Xcc0000	//Scarlett Red

//Tango dark
#define DEFAULT_EXP_COLOR		0xce5c00	//Orange
#define DEFAULT_INC_COLOR		0x4e9a36	//Chameleon
#define DEFAULT_WARN_COLOR		0Xa40000	//Scarlett Red


/*
** Preference datas
*/
struct Currency
{
	gchar		*prefix_symbol;		/* max symbol is 3 digits in unicode */
	gchar		*suffix_symbol;		/* but mostly is 1 digit */
	gchar		*decimal_char;	
	gchar		*grouping_char;	
	gshort		frac_digits;
	gchar		format[8];
};

struct WinGeometry
{
	gint		l, t, w, h;
};


struct Preferences
{
	//general
	gchar		*path_wallet;
	gchar		*path_import;
	gboolean	runwizard;
	gint		filter_range;

	//interface
	gshort		toolbar_style;
	//gint		image_size;
	guint32		color_exp;
	guint32		color_inc;
	guint32		color_warn;

	//display format
	gchar		*date_format;

	struct	Currency base_cur;

	//gshort	num_nbdecimal;
	//gboolean	num_separator;
	gboolean	british_unit;

	//help system
	//gboolean	show_tooltips;
	//gboolean	show_help_button;
	//gboolean	show_tipofday;
	//gchar		*path_navigator;

	//euro zone
	gboolean	euro_active;
	gint		euro_country;
	gdouble		euro_value;

	struct	Currency minor_cur;

	//gshort		euro_nbdec;
	//gboolean	euro_thsep;
	//gchar		*euro_symbol;

	//report options
	gboolean	stat_byamount;
	gboolean	stat_showrate;
	gboolean	stat_showdetail;
	gboolean	budg_showdetail;

	//chart options
	gboolean	chart_legend;

	/* internal */
	//gint		last_page;
	gboolean 	lst_ope_columns[NUM_COL_OPE+1];

	/* windows size an position */
	struct WinGeometry	wal_wg;
	struct WinGeometry	acc_wg;
	struct WinGeometry	sta_wg;
	struct WinGeometry	ove_wg;
	struct WinGeometry	bud_wg;
	struct WinGeometry	car_wg;
};


void homebank_prefs_set_default(void);
void homebank_pref_free(void);
void homebank_pref_createformat(void);
gboolean homebank_pref_load(void);
gboolean homebank_pref_save(void);

#endif /* __PREFERENCES_H__ */
