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

#ifndef __HOMEBANK_DATAACCESS_H__
#define __HOMEBANK_DATAACCESS_H__


typedef struct _account		Account;
typedef struct _payee		Payee;
typedef struct _category	Category;
typedef struct _archive		Archive;
typedef struct _budget		Budget;
typedef struct _operation	Operation;

typedef struct _filter	Filter;
typedef struct _carcost	CarCost;

struct _account
{
	guint	key;
	gushort	flags;
	gchar	*name;
	gchar	*number;
	gchar	*bankname;
	gdouble	initial;
	gdouble	minimum;
	guint	cheque1;
	guint	cheque2;
};

#define AF_BUDGET	(1<<0)
#define AF_CLOSED	(1<<1)
#define AF_ADDED	(1<<2)
#define AF_CHANGED	(1<<3)

struct _payee
{
	guint	key;
	gushort	flags;
	gchar	*name;
};

struct _category
{
	guint	key;
	guint	parent;
	gushort	flags;
	gchar	*name;
	gdouble	budget[13];
};

#define GF_SUB		(1<<0)
#define GF_INCOME	(1<<1)
#define GF_CUSTOM	(1<<2)
#define GF_BUDGET	(1<<3)

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

struct _operation
{
	guint	date;

	gdouble	amount;
	guint	account;
	guint	dst_account;
	gushort	paymode;
	gushort	flags;
	guint	payee;
	guint	category;
	gchar	*wording;
	gchar	*info;
	GList	*same;		//used for import
};

#define OF_VALID	(1<<0)
#define OF_INCOME	(1<<1)
#define OF_AUTO		(1<<2)
#define OF_ADDED	(1<<3)
#define OF_CHANGED	(1<<4)
#define OF_REMIND	(1<<5)
#define OF_CHEQ2	(1<<6)
#define OF_LIMIT	(1<<7)


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
	gboolean	*acc;
	gboolean	*pay;
	gboolean	*cat;
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



Account *da_account_malloc(void);
Account *da_account_clone(Account *src_item);
void da_account_free(Account *item);
void da_account_destroy(GList *list);

Payee *da_payee_malloc(void);
Payee *da_payee_clone(Payee *src_item);
void da_payee_free(Payee *item);
void da_payee_destroy(GList *list);
gint da_payee_exists(GList *src_list, gchar *name);

Category *da_category_malloc(void);
Category *da_category_clone(Category *src_item);
void da_category_free(Category *item);
void da_category_destroy(GList *list);
gint da_category_exists(GList *src_list, gchar *name);

Archive *da_archive_malloc(void);
Archive *da_archive_clone(Archive *src_item);
void da_archive_free(Archive *item);
void da_archive_destroy(GList *list);

Operation *da_operation_malloc(void);
Operation *da_operation_clone(Operation *src_item);
void da_operation_free(Operation *item);
void da_operation_destroy(GList *list);
GList *da_operation_sort(GList *list);

Filter *da_filter_malloc(void);
void da_filter_free(Filter *flt);

CarCost *da_carcost_malloc(void);
void da_carcost_free(CarCost *item);
void da_carcost_destroy(GList *list);

void populate_view_acc(GtkWidget *dst_view, GList *src_list, gboolean clone);
void populate_view_pay(GtkWidget *dst_view, GList *src_list, gboolean clone);
void populate_view_cat(GtkWidget *dst_view, GList *src_list, gboolean clone);

#endif /* __HOMEBANK_DATAACCESS_H__ */
