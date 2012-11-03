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

extern *homebank_pixmaps_dir;

struct defpref_data
{
	GtkWidget	*LV_page;
	GtkWidget	*GR_page;

	GtkWidget	*label;
	GtkWidget	*image;

	GtkWidget	*CY_toolbar;
	//GtkWidget	*NB_image_size;

	GtkWidget	*CP_exp_color;
	GtkWidget	*CP_inc_color;
	GtkWidget	*CP_warn_color;
	GtkWidget	*LV_opecolumns;
	GtkWidget	*BT_go_up;
	GtkWidget	*BT_go_down;

	GtkWidget	*CM_runwizard;

	GtkWidget	*ST_path_wallet, *BT_path_wallet;
	GtkWidget	*ST_path_import, *BT_path_import;

	//GtkWidget	*ST_path_navigator;

	GtkWidget	*ST_datefmt;
	GtkWidget	*LB_date;
	
	GtkWidget	*ST_num_presymbol;
	GtkWidget	*ST_num_sufsymbol;
	GtkWidget	*ST_num_decimalchar;	
	GtkWidget	*ST_num_groupingchar;	
	GtkWidget	*NB_num_fracdigits;
	GtkWidget	*LB_numberbase;

	//GtkWidget	*NB_numnbdec;
	//GtkWidget	*CM_numseparator;
	GtkWidget	*CM_british;

	GtkWidget	*CY_range;

	GtkWidget	*CM_euro_enable;
	GtkWidget	*CY_euro_preset;
	GtkWidget	*ST_euro_country;
	GtkWidget	*NB_euro_value;

	GtkWidget	*ST_euro_presymbol;
	GtkWidget	*ST_euro_sufsymbol;
	GtkWidget	*ST_euro_decimalchar;	
	GtkWidget	*ST_euro_groupingchar;	
	GtkWidget	*NB_euro_fracdigits;
	GtkWidget	*LB_numbereuro;

	//GtkWidget	*ST_euro_symbol;
	//GtkWidget	*NB_euro_nbdec;
	//GtkWidget	*CM_euro_thsep;

	GtkWidget	*CM_stat_byamount;
	GtkWidget	*CM_stat_showdetail;
	GtkWidget	*CM_stat_showrate;
	
	GtkWidget	*CM_budg_showdetail;	

	GtkWidget	*CM_chartlegend;

};

enum {
	LST_PREF_SMALLPIXBUF,
	LST_PREF_ICON,
	LST_PREF_NAME,
	LST_PREF_PAGE,
	LST_PREF_MAX
};


enum
{
	PREF_GENERAL,
	PREF_INTERFACE,
	PREF_COLUMNS,
	PREF_DISPLAY,
//	PREF_HELP,
	PREF_EURO,
	PREF_REPORT,
	PREF_MAX
};

enum
{
  COLUMN_VISIBLE,
  COLUMN_NAME,
  COLUMN_ID,
  NUM_COLUMNS
};

GdkPixbuf *pref_pixbuf[PREF_MAX];


static gchar *pref_pixname[PREF_MAX] = {
"prf_general.svg",
"prf_interface.svg",
"prf_columns.svg",
"prf_display.svg",
//"prf_help.svg",
"prf_euro.svg",
"prf_report.svg"
//"prf_charts.svg"
};

static gchar *pref_name[PREF_MAX]    = {
N_("General"),
N_("Interface"),
N_("Columns"),
N_("Display format"),
//N_("Help system"),
N_("Euro options"),
N_("Report options"),
//
};

gchar *CYA_TOOLBAR_STYLE[] = {
N_("System defaults"),
N_("Icons only"),
N_("Text only"),
N_("Text under icons"),
N_("Text beside icons"),
NULL
};

extern gchar *CYA_RANGE[];

typedef struct
{
	gchar		*iso;
	gchar		*name;
	gdouble		value;
	gchar		*prefix_symbol;		/* max symbol is 3 digits in unicode */
	gchar		*suffix_symbol;		/* but mostly is 1 digit */
	gchar		*decimal_char;	
	gchar		*grouping_char;	
	gshort		frac_digits;
} EuroParams;


/*
source:
 http://www.xe.com/euro.php
 http://fr.wikipedia.org/wiki/Liste_des_unit%C3%A9s_mon%C3%A9taires_remplac%C3%A9es_par_l%27euro
 http://www.inter-locale.com/LocalesDemo.jsp
*/
static EuroParams euro_params[] =
{
//                                        pre  , suf   , dec, grp, frac    
// ---------------------------------------------------------------------
	{ ""   , "--------"      , 1.0		, ""   , ""    , ",", ".", 2  },
	{ "ATS", "Austria"       , 13.7603	, "S"  , ""    , ",", ".", 2  },	// -S 1.234.567,89
	{ "BEF", "Belgium"       , 40.3399	, "BF" , ""    , ",", ".", 2  },	// BF 1.234.567,89 -
	{ "FIM", "Finland"       , 5.94573	, ""   , "mk"  , ",", " ", 2  },	// -1 234 567,89 mk
	{ "FRF", "France"        , 6.55957	, ""   , "F"   , ",", " ", 2  },	// -1 234 567,89 F
	{ "DEM", "Germany"       , 1.95583	, ""   , "DM"  , ",", ".", 2  },	// -1.234.567,89 DM
	{ "GRD", "Greece"        , 340.750	, "d"  , ""    , ".", ",", 2  },	// ??
	{ "IEP", "Ireland"       , 0.787564 , "£"  , ""    , ".", ",", 2  },	// -£1,234,567.89
	{ "ITL", "Italy"         , 1936.27	, "L"  , ""    , "" , ".", 0  },	// L -1.234.567
	{ "LUF", "Luxembourg"    , 40.3399	, "LU" , ""    , ",", ".", 2  },	// LU 1.234.567,89 -
	{ "NLG", "Netherlands"   , 2.20371	, "F"  , ""    , ",", ".", 2  },	// F 1.234.567,89-
	{ "PTE", "Portugal"      , 200.482	, ""   , "Esc.", "$", ".", 2  },	// -1.234.567$89 Esc.
	{ "ESP", "Spain"         , 166.386	, "Pts", ""    , "" , ".", 0  },	// -Pts 1.234.567
/* 2007 */
	{ "SIT", "Slovenia"      , 239.640	, "tol", ""    , ",", ".", 2  },	//
/* 2008 */
//-	{ "EEK", "Estonia"       , 15.6466  , ""   , "kr"  , ",", " ", 2  },	//
//-	{ "LVL", "Lettonia"      , 0.702804 , ""   , ""    , ",", "", 2  },	//
//-	{ "CYP", "Cyprus"        , 0.585274 , ""   , ""    , ",", "", 2  },	//
//-	{ "MTL", "Malta"         , 0.429300 , ""   , ""    , ",", "", 2  },	//
/* 2009 */
//	{ "SKK", "Slovaquia"     , 38.4550  , ""   , "Sk"  , ",", " ", 2  },	//
//	{ "BGN", "Bulgaria"      , 1.00000	, "лв.", ""    , ",", " ", 2  },	//
/* 2010 */
//	{ "CZK", "Czech republic", 1.00000	, ""   , "Kč"  , ",", " ", 2  },	//
//	{ "HUF", "Hungary"       , 1.00000	, "Ft" , ""    , ",", " ", 2  },	//
/* 2012 */
//	{ "RON", "Romania"       , 1.00000	, ""   , "LEI" , ",", ".", 2  },	//

//	{ "   ", ""    , 1.00000	, ""   , ""  , ",", "", 2  },
};


GtkWidget *pref_list_create(void);
GtkWidget *list_opecolumncreate(void);

static defpref_entry_get_text(GtkWidget *widget, gchar **storage);
static void list_opecolumn_get(GtkTreeView *treeview, gboolean *columns);
static void list_opecolumn_set(GtkListStore *store, gint *columns);

/*
**
*/
GtkWidget *make_euro_presets(GtkWidget *label)
{
GtkWidget *combobox;
guint i;

	DB( g_print("(defpref) make euro preset\n") );

	combobox = gtk_combo_box_new_text();
	for (i = 0; i < G_N_ELEMENTS (euro_params); i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), euro_params[i].name);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}


void defpref_pathselect(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint type = GPOINTER_TO_INT(user_data);
gchar **path;
GtkWidget *entry;
gboolean r;

	DB( g_print("(defpref) path select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	switch( type )
	{
		case 1:
			path = &PREFS->path_wallet;
			entry = data->ST_path_wallet;
			break;
		case 2:
			path = &PREFS->path_import;
			entry = data->ST_path_import;
			break;
	}		

	DB( g_print(" - wallet %x %s at %x\n" , PREFS->path_wallet, PREFS->path_wallet, &PREFS->path_wallet) );
	DB( g_print(" - import %x %s at %x\n" , PREFS->path_import, PREFS->path_import, &PREFS->path_import) );


	DB( g_print(" - before: %s %x\n" , *path, path) );

	r = homebank_folder_chooser(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW), "title", path);


	DB( g_print(" - after: %s\n", *path) );

	if( r = TRUE )
		gtk_entry_set_text(GTK_ENTRY(entry), *path);
	

}


/*
** update the date sample label
*/
void defpref_date_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gchar buffer[256];
const gchar *fmt;
GDate *date;

	DB( g_print("(defpref) date sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	fmt = gtk_entry_get_text(GTK_ENTRY(data->ST_datefmt));
	date = g_date_new_julian (GLOBALS->today);
	g_date_strftime (buffer, 256-1, fmt, date);
	g_date_free(date);

	gtk_label_set_text(GTK_LABEL(data->LB_date), buffer);

}



/*
** update the number samlpe label
*/
void defpref_numberbase_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct Currency cur;
gint   frac_digits; 
gchar  buf[128];

	DB( g_print("(defpref) number sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.prefix_symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_presymbol));
	cur.suffix_symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_sufsymbol));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits));
	g_snprintf(cur.format, 8-1, "%%.%df", cur.frac_digits);
	
	DB( g_print("fmt: %s\n", cur.format) );
	
	real_mystrfmon(buf, 127, 12345.67, &cur);
	gtk_label_set_text(GTK_LABEL(data->LB_numberbase), buf);

}

/*
** update the number samlpe label
*/
void defpref_numbereuro_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct Currency cur;
gint   frac_digits; 
gchar  buf[128];

	DB( g_print("(defpref) number sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.prefix_symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_presymbol));
	cur.suffix_symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_sufsymbol));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));
	g_snprintf(cur.format, 8-1, "%%.%df", cur.frac_digits);
	
	DB( g_print("fmt: %s\n", cur.format) );
	
	real_mystrfmon(buf, 127, 12345.67, &cur);
	gtk_label_set_text(GTK_LABEL(data->LB_numbereuro), buf);

}






/*
** enable/disable euro
*/
void defpref_eurotoggle(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gboolean bool;

	DB( g_print("(defpref) euro toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));

	

	gtk_widget_set_sensitive(data->CY_euro_preset	, bool);
	gtk_widget_set_sensitive(data->ST_euro_country	, bool);
	gtk_widget_set_sensitive(data->NB_euro_value	, bool);

	gtk_widget_set_sensitive(data->ST_euro_presymbol, bool);
	gtk_widget_set_sensitive(data->ST_euro_sufsymbol, bool);
	gtk_widget_set_sensitive(data->ST_euro_decimalchar, bool);
	gtk_widget_set_sensitive(data->ST_euro_groupingchar, bool);	
	gtk_widget_set_sensitive(data->NB_euro_fracdigits, bool);
	gtk_widget_set_sensitive(data->LB_numbereuro, bool);

}



/*
** set euro value widget from a country
*/
void defpref_europreset(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint country;

	DB( g_print("(defpref) euro preset\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	country = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_euro_preset));

	gtk_label_set_text(GTK_LABEL(data->ST_euro_country), euro_params[country].name);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), euro_params[country].value);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits), euro_params[country].frac_digits);

	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_presymbol)   , euro_params[country].prefix_symbol);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_sufsymbol)   , euro_params[country].suffix_symbol);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_decimalchar) , euro_params[country].decimal_char);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_groupingchar), euro_params[country].grouping_char);

}



void setGdkColor_from_RGB(GdkColor *color, guint32 value)
{
guint	tmp;

	tmp = value >> 16;
	color->red   = tmp | tmp<<8;

	tmp = value >> 8 & 0xFF;
	color->green = tmp | tmp<<8;

	tmp = value & 0xFF;
	color->blue  = tmp | tmp<<8;
}



/*
** set :: fill in widgets from PREFS structure
*/
static defpref_entry_set_text(GtkWidget *widget, gchar *text)
{
	DB( g_print(" set text to '%s'\n", text) );
	
	if( text != NULL )
	{
		gtk_entry_set_text(GTK_ENTRY(widget), text);
	}
}

void defpref_set(struct defpref_data *data)
{
GdkColor color;

	DB( g_print("(defpref) set\n") );

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_toolbar), PREFS->toolbar_style);
	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_image_size), PREFS->image_size);

	setGdkColor_from_RGB(&color, PREFS->color_exp);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_exp_color), &color);

	setGdkColor_from_RGB(&color, PREFS->color_inc);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_inc_color), &color);

	setGdkColor_from_RGB(&color, PREFS->color_warn);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_warn_color), &color);

	#if DOWIZARD == 1
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_runwizard), PREFS->runwizard);
	#endif

	gtk_entry_set_text(GTK_ENTRY(data->ST_path_wallet), PREFS->path_wallet);
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_import), PREFS->path_import);
	//gtk_entry_set_text(GTK_ENTRY(data->ST_path_navigator), PREFS->path_navigator);

	/* display */
	gtk_entry_set_text(GTK_ENTRY(data->ST_datefmt), PREFS->date_format);

	defpref_entry_set_text(data->ST_num_presymbol, PREFS->base_cur.prefix_symbol);
	defpref_entry_set_text(data->ST_num_sufsymbol, PREFS->base_cur.suffix_symbol);
	defpref_entry_set_text(data->ST_num_decimalchar, PREFS->base_cur.decimal_char);
	defpref_entry_set_text(data->ST_num_groupingchar, PREFS->base_cur.grouping_char);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits), PREFS->base_cur.frac_digits);

	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_numnbdec), PREFS->num_nbdecimal);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_numseparator), PREFS->num_separator);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_british), PREFS->british_unit);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->filter_range);

	/* euro */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable), PREFS->euro_active);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_euro_preset), PREFS->euro_country);
	gchar *buf;
	buf = g_strdup_printf("<b>%s</b>", euro_params[PREFS->euro_country].name);
	gtk_label_set_markup(GTK_LABEL(data->ST_euro_country), buf);
	g_free(buf);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), PREFS->euro_value);

	defpref_entry_set_text(data->ST_euro_presymbol, PREFS->minor_cur.prefix_symbol);
	defpref_entry_set_text(data->ST_euro_sufsymbol, PREFS->minor_cur.suffix_symbol);
	defpref_entry_set_text(data->ST_euro_decimalchar, PREFS->minor_cur.decimal_char);
	defpref_entry_set_text(data->ST_euro_groupingchar, PREFS->minor_cur.grouping_char);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits), PREFS->minor_cur.frac_digits);


	//gtk_entry_set_text(GTK_ENTRY(data->ST_euro_symbol), PREFS->euro_symbol);
	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_nbdec), PREFS->euro_nbdec);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_thsep), PREFS->euro_thsep);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_byamount), PREFS->stat_byamount);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_showrate), PREFS->stat_showrate);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_showdetail), PREFS->stat_showdetail);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_budg_showdetail), PREFS->budg_showdetail);

	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_chartlegend), PREFS->chart_legend);

}


/*
** get :: fill PREFS structure from widgets
*/


static defpref_entry_get_text(GtkWidget *widget, gchar **storage)
{
const gchar *text;
gchar *duptext;

	DB( g_print(" storage is '%x' at '%x'\n", *storage, storage) );
	
	/* free any previous string */
	if( *storage != NULL )
	{
		DB( g_print(" storage was not null, freeing\n") );
		
		g_free(*storage);
		*storage = NULL;
	}
		
	/* store if nom epty string */
	text = gtk_entry_get_text(GTK_ENTRY(widget));
	if( text != "" )
	{
		*storage = g_strdup(text);
		DB( g_print(" storing %s at %x\n", *storage, storage) );
	}
	
	DB( g_print(" get text to '%s' '%s'\n", text, *storage) );
}


void defpref_get(struct defpref_data *data)
{
GdkColor color;

	DB( g_print("(defpref) get\n") );

	PREFS->toolbar_style = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_toolbar));
	//PREFS->image_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_image_size));

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_exp_color), &color);
	PREFS->color_exp = (color.red & 0x00FF) << 16 | (color.green & 0x00FF) << 8 | (color.blue & 0x00FF);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_inc_color), &color);
	PREFS->color_inc = (color.red & 0x00FF) << 16 | (color.green & 0x00FF) << 8 | (color.blue & 0x00FF);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_warn_color), &color);
	PREFS->color_warn = (color.red & 0x00FF) << 16 | (color.green & 0x00FF) << 8 | (color.blue & 0x00FF);
	
	
	list_opecolumn_get(GTK_TREE_VIEW(data->LV_opecolumns), PREFS->lst_ope_columns);

	#if DOWIZARD == 1
	PREFS->runwizard = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_runwizard));
	#endif

	g_free(PREFS->path_wallet);
	PREFS->path_wallet = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_wallet)));

	defpref_entry_get_text(data->ST_path_import, &PREFS->path_import);

	//g_free(PREFS->path_navigator);
	//PREFS->path_navigator = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_navigator)));

	g_free(PREFS->date_format);
	PREFS->date_format = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_datefmt)));

	defpref_entry_get_text(data->ST_num_presymbol, &PREFS->base_cur.prefix_symbol);
	defpref_entry_get_text(data->ST_num_sufsymbol, &PREFS->base_cur.suffix_symbol);
	defpref_entry_get_text(data->ST_num_decimalchar, &PREFS->base_cur.decimal_char);
	defpref_entry_get_text(data->ST_num_groupingchar, &PREFS->base_cur.grouping_char);
	PREFS->base_cur.frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits));

	//PREFS->num_nbdecimal = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_numnbdec));
	//PREFS->num_separator = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_numseparator));
	PREFS->british_unit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_british));

	PREFS->filter_range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	PREFS->euro_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));
	PREFS->euro_country = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_euro_preset));
	PREFS->euro_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_value));
	//strcpy(PREFS->euro_symbol, gtk_entry_get_text(GTK_ENTRY(data->ST_euro_symbol)));
	//PREFS->euro_nbdec = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_nbdec));
	//PREFS->euro_thsep = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_thsep));

	g_free(PREFS->minor_cur.prefix_symbol);
	PREFS->minor_cur.prefix_symbol = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_presymbol)) );
	
	g_free(PREFS->minor_cur.suffix_symbol);
	PREFS->minor_cur.suffix_symbol = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_sufsymbol)) );
	
	g_free(PREFS->minor_cur.decimal_char);
	PREFS->minor_cur.decimal_char  = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_decimalchar)) );
	
	g_free(PREFS->minor_cur.grouping_char);
	PREFS->minor_cur.grouping_char = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_groupingchar)) );
	PREFS->minor_cur.frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));

	PREFS->stat_byamount   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_byamount));
	PREFS->stat_showrate   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showrate));
	PREFS->stat_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showdetail));

	PREFS->budg_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budg_showdetail));

	//PREFS->chart_legend = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_chartlegend));

}

/*
GtkWidget *defpref_page_charts (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_hbox_new(FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	widget = gtk_check_button_new_with_mnemonic (_("Show legend"));
	data->CM_chartlegend = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Bar width:"), 1.0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 8.0, 32.0);
	//data->NB_numnbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);



	return(container);
}
*/



GtkWidget *defpref_page_reports (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (5, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Statistics options</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	widget = gtk_check_button_new_with_mnemonic (_("Show by _amount"));
	data->CM_stat_byamount = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _rate column"));
	data->CM_stat_showrate = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_stat_showdetail = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Budget options</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_budg_showdetail = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	return(container);
}


GtkWidget *defpref_page_euro (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (7, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic ("_Enable");
	data->CM_euro_enable = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Fill from:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_euro_presets(label);
	data->CY_euro_preset = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Country:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.0);
	data->ST_euro_country = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Value:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_euro(label);
	data->NB_euro_value = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Numbers format</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Prefix symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_euro_presymbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Suffix symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_euro_sufsymbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Decimal char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_decimalchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Grouping char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_groupingchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_euro_fracdigits = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_numbereuro = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

/*
	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_euro_nbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Thousand separator"));
	data->CM_euro_thsep = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Symbol:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 8);
	data->ST_euro_symbol = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
*/

	return(container);
}


/*
GtkWidget *defpref_page_help (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);
	
	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Web navigator</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("Web _navigator to use:"), 1.0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_path_navigator = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	



	return(container);
}
*/

GtkWidget *defpref_page_display (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (7, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Date options</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("_Date format:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_datefmt = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_date = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Numbers options</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Prefix symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_num_presymbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Suffix symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_num_sufsymbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Decimal char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label,1);
	data->ST_num_decimalchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Grouping char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_num_groupingchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_num_fracdigits = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_numberbase = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* obsolete */

	/*
	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_numnbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Thousand separator"));
	data->CM_numseparator = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	*/

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Measure units</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Use _British units"));
	data->CM_british = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	return(container);
}


GtkWidget *defpref_page_columns (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *hbox, *vbox, *label, *sw, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (3, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Operation list</b>"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* operation column */

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 0);


	// create tree view
	widget = (GtkWidget *)list_opecolumncreate();
	data->LV_opecolumns = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

/*
	vbox = gtk_vbox_new(FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	data->BT_go_up = widget;
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
	data->BT_go_down = widget;
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);


	row++;
	label = make_label(_("Use drag&Drog to reorder"), 0, 0);
	//----------------------------------------- l, r, t, b

	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             //PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);

	gtk_table_attach (GTK_TABLE (table), label, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
*/

	return(container);
}



GtkWidget *defpref_page_interface (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *hbox, *vbox, *label, *sw, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (6, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>General</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Toolbar:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_TOOLBAR_STYLE);
	data->CY_toolbar = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/*
	row++;
	label = make_label(_("_Size:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	widget = make_numeric(label, 16.0, 48.0);
	data->NB_image_size = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	label = make_label(_("pixels"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	*/

	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Amount colors</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Expense:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	widget = gtk_color_button_new ();
	data->CP_exp_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 3, row, row+1);

	row++;
	label = make_label(_("_Income:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	widget = gtk_color_button_new ();
	data->CP_inc_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 3, row, row+1);

	row++;
	label = make_label(_("_Warning:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	widget = gtk_color_button_new ();
	data->CP_warn_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 3, row, row+1);

	return(container);
}

GtkWidget *defpref_page_general (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget, *hbox;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (4, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Folders</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("_Wallet:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	widget = make_string(label);
	data->ST_path_wallet = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("...");
	data->BT_path_wallet = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	row++;
	label = make_label(_("_Import:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	widget = make_string(label);
	data->ST_path_import = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("...");
	data->BT_path_import = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	#if DOWIZARD == 1
	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Run the wizard"));
	data->CM_runwizard = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	#endif


	row++;
	label = make_label(NULL, 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Initial date filter</b>"));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Range:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_RANGE);
	data->CY_range = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);


/*
	row++;
	label = make_label("Wallet:", 1.0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	widget = gtk_file_chooser_button_new("toto", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
*/




	return(container);
}

void defpref_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
struct defpref_data *data;
GtkWidget *notebook;
GtkTreeView *treeview;
GtkTreeModel *model;
GtkTreeIter iter;

GValue        val = { 0, };
gint page;

	DB( g_print("(defpref) selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		notebook = GTK_WIDGET(user_data);
		treeview = gtk_tree_selection_get_tree_view(treeselection);
		data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");


		gtk_tree_model_get_value(model, &iter, LST_PREF_PAGE, &val);
		page = g_value_get_int (&val);
		DB( g_print(" - active is %d\n", page) );
		g_value_unset (&val);


		gtk_tree_model_get_value(model, &iter, LST_PREF_NAME, &val);
		gtk_label_set_text (GTK_LABEL (data->label), g_value_get_string (&val));
		g_value_unset (&val);

		gtk_tree_model_get_value(model, &iter, LST_PREF_ICON, &val);
		gtk_image_set_from_pixbuf (GTK_IMAGE (data->image),
                             g_value_get_object (&val));
		g_value_unset (&val);



		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);

		//defpref_change_page(GTK_WIDGET(gtk_tree_selection_get_tree_view(treeselection)), GINT_TO_POINTER(page));
	}

}


/*
** set the notebook active page from treeview
*/
/*void defpref_change_page(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint page = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;


	DB( g_print("(defpref) page\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_page));







	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_page), page);
}

*/

// the window creation
GtkWidget *defpref_dialog_new (void)
{
struct defpref_data data;
GtkWidget *window, *mainvbox;

GtkWidget *hbox, *vbox, *sw, *widget, *notebook, *page, *frame, *image, *label;

      window = gtk_dialog_new_with_buttons (_("Preferences"),
				//GTK_WINDOW (do_widget),
				NULL,
				0,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_REJECT,
				GTK_STOCK_CLEAR,
				666,
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);

	gtk_dialog_set_has_separator(GTK_DIALOG (window), FALSE);
	
	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "settings.svg");

	mainvbox = gtk_vbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), mainvbox, TRUE, TRUE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(mainvbox), 8);

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_box_pack_start (GTK_BOX (hbox), sw, FALSE, TRUE, 0);
	widget = pref_list_create();
	data.LV_page = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	//notebook
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  
  gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
                             
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 6);
  gtk_widget_show (label);
  data.label = label;

  image = gtk_image_new ();
  gtk_box_pack_end (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);
	data.image = image;


	//notebook
	notebook = gtk_notebook_new();
	data.GR_page = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

/*
"general",
"interface",
"display",
"help",
"euro",
"report"
*/

	//general
	page = defpref_page_general(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//interface
	page = defpref_page_interface(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//columns
	page = defpref_page_columns(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//display
	page = defpref_page_display(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//help
	//page = defpref_page_help(&data);
	//gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//euro
	page = defpref_page_euro(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//report
	page = defpref_page_reports(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data.CM_euro_enable), PREFS->euro_active);

	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	//path selector
	g_signal_connect (data.BT_path_wallet, "pressed", G_CALLBACK (defpref_pathselect), GINT_TO_POINTER(1));
	g_signal_connect (data.BT_path_import, "pressed", G_CALLBACK (defpref_pathselect), GINT_TO_POINTER(2));


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_page)), "changed", G_CALLBACK (defpref_selection), notebook);

	g_signal_connect (data.CM_euro_enable, "toggled", G_CALLBACK (defpref_eurotoggle), NULL);

    g_signal_connect (data.CY_euro_preset, "changed", G_CALLBACK (defpref_europreset), NULL);

	//date
    g_signal_connect (data.ST_datefmt, "changed", G_CALLBACK (defpref_date_sample), NULL);

	//base number
    g_signal_connect (data.ST_num_presymbol   , "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.ST_num_sufsymbol   , "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.ST_num_decimalchar , "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.ST_num_groupingchar, "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.NB_num_fracdigits, "value-changed", G_CALLBACK (defpref_numberbase_sample), NULL);

	//euro number
    g_signal_connect (data.ST_euro_presymbol   , "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.ST_euro_sufsymbol   , "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.ST_euro_decimalchar , "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.ST_euro_groupingchar, "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.NB_euro_fracdigits, "value-changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
	


	//setup, init and show window
	//defwallet_setup(&data);
	//defwallet_update(data.LV_arc, NULL);

	defpref_eurotoggle(window, NULL);

	defpref_set(&data);

	gtk_window_resize(GTK_WINDOW(window), 640, 256);

	gtk_widget_show_all (window);

	gint result = 666;
	
	while( result == 666 )
	{
		//wait for the user
		result = gtk_dialog_run (GTK_DIALOG (window));

		switch( result )
		{
			case GTK_RESPONSE_ACCEPT:
				defpref_get(&data);
				wallet_update(GLOBALS->mainwindow, (gpointer)UF_BALANCE+UF_VISUAL);
				homebank_pref_save();
				break;
			case 666:
				homebank_pref_setdefault();
				defpref_set(&data);
				break;
		}
	}

	// cleanup and destroy
	//defwallet_cleanup(&data, result);
	gtk_widget_destroy (window);

	return window;
}

// -------------------------------


GtkWidget *pref_list_create(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;
GtkTreeIter    iter;
gint i;

	/* create list store */
	store = gtk_list_store_new(
	  	LST_PREF_MAX,
		GDK_TYPE_PIXBUF,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_INT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (view), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

	/* column 1: icon */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", LST_PREF_SMALLPIXBUF, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", LST_PREF_NAME, NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	//populate our combobox model
	for(i=0;i<PREF_MAX;i++)
	{
	GdkPixbuf *small_pixbuf = NULL;
	
		gtk_list_store_append(store, &iter);
		
		if( pref_pixbuf[i] )
			small_pixbuf = gdk_pixbuf_scale_simple (pref_pixbuf[i],
                                                24, 24,
                                                GDK_INTERP_BILINEAR);
		
		
		gtk_list_store_set(store, &iter,
			LST_PREF_SMALLPIXBUF, small_pixbuf,
			LST_PREF_ICON, pref_pixbuf[i],
			LST_PREF_NAME, _(pref_name[i]),
			LST_PREF_PAGE, i,
			-1);
	}

	return(view);
}



void free_pref_icons(void)
{
guint i;

	for(i=0;i<PREF_MAX;i++)
	{
		if(pref_pixbuf[i] != NULL)
			g_object_unref(pref_pixbuf[i]);
	}
}

void load_pref_icons(void)
{
//GError *error = NULL;
guint i;

	for(i=0;i<PREF_MAX;i++)
	{
	gchar *pathfilename;

	pathfilename = g_build_filename((const gchar *)homebank_pixmaps_dir, pref_pixname[i], NULL);

	DB( g_print("loading %s\n", pathfilename) );

		pref_pixbuf[i] = gdk_pixbuf_new_from_file(pathfilename, NULL);
		//pref_pixbuf[i] = gdk_pixbuf_new_from_file_at_size(pathname, 24, 24, NULL);
		/*
		if (error)
		{
			g_warning ("Could not load icon: %s\n", error->message);
			g_error_free(error);
			error = NULL;
		}
		*/
		g_free (pathfilename);
	}
}

// -------------------------------
static struct { 
	gchar		*name;
	gint		id;
} ope_list_columns[] = {

//	{	N_(""), COL_OPE_STATUS,		},
//	{	N_(""), COL_OPE_DATE,		},
	{	N_("Info"    ), COL_OPE_INFO,		},
	{	N_("Payee"   ), COL_OPE_PAYEE,		},
	{	N_("Wording" ), COL_OPE_WORDING,	},
	{	N_("Amount"  ), COL_OPE_AMOUNT,		},
	{	N_("Expense" ), COL_OPE_EXPENSE,	},
	{	N_("Income"  ), COL_OPE_INCOME,		},
	{	N_("Category"), COL_OPE_CATEGORY,	},
};

static gint n_ope_list_columns = G_N_ELEMENTS (ope_list_columns);

static void
fixed_toggled (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_VISIBLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_VISIBLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}



static void list_opecolumn_get(GtkTreeView *treeview, gboolean *columns)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean visible;
gint i, id;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			COLUMN_VISIBLE, &visible,
			COLUMN_ID, &id,
			-1);

		DB( g_print("list_opecolumn_get %d: %d\n",id, visible) );
	
		columns[i+2] = visible;

		 /* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		i++;
	}

}

static void list_opecolumn_selection(GtkTreeSelection *treeselection, gpointer user_data)
{

	DB( g_print("list_opecolumn_selection\n") );
	
	/*void        gtk_list_store_move_before      (GtkListStore *store,
                                             GtkTreeIter *iter,
                                             GtkTreeIter *position);
	
		//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay)), &model, &iter);

	gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	
	*/
	
	
}

GtkWidget *list_opecolumncreate(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;
GtkTreeIter    iter;
gint i;
gboolean visible;

	/* create list store */
	store = gtk_list_store_new(
	  	3,
		G_TYPE_BOOLEAN,
		G_TYPE_STRING,
		G_TYPE_UINT
		);

	/* populate */
	for (i = 0; i < n_ope_list_columns; i++)
	{
		visible = PREFS->lst_ope_columns[ope_list_columns[i].id];
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
			COLUMN_VISIBLE, visible,
      		COLUMN_NAME, ope_list_columns[i].name,
      		COLUMN_ID  , ope_list_columns[i].id,
      		-1);
	}



	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", COLUMN_VISIBLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (fixed_toggled), store);


		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Column"),
							     renderer,
							     "text", COLUMN_NAME,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), "changed", G_CALLBACK (list_opecolumn_selection), NULL);


	return(view);
}

