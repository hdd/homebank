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


};


void homebank_prefs_set_default(void);
void homebank_pref_free(void);
void homebank_pref_createformat(void);
gboolean homebank_pref_load(void);
gboolean homebank_pref_save(void);

#endif /* __PREFERENCES_H__ */
