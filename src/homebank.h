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

#ifndef __HOMEBANK_H__
#define __HOMEBANK_H__

#include <config.h>

#include <glib-2.0/glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>		/* atoi, atof, atol */
#include <string.h>		/* memset, memcpy, strcmp, strcpy */

#include <locale.h>
#include <time.h>

#include "enums.h"
#include "data_access.h"
#include "widgets.h"
#include "misc.h"
#include "preferences.h"

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


#define FILE_VERSION	0.1



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
	gchar		*filename;
	gchar		*oldfilename;
	gboolean	wallet_is_new;
	gboolean	exists_old;

	gint		change;

	gint		define_off;		//>0 when a stat, account window is opened
	gboolean	minor;

	//gchar		fmt_maj_number[16];
	//gchar		fmt_min_number[16];

	// pixbuf datas
	GdkPixbuf	*lst_pixbuf[NUM_LST_PIXBUF];
	gint		lst_pixbuf_maxwidth;



};


void homebank_message_dialog(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...);
gboolean homebank_csv_file_chooser(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr);
gboolean homebank_alienfile_chooser(gchar *title);

gchar *homebank_get_filename_with_extension(gchar *path, gchar *extension);

#endif /* __HOMEBANK_H__ */
