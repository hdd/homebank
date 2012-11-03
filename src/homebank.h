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

#ifndef __HOMEBANK_H__
#define __HOMEBANK_H__

#include <config.h>

#include <ctype.h> 		/* isprint */
#include <errno.h>
#include <math.h>
#include <libintl.h>
#include <locale.h>
#include <stdlib.h>		/* atoi, atof, atol */
#include <string.h>		/* memset, memcpy, strcmp, strcpy */
#include <time.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "enums.h"
#include "preferences.h"
#include "da_encoding.h"
#include "da_other.h"
#include "da_account.h"
#include "da_category.h"
#include "da_payee.h"
#include "da_transaction.h"
#include "da_tag.h"
#include "da_assign.h"
#include "gtkdateentry.h"
#include "misc.h"
#include "widgets.h"

#define _(str) gettext (str)
#define gettext_noop(str) (str)
#define N_(str) gettext_noop (str)

/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#define HB_UNSTABLE			0
#define HB_VERSION			"4.4"
#define FILE_VERSION		0.6

#if HB_UNSTABLE != 1
	#define	PROGNAME		"HomeBank"
	#define HB_DATA_PATH	"homebank"
#else
	#define	PROGNAME		"HomeBank " HB_VERSION " (unstable)"
	#define HB_DATA_PATH	"homebank_unstable"
#endif

#ifdef G_OS_WIN32
	#define GETTEXT_PACKAGE "homebank"
	#define LOCALE_DIR      "locale"
	#define PIXMAPS_DIR     "images"
	#define HELP_DIR        "help"
	#define PACKAGE_VERSION HB_VERSION
	#define PACKAGE         "homebank"
	#define VERSION         HB_VERSION
	//#define NOOFX

	#define ENABLE_NLS 1
#endif

/* container spacing */
#define HB_MAINBOX_SPACING	12
#define HB_BOX_SPACING		6
#define HB_HSPACE_SPACING	12
#define HB_TABROW_SPACING	6
#define HB_TABCOL_SPACING	6

/* for operation dialog */
#define GTK_RESPONSE_ADD	 1

/* default zoomx for charts */
#define GTK_CHART_MINBARW 	8
#define GTK_CHART_BARW 		24
#define GTK_CHART_MAXBARW 	64

enum
{
	FILETYPE_UNKNOW,
	FILETYPE_HOMEBANK,
	FILETYPE_OFX,
	FILETYPE_QIF,
	FILETYPE_CSV_HB,
	FILETYPE_AMIGA_HB,
	NUM_FILETYPE
};

/*
** stock icons
*/

/* Custom HomeBank named icons */
#define HB_STOCK_ACCOUNT         "hb-account"
#define HB_STOCK_ARCHIVE         "hb-archive"
#define HB_STOCK_ASSIGN          "hb-assign"
#define HB_STOCK_BUDGET          "hb-budget"
#define HB_STOCK_CATEGORY        "hb-category"
#define HB_STOCK_PAYEE           "hb-payee"
#define HB_STOCK_FILTER          "hb-filter"
#define HB_STOCK_OPE_ADD         "hb-ope-add"
#define HB_STOCK_OPE_HERIT       "hb-ope-herit"
#define HB_STOCK_OPE_EDIT        "hb-ope-edit"
#define HB_STOCK_OPE_SHOW        "hb-ope-show"
#define HB_STOCK_OPE_DELETE      "hb-ope-delete"
#define HB_STOCK_OPE_VALID       "hb-ope-valid"
#define HB_STOCK_OPE_REMIND      "hb-ope-remind"
#define HB_STOCK_OPE_AUTO        "hb-ope-auto"
#define HB_STOCK_REP_STATS       "hb-rep-stats"
#define HB_STOCK_REP_TIME        "hb-rep-time"
#define HB_STOCK_REP_BALANCE     "hb-rep-balance"
#define HB_STOCK_REP_BUDGET      "hb-rep-budget"
#define HB_STOCK_REP_CAR         "hb-rep-vehicle"


/*
** Global application datas
*/
struct HomeBank
{
	// data storage
//	GHashTable	*h_cur;			//currencies
	GHashTable	*h_acc;			//accounts
	GHashTable	*h_pay;			//payees
	GHashTable	*h_cat;			//categories
	GHashTable	*h_tag;			//tags
	GHashTable	*h_rul;			//assign rules

	GHashTable	*h_memo;		//memo/description

	GList		*arc_list;		//archives
	GList		*ope_list;		//operations

	gboolean	first_run;
	
	// wallet properties
	gchar		*title;
	guint32		vehicle_category;
	guint		auto_nbdays;
//	guint32		kcur;			// base currency

	// current filename
	gchar		*filename;
	gchar		*oldfilename;
	gboolean	wallet_is_new;
	gboolean	exists_old;

	// other stuffs
	guint32		today;			//today's date
	gint		change;
	gint		define_off;		//>0 when a stat, account window is opened
	gboolean	minor;
	GtkWidget	*mainwindow;	//should be global to access attached window data
	GdkPixbuf	*lst_pixbuf[NUM_LST_PIXBUF];
	gint		lst_pixbuf_maxwidth;
	
};

gint homebank_question_dialog(GtkWindow *parent, gchar *title, gchar *message_format, ...);
void homebank_message_dialog(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...);
gboolean homebank_chooser_open_qif(GtkWindow *parent, gchar **storage_ptr);
gboolean homebank_csv_file_chooser(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr, gchar *name);
gboolean homebank_file_chooser(GtkFileChooserAction action);
gboolean homebank_folder_chooser(GtkWindow *parent, gchar *title, gchar **storage_ptr);
gboolean homebank_alienfile_chooser(gchar *title);
gint homebank_alienfile_recognize(gchar *filename);
gchar *homebank_get_filename_with_extension(gchar *path, gchar *extension);
gchar *homebank_get_filename_without_extension(gchar *path);
void homebank_file_ensure_xhb(void);
void homebank_backup_current_file(gchar *pathname);
gboolean homebank_util_url_show (const gchar *url);
gboolean homebank_lastopenedfiles_load(void);
gboolean homebank_lastopenedfiles_save(void);


void homebank_window_set_icon_from_file(GtkWindow *window, gchar *filename);

const gchar *homebank_app_get_config_dir (void);
const gchar *homebank_app_get_images_dir (void);
const gchar *homebank_app_get_pixmaps_dir (void);
const gchar *homebank_app_get_locale_dir (void);
const gchar *homebank_app_get_help_dir (void);
const gchar *homebank_app_get_datas_dir (void);

#endif /* __HOMEBANK_H__ */
