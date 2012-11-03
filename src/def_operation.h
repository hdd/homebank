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

#ifndef __HOMEBANK_DEFOPERATION_H__
#define __HOMEBANK_DEFOPERATION_H__

GtkWidget *create_defoperation_window (Operation *ope, gint type, gint accnum);
void defoperation_set_operation(GtkWidget *widget, Operation *ope);
void defoperation_get			(GtkWidget *widget, gpointer user_data);
void defoperation_add			(GtkWidget *widget, gpointer user_data);
void defoperation_dispose(GtkWidget *widget, gpointer user_data);


#endif /* __HOMEBANK_DEFOPERATION_H__ */
