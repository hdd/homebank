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

#ifndef __HOMEBANK_DATAACCESS_H__
#define __HOMEBANK_DATAACCESS_H__

#include "da_transaction.h"

typedef struct _archive		Archive;
typedef struct _budget		Budget;

typedef struct _filter	Filter;
typedef struct _carcost	CarCost;


struct _budget
{
	guint	key;
	gushort	flags;
	guint	cat_key;
	guint	year;
	gdouble	value[13];
};

#define BF_CUSTOM	(1<<2)


struct _archive
{
	gdouble	amount;
	guint	account;
	guint	dst_account;
	gushort	paymode;
	gushort	flags;
	guint	payee;
	guint	category;
	gchar	*wording;

	guint32	nextdate;
	gushort	every;
	gushort	unit;
	gushort	limit;
};


struct _investment
{
	guint	date;
	gdouble	buy_amount;
	gdouble	curr_amount;
	gdouble	commission;
	guint	number;
	guint	account;
	gchar	*name;
	gchar	*symbol;
	gchar	*note;
};


/* secondary structures */

struct _filter
{
	guint		option[FILTER_MAX];
	guint		mindate, maxdate;
	gushort		range, month, year;
	gboolean	forceadd;
	gboolean	forcechg;
	gboolean	paymode[NUM_PAYMODE_MAX];
	gdouble		minamount, maxamount;
	//gboolean	*acc;
	//gboolean	*pay;
	//gboolean	*cat;
	guint		last_tab;
};

struct _carcost
{
	Operation	*ope;
	guint		meter;
	gdouble		fuel;
	guint		dist;
};

typedef struct
{
  const gint     type;
  const gchar    *stock;
  const gchar    *image;
  const gchar    *label;
  const gchar    *tip;
  const gchar	action;
} TB_toolbar;

Archive *da_archive_malloc(void);
Archive *da_archive_clone(Archive *src_item);
void da_archive_free(Archive *item);
void da_archive_destroy(GList *list);


Filter *da_filter_malloc(void);
void da_filter_free(Filter *flt);

CarCost *da_carcost_malloc(void);
void da_carcost_free(CarCost *item);
void da_carcost_destroy(GList *list);

#endif /* __HOMEBANK_DATAACCESS_H__ */
