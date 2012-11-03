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

#include <string.h>
#include <time.h>

#include <gtk/gtkhbox.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkcalendar.h>
#include <gtk/gtkentry.h>

#include "gtkdateentry.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

enum {
  CHANGED,
  LAST_SIGNAL
};

enum {
	PROPERTY_DATE = 5,
};

static void         gtk_dateentry_class_init      (GtkDateEntryClass *klass);
static void         gtk_dateentry_init            (GtkDateEntry      *dateentry);
static void         gtk_dateentry_destroy         (GtkObject     *dateentry);
static void         gtk_dateentry_popup_display   (GtkDateEntry *dateentry);
static gint	    gtk_dateentry_arrow_press     (GtkWidget * widget,
					  	  GtkDateEntry * dateentry);
static gint         gtk_dateentry_button_press    (GtkWidget     *widget,
				                  GdkEvent      *event,
                                                  gpointer data);

static void gtk_dateentry_entry_new(GtkWidget * calendar, gpointer user_data);
static gint gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry);
static void gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data);
static void gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry);
static gint gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry);
static gint key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gint gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer data);

static void gtk_dateentry_datetoentry(GtkDateEntry * dateentry);


static void
gtk_dateentry_set_property (GObject         *object,
                        guint            prop_id,
                        const GValue    *value,
                        GParamSpec      *pspec);

static void
gtk_dateentry_get_property (GObject         *object,
                        guint            prop_id,
                        GValue          *value,
                        GParamSpec      *pspec);

static GtkHBoxClass *parent_class = NULL;
static guint dateentry_signals[LAST_SIGNAL] = {0,};


GType
gtk_dateentry_get_type ()
{
static GType dateentry_type = 0;

	DB( g_print(" (dateentry) get_type\n") );

	if (!dateentry_type)
    {
		static const GTypeInfo dateentry_info =
		{
		sizeof (GtkDateEntryClass),
		NULL,		/* base_init */
		NULL,		/* base_finalize */
		(GClassInitFunc) gtk_dateentry_class_init,
		NULL,		/* class_finalize */
		NULL,		/* class_data */
		sizeof (GtkDateEntry),
		0,		/* n_preallocs */
		(GInstanceInitFunc) gtk_dateentry_init,

		};

		//dateentry_type = gtk_type_unique (gtk_hbox_get_type (), &dateentry_info);

		dateentry_type = g_type_register_static (GTK_TYPE_HBOX, "GtkDateEntry",
							 &dateentry_info, 0);


	}
	return dateentry_type;
}

static void
gtk_dateentry_class_init (GtkDateEntryClass * klass)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass*) klass;
  object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;

  parent_class = g_type_class_peek_parent (klass);

	DB( g_print(" (dateentry) class_init\n") );


	object_class->destroy = gtk_dateentry_destroy;

  dateentry_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkDateEntryClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

 /*
	gobject_class->set_property = gtk_dateentry_set_property;
	gobject_class->get_property = gtk_dateentry_get_property;

	g_object_class_install_property (gobject_class,
			PROPERTY_DATE,
			g_param_spec_uint(	"date",
								"Date",
							    "The date currently selected",
							    0, G_MAXUINT,
							    0,
							    (G_PARAM_READABLE | G_PARAM_WRITABLE)
							   )
			);
	*/
}

static void
gtk_dateentry_init (GtkDateEntry * dateentry)
{
GtkWidget *widget;
GtkWidget *arrow;

	DB( g_print(" (dateentry) init\n") );

	widget=GTK_WIDGET(dateentry);

	GTK_BOX(widget)->homogeneous = FALSE;

	dateentry->date = g_date_new();
	/* today's date */
	g_date_set_time(dateentry->date, time(NULL));

	dateentry->entry = gtk_entry_new ();
	gtk_widget_set_size_request(dateentry->entry, 90, -1);
	gtk_box_pack_start (GTK_BOX (dateentry), dateentry->entry, TRUE, TRUE, 0);

	dateentry->arrow = gtk_toggle_button_new ();
	arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (dateentry->arrow), arrow);
	gtk_box_pack_end (GTK_BOX (dateentry), dateentry->arrow, FALSE, FALSE, 0);

	gtk_widget_show (dateentry->entry);
	gtk_widget_show (dateentry->arrow);

	g_signal_connect (GTK_OBJECT (dateentry->arrow), "toggled",
				(GtkSignalFunc) gtk_dateentry_arrow_press, dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->entry), "activate",
				(GtkSignalFunc) gtk_dateentry_entry_new, dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->entry), "key_press_event",
				G_CALLBACK (gtk_dateentry_entry_key), dateentry);

    /* our popup window */
	dateentry->popwin = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (dateentry->popwin,
				gtk_widget_get_events(dateentry->popwin) | GDK_KEY_PRESS_MASK);

	dateentry->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (dateentry->popwin), dateentry->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (dateentry->frame), GTK_SHADOW_OUT);
	gtk_widget_show (dateentry->frame);

	dateentry->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (dateentry->frame), dateentry->calendar);
	gtk_widget_show (dateentry->calendar);


	g_signal_connect (GTK_OBJECT (dateentry->popwin), "key_press_event",
				G_CALLBACK (key_press_popup), dateentry);


	g_signal_connect (GTK_OBJECT (dateentry->popwin), "button_press_event",
				G_CALLBACK (gtk_dateentry_button_press), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->calendar), "day-selected",
				G_CALLBACK (gtk_dateentry_calendar_getfrom), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->calendar), "day-selected-double-click",
				G_CALLBACK (gtk_dateentry_calendar_select), dateentry);

	gtk_dateentry_calendar_getfrom(NULL, dateentry);
}

// the rest
GtkWidget *
gtk_dateentry_new ()
{
GtkDateEntry *dateentry;

	DB( g_print(" (dateentry) new\n") );

	dateentry = g_object_new (GTK_TYPE_DATE_ENTRY, NULL);

	return GTK_WIDGET(dateentry);

}




static void
gtk_dateentry_destroy (GtkObject * object)
{
GtkDateEntry *dateentry;

	DB( g_print(" \n(dateentry) destroy\n") );

  g_return_if_fail (GTK_IS_DATE_ENTRY (object));

	dateentry = GTK_DATE_ENTRY (object);

	DB( g_print(" free gtkentry: %x\n", dateentry->entry) );
	DB( g_print(" free arrow: %x\n", dateentry->arrow) );
	DB( g_print(" free popwin: %x\n", dateentry->popwin) );

	DB( g_print(" free dateentry: %x\n", dateentry) );

	if(dateentry->popwin)
		gtk_widget_destroy (dateentry->popwin);
	dateentry->popwin = NULL;

	if(dateentry->date)
		g_date_free(dateentry->date);
	dateentry->date = NULL;

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/*
**
*/
void gtk_dateentry_set_date(GtkDateEntry * dateentry, guint julian_days)
{
	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (dateentry->date, julian_days);
	}
	else
	{
		g_date_set_time(dateentry->date, time(NULL));
	}
	gtk_dateentry_datetoentry(dateentry);
}

/*
**
*/
guint gtk_dateentry_get_date(GtkDateEntry * dateentry)
{
	g_return_val_if_fail (GTK_IS_DATE_ENTRY (dateentry), 0);

	return(g_date_get_julian(dateentry->date));
}


/*
static void
gtk_dateentry_set_property (GObject         *object,
                        guint            prop_id,
                        const GValue    *value,
                        GParamSpec      *pspec)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);

	DB( g_print(" (dateentry) set %d\n", prop_id) );


  switch (prop_id)
    {
	case PROPERTY_DATE:
	   DB( g_print(" -> date to %d\n", g_value_get_uint (value)) );

		g_date_set_julian (dateentry->date, g_value_get_uint (value));
		gtk_dateentry_datetoentry(dateentry);
	break;


    default:
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
gtk_dateentry_get_property (GObject         *object,
                        guint            prop_id,
                        GValue          *value,
                        GParamSpec      *pspec)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);

	DB( g_print(" (dateentry) get\n") );

  switch (prop_id)
    {
	case PROPERTY_DATE:
	   DB( g_print(" -> date is %d\n", 0) );
		g_value_set_uint (value, g_date_get_julian(dateentry->date));
		break;

    default:
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}
*/

/*
** fill in our gtkentry from our GDate
*/
static void gtk_dateentry_datetoentry(GtkDateEntry * dateentry)
{
char buffer[256];

	DB( g_print(" (dateentry) date2entry\n") );

	if(g_date_valid(dateentry->date) == TRUE)
	{
		g_date_strftime (buffer, 256 - 1, "%x", dateentry->date);
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), buffer);
	}
	else
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), "??");


	/* emit the signal */
	if(dateentry->lastdate != g_date_get_julian(dateentry->date))
	{
		DB( g_print(" **emit signal**\n") );

		g_signal_emit_by_name (dateentry, "changed", NULL, NULL);
	}

	dateentry->lastdate = g_date_get_julian(dateentry->date);

}

/*
** parse the gtkentry and store the GDate
*/
static void gtk_dateentry_entry_new(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GDate *date;
const gchar *str;

	DB( g_print(" (dateentry) entry validation\n") );

 	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));

	date = g_date_new ();
	g_date_set_parse (date, str);

	if(g_date_valid(date) == TRUE)
	{
		g_date_free(dateentry->date);
		dateentry->date = date;

	}
	else
	{
		g_date_free (date);
		/* today's date */
		g_date_set_time(dateentry->date, time(NULL));
	}

	gtk_dateentry_datetoentry(dateentry);

}

/*
** store the calendar date to GDate, update our gtkentry
*/
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry)
{
guint year, month, day;

	DB( g_print(" (dateentry) get from calendar\n") );

	gtk_calendar_get_date (GTK_CALENDAR (dateentry->calendar), &year, &month, &day);
	g_date_set_dmy (dateentry->date, day, month + 1, year);
	gtk_dateentry_datetoentry(dateentry);
}


static void gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print(" (dateentry) calendar_select\n") );

	gtk_dateentry_hide_popdown_window(dateentry);
	gtk_dateentry_calendar_getfrom(NULL, dateentry);
}




static gint
gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print(" (dateentry) entry key pressed: state=%d, keyval=%d\n", event->state, event->keyval) );

	if(event->state & GDK_SHIFT_MASK)
	{
		switch(event->keyval)
		{
			case GDK_Up:
				g_date_add_days (dateentry->date, 1);
				gtk_dateentry_datetoentry(dateentry);
				break;
			case GDK_Down:
				g_date_subtract_days (dateentry->date, 1);
				gtk_dateentry_datetoentry(dateentry);
				break;
		}



	}




/*
	switch(event->keyval)
	{
		case GDK_Up:
			if(!event->state)
				g_date_add_days (dateentry->date, 1);
			else
				if((event->state==GDK_SHIFT_MASK))
					g_date_add_months (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
			break;

		case GDK_Down:
			if(!event->state)
				g_date_subtract_days (dateentry->date, 1);
			else
				if((event->state==GDK_SHIFT_MASK))
					g_date_subtract_months (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
			break;
	}
*/
	return FALSE;
}





static void
position_popup (GtkDateEntry * dateentry)
{
	gint x, y;
	gint bwidth, bheight;
	GtkRequisition req;

	DB( g_print(" (dateentry) position popup\n") );

	gtk_widget_size_request (dateentry->popwin, &req);

	gdk_window_get_origin (dateentry->arrow->window, &x, &y);

	x += dateentry->arrow->allocation.x;
	y += dateentry->arrow->allocation.y;
	bwidth = dateentry->arrow->allocation.width;
	bheight = dateentry->arrow->allocation.height;

	x += bwidth - req.width;
	y += bheight;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	gtk_window_move (GTK_WINDOW (dateentry->popwin), x, y);
}



static void
gtk_dateentry_popup_display (GtkDateEntry * dateentry)
{
const char *str;
	int month;

  //gint height, width, x, y;
  gint old_width, old_height;

	DB( g_print(" (dateentry) popup_display\n****\n\n") );

  old_width = dateentry->popwin->allocation.width;
  old_height  = dateentry->popwin->allocation.height;


/* update */
	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));
	g_date_set_parse (dateentry->date, str);

	if(g_date_valid(dateentry->date) == TRUE)
	{
		/* GtkCalendar expects month to be in 0-11 range (inclusive) */
		month = g_date_get_month (dateentry->date) - 1;
		gtk_calendar_select_month (GTK_CALENDAR (dateentry->calendar),
				   CLAMP (month, 0, 11),
				   g_date_get_year (dateentry->date));
        gtk_calendar_select_day (GTK_CALENDAR (dateentry->calendar),
				 g_date_get_day (dateentry->date));
	}

	position_popup(dateentry);

  gtk_widget_show (dateentry->popwin);

  gtk_grab_add (dateentry->popwin);

  // this close the popup */

  gdk_pointer_grab (dateentry->popwin->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

}

static void
gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry)
{
	DB( g_print(" (dateentry) hide_popdown_window\n") );

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dateentry->arrow), FALSE);

  gtk_grab_remove(dateentry->popwin);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_widget_hide(dateentry->popwin);
}

static gint
gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry)
{
  GtkToggleButton *button;

	DB( g_print(" (dateentry) arrow_press\n") );

  button = GTK_TOGGLE_BUTTON(widget);

  if(!button->active){
     gtk_widget_hide (dateentry->popwin);
     gtk_grab_remove (dateentry->popwin);
     gdk_pointer_ungrab (GDK_CURRENT_TIME);

	gtk_dateentry_calendar_getfrom(NULL, dateentry);
     return TRUE;
  }

  gtk_dateentry_popup_display(dateentry);
  return TRUE;
}

static gint
key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;


	DB( g_print(" (dateentry) key pressed%d\n", event->keyval) );

	if (event->keyval != GDK_Escape)
		return FALSE;

	g_signal_stop_emission_by_name (widget, "key_press_event");

	gtk_dateentry_hide_popdown_window(dateentry);


	return TRUE;
}






static gint
gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  GtkWidget *child;

DB( g_print(" (dateentry) button_press\n") );

  child = gtk_get_event_widget (event);

  if (child != widget)
    {
      while (child)
	{
	  if (child == widget)
	    return FALSE;
	  child = child->parent;
	}
    }

  gtk_widget_hide (widget);
  gtk_grab_remove (widget);
  gdk_pointer_ungrab (GDK_CURRENT_TIME);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_DATE_ENTRY(data)->arrow), FALSE);

  return TRUE;
}

