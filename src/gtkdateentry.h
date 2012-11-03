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

#ifndef __GTK_DATE_ENTRY_H__
#define __GTK_DATE_ENTRY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_DATE_ENTRY            (gtk_dateentry_get_type ())
#define GTK_DATE_ENTRY(obj)			   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntry))
#define GTK_DATE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass)
#define GTK_IS_DATE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATE_ENTRY))
#define GTK_IS_DATE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATE_ENTRY))
#define GTK_DATE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass))


typedef struct _GtkDateEntry		GtkDateEntry;
typedef struct _GtkDateEntryClass	GtkDateEntryClass;

/* you should access only the entry and list fields directly */
struct _GtkDateEntry
{
	/*< private >*/
	GtkHBox hbox;

	/*< public >*/
	GtkWidget *entry;
    GtkWidget *arrow;
	GtkWidget *popup;
	GtkWidget *popwin;
	GtkWidget *frame;
	GtkWidget *calendar;

	GDate	*date;
	guint32	lastdate;
};

struct _GtkDateEntryClass
{
	GtkHBoxClass parent_class;

  /* signals */
  void     (* changed)          (GtkDateEntry *dateentry);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GtkType		gtk_dateentry_get_type(void);

GtkWidget	*gtk_dateentry_new(void);

void		gtk_dateentry_set_date(GtkDateEntry * dateentry, guint julian_days);
guint		gtk_dateentry_get_date(GtkDateEntry * dateentry);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_DATE_ENTRY_H__ */


