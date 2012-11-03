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

#ifndef __HB_BC_PAYEE_H__
#define __HB_BC_PAYEE__

gboolean payee_is_used(guint32 key);
void payee_move(guint32 key1, guint32 key2);
gboolean payee_rename(Payee *item, const gchar *newname);
Payee *payee_append_if_new(gchar *name);

void payee_load_csv(gchar *filename);
void payee_save_csv(gchar *filename);

#endif /* __HOMEBANK_DEFPAYEE_H__ */
