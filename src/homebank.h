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

#ifndef __HOMEBANK_H__
#define __HOMEBANK_H__

#include <config.h>

#include <glib-2.0/glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <errno.h>
#include <math.h>
#include <monetary.h>	/* strfmon */
#include <stdlib.h>		/* atoi, atof, atol */
#include <string.h>		/* memset, memcpy, strcmp, strcpy */

#include <locale.h>
#include <time.h>

#include "enums.h"
#include "data_access.h"
#include "widgets.h"
#include "misc.h"

#include "gtkdateentry.h"
#include "gtkchart.h"

#include <libintl.h>
#include <locale.h>

#define _(str) gettext (str)
#define gettext_noop(str) (str)
#define N_(str) gettext_noop (str)

/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#define	PROGNAME		"HomeBank"
//#define PROGVERSION		"3.2"


#define FILE_VERSION	"0.1"
#define PREF_VERSION	"0.1"

#define DEFAULT_PATH_WALLET			"/home/max/dev/gnomebank/accounts"
#define DEFAULT_PATH_NAVIGATOR		"mozilla"

#define DEFAULT_FORMAT_DECIMAL		"%.2f"
#define DEFAULT_FORMAT_DATE			"%x"

#define DEFAULT_EXP_COLOR		0xE88C00
#define DEFAULT_INC_COLOR		0x00C800
#define DEFAULT_WARN_COLOR		0XC80000


/* for operatio dialog */
#define GTK_RESPONSE_ADD 1

/* container spacing */
#define HB_MAINBOX_SPACING	12
#define HB_BOX_SPACING	6
#define HB_HSPACE_SPACING	12
#define HB_TABROW_SPACING	6
#define HB_TABCOL_SPACING	6

/*
** Global application datas
*/
struct HomeBank
{
	GtkWidget	*mainwindow;	//should be global to access attached window data
	guint32		today;			//today's date

	GList		*acc_list;		//accounts
	GList		*pay_list;		//payees
	GList		*cat_list;		//categories
	GList		*arc_list;		//archives
	GList		*ope_list;		//operations

	// wallet properties
	gchar		*title;
	guint		car_category;
	guint		auto_nbdays;

	// current filename
	gchar		*lastfilename;
	gchar		*filename;
	gboolean	wallet_is_new;

	gint		change;

	gint		define_off;		//>0 when a stat, account window is opened
	gboolean	minor;

	gchar		fmt_maj_number[16];
	gchar		fmt_min_number[16];

	// pixbuf datas
	GdkPixbuf	*lst_pixbuf[NUM_LST_PIXBUF];
	gint		lst_pixbuf_maxwidth;



};




/*
** Preference datas
*/
struct Preferences
{
	//general
	gchar		*path_wallet;
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
	gshort		num_nbdecimal;
	gboolean	num_separator;
	gboolean	british_unit;

	//help system
	//gboolean	show_tooltips;
	//gboolean	show_help_button;
	//gboolean	show_tipofday;
	gchar		*path_navigator;

	//euro zone
	gboolean	euro_active;
	gint		euro_country;
	gdouble		euro_value;

	gshort		euro_nbdec;
	gboolean	euro_thsep;
	gchar		*euro_symbol;

	//report options
	gboolean	stat_byamount;
	gboolean	stat_showrate;
	gboolean	stat_showdetail;
	gboolean	budg_showdetail;

	//chart options
	gboolean	chart_legend;

	/* internal */
	//gint		last_page;
};

void homebank_message_dialog(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...);
gboolean homebank_csv_file_chooser(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr);
gboolean homebank_alienfile_chooser(gchar *title);

void homebank_init_prefs(void);
void homebank_pref_createformat(void);
gboolean homebank_pref_load(void);
gboolean homebank_pref_save(void);

#endif /* __HOMEBANK_H__ */
