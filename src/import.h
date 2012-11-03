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

#ifndef __HOMEBANK_IMPORT_H__
#define __HOMEBANK_IMPORT_H__

#define QIF_UNKNOW_ACCOUNT_NAME "(unknown)"

enum
{
	FILETYPE_UNKNOW,
	FILETYPE_AMIGA_HB,
	FILETYPE_CSV_HB,
	FILETYPE_OFX,
	FILETYPE_QIF,
	NUM_FILETYPE
};


enum
{
	PAGE_INTRO,
	PAGE_FILE,
	PAGE_ANALYSIS,
	PAGE_TRANSACTION,
	PAGE_CONFIRM,
	NUM_PAGE
};


typedef struct _ImportContext ImportContext;
struct _ImportContext
{
	GList		*trans_list;
	gboolean	has_unknow_account;
	gint 		cnt_initial_acc;
	gint		cnt_new_acc;
	gint		cnt_new_ope;
	gint		cnt_new_pay;
	gint		cnt_new_cat;	
};


typedef struct _OfxContext OfxContext;
struct _OfxContext
{
	GList		*trans_list;
	Account 	*curr_acc;
	gboolean	curr_acc_isnew;
};



struct import_data
{
	GtkWidget	*assistant;
	GtkWidget	*pages[NUM_PAGE];

	GtkWidget	*filechooser;
	GtkWidget	*user_info;
	GtkWidget	*ok_image;
	GtkWidget	*ko_image;

	GtkWidget	*TX_filename;
	GtkWidget	*TX_details;

	GtkWidget	*CM_type[2];
	GtkWidget	*LA_acc;
	GtkWidget	*PO_acc;
	GtkWidget	*ST_acc;
	GtkWidget	*NB_decay;
	
	GtkWidget	*imported_ope;
	GtkWidget	*duplicat_ope;
	
	GtkWidget	*last_info;
	
	gchar		*filename;
	guint		filetype;

	guint		imported;
	guint		total;

	gboolean	valid;

//	guint		step;
//	guint		maxstep;


	// import context
	ImportContext	ictx;
	
};


GtkWidget *create_import_window (void);

#endif


