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
#include "da_other.h"
#include "da_account.h"
#include "da_category.h"
#include "da_payee.h"
#include "da_transaction.h"
#include "da_tag.h"
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

#define FILE_VERSION		0.3

#if HB_UNSTABLE != 1
	#define	PROGNAME		"HomeBank"
	#define HB_DATA_PATH	".homebank"
#else
	#define	PROGNAME		"HomeBank (unstable)"
	#define HB_DATA_PATH	".homebank_unstable"
#endif

#ifdef G_OS_WIN32
#define GETTEXT_PACKAGE "homebank"
#define LOCALE_DIR      "locale"
#define PIXMAPS_DIR     "images"
#define HELP_DIR        "help"
#define PACKAGE_VERSION "4.0.1"
#define PACKAGE         "homebank"
#define VERSION         "4.0.1"
//#define NOOFX

#define ENABLE_NLS 1
#endif


/* for operation dialog */
#define GTK_RESPONSE_ADD	 1

/* container spacing */
#define HB_MAINBOX_SPACING	12
#define HB_BOX_SPACING		6
#define HB_HSPACE_SPACING	12
#define HB_TABROW_SPACING	6
#define HB_TABCOL_SPACING	6

/*
** Global application datas
*/
struct HomeBank
{
	// data storage
	//GList		*acc_list;		//accounts
	GHashTable	*h_acc;			//accounts
	//GList		*pay_list;		//payees
	GHashTable	*h_pay;			//payees
	//GList		*cat_list;		//categories
	GHashTable	*h_cat;			//categories
	GHashTable	*h_tag;			//tags

	GList		*arc_list;		//archives
	GList		*ope_list;		//operations
	GHashTable	*h_memo;		//memo/description

	// wallet properties
	gchar		*title;
	guint		car_category;
	guint		auto_nbdays;

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
gboolean homebank_csv_file_chooser(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr);
gboolean homebank_file_chooser(GtkFileChooserAction action);
gboolean homebank_folder_chooser(GtkWindow *parent, gchar *title, gchar **storage_ptr);
gboolean homebank_alienfile_chooser(gchar *title);
gchar *homebank_get_filename_with_extension(gchar *path, gchar *extension);
gchar *homebank_get_filename_without_extension(gchar *path);
void homebank_file_ensure_xhb(void);
void homebank_backup_current_file(gchar *pathname);
gboolean homebank_util_url_show (const gchar *url);
gboolean homebank_lastopenedfiles_load(void);
gboolean homebank_lastopenedfiles_save(void);


void homebank_window_set_icon_from_file(GtkWindow *window, gchar *filename);
gboolean homebank_folder_chooser(GtkWindow *parent, gchar *title, gchar **storage_ptr);

gchar *homebank_get_filename_with_extension(gchar *path, gchar *extension);

const gchar *homebank_app_get_pixmaps_dir (void);
const gchar *homebank_app_get_locale_dir (void);
const gchar *homebank_app_get_help_dir (void);

#endif /* __HOMEBANK_H__ */
