/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2011 Maxime DOYEN
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

#include "homebank.h"
#include "misc.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static gdouble fint(gdouble amount)
{
gdouble fi;

	modf(amount, &fi);
	return(fi);
}

static unsigned dix_puissance_n(unsigned n)
{
    unsigned i, res = 1;
    
    for(i = 0; i < n; i++)
        res *= 10;
    
    return res;
}

double arrondi(const double x, unsigned n)
{
    unsigned N = dix_puissance_n(n);
    return floor(x * N + 0.5) / N;
}

/*
gint mystrfmon_int(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
struct CurrencyFmt *cur;
gint size = 0;
gchar buf1[G_ASCII_DTOSTR_BUF_SIZE];
gchar groupbuf[G_ASCII_DTOSTR_BUF_SIZE];
guint i, length;
gchar *monstr;

	cur = minor ? &PREFS->minor_cur : &PREFS->base_cur;	

	if(minor == TRUE)
	{
		value = (value * PREFS->euro_value);
		value += (value > 0.0) ? 0.005 : -0.005;
		value = (fint(value * 100) / 100);
	}

	g_ascii_formatd(buf1, sizeof (buf1), "%0.f", value);

	length = strlen(buf1);
	
	if( cur->grouping_char == NULL )
	{
		monstr = buf1;
	}
	else
	{
	gchar *s = buf1;
	gchar *d = groupbuf;

		i = 0;
		// avoid the - for negative amount
		if( *s == '-')
		{
			*d++ = *s++;
			length--;
		}
		
		// do the grouping
		do
		{
			if( i!=0 && (length % 3) == 0 )
			{
			gchar *gc = cur->grouping_char;
			
				while( *gc )
					*d++ = *gc++;
			}
		
			*d++ = *s;
			length--;
			i++;	
		}
		while (length && *s++ != '\0');
		*d = 0;

		monstr = groupbuf;

	}

	//debug
	//g_print("mystrfmon %.2f %s [%d] %s\n", value, str_array[0], length, str_array[1] );
	//g_print(" => %s :: %s\n", monstr, groupbuf);

	if(monstr!=NULL)
	{
	gchar *ptr = outstr;
	
		//todo: improve this
		if(cur->prefix_symbol != NULL) ptr = g_stpcpy(ptr, cur->prefix_symbol);
		ptr = g_stpcpy(ptr, " ");
		if(monstr != NULL) ptr = g_stpcpy(ptr, monstr);
		ptr = g_stpcpy(ptr, " ");
		if(cur->suffix_symbol != NULL) ptr = g_stpcpy(ptr, cur->suffix_symbol);
	
		
		//strncpy(outstr, monstr, outlen-1);
		
		//g_free(monstr);
	}

	return size;
}
*/

gint real_mystrfmon(gchar *outstr, gint outlen, gchar *buf1, struct CurrencyFmt *cur)
{
gint size = 0;
gchar groupbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar **str_array;
guint i, length;
gchar *monstr;

	str_array = g_strsplit(buf1, ".", 2);
	monstr = NULL;

	length = strlen(str_array[0]);
	
	if( cur->grouping_char == NULL || !strlen(cur->grouping_char) )
	{
		monstr = g_strjoinv(cur->decimal_char, str_array);
	}
	else
	{
	gchar *s = str_array[0];
	gchar *d = groupbuf;

		i = 0;
		// avoid the - for negative amount
		if( *s == '-')
		{
			*d++ = *s++;
			length--;
		}
		
		// do the grouping
		do
		{
			if( i!=0 && (length % 3) == 0 )
			{
			gchar *gc = cur->grouping_char;
			
				while( *gc )
					*d++ = *gc++;
			}
		
			*d++ = *s;
			length--;
			i++;	
		}
		while (length && *s++ != '\0');
		*d = 0;

		monstr = g_strjoin(cur->decimal_char, groupbuf, str_array[1], NULL);

	}

	//debug
	//g_print("**\nmystrfmon %.2f\n 0=%s\n 1=%s\n [%d]\n", value, str_array[0], str_array[1], length );
	//g_print(" => %s :: %s\n", monstr, groupbuf);

	g_strfreev(str_array);

	/* insert our formated number with symbol */
	g_snprintf(outstr, outlen, cur->monfmt, monstr);

	/*
	if(monstr!=NULL)
	{
	gchar *ptr = outstr;
	
		//todo: improve this
		if(cur->prefix_symbol != NULL) ptr = g_stpcpy(ptr, cur->prefix_symbol);
		ptr = g_stpcpy(ptr, " ");
		if(monstr != NULL) ptr = g_stpcpy(ptr, monstr);
		ptr = g_stpcpy(ptr, " ");
		if(cur->suffix_symbol != NULL) ptr = g_stpcpy(ptr, cur->suffix_symbol);
	

		g_print("%s %s %s\n", cur->prefix_symbol, monstr, cur->suffix_symbol);

		
		//strncpy(outstr, monstr, outlen-1);
		
	
	}
	*/

	g_free(monstr);
	
	return size;
}


gint mystrfmon_int(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
struct CurrencyFmt *cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval = value;
gint size;

	cur = minor ? &PREFS->minor_cur : &PREFS->base_cur;	

	if(minor == TRUE)
	{
		monval = (value * PREFS->euro_value);
		monval += (monval > 0.0) ? 0.005 : -0.005;
		monval = (fint(monval * 100) / 100);
	}

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), "%0.f", monval);

	size = real_mystrfmon(outstr, outlen, formatd_buf, cur);

	return size;
}


/*
gint mystrfmoncurr(gchar *outstr, gint outlen, gdouble value, guint32 currkey)
{
Currency *cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval;
gint size;

	if(currkey == 0)
	{
		*outstr = 0;
		return 0;
	}

	

	
	cur = da_cur_get(currkey);
	
	monval = arrondi(value, cur->frac_digits);
	
	//if(minor == TRUE)
	//{

	//todo: debug test
	//	monval = (value * cur->rate);
	//	monval += (monval > 0.0) ? 0.005 : -0.005;
	//	monval = (fint(monval * 100) / 100);
	//}

	//todo: debug test
	//cur = da_cur_get(GLOBALS->kcur);

	
	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur->format, monval);

	//g_print("k=%d, value %.2f, fd=%d, fmt=%s => %s\n", currkey, value, cur->frac_digits, cur->monfmt, formatd_buf);

	
	size = real_mystrfmoncurr(outstr, outlen, formatd_buf, cur);

	return size;
}


gint real_mystrfmoncurr(gchar *outstr, gint outlen, gchar *buf1, Currency *cur)
{
gint size = 0;
gchar groupbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar **str_array;
guint i, length;
gchar *monstr;

	str_array = g_strsplit(buf1, ".", 2);
	monstr = NULL;

	length = strlen(str_array[0]);
	
	if( cur->grouping_char == NULL || !strlen(cur->grouping_char) )
	{
		monstr = g_strjoinv(cur->decimal_char, str_array);
	}
	else
	{
	gchar *s = str_array[0];
	gchar *d = groupbuf;

		i = 0;
		// avoid the - for negative amount
		if( *s == '-')
		{
			*d++ = *s++;
			length--;
		}
		
		// do the grouping
		do
		{
			if( i!=0 && (length % 3) == 0 )
			{
			gchar *gc = cur->grouping_char;
			
				while( *gc )
					*d++ = *gc++;
			}
		
			*d++ = *s;
			length--;
			i++;	
		}
		while (length && *s++ != '\0');
		*d = 0;

		monstr = g_strjoin(cur->decimal_char, groupbuf, str_array[1], NULL);

	}

	//debug
	//g_print("**\nmystrfmon %.2f\n 0=%s\n 1=%s\n [%d]\n", value, str_array[0], str_array[1], length );
	//g_print(" => %s :: %s\n", monstr, groupbuf);

	g_strfreev(str_array);

	// insert our formated number with symbol
	g_snprintf(outstr, outlen, cur->monfmt, monstr);

	g_free(monstr);
	
	return size;
}
*/




gint mystrfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
struct CurrencyFmt *cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval;
gint size;

	cur = minor ? &PREFS->minor_cur : &PREFS->base_cur;	

	monval = arrondi(value, cur->frac_digits);
	
	if(minor == TRUE)
	{
		monval = (value * PREFS->euro_value);
		monval += (monval > 0.0) ? 0.005 : -0.005;
		monval = (fint(monval * 100) / 100);
	}

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur->format, monval);

	size = real_mystrfmon(outstr, outlen, formatd_buf, cur);

	return size;
}


/*
** format a monetary number major/minor (null value return empty string)
*/
/*
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
*/

/*
** format a monetary number major/minor (null value return a 0)
*/
/*gint hb_strfmonall(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
gint size = 0;
gchar *monfmt = GLOBALS->fmt_maj_number;
gdouble monval = value;

	if(minor == TRUE)
	{
		monval = (value * PREFS->euro_value);
		monval += (monval > 0.0) ? 0.005 : -0.005;
		monval = (fint(monval * 100) / 100);
		monfmt = GLOBALS->fmt_min_number;
	}

	//size = strfmon(outstr, outlen-1, monfmt, monval);
	size = mystrfmon(outstr, outlen-1, monval, minor);

	return size;
}
*/



gchar *get_normal_color_amount(gdouble value)
{
gchar *color = NULL;
gdouble amount;
	
	//fix: 400483
	amount = arrondi(value, PREFS->base_cur.frac_digits);

	if(amount != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = (amount > 0.0) ? PREFS->color_inc : PREFS->color_exp;
	}

	return color;
}


gchar *get_minimum_color_amount(gdouble value, gdouble minvalue)
{
gchar *color = NULL;

	//fix: 400483
	value = arrondi(value, PREFS->base_cur.frac_digits);
	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = (value > 0.0) ? PREFS->color_inc : PREFS->color_exp;
		if( value < minvalue) 
			color = PREFS->color_warn;
	}
		
	return color;
}

/*
** format/color and set a label text with a amount value
*/
void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];
gchar *markuptxt;
gchar *color = NULL;

	mystrfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, minor);

	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = get_normal_color_amount(value);

		if(color)
		{
			markuptxt = g_strdup_printf("<span color='%s'>%s</span>", color, strbuffer);
			gtk_label_set_markup(GTK_LABEL(label), markuptxt);
			g_free(markuptxt);
			return;
		}
	}
		
	gtk_label_set_text(GTK_LABEL(label), strbuffer);

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
gchar *p = str;

	if(str)
	{
		while( *p )
		{
			if( *p == '\n' || *p == '\r')
			{
				*p = '\0';
			}
			p++;
		}
	}
}

gchar*
hb_strdup_nobrackets (const gchar *str)
{
  const gchar *s;
  gchar *new_str, *d;
  gsize length;

  if (str)
    {
      length = strlen (str) + 1;
      new_str = g_new (char, length);
      s = str;
      d = new_str;
      while(*s != '\0')
      {
		if( *s != '[' && *s != ']' )
			*d++ = *s;
      	s++;
      }
      *d = '\0';
    }
  else
    new_str = NULL;

  return new_str;
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

/*
static gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isprint(*str++);
	return valid;
}
*/



static gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
gchar *p;
gunichar c;

	if(g_utf8_validate(str, -1, NULL))
	{
		p = str;
		while(*p && valid)
		{
			c = g_utf8_get_char(p);
			valid = g_unichar_isprint(c);
			p = g_utf8_next_char(p); 
		}
	}
	return valid;
}


gboolean hb_string_csv_valid(gchar *str, gint nbcolumns, gint *csvtype)
{
gchar **str_array;
gboolean valid = TRUE;
guint i, lasttype;
extern int errno;
#if MYDEBUG == 1
 gchar *type[5] = { "string", "date", "int", "double" };
#endif

	DB( g_print("\n** hb_string_csv_valid: init %d\n", valid) );

	hb_string_strip_crlf(str);
	str_array = g_strsplit (str, ";", 0);
	
	if( g_strv_length( str_array ) != nbcolumns )
		valid = FALSE;
	
	DB( g_print(" -> length %d, nbcolumns %d\n", g_strv_length( str_array ), nbcolumns) );
	
	for(i=0;i<nbcolumns;i++)
	{
		lasttype = csvtype[i];

		if(valid == FALSE)
		{
			DB( g_print(" -> fail on column %d, type: %s\n", i, type[lasttype]) );
			break;
		}
		
		DB( g_print(" -> control column %d, type: %d, valid: %d '%s'\n", i, lasttype, valid, str_array[i]) );
		
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
				//todo : see this errno
				if( errno )
				{
					DB( g_print("errno: %d\n", errno) );
					valid = FALSE;
				}
				break;
		}
	}
	g_strfreev (str_array);

	DB( g_print(" --> return %d\n", valid) );

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

		//todo: here if month is > 12 then the format is probably mm/dd/yy(yy)
		//or maybe check with g_date_valid_julian(julian)



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

#if MYDEBUG == 1

/*
** hex memory dump
*/
#define MAX_DUMP 16
void hex_dump(guchar *ptr, guint length)
{
guchar ascii[MAX_DUMP+4];
guint i,j;

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

#endif
