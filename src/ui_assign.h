/*  HomeBank -- Free, easy, personal rulounting for everyone.
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

#ifndef __HB_ASSIGN_GTK_H__
#define __HB_ASSIGN_GTK_H__

enum
{
	LST_DEFASG_TOGGLE,
	LST_DEFASG_DATAS,
	NUM_LST_DEFASG
};

gchar *ui_asg_comboboxentry_get_name(GtkComboBoxEntry *entry_box);
guint32 ui_asg_comboboxentry_get_key(GtkComboBoxEntry *entry_box);
gboolean ui_asg_comboboxentry_set_active(GtkComboBoxEntry *entry_box, guint32 key);
void ui_asg_comboboxentry_add(GtkComboBoxEntry *entry_box, Assign *asg);
void ui_asg_comboboxentry_populate(GtkComboBoxEntry *entry_box, GHashTable *hash);
void ui_asg_comboboxentry_populate_except(GtkComboBoxEntry *entry_box, GHashTable *hash, guint except_key);
GtkWidget *ui_asg_comboboxentry_new(GtkWidget *label);

/* = = = = = = = = = = */

void ui_asg_listview_add(GtkTreeView *treeview, Assign *item);
guint32 ui_asg_listview_get_selected_key(GtkTreeView *treeview);
void ui_asg_listview_remove_selected(GtkTreeView *treeview);
void ui_asg_listview_populate(GtkWidget *view);
GtkWidget *ui_asg_listview_new(gboolean withtoggle);

/* = = = = = = = = = = */

struct ui_asg_manage_data
{
	GList	*tmp_list;
	gint	change;
	gint	action;
	guint32	lastkey;

	GtkWidget	*window;

	GtkWidget	*LV_rul;
	GtkWidget	*ST_name;
	GtkWidget	*CM_exact;
	
	GtkWidget	*PO_pay;
	GtkWidget	*PO_cat;


	GtkWidget	*BT_new, *BT_rem;

	//gulong		handler_id[MAX_RUL_FIELD];

};

struct rulPopContext
{
	GtkTreeModel *model;
	guint	except_key;
};



GtkWidget *ui_asg_manage_dialog (void);


#endif

