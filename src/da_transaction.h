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
 
#ifndef __DA_TRANSACTION_H__
#define __DA_TRANSACTION_H__


typedef struct _operation	Operation;


struct _operation
{
	guint32		date;

	gdouble		amount;
	guint32		account;
	guint32		dst_account;
	gushort		paymode;
	gushort		flags;
	guint32		payee;
	guint32		category;
	gchar		*wording;
	gchar		*info;
	guint32		*tags;
	guint32		kxfer;		//internal xfer key

	//non saved datas
	GList		*same;		//used for import todo: change this
};

#define OF_VALID	(1<<0)
#define OF_INCOME	(1<<1)
#define OF_AUTO		(1<<2)
#define OF_ADDED	(1<<3)
#define OF_CHANGED	(1<<4)
#define OF_REMIND	(1<<5)
#define OF_CHEQ2	(1<<6)
#define OF_LIMIT	(1<<7)

Operation *da_operation_malloc(void);
Operation *da_operation_clone(Operation *src_item);
void da_operation_free(Operation *item);

GList *
da_operation_new(void);
void da_operation_destroy(GList *list);

GList *da_operation_sort(GList *list);
gboolean da_operation_append(Operation *item);

guint32 da_operation_get_max_kxfer(void);

#endif

