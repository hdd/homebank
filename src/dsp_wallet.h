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

#ifndef __HOMEBANK_DSPWALLET_H__
#define __HOMEBANK_DSPWALLET_H__

struct wallet_data
{
	GtkWidget	*window;

	GtkWidget	*toolbar;
	GtkWidget	*menubar;
	GtkWidget	*vpaned;
	
	GtkWidget	*LV_acc;
	GtkWidget	*GR_upc;
	GtkWidget	*LV_upc;

	GtkWidget	*statusbar; /* A pointer to the status bar. */
	guint statusbar_menu_context_id; /* The context id of the menu bar */
	guint statusbar_actions_context_id; /* The context id of actions messages */


	gchar	*wintitle;
	gdouble	bank, today, future;

	//struct	Base base;

	Account *acc;

	gint	busy;

	GtkUIManager	*manager;
	GtkActionGroup *actions;

	GtkRecentManager *recent_manager;
	GtkWidget *recent_menu;

	/*
	UBYTE	accnum;
	UBYTE	pad0;
	struct	Account *acc;

	ULONG	mindate, maxdate;
	ULONG	change;
	ULONG	keyvalue;
	UBYTE	title[140];
	UBYTE	Filename[108];
	UBYTE	csvpath[108];
	*/
};


GtkWidget *create_wallet_window(GtkWidget *do_widget);
void wallet_open_internal(GtkWidget *widget, gpointer user_data);
void wallet_updateacc(GtkWidget *widget, gpointer user_data);
void wallet_update(GtkWidget *widget, gpointer user_data);
void wallet_compute_balances(GtkWidget *widget, gpointer user_data);
void wallet_populate_listview(GtkWidget *widget, gpointer user_data);
void wallet_action_help_welcome(void);

#endif /* __HOMEBANK_DSPWALLET_H__ */
