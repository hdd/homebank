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

#ifndef __HOMEBANK_DEFLISTS_H__
#define __HOMEBANK_DEFLISTS_H__

GtkWidget *defaccount_list_new(gboolean dotoggle);
GtkWidget *defpayee_list_new(gboolean dotoggle);
GtkWidget *defcategory_list_new(gboolean dotoggle);
GtkWidget *defarchive_list_new(void);
GtkWidget *defbudget_list_new(void);

gint defarchive_list_sort(Archive *a, Archive *b);

#endif /* __HOMEBANK_DEFLISTS_H__ */
