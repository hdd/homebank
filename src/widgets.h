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

#ifndef __HOMEBANK_WIDGETS_H__
#define __HOMEBANK_WIDGETS_H__

void load_paymode_icons(void);
void free_paymode_icons(void);

GtkWidget *make_label(char *str, gfloat xalign, gfloat yalign);
GtkWidget *make_text(gfloat xalign);
GtkWidget *make_string(GtkWidget *label);
GtkWidget *make_string_maxlength(GtkWidget *label, guint max_length);
GtkWidget *make_amount(GtkWidget *label);
GtkWidget *make_euro(GtkWidget *label);
GtkWidget *make_numeric(GtkWidget *label, gdouble min, gdouble max);
GtkWidget *make_long(GtkWidget *label);
GtkWidget *make_year(GtkWidget *label);
GtkWidget *make_cycle(GtkWidget *label, gchar **items);
GtkWidget *make_radio(GtkWidget *label, gchar **items);

void
gimp_label_set_attributes (GtkLabel *label,
                           ...);

guint make_popaccount_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popaccount(GtkWidget *label);

guint make_poppayee_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poppayee(GtkWidget *label);

guint make_poparchive_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poparchive(GtkWidget *label);

guint make_popcategory_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popcategory(GtkWidget *label);

GtkWidget *make_paymode(GtkWidget *label);

#endif /* __HOMEBANK_WIDGETS_H__ */
