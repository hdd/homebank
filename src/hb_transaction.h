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

#ifndef __HB_TRANSACTION_H__
#define __HB_TRANSACTION_H__

void operation_add_treeview(Operation *ope, GtkWidget *treeview, gint accnum);
void operation_add(Operation *ope, GtkWidget *treeview, gint accnum);
Operation *operation_get_child_transfer(Operation *src);
void operation_warn_transfer(Operation *src, gchar *msg2);
void operation_add_transfer(Operation *ope, GtkWidget *treeview);
void operation_delete_child_transfer(Operation *src);

guint transaction_count_tags(Operation *ope);
guint transaction_set_tags(Operation *ope, const gchar *tagstring);
gchar *transaction_get_tagstring(Operation *ope);
gint transaction_auto_assign(GList *ope_list, guint key);

#endif
