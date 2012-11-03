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

#ifndef __HB_ACCOUNT_DATA_H__
#define __HB_ACCOUNT_DATA_H__


typedef struct _account		Account;


struct _account
{
	guint	key;
	gushort	flags;
	
	//todo: for stock account
	//gushort	type;
	gchar	*name;
	gchar	*number;
	gchar	*bankname;
	gdouble	initial;
	gdouble	minimum;
	guint	cheque1;
	guint	cheque2;
	//currency ?
	//note ?
	// non persitent datas
	GtkWindow	*window;	//dsp_account opened
	guint	pos;			//position in list
	gboolean	filter;
	gboolean	imported;
};

#define AF_BUDGET	(1<<0)
#define AF_CLOSED	(1<<1)
#define AF_ADDED	(1<<2)
#define AF_CHANGED	(1<<3)



Account *da_acc_clone(Account *src_item);
Account *da_acc_malloc(void);
void da_acc_free(Account *item);
Account *da_acc_malloc(void);

void da_acc_destroy(void);
void da_acc_new(void);

guint		da_acc_length(void);
gboolean	da_acc_create_none(void);
gboolean	da_acc_remove(guint32 key);
gboolean	da_acc_insert(Account *acc);
gboolean	da_acc_append(Account *item);
guint32		da_acc_get_max_key(void);
Account		*da_acc_get_by_name(gchar *name);
Account		*da_acc_get(guint32 key);

#endif

