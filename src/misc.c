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


#include "homebank.h"

#include "misc.h"

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


gdouble fint(gdouble amount)
{
gdouble fi;

	modf(amount, &fi);
	return(fi);
}

/*
** format a monetary number major/minor (null value return empty string)
*/
gint hb_strfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
gint size = 0;

	if(!value)
		*outstr = 0;
	else
	{
		size = hb_strfmonall(outstr, outlen, value, minor);
	}
	return size;
}

/*
** format a monetary number major/minor (null value return a 0)
*/
gint hb_strfmonall(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
gint size = 0;

	if(minor != TRUE)
	{
		size = strfmon(outstr, outlen-1, GLOBALS->fmt_maj_number, value);
	}
	else
	{
	gdouble minorvalue;

		minorvalue = (value * PREFS->euro_value);
		minorvalue += (minorvalue > 0.0) ? 0.005 : -0.005;
		minorvalue = (fint(minorvalue * 100) / 100);
		size = strfmon(outstr, outlen-1, GLOBALS->fmt_min_number, minorvalue);
	}

	return size;
}

/*
** format/color and set a label text with a amount value
*/
void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor)
{
gchar strbuffer[64];
gchar *markuptxt;
guint32 color;

	hb_strfmon(strbuffer, 64-1, value, minor);

	color = (value > 0) ? PREFS->color_inc : PREFS->color_exp;

	markuptxt = g_strdup_printf("<span color='#%06x'>%s</span>", color, strbuffer);
	gtk_label_set_markup(GTK_LABEL(label), markuptxt);
	g_free(markuptxt);
}


void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate)
{
GDate *date;

	date = g_date_new();
	if(month)
	{
		/* mindate is month 1, year :: maxdate is last day of month, year */
		g_date_set_dmy(date, 1, month, year);
		*mindate = g_date_get_julian(date);
		g_date_add_days(date, g_date_get_days_in_month(month, year));
		*maxdate = g_date_get_julian(date)-1;
	}
	else
	{
		g_date_set_dmy(date, 1, 1, year);
		*mindate = g_date_get_julian(date);
		g_date_set_dmy(date, 31, 12, year);
		*maxdate = g_date_get_julian(date);
	}
	g_date_free(date);
}

void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate)
{
GDate *date;
guint month, year, n_months;

	if(refdate > *maxdate)
		refdate = *maxdate;

	date  = g_date_new_julian(refdate);
	month = g_date_get_month(date);
	year  = g_date_get_year(date);

	switch( range )
	{
	/* current month */
		case 1:
			g_date_set_day(date, 1);
			*mindate = g_date_get_julian(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year));
			*maxdate = g_date_get_julian(date)-1;
			break;
	/* previous month */
		case 3:
			g_date_set_day(date, 1);
			g_date_subtract_months(date, 1);
			*mindate = g_date_get_julian(date);
			month = g_date_get_month(date);
			year = g_date_get_year(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year));
			*maxdate = g_date_get_julian(date)-1;
			break;
	/* previous year */
		case 4:
			year--;
	/* current year */
		case 2:
			g_date_set_dmy(date, 1, 1, year);
			*mindate = g_date_get_julian(date);
			g_date_set_dmy(date, 31, 12, year);
			*maxdate = g_date_get_julian(date);
			break;
	/* last 30 days */
		case 5:
			*mindate = refdate - 30;
			*maxdate = refdate;
			break;
	/* last 2 months */
		case 6:
			n_months = 2;
			goto lastmonth;
	/* last 3 months */
		case 7:
			n_months = 3;
			goto lastmonth;
	/* last 4 months */
		case 8:
			n_months = 4;
			goto lastmonth;
	/* last 6 months */
		case 9:
			n_months = 6;
			goto lastmonth;
	/* last 12 months */
		case 10:
			n_months = 12;
lastmonth:
			g_date_subtract_months(date, n_months);
			*mindate = g_date_get_julian(date);
			*maxdate = refdate;
			break;
	}
	g_date_free(date);
}

/*
** String utility
*/ 

void hb_string_strip_crlf(gchar *str)
{
gchar *ptr;

	ptr = g_strrstr (str, "\n");
	if(ptr != NULL)
		*ptr = 0;
	ptr = g_strrstr (str, "\r");
	if(ptr != NULL)
		*ptr = 0;
}

static gboolean hb_string_isdate(gchar *str)
{
guint32 julian;

	julian = hb_date_get_julian_parse(str);

	return(g_date_valid_julian(julian));
}


static gboolean hb_string_isdigit(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isdigit(*str++);
	return valid;
}

static gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isprint(*str++);
	return valid;
}

gboolean hb_string_csv_valid(gchar *str, gint nbcolumns, gint *csvtype)
{
gchar **str_array;
gboolean valid = TRUE;
guint i, lasttype;
extern int errno;

	DB( g_print("hb_string_csv_valid: init %d\n", valid) );

	hb_string_strip_crlf(str);
	str_array = g_strsplit (str, ";", nbcolumns);
	
	if( g_strv_length( str_array ) != nbcolumns )
		valid = FALSE;
	
	DB( g_print("hb_string_csv_valid: %d %d\n", g_strv_length( str_array ), nbcolumns) );
	
	for(i=0;i<nbcolumns;i++)
	{
		if(valid == FALSE)
		{
			DB( g_print("hb_string_csv_valid: fail on column %d, type: %d\n", i, lasttype) );
			break;
		}
		
		lasttype = csvtype[i];
		DB( g_print("hb_string_csv_valid: control column %d, type: %d, valid: %d '%s'\n", i, lasttype, valid, str_array[i]) );
		
		switch( csvtype[i] )
		{
			case CSV_DATE:
				valid = hb_string_isdate(str_array[i]);
				break;
			case CSV_STRING:
				valid = hb_string_isprint(str_array[i]);
				break;
			case CSV_INT:
				valid = hb_string_isdigit(str_array[i]);
				break;
			case CSV_DOUBLE	:
				g_ascii_strtod(str_array[i], NULL);
				//todo 
				if( errno )
				{
					DB( g_print("errno: %d\n", errno) );
					valid = FALSE;
				}
				break;
		}
	}
	g_strfreev (str_array);

	DB( g_print("hb_string_csv_valid: return %d\n", valid) );

	return valid;
}



/*
** parse a string an retrieve an iso date (dd-mm-yy(yy) or dd/mm/yy(yy))
**
*/
guint32 hb_date_get_julian_parse(gchar *str)
{
gchar **str_array = NULL;
GDate *date;
guint d, m, y;
guint32 julian = GLOBALS->today;

	/* try with - separator */
	if( g_strrstr(str, "-") != NULL )
	{
		str_array = g_strsplit (str, "-", 3);
	}
	else
	{
		if( g_strrstr(str, "/") != NULL )
		{
			str_array = g_strsplit (str, "/", 3);
		}	
	}

	if( g_strv_length( str_array ) == 3 )
	{
		d = atoi(str_array[0]);
		m = atoi(str_array[1]);
		y = atoi(str_array[2]);


		//correct for 2 digits year
		if(y < 1970)
		{
			if(y < 60)
				y += 2000;
			else
				y += 1900;
		}

		date = g_date_new();
		g_date_set_dmy(date, d, m, y);
		julian = g_date_get_julian (date);
		g_date_free(date);

		DB( g_print("date: %s :: %d %d %d :: %d\n", str, d, m, y, julian ) );

	}

	g_strfreev (str_array);

	return julian;
}


/* -------------------- */

/*
** hex memory dump
*/
#define MAX_DUMP 16
void hex_dump(guchar *ptr, gint length)
{
guchar ascii[MAX_DUMP+4];
gint i,j;

	g_print("**hex_dump - %d bytes\n", length);

	for(i=0;i<length;)
	{
		g_print("%08x: ", (guint)ptr+i);

		for(j=0;j<MAX_DUMP;j++)
		{
			if(i >= length) break;

			//store ascii value
			if(ptr[i] >= 32 && ptr[i] <= 126)
				ascii[j] = ptr[i];
			else
				ascii[j] = '.';

			g_print("%02x ", ptr[i]);
			i++;
		}
		//newline
		ascii[j] = 0;
		g_print(" '%s'\n", ascii);
	}
}

