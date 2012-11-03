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

#include <math.h>
#include <monetary.h>	/* strfmon */
#include <string.h>

#include <gtk/gtk.h>

#include "gtkchart.h"

#define DEFAULT_DELAY 500           /* Default delay in ms */

#define MASKCOL 255
#define COLTO16(col8) ( (col8 | col8<<8 ) )
#define COLTOOVER(col8) ( (col8 + MASKCOL) / 2 )

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

#define PROPERTY_DATE		5
#define PROP_SHOW_MINOR		6
#define PROP_SHOW_LEGEND	7

static void         gtk_chart_class_init      (GtkChartClass *klass);
static void         gtk_chart_init            (GtkChart      *chart);
static void         gtk_chart_destroy         (GtkObject     *chart);


double GetBase10(double num);
double GetUnit(double value);
gchar *chart_printval(GtkChart *chart, gdouble value);
void chart_clear(GtkChart *chart, gboolean store);
void chart_setup_with_model(GtkChart *chart, GtkTreeModel *list_store, guint column1, guint column2);
void chart_sizeallocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
static gboolean chart_map( GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean chart_expose( GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static gboolean chart_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static gboolean chart_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean chart_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean chart_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
static gboolean chart_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);

void chart_tooltip_start_delay(GtkChart *chart);


void barchart_calculation(GtkChart *chart);
static void barchart_draw_scale(GtkWidget *widget, GdkGC *gc, gpointer user_data);
static void barchart_draw_bars(GtkWidget *widget, GdkGC *gc, gpointer user_data);
static gint barchart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data);
static void barchart_first_changed( GtkAdjustment *adj, gpointer user_data);
void barchart_scrollbar_setvalues(GtkChart *chart);
void barchart_compute_range(GtkChart *chart);

void piechart_calculation(GtkChart *chart);
static void piechart_draw_slices(GtkWidget *widget, GdkGC *gc, gpointer user_data);
static gint piechart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data);

void chart_tooltip_show(GtkChart *chart, gint xpos, gint ypos);
void chart_cleanup_colors(GtkWidget *widget);

gboolean chart_setup_colors(GtkWidget *widget, GtkChart *chart );
static GdkPixbuf *create_color_pixbuf (GdkColor *col);
GtkWidget *legend_list_new(void);

void chart_recompute(GtkChart *chart);


static GtkHBoxClass *parent_class = NULL;


struct mycolors
{
	guint8	r, g, b;
};

struct mycolors colors[] =
{
	{ 255, 193,  96 },
	{  92, 131, 180 },
	{ 165,  88, 124 },
	{ 108, 124, 101 },
	{ 230, 121,  99 },
	{  91, 160, 154 },
	{ 207,  93,  96 },
	{  70, 136, 106 },
	{ 245, 163,  97 },
	{ 158, 153,  88 },
	{ 255, 140,  90 },
	{ 122, 151, 173 },
	{  84, 142, 128 },
	{ 185, 201, 149 },
	{ 165,  99, 103 },
	{  77, 140, 172 },
	{ 251, 228, 128 },
	{  73,  99, 149 },
	{ 192,  80,  77 },
	{ 139, 180, 103 },
	{ 132, 165, 214 },
	{ 221, 216, 115 },
	{  77, 103, 137 },
	{ 165, 181, 156 },
};

GType
gtk_chart_get_type ()
{
static GType chart_type = 0;

	if (!chart_type)
    {
		static const GTypeInfo chart_info =
		{
		sizeof (GtkChartClass),
		NULL,		/* base_init */
		NULL,		/* base_finalize */
		(GClassInitFunc) gtk_chart_class_init,
		NULL,		/* class_finalize */
		NULL,		/* class_data */
		sizeof (GtkChart),
		0,		/* n_preallocs */
		(GInstanceInitFunc) gtk_chart_init,

		};

		chart_type = g_type_register_static (GTK_TYPE_HBOX, "GtkChart",
							 &chart_info, 0);

	}
	return chart_type;
}

static void
gtk_chart_class_init (GtkChartClass * class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass*) class;
  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = g_type_class_peek_parent (class);

	DB( g_print(" (chart) class_init\n") );

	object_class->destroy = gtk_chart_destroy;

}

static void
gtk_chart_init (GtkChart * chart)
{
GtkWidget *widget, *vbox, *frame, *scrollwin, *treeview;

	DB( g_print(" (chart) init\n") );

 	chart->entries = 0;
 	chart->model = NULL;
 	chart->id = NULL;
 	chart->titles = NULL;
 	chart->datas1 = NULL;
 	chart->datas2 = NULL;
	chart->dual = FALSE; 
 	
	chart->tooltipwin = NULL;
	chart->active = -1;

 	chart->minor_rate = 1.0;

	widget=GTK_WIDGET(chart);

	GTK_BOX(widget)->homogeneous = FALSE;

	vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (widget), vbox, TRUE, TRUE, 0);

	/* drawing area */
	frame = gtk_frame_new(NULL);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);

	chart->drawarea = gtk_drawing_area_new();
	gtk_container_add( GTK_CONTAINER(frame), chart->drawarea );
	gtk_widget_set_size_request(chart->drawarea, 200, 200 );
	gtk_widget_show(chart->drawarea);

	/* scrollbar */
    chart->adjustment = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
    chart->scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (chart->adjustment));
    gtk_range_set_update_policy (GTK_RANGE (chart->scrollbar), GTK_UPDATE_CONTINUOUS);
    gtk_box_pack_start (GTK_BOX (vbox), chart->scrollbar, FALSE, TRUE, 0);

	/* legend treeview */
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	chart->scrollwin = scrollwin;
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	treeview = legend_list_new();
	chart->legend = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_box_pack_start (GTK_BOX (widget), scrollwin, FALSE, FALSE, 0);

	gtk_widget_set_events(GTK_WIDGET(chart->drawarea),
		GDK_EXPOSURE_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK |
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK
		);

	g_signal_connect( G_OBJECT(chart->drawarea), "size-allocate", G_CALLBACK(chart_sizeallocate), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "map-event", G_CALLBACK(chart_map), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "expose-event", G_CALLBACK(chart_expose), chart ) ;

	g_signal_connect( G_OBJECT(chart->drawarea), "motion-notify-event", G_CALLBACK(chart_motion), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "leave-notify-event", G_CALLBACK(chart_leave), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "enter-notify-event", G_CALLBACK(chart_enter), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "button-press-event", G_CALLBACK(chart_button_press), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "button-release-event", G_CALLBACK(chart_button_release), chart );

	g_signal_connect (G_OBJECT(chart->adjustment), "value_changed", G_CALLBACK (barchart_first_changed), chart);

}

/* --- */

GtkWidget *
gtk_chart_new (gint type)
{
GtkChart *chart;

	DB( g_print(" (chart) new\n") );

	chart = g_object_new (GTK_TYPE_CHART, NULL);
	chart->type = type;

	return GTK_WIDGET(chart);
}

static void
gtk_chart_destroy (GtkObject * object)
{
GtkChart *chart;

	DB( g_print(" (chart) destroy\n") );

  g_return_if_fail (GTK_IS_CHART (object));

	chart = GTK_CHART (object);

	chart_clear(chart, FALSE);

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* public functions */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
** change the model and/or column
*/
void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column, -1);
		chart_recompute(chart);
	}
	else
	{	
		chart_clear(chart, TRUE);
		gtk_widget_queue_draw( chart->drawarea );
	}
}

/*
** change the model and/or column
*/
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column1, column2 );
		chart_recompute(chart);
	}
	else
	{	
		chart_clear(chart, TRUE);
		gtk_widget_queue_draw( chart->drawarea );
	}
}


/*
** change the type dynamically
*/
void gtk_chart_set_type(GtkChart * chart, gint type)
{
	g_return_if_fail (GTK_IS_CHART (chart));
	//g_return_if_fail (type < CHART_TYPE_MAX);

	DB( g_print(" (chart) set type %d\n", type) );

	chart->type = type;
	chart->dual = FALSE;
	chart_recompute(chart);
}

/*
** change the legend visibility
*/
void gtk_chart_set_legend(GtkChart * chart, gboolean visible)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	if(visible == TRUE)
		gtk_widget_show(chart->scrollwin);
	else
		gtk_widget_hide(chart->scrollwin);
}

/*
** change the minor visibility
*/
void gtk_chart_set_minor(GtkChart * chart, gboolean minor)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->minor = minor;

	if(chart->type != CHART_PIE_TYPE)
		chart_recompute(chart);

}

/*
** change the legend visibility
*/
void gtk_chart_set_minor_prefs(GtkChart * chart, gdouble rate, gchar *symbol)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->minor_rate   = rate;
	chart->minor_symbol = symbol;




}

// ---------------------------------------------
/*
**
*/
double GetBase10(double num)
{
double result;
double cnt;

	for(cnt=0;;cnt++)
	{
		if(floor(num) == 0) break;
		num = num / 10;
	}
	result = pow(10.0, cnt-1);
	return(result);
}

/*
**
*/
double GetUnit(double value)
{
double truc, base10, unit;

	value = ABS(value);

	base10 = GetBase10(value);
	truc = value / base10;
	if(truc > 5) unit = base10;
	else
		if(truc > 2) unit = 0.5*base10;
		else
			if(truc > 1) unit = 0.2*base10;
			else
				unit = 0.1*base10;

	return(unit);
}

/*
** print a integer number
*/
gchar *chart_print_int(GtkChart *chart, gint value)
{
	if(chart->minor)
	{
		value *= chart->minor_rate;
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%!.0n ", (gdouble)value);
		strcat(chart->buffer, chart->minor_symbol);
	}
	else
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%.0n", (gdouble)value);

	return chart->buffer;
}

/*
** print a double number
*/
gchar *chart_print_double(GtkChart *chart, gdouble value)
{

	if(chart->minor)
	{
		value *= chart->minor_rate;
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%!n ", (gdouble)value);
		strcat(chart->buffer, chart->minor_symbol);
	}
	else
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%n", (gdouble)value);

	return chart->buffer;
}


/*
** clear any allocated memory
*/
void chart_clear(GtkChart *chart, gboolean store)
{
guint i;

	DB( g_print("** chart clear\n") );

	//free & clear any previous allocated datas
	if(chart->titles != NULL)
	{
		for(i=0;i<chart->entries;i++)
		{
			g_free(chart->titles[i]);
		}
		g_free(chart->titles);
		chart->titles = NULL;
	}

	if(chart->datas1 != NULL)
	{
		g_free(chart->datas1);
		chart->datas1 = NULL;
	}

	if(chart->datas2 != NULL)
	{
		g_free(chart->datas1);
		chart->datas2 = NULL;
	}

	if(chart->id != NULL)
	{
		g_free(chart->id);
		chart->id = NULL;
	}

	if(store == TRUE)
	{
		gtk_list_store_clear (GTK_LIST_STORE(chart->legend));

	}

	chart->entries = 0;

	chart->total = 0;
	chart->range = 0;
	chart->min = 0;
	chart->max = 0;
	chart->minimum = -250;

}

/*
** setup our chart with a model and column
*/
void chart_setup_with_model(GtkChart *chart, GtkTreeModel *list_store, guint column1, guint column2)
{
guint i;
gboolean valid;
GtkTreeIter iter, l_iter;
gint color;
GdkColor colour;

	DB( g_print("** setup with model\n") );

	chart_clear(chart, TRUE);

	chart->entries = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_store), NULL);

	DB( g_print(" nb=%d\n", chart->entries) );


	chart->id     = g_malloc0(chart->entries * sizeof(gint));
	chart->titles = g_malloc0(chart->entries * sizeof(gchar *));
	chart->datas1 = g_malloc0(chart->entries * sizeof(gdouble));
	chart->datas2 = g_malloc0(chart->entries * sizeof(gdouble));

	/* dual mode ? */
	if( chart->type == CHART_BAR_TYPE )
		chart->dual = column2 != -1 ? TRUE : FALSE;
	else
		chart->dual = FALSE;

	/* Get the first iter in the list */
	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store), &iter);
	i = 0;
	while (valid)
    {
	gint id;
	gchar *str_data;
	gdouble	value1, value2;

		/* column 0: pos (gint) */
		/* column 1: key (gint) */
		/* column 2: name (gchar) */
		/* column x: values (double) */

		if( !chart->dual )
		{
			gtk_tree_model_get (GTK_TREE_MODEL(list_store), &iter,
				0, &id,
				2, &str_data,
				column1, &value1,
				-1);
		}
		else
		{
			gtk_tree_model_get (GTK_TREE_MODEL(list_store), &iter,
				0, &id,
				2, &str_data,
				column1, &value1,
				column2, &value2,
				-1);
		}

		/* ignore negative value: total, partial total */
		//todo: remove this (was a test)
		if(id < 0)
		{
			chart->entries--;
	     	 //DB( g_print ("ignoring Row %d: (%s, %2.f)\n", id, str_data, value) );
		}
		else
		{
			chart->id[i] = id;
			chart->titles[i] = str_data;

			/* data1 value storage & min, max compute */
			chart->datas1[i] = (chart->dual) ? ABS(value1) : value1;
			chart->min = MIN(chart->min, chart->datas1[i]);
			chart->max = MAX(chart->max, chart->datas1[i]);

			if( chart->dual )
			{
				/* data2 value storage & min, max compute */
				chart->datas2[i] = (chart->dual) ? ABS(value2) : value2;
				chart->min = MIN(chart->min, chart->datas2[i]);
				chart->max = MAX(chart->max, chart->datas2[i]);
			}

			/* pie chart total sum */
			chart->total += ABS(chart->datas1[i]);

	     	 /* Do something with the data */

		//populate our legend list
			//color = i%NUM_COLORMAP_MAX;
			color = id%NUM_COLORMAP_MAX;

			//DB( g_print ("Row %d: (%s, %2.f) color %d\n", id, str_data, value, color) );

			colour.red   = COLTO16(colors[color].r);
			colour.green = COLTO16(colors[color].g);
			colour.blue  = COLTO16(colors[color].b);

	        gtk_list_store_append (GTK_LIST_STORE(chart->legend), &l_iter);
	        gtk_list_store_set (GTK_LIST_STORE(chart->legend), &l_iter,
	                            0, create_color_pixbuf (&colour),
	                            1, str_data,
	                            -1);

			i++;
		}
		valid = gtk_tree_model_iter_next (list_store, &iter);
	}

	//g_print("total is %.2f\n", total);


}

/*
** recompute according to type
*/
void chart_recompute(GtkChart *chart)
{

	DB( g_print("** chart recompute\n") );

	switch(chart->type)
	{
		case CHART_LINE_TYPE:
			//gtk_widget_show(chart->scrollwin);

		case CHART_BAR_TYPE:
			barchart_compute_range(chart);

			barchart_calculation(chart);
			chart->adjustment->value = 0;
			barchart_scrollbar_setvalues(chart);
			gtk_widget_show(chart->scrollbar);
			break;
		case CHART_PIE_TYPE:
			piechart_calculation(chart);
			gtk_widget_hide(chart->scrollbar);
			break;
	}
	gtk_widget_queue_draw( chart->drawarea );
}




/* bar section */

void barchart_compute_range(GtkChart *chart)
{
gint div;

	DB( g_print("** bar compute range\n") );

	chart->range = chart->max - chart->min;

	//DB(g_print(" min=%.2f, max=%.2f, range=%.2f\n", chart->min, chart->max, chart->range) );

	chart->unit  = GetUnit(chart->range);

	div = ceil(chart->max / chart->unit);
	chart->max   = div * chart->unit;
	chart->div   = div;

	div = ceil(-chart->min / chart->unit);
	chart->min   = -div * chart->unit;
	chart->div  += div;

	chart->range = chart->max - chart->min;

	/*
	chart->unit  = GetUnit(chart->range);
	chart->div   = (ceil(chart->range / chart->unit));
	chart->min   = -chart->unit*ceil(-chart->min/chart->unit);
	chart->max   = chart->unit*ceil(chart->max/chart->unit);
	chart->range = chart->unit*chart->div;
	*/

	//DB(g_print(" unit=%.2f, div=%d => %d\n", chart->unit, chart->div, (gint)chart->unit*chart->div) );
	//DB(g_print(" min=%.2f, max=%.2f, range=%.2f\n", chart->min, chart->max, chart->range) );


}

/*
**
** here we will compute every value which is size dependent
*/
void barchart_calculation(GtkChart *chart)
{
GtkWidget *drawarea = chart->drawarea;
	PangoLayout *layout;
	gint w, h;
	gchar *valstr;

	DB( g_print("** bar calculation\n") );

	/* compute width inc pixel of x scale text */
	valstr = chart_print_int(chart, (gint)MAX(-chart->min, chart->max) * -1);

	layout = gtk_widget_create_pango_layout(chart->drawarea, NULL);
	pango_layout_set_text(layout, valstr, strlen(valstr));
	pango_layout_get_pixel_size(layout, &w, &h);
	g_object_unref(layout);

	chart->scale_width = w + 4;
	chart->scale_height = h;

	chart->graph_width  = drawarea->allocation.width - chart->scale_width;
	chart->graph_height = drawarea->allocation.height - chart->scale_height - 4;

	//if expand : we compute available space
	//barw = MAX(32, (widget->allocation.width-scalew-8)/entries);
	chart->barw = 32;

	chart->visible = chart->graph_width / chart->barw;
	chart->visible = MIN(chart->visible, chart->entries);

	chart->unit_height = (chart->unit / chart->range) * chart->graph_height;

	chart->ox = 0 + chart->scale_width;
	chart->oy = 0 + chart->scale_height + (chart->max/chart->range) * chart->div * chart->unit_height;

	chart->posbarh = (chart->max/chart->range) * chart->div * chart->unit_height;
	chart->negbarh = (-chart->min/chart->range) * chart->div * chart->unit_height;

	//debug
	//DB(g_print("scale  :: w=%d, h=%d\n", scale_width, scale_height) );
	//DB(g_print("graph  :: w=%d, h=%d\n", graph_width, graph_height) );

	//DB(g_print("unit_height=%d\n", unit_height) );
	//DB(g_print("ox=%d, oy=%d\n", ox, oy) );


}


/*
** draw the scale
*/
static void barchart_draw_scale(GtkWidget *widget, GdkGC *gc, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
PangoLayout *layout;
GdkColor colour;
gint w, h;
gint i, y;
gdouble curxval;
gchar *valstr;

	DB( g_print("** bar draw scale\n") );

	/* draw minimum ? */
	//DB( g_print(" %f > %f\n", chart->minimum , chart->min) );

	if(chart->type == CHART_LINE_TYPE && chart->minimum != 0 && chart->minimum >= chart->min)
	{
		//todo: use a colormap
		colour.red   = COLTO16(200);
		colour.green = COLTO16(0);
		colour.blue  = COLTO16(0);
		gdk_gc_set_rgb_fg_color(gc, &colour);

		//DB( g_print(" \n", chart->minimum , chart->min) );

		y = chart->oy + (ABS(chart->minimum)/chart->range) * chart->div * chart->unit_height;

		gdk_draw_line (widget->window, gc,
                    chart->ox-2, y, chart->ox+chart->graph_width, y);

		//todo: use a colormap
		colour.red   = COLTO16(241);
		colour.green = COLTO16(192);
		colour.blue  = COLTO16(192);
		gdk_gc_set_rgb_fg_color(gc, &colour);


		gdk_draw_rectangle (widget->window,
			      gc,
			      TRUE,
			      chart->ox+1,
			      y+1,
			      chart->ox+chart->graph_width,
			      chart->oy+chart->negbarh - y - 1);


	}

	/* -- draw the y-axis (horizontal lines + scale) -- */

	//todo: use a colormap
	colour.red   = COLTO16(200);
	colour.green = COLTO16(200);
	colour.blue  = COLTO16(200);
	gdk_gc_set_rgb_fg_color(gc, &colour);

	layout = gtk_widget_create_pango_layout(widget, NULL);

	curxval = chart->max;
	for(i=0;i<=chart->div;i++)
	{
		y = chart->scale_height + (i * chart->unit_height);
		gdk_draw_line (widget->window, curxval ? gc : widget->style->fg_gc[widget->state],
                    chart->ox-2, y, chart->ox+chart->graph_width, y);

		valstr = chart_print_int(chart, (gint)curxval);
		pango_layout_set_text(layout, valstr, strlen(valstr));
		pango_layout_get_pixel_size(layout, &w, &h);
		gdk_draw_layout(widget->window, widget->style->fg_gc[widget->state], chart->ox - w - 2, y-h, layout);

		curxval -= chart->unit;
	}

	g_object_unref(layout);




	/* x-axis (vertical line) */
	gdk_draw_line (widget->window, widget->style->fg_gc[widget->state],
                     chart->ox, chart->oy-chart->posbarh, chart->ox, chart->oy+chart->negbarh);
}

/*
** draw all visible bars
*/
static void barchart_draw_bars(GtkWidget *widget, GdkGC *gc, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint x,y, scale;
gint first;
gint i;

	DB( g_print("** bar draw bars\n") );

	x = chart->ox + 1;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	for(i=first; i<(first+chart->visible) ;i++)
	{
	gint color;
	gint barw = chart->barw;
	gint blkw = chart->barw;

		if( chart->dual ) barw /= 2;

		//if(!chart->datas1[i]) goto nextbar;

		//color = i%NUM_COLORMAP_MAX;
		color = chart->id[i]%NUM_COLORMAP_MAX;

		if(i == chart->active)
			gdk_gc_set_foreground(gc, &chart->over_colors[color]);
		else
			gdk_gc_set_foreground(gc, &chart->normal_colors[color]);

		if(chart->datas1[i])
		{
			if(chart->datas1[i] > 0.0)
			{
				scale = (chart->datas1[i]/chart->max) * (chart->posbarh);
				//gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], FALSE, x+1, y-scale, barw-2, scale );
				//gdk_draw_rectangle(widget->window, gc, TRUE, x+2, y-scale+1, barw-3, scale-1 );
				gdk_draw_rectangle(widget->window, gc, TRUE, x+2, y-scale, barw-3, scale );
			}
			else
			{
				scale = (chart->datas1[i]/chart->min) * (chart->negbarh);
				//gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], FALSE, x+1, y, barw-2, scale );
				//gdk_draw_rectangle(widget->window, gc, TRUE, x+2, y+1, barw-3, scale-1 );
				gdk_draw_rectangle(widget->window, gc, TRUE, x+2, y+1, barw-3, scale+1 );
			}
		}

		if( chart->dual && chart->datas2[i])
		{

			if(chart->datas2[i] > 0.0)
			{
				scale = (chart->datas2[i]/chart->max) * (chart->posbarh);
				gdk_draw_rectangle(widget->window, gc, TRUE, x+barw+1, y-scale, barw-3, scale );
			}
			else
			{
				scale = (chart->datas2[i]/chart->min) * (chart->negbarh);
				gdk_draw_rectangle(widget->window, gc, TRUE, x+barw+1, y+1, barw-3, scale+1 );
			}
		
		}

nextbar:
		x += blkw;

		//debug
		//gdk_draw_line (widget->window, widget->style->fg_gc[widget->state], x, chart->oy-chart->posbarh, x, chart->oy+chart->negbarh);

	}
}

/*
** get the bar under the mouse pointer
*/
static gint barchart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval, first, index, px, py, scale;

	px = (x - chart->ox);
	py = (y - chart->oy);
	first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
	index = first + (px / (chart->barw));
	retval = -1;

	//DB(g_printf(" -> x-test :: px=%d, index=%d\n", px, index) );

	if( px > 0 )
	{
	gdouble value;

		/*chart->test = (px / (chart->barw>>1)); */
	
		if( !chart->dual )
		{ 
			value = chart->datas1[index];
		}
		else
		{
			value = MAX( ABS(chart->datas1[index]), ABS(chart->datas2[index]) );
			//value = (chart->test & 1) == 1 ? chart->datas2[index] : chart->datas1[index];
		}

		if(value > 0.0)
		{
			scale = (value/chart->max) * (chart->posbarh);
			if(py < 0 && py >= -scale ) retval = index;
		}
		else
		{
			scale = (value/chart->min) * (chart->negbarh);
			if(py > 0 &&  py <= scale) retval = index;
		}

	//	DB(g_printf(" -> y-test :: scale=%d, py=%d => scale-py=%d\n", scale, py, scale - py) );
	}

	return(retval);
}

static void barchart_first_changed( GtkAdjustment *adj, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint first;

	DB( g_print("\n** bar first changed\n") );

	first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	DB( g_print(" first=%d\n", first) );

/*
	DB( g_print("scrollbar\n adj=%8x, low=%.2f upp=%.2f val=%.2f step=%.2f page=%.2f size=%.2f\n", adj,
		adj->lower, adj->upper, adj->value, adj->step_increment, adj->page_increment, adj->page_size) );
 */
    /* Set the number of decimal places to which adj->value is rounded */
    //gtk_scale_set_digits (GTK_SCALE (hscale), (gint) adj->value);
    //gtk_scale_set_digits (GTK_SCALE (vscale), (gint) adj->value);
	gtk_widget_queue_draw(chart->drawarea);

}

/*
** scrollbar set values for upper, page size, and also show/hide
*/
void barchart_scrollbar_setvalues(GtkChart *chart)
{
GtkAdjustment *adj = chart->adjustment;
gint first;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	DB( g_print("\n** sb_set_values\n") );

	//if(visible < entries)
	//{
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

		DB( g_print(" entries=%d, visible=%d\n", chart->entries, chart->visible) );
		DB( g_print(" first=%d, upper=%d, pagesize=%d\n", first, chart->entries, chart->visible) );

		adj->upper = (gdouble)chart->entries;
		adj->page_size = (gdouble)chart->visible;
		adj->page_increment = (gdouble)chart->visible;

		if(first+chart->visible > chart->entries)
			adj->value = (gdouble)chart->entries - chart->visible;
			//g_object_set(adj1, "value", (gdouble)entries-visible);

		gtk_adjustment_changed (adj);

		//gtk_widget_show(GTK_WIDGET(scrollbar));
	//}
	//else
		//gtk_widget_hide(GTK_WIDGET(scrollbar));


}

/* line section */

/*
** draw all visible bars
*/
static void linechart_draw_lines(GtkWidget *widget, GdkGC *gc, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint x,y, scale, x1, y1, x2, y2;
gint first;
gint i;
gint color;

	DB( g_print("** line draw lines\n") );

	x = chart->ox + 1;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	//gcvalues
	//GdkGCValues values;
	//values.line_width = 2;
	//gdk_gc_set_values(gc, &values, GDK_GC_LINE_WIDTH);

	color = 1;
	gdk_gc_set_foreground(gc, &chart->normal_colors[color]);

	for(i=first; i<(first+chart->visible) ;i++)
	{

		if(chart->datas1[i] > 0.0)
		{
			scale = (chart->datas1[i]/chart->max) * (chart->posbarh);
			//gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], FALSE, x+2, y-scale, chart->barw-3, scale );

			x2 = x + (chart->barw)/2;
			y2 = y - scale + 1;
		}
		else
		{
			scale = (chart->datas1[i]/chart->min) * (chart->negbarh);
			//gdk_draw_rectangle(widget->window, widget->style->fg_gc[widget->state], FALSE, x+2, y+1, chart->barw-3, scale+1 );

			x2 = x + (chart->barw)/2;
			y2 = y + scale + 1;
		}


		if( i > first)
		{
			gdk_draw_line(widget->window, gc, x1, y1, x2, y2 );

		}

		gdk_draw_rectangle(widget->window, gc, TRUE, x2-2, y2-2, 5, 5 );

		x1 = x2;
		y1 = y2;

		x += chart->barw;

		//debug
		//gdk_draw_line (widget->window, widget->style->fg_gc[widget->state], x, 0, x, oy);
	}

}


/*
** get the point under the mouse pointer
*/
static gint linechart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval, first, index, px, py, x2, y2, scale;

	px = (x - chart->ox);
	py = (y - chart->oy);
	first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
	index = first + (px / (chart->barw));

	retval = -1;

	x2 = ((index-first)*chart->barw) + (chart->barw)/2;

	//DB(g_printf(" -> line x-test :: px=%d, index=%d, x2=%d, px-x2=%d\n", px, index, x2, px-x2) );

	if(chart->datas1[index] > 0.0)
	{
		scale = (chart->datas1[index]/chart->max) * (chart->posbarh);
		y2 = - scale + 1;
	//	if(py <= -scale-5 && py >= -scale+5 ) retval = index;
	}
	else
	{
		scale = (chart->datas1[index]/chart->min) * (chart->negbarh);
		y2 = scale + 1;
		//if(py >= scale+5 &&  py <= scale-5) retval = index;
	}

	//DB(g_printf(" -> line y-test :: scale=%d, y2=%d, py=%d => py - y2=%d\n", scale, y2, py, py - y2) );


	if( ABS(px - x2) < 5 && ABS(py - y2) < 5  )
		retval = index;

	return(retval);
}



/* pie section */

void piechart_calculation(GtkChart *chart)
{
GtkWidget *drawarea = chart->drawarea;

	chart->ox    = drawarea->allocation.width / 2;
	chart->oy    = drawarea->allocation.height / 2;
	chart->rayon = MIN(drawarea->allocation.width-2, drawarea->allocation.height-2);
	chart->left  = (drawarea->allocation.width - chart->rayon) / 2;
	chart->top   = (drawarea->allocation.height - chart->rayon) / 2;
}

static void piechart_draw_slices(GtkWidget *widget, GdkGC *gc, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint i;
gint a1, a2;
gdouble sum;

	if(chart->entries)
	{
		a1 = 90;
		a2 = 0;
		sum = 0;
		for(i=0; i< chart->entries ;i++)
		{
		gint color;

			//color = i%NUM_COLORMAP_MAX;
			color = chart->id[i]%NUM_COLORMAP_MAX;

			sum += ABS(chart->datas1[i]);
			a2 = ((sum/chart->total) * -360) + 90;

			//DB( g_print("%d :: %.2f\n", i, (ABS(chart->datas1[i])/chart->total)) );
			DB( g_print("draw %2d: val:%9.2f max:%9.2f sum=%9.2f angle:%4d a1:%4d a2:%4d\n", i, chart->datas1[i], chart->total, sum, a2-a1, a1, a2) );

			if(i == chart->active)
				gdk_gc_set_foreground(gc, &chart->over_colors[color]);
			else
				gdk_gc_set_foreground(gc, &chart->normal_colors[color]);

			gdk_draw_arc(widget->window, gc, TRUE, chart->left, chart->top, chart->rayon, chart->rayon, a1*64, (a2-a1)*64);

			a1 = a2;
		}
		//gdk_draw_arc(widget->window, widget->style->fg_gc[widget->state], FALSE, chart->left, chart->top, chart->rayon, chart->rayon, 0*64, 360*64);
	}

}


static gint piechart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval, index, px, py;
double h;

	px = x - chart->ox;
	py = y - chart->oy;
	h  = sqrt( pow(px,2) + pow(py,2) );
	retval = -1;

	if(h < (chart->rayon/2))
	{
	double angle, b;

		b     = (acos(px / h) * 180) / M_PI;
		angle = py > 0 ? b : 360 - b;
		angle += 90;
		if(angle > 360) angle -= 360;
		//angle = 360 - angle;

		//todo optimize
		gdouble cumul = 0;
		for(index=0; index< chart->entries ;index++)
		{
			cumul += (ABS(chart->datas1[index])/chart->total)*360;
			if( cumul > angle ) break;
		}

		//DB( g_printf(" inside: x=%d, y=%d\n", x, y) );
		//DB( g_printf(" inside: b=%f angle=%f, slice is %d\n", b, angle, index) );

		retval = index;
	}
	return(retval);
}






void chart_sizeallocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	if(chart->entries == 0)
		return;

	DB( g_print("** sizeallocate\n") );
	DB( g_print("w=%d h=%d\n", widget->allocation.width, widget->allocation.height) );

	//here we will compute any widget size dependend values


	switch(chart->type)
	{
		case CHART_BAR_TYPE:
		case CHART_LINE_TYPE:
			barchart_calculation(chart);
			barchart_scrollbar_setvalues(chart);
			break;
		case CHART_PIE_TYPE:
			piechart_calculation(chart);
			break;
	}

}


/* global widget function */


static gboolean chart_map( GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	DB( g_print("** map\n") );

	/* our colors */
	chart_setup_colors(widget, chart);

	DB( g_print("** end map\n") );

	return TRUE;
}


static gboolean chart_expose( GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
GdkGC	*gc1;

  if (event->count > 0)
    return FALSE;

	if(chart->entries == 0)
		return FALSE;

	//DB( g_print("** expose\n") );

	gdk_draw_rectangle (widget->window,
		      widget->style->white_gc,
		      TRUE,
		      0, 0,
		      widget->allocation.width,
		      widget->allocation.height);

	gc1 = gdk_gc_new(widget->window);

	switch(chart->type)
	{
		case CHART_BAR_TYPE:
			barchart_draw_scale(widget, gc1, chart);
			barchart_draw_bars(widget, gc1, chart);
			break;
		case CHART_LINE_TYPE:
			barchart_draw_scale(widget, gc1, chart);
			linechart_draw_lines(widget, gc1, chart);
			break;
		case CHART_PIE_TYPE:
			piechart_draw_slices(widget, gc1, chart);
			break;
	}

	g_object_unref(gc1);

	return TRUE;
}

static gboolean chart_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint x, y;
GdkModifierType state;

	if(chart->entries == 0)
		return FALSE;

	//DB( g_print("**motion\n") );

	//todo see this
	if(event->is_hint)
	{
		//DB( g_print("is hint\n") );

		gdk_window_get_pointer(widget->window, &x, &y, &state);
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}

	//DB( g_print(" x=%d, y=%d, time=%d (%d)\n", x, y, event->time, event->time - chart->lasttime) );

	switch(chart->type)
	{
		case CHART_BAR_TYPE:
			chart->active = barchart_get_active(widget, x, y, chart);
			break;
		case CHART_LINE_TYPE:
			chart->active = linechart_get_active(widget, x, y, chart);
			break;
		case CHART_PIE_TYPE:
			chart->active = piechart_get_active(widget, x, y, chart);
			break;
	}

	// rollover redraw ?
	if(chart->lastactive != chart->active)
	{
		//DB( g_print("rollover redraw :: active=%d\n", chart->active) );

		//set_info(active);
		gtk_widget_queue_draw( widget );
		chart_tooltip_start_delay(chart);
	}



	chart->lastactive = chart->active;

	// hide the tooltip ?
	if(chart->tooltipwin != NULL && (x != chart->lastpress_x || y != chart->lastpress_y) )
	{
		//DB( g_print("hide tooltip\n") );
		gtk_widget_hide(chart->tooltipwin);

	}

	if( (event->time - chart->lasttime) > DEFAULT_DELAY)
		chart_tooltip_start_delay(chart);

	chart->lasttime = event->time;


	return TRUE;
}


static gboolean chart_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint x, y;

	if(chart->entries == 0)
		return FALSE;

	DB( g_print("button press\n") );
   	DB( g_print(" active is %d\n", chart->active) );

	if(chart->active < 0) return TRUE;

	gdk_window_get_origin (GDK_WINDOW (event->window), &x, &y);

	chart_tooltip_show(chart, x + (gint)event->x, y + (gint)event->y);

	DB( g_print("x=%d, y=%d\n", (gint)event->x, (gint)event->y) );

	chart->lastpress_x = event->x;
	chart->lastpress_y = event->y;

	return TRUE;
}




static gboolean chart_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	if(chart->entries == 0)
		return FALSE;

	DB( g_print("** release\n") );

	if(chart->tooltipwin != NULL)
		gtk_widget_hide(chart->tooltipwin);

	//chart_tooltip_start_delay(chart);

	return TRUE;
}


static gboolean chart_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	if(chart->entries == 0)
		return FALSE;

	DB( g_print("** enter\n") );

	chart_tooltip_start_delay(chart);


	return TRUE;
}

static gboolean chart_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	if(chart->entries == 0)
		return FALSE;

	DB( g_print("** leave\n") );

	chart->active = -1;
	gtk_widget_queue_draw( widget );

	if (chart->timer_tag)
	{
		g_source_remove (chart->timer_tag);
		chart->timer_tag = 0;
	}

	return TRUE;
}

static gint chart_tooltip_timeout (gpointer data)
{
GtkChart *chart = GTK_CHART(data);
gint x, y, wx, wy;
GdkModifierType state;
GtkWidget *widget = GTK_WIDGET(chart);

	GDK_THREADS_ENTER ();

	DB( g_print("** SHOULD TOOLTIP %x\n", (int)chart) );


 	//debug
 	gdk_window_get_origin (widget->window, &wx, &wy);

	gdk_window_get_pointer(widget->window, &x, &y, &state);
 	chart_tooltip_show(chart, wx+x, wy+y);
	chart->timer_tag = 0;

	GDK_THREADS_LEAVE ();

	return FALSE;
}


void chart_tooltip_start_delay(GtkChart *chart)
{

	DB( g_print("** start delay %x\n", (int)chart) );

/*
      tooltips->timer_tag = g_timeout_add (delay,
					   gtk_tooltips_timeout,
					   (gpointer) tooltips);

*/
	if(chart->timer_tag == 0 )
		chart->timer_tag = g_timeout_add( DEFAULT_DELAY, chart_tooltip_timeout, (gpointer)chart);

}

void chart_tooltip_show(GtkChart *chart, gint xpos, gint ypos)
{
GtkWidget *window, *frame, *label;
GtkRequisition req;
gint active;
gchar *strval, *buffer;

	DB( g_print("** tooltip show\n") );
   	DB( g_print(" x=%d, y=%d\n", xpos, ypos) );

	if(!chart->tooltipwin)
	{
		window = gtk_window_new (GTK_WINDOW_POPUP);
		//gtk_tooltips_update_screen (tooltips, TRUE);
		gtk_widget_set_app_paintable (window, TRUE);
		gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
		gtk_widget_set_name (window, "gtk-tooltips");

		frame = gtk_frame_new(NULL);
	    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_container_add (GTK_CONTAINER (window), frame);
		//gtk_container_set_border_width (GTK_CONTAINER (frame), 4);

		label = gtk_label_new ("tooltip");
		chart->ttlabel= label;
		gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
		//gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
		gtk_misc_set_padding(GTK_MISC(label), 2, 2);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
	

		gtk_container_add( GTK_CONTAINER(frame), label);

		//gtk_container_add (GTK_CONTAINER (window), label);

		//gtk_widget_reset_rc_styles (tooltipwin);

		chart->tooltipwin = window;

		gtk_widget_show_all(chart->tooltipwin);
	}

	/* create tooltip text */
	active = chart->active;

	if( active >= 0 )
	{
		if( !chart->dual )
		{
			strval = chart_print_double(chart, chart->datas1[active]);

			if( chart->type == CHART_PIE_TYPE )			
				buffer = g_strdup_printf("<b>%s</b>\n%s\n%.2f%%", chart->titles[active], strval, ABS(chart->datas1[active])*100/chart->total);
			else
				buffer = g_strdup_printf("<b>%s</b>\n%s", chart->titles[active], strval);
			
			gtk_label_set_markup(GTK_LABEL(chart->ttlabel), buffer);
			g_free(buffer);
		}
		else
		{
		gchar *buf1;
		
			strval = chart_print_double(chart, chart->datas1[active]);
			buf1 = g_strdup(strval);
			strval = chart_print_double(chart, chart->datas2[active]);

			buffer = g_strdup_printf("<b>%s</b>\n-%s\n+%s",
				chart->titles[active],
				buf1,
				strval
				);
				
			gtk_label_set_markup(GTK_LABEL(chart->ttlabel), buffer);
			g_free(buf1);
			g_free(buffer);
		}


		/*
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(chart->ttlabel), "Please move to:\na bar,\na slice\nor a point.");

		}
		*/

		/* position and show our tooltip */
		//gtk_widget_queue_resize(chart->tooltipwin);
		gtk_widget_size_request (chart->tooltipwin, &req);

		//DB( g_printf("size is: w%d h%d :: xpos=%d ypos=%d\n", req.width, req.height, xpos,ypos) );

		gtk_window_move(GTK_WINDOW(chart->tooltipwin), xpos - (req.width/2), ypos - req.height);

		gtk_widget_show_all(chart->tooltipwin);

	}

}


void chart_cleanup_colors(GtkWidget *widget)
{
GdkColormap *colormap;

	colormap = gtk_widget_get_colormap(widget);

	//todo freeclormap ?

}

gboolean chart_setup_colors(GtkWidget *widget, GtkChart *chart )
{
GdkColormap *colormap;
guint col;
GdkColor *normal_colors = chart->normal_colors;
GdkColor *over_colors = chart->over_colors;

	colormap = gtk_widget_get_colormap(widget);

	if(!colormap) return FALSE;
	for(col=0; col<NUM_COLORMAP_MAX; col++)
	{
		normal_colors[col].red   = COLTO16(colors[col].r);
		normal_colors[col].green = COLTO16(colors[col].g);
		normal_colors[col].blue  = COLTO16(colors[col].b);
		gdk_colormap_alloc_color( colormap, &normal_colors[col], FALSE, TRUE);

		over_colors[col].red   = COLTO16(COLTOOVER(colors[col].r));
		over_colors[col].green = COLTO16(COLTOOVER(colors[col].g));
		over_colors[col].blue  = COLTO16(COLTOOVER(colors[col].b));
		gdk_colormap_alloc_color( colormap, &over_colors[col], FALSE, TRUE);

	}
	return TRUE;
}







/* legend list */

static GdkPixbuf *
//create_color_pixbuf (const char *color)
create_color_pixbuf (GdkColor *col)
{
        GdkPixbuf *pixbuf;
        //GdkColor col = color;

        int x;
        int num;
        int rowstride;
        guchar *pixels, *p;

/*
        if (!gdk_color_parse (color, &col))
                return NULL;
                */

        pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                 FALSE, 8, //bits
                                 12, 12);		//width,height

        rowstride = gdk_pixbuf_get_rowstride (pixbuf);
        p = pixels = gdk_pixbuf_get_pixels (pixbuf);

        num = gdk_pixbuf_get_width (pixbuf) *
                gdk_pixbuf_get_height (pixbuf);

        for (x = 0; x < num; x++) {

                p[0] = col->red;
                p[1] = col->green;
                p[2] = col->blue;

				/*
                p[0] = col->red / 65535 * 255;
                p[1] = col->green / 65535 * 255;
                p[2] = col->blue / 65535 * 255;
                */
                p += 3;
        }

        return pixbuf;
}

GtkWidget *legend_list_new(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;


	//store
	store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING );

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_NONE);

	// column 1
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	// column 2
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

	//gtk_widget_style_set(view, "vertical-separator", 0);

/*
	GValue value = { 0, };
	g_value_init (&value, G_TYPE_INT);
	g_value_set_int (&value, 0);
	g_object_set_property(view, "vertical-separator", &value);
	g_value_unset (&value);
*/

	return(view);
}



