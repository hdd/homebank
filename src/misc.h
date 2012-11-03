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

#ifndef __MISC__H__
#define __MISC__H__

gint hb_strfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor);
gint hb_strfmonall(gchar *outstr, gint outlen, gdouble value, gboolean minor);
void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor);

void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate);
void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate);

gchar hb_string_strip_crlf(gchar *str);

gboolean hb_string_csv_valid(gchar *str, gint nbcolumns, gint *csvtype);

guint32 hb_date_get_julian_parse(gchar *str);

void hex_dump(guchar *ptr, gint length);


#endif /* __MISC__H__ */
