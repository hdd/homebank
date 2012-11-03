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

#ifndef __HOMEBANK_DSPWALLET_H__
#define __HOMEBANK_DSPWALLET_H__

GtkWidget *create_wallet_window(GtkWidget *do_widget);
void wallet_open_internal(GtkWidget *widget, gpointer user_data);
void wallet_updateacc(GtkWidget *widget, gpointer user_data);
void wallet_update(GtkWidget *widget, gpointer user_data);
void wallet_compute_balances(GtkWidget *widget, gpointer user_data);
void wallet_populate_listview(GtkWidget *widget, gpointer user_data);

#endif /* __HOMEBANK_DSPWALLET_H__ */
