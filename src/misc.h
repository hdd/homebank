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

#ifndef __MISC__H__
#define __MISC__H__

gint real_mystrfmon(gchar *outstr, gint outlen, gchar *buf1, struct Currency *cur);
gint mystrfmon_int(gchar *outstr, gint outlen, gdouble value, gboolean minor);
gint mystrfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor);

void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor);

void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate);
void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate);

void hb_string_strip_crlf(gchar *str);
gchar* hb_strdup_nobrackets (const gchar *str);

gboolean hb_string_csv_valid(gchar *str, gint nbcolumns, gint *csvtype);

guint32 hb_date_get_julian_parse(gchar *str);

void hex_dump(guchar *ptr, guint length);


#endif /* __MISC__H__ */
