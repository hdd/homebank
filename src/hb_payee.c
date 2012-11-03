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

#include "hb_payee.h"
#include "import.h"

/****************************************************************************/
/* Debug macros										 */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */




gboolean
payee_is_used(guint32 key)
{
GList *list;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if( key == entry->payee )
			return TRUE;
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if( key == entry->payee )
			return TRUE;
		list = g_list_next(list);
	}

	list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if( key == entry->payee)
			return TRUE;
		list = g_list_next(list);
	}
	g_list_free(list);

	
	return FALSE;
}

void
payee_move(guint32 key1, guint32 key2)
{
GList *list;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if(entry->payee == key1)
		{
			entry->payee = key2;		
			entry->flags |= OF_CHANGED;			
		}
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if(entry->payee == key1)
		{
			entry->payee = key2;		
		}
		list = g_list_next(list);
	}
	
	list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if(entry->payee == key1)
		{	
			entry->payee = key2;
		}		
		list = g_list_next(list);
	}
	g_list_free(list);
}

gboolean
payee_rename(Payee *item, const gchar *newname)
{
Payee *existitem;
gchar *stripname;

	stripname = g_strdup(newname);
	g_strstrip(stripname);

	existitem = da_pay_get_by_name(stripname);

	if( existitem != NULL )
	{
		if( existitem->key == item->key )
			return TRUE;
	}	
	else
	{
		g_free(item->name);
		item->name = g_strdup(stripname);
		return TRUE;	
	}

	g_free(stripname);

	return FALSE;
}


/**
 * payee_append_if_new:
 * 
 * append a new payee into the GHashTable
 * 
 * Return value: a new Payee or NULL
 *
 */
Payee *
payee_append_if_new(gchar *name)
{
gchar *stripname;
Payee *item;

	stripname = g_strdup(name);
	g_strstrip(stripname);
	item = da_pay_get_by_name(stripname);

	if(item == NULL)
	{
		item = da_pay_malloc();
		item->name = g_strdup(stripname);
		da_pay_append(item);
	}
	else
		item = NULL;

	g_free(stripname);

	return item;
}




void
payee_load_csv(gchar *filename)
{
GIOChannel *io;
gchar *tmpstr;
gint io_stat;
const gchar *encoding;

	encoding = homebank_file_getencoding(filename);
	
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		DB( g_print(" -> encoding should be %s\n", encoding) );
		if( encoding != NULL )
		{
			g_io_channel_set_encoding(io, encoding, NULL);
		}

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != NULL)
				{
					hb_string_strip_crlf(tmpstr);

					DB( g_print(" read %s\n", tmpstr) );

					if( payee_append_if_new( tmpstr ) )
					{
						GLOBALS->change++;
					}

				}
				g_free(tmpstr);
			}

		}
		g_io_channel_unref (io);
	}

}

static gint
payee_save_csv_compare_func(Payee *a, Payee *b)
{
gint retval = 0;

    if (a->name == NULL || b->name == NULL)
    {
        retval = (a->name == NULL) ? -1 : 1;
    }
    else
    {
        retval = g_utf8_collate(a->name, b->name);
    }

	return retval;
}



void
payee_save_csv(gchar *filename)
{
GIOChannel *io;
GList *list;
gchar *outstr;

	io = g_io_channel_new_file(filename, "w", NULL);
	if(io != NULL)
	{
		list = g_hash_table_get_values(GLOBALS->h_pay);
		
		list = g_list_sort(list, (GCompareFunc)payee_save_csv_compare_func);
		
		while (list != NULL)
		{
		Payee *item = list->data;

			if(item->key != 0)
			{
				outstr = g_strdup_printf("%s\n", item->name);
				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				DB( g_print("%s", outstr) );

				g_free(outstr);
			}
			list = g_list_next(list);
		}
		g_list_free(list);

		g_io_channel_unref (io);
	}

}


