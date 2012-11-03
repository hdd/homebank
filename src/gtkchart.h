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

#ifndef __GTK_CHART_H__
#define __GTK_CHART_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_CHART            (gtk_chart_get_type ())
#define GTK_CHART(obj)			  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHART, GtkChart))
#define GTK_CHART_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHART, GtkChartClass)
#define GTK_IS_CHART(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHART))
#define GTK_IS_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHART))
#define GTK_CHART_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CHART, GtkChartClass))

typedef struct _GtkChart		GtkChart;
typedef struct _GtkChartClass	GtkChartClass;

typedef gchar (* GtkChartPrintIntFunc)    (gint value, gboolean minor);
typedef gchar (* GtkChartPrintDoubleFunc) (gdouble value, gboolean minor);

/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

enum
{
	CHART_BAR_TYPE,
	CHART_LINE_TYPE,
	CHART_PIE_TYPE,
	CHART_TYPE_MAX
};


#define CHART_BUFFER_LENGTH 128

#define	NUM_COLORMAP_MAX	24



/* you should access only the entry and list fields directly */
struct _GtkChart
{
	/*< private >*/
	GtkHBox			hbox;

	GtkWidget		*drawarea;
	GtkTreeModel	*legend;
	GtkAdjustment	*adjustment;
	GtkWidget		*scrollbar;
	GtkWidget		*scrollwin;

	gint			type;
	gboolean		minor;

	gdouble			minor_rate;
	gchar			*minor_symbol;

	GtkTreeModel	*model;
	GtkWidget		*tooltipwin;
	GtkWidget		*tttitle;
	GtkWidget		*ttlabel;

	GdkColor		normal_colors[NUM_COLORMAP_MAX];
	GdkColor		over_colors[NUM_COLORMAP_MAX];



	guint		entries;
	gchar		*title;
	gint		*id;
	gchar		**titles;
	gdouble		*datas1;
	gdouble		*datas2;
	gboolean	dual;
	/*gint		test;*/

	gint		ox, oy;
	gint		lastx, lasty, lastactive;
	gint		lastpress_x, lastpress_y;
	gint		active;
	guint		timer_tag;

	/* pie specifics */
	gdouble		total;
	gint		rayon, left, top;

	/* bar specifics */
	gdouble	range, min, max, unit, minimum;
	gint	div;
	gint visible;

	gint scale_width, scale_height;	//scale text dimension
	gint graph_width, graph_height;	//graph dimension
	gint unit_height;
	gint barw, posbarh, negbarh;

	gchar			buffer[CHART_BUFFER_LENGTH];
};

struct _GtkChartClass {
	GtkHBoxClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GtkType      gtk_chart_get_type              (void);

/* public function */
GtkWidget *gtk_chart_new(gint type);

void gtk_chart_set_type(GtkChart *chart, gint type);

void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column);
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2);

void gtk_chart_set_title(GtkChart * chart, gchar *title);

void gtk_chart_set_legend(GtkChart * chart, gboolean visible);
void gtk_chart_set_minor(GtkChart * chart, gboolean minor);
void gtk_chart_set_minor_prefs(GtkChart * chart, gdouble rate, gchar *symbol);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_CHART_H__ */
