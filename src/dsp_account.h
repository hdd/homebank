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

#ifndef __HOMEBANK_DSPACCOUNT_H__
#define __HOMEBANK_DSPACCOUNT_H__

GtkWidget *create_account_window(gint accnum, Account *acc);
void operation_add_treeview(Operation *ope, GtkWidget *treeview, gint accnum);
void operation_add(Operation *ope, GtkWidget *treeview, gint accnum);

Operation *operation_get_child_transfert(Operation *src);
void operation_warn_transfert(Operation *src, gchar *msg2);
void operation_delete_child_transfert(Operation *src);

void account_init_window(GtkWidget *widget, gpointer user_data);
void account_busy(GtkWidget *widget, gboolean state);

#endif /* __HOMEBANK_DSPACCOUNT_H__ */
