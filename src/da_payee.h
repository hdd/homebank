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

#ifndef __DA_PAYEE_H__
#define __DA_PAYEE_H__


typedef struct _payee		Payee;


struct _payee
{
	guint32   	key;
	gushort 	flags;
	gchar   	*name;
	gboolean	filter;
	gboolean	imported;
};

void da_pay_free(Payee *item);
Payee *da_pay_malloc(void);

void da_pay_destroy(void);
void da_pay_new(void);

guint		da_pay_length(void);
gboolean	da_pay_create_none(void);
gboolean	da_pay_remove(guint32 key);
gboolean	da_pay_insert(Payee *acc);
gboolean	da_pay_append(Payee *acc);
guint32		da_pay_get_max_key(void);
Payee		*da_pay_get_by_name(gchar *name);
Payee		*da_pay_get(guint32 key);

#endif

