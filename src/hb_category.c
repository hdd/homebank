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

#include "hb_category.h"
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
category_is_used(guint32 key)
{
GList *list;

	//todo: add budget use here



	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if( key == entry->category )
			return TRUE;
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if( key == entry->category )
			return TRUE;
		list = g_list_next(list);
	}

	list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if( key == entry->category)
			return TRUE;
		list = g_list_next(list);
	}
	g_list_free(list);
	
	return FALSE;
}

void
category_move(guint32 key1, guint32 key2)
{
GList *list;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if(entry->category == key1)
		{
			entry->category = key2;
			entry->flags |= OF_CHANGED;			
		}
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if(entry->category == key1)
		{	
			entry->category = key2;
		}		
		list = g_list_next(list);
	}
	
	list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if(entry->category == key1)
		{	
			entry->category = key2;
		}		
		list = g_list_next(list);
	}
	g_list_free(list);
	
}


gboolean
category_rename(Category *item, const gchar *newname)
{
Category *parent, *existitem;
gchar *fullname = NULL;
gchar *stripname;
gboolean retval;

	DB( g_print("(category) rename\n") );
	
	stripname = g_strdup(newname);
	g_strstrip(stripname);

	
	if( item->parent == 0)	
		fullname = g_strdup(stripname);
	else
	{
		parent = da_cat_get(item->parent);
		if( parent )
		{
			fullname = g_strdup_printf("%s:%s", parent->name, stripname);
		}
	}

	DB( g_print(" - search: %s\n", fullname) );
	
	existitem = da_cat_get_by_fullname( fullname );

	if( existitem != NULL && existitem->key != item->key)
	{
		DB( g_print("error, same name already exist with other key %d <> %d\n",existitem->key, item->key) );
		retval = FALSE;
	}	
	else
	{
		DB( g_print(" -renaming\n") );

		g_free(item->name);
		item->name = g_strdup(stripname);
		retval = TRUE;	
	}

	g_free(fullname);
	g_free(stripname);

	return retval;
}

gboolean
category_load_csv(gchar *filename, gchar **error)
{
gboolean retval;
GIOChannel *io;
gchar *tmpstr;
gint io_stat;
gchar **str_array;
gchar *lastcatname = NULL;
gchar *fullcatname;
GError *err = NULL; 
Category *item;
gint type = 0;
const gchar *encoding;

	encoding = homebank_file_getencoding(filename);

			DB( g_print(" -> encoding should be %s\n", encoding) );

	
	retval = TRUE;
	*error = NULL;
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{

		if( encoding != NULL )
		{
			g_io_channel_set_encoding(io, encoding, NULL);
		}

		for(;;)
		{
			if( *error != NULL )
				break;			
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, &err);

			DB( g_print(" + iostat %d\n", io_stat) );

			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != NULL )
				{
					DB( g_print(" + strip %s\n", tmpstr) );

					hb_string_strip_crlf(tmpstr);

					DB( g_print(" + split\n") );

					str_array = g_strsplit (tmpstr, ";", 3);
					// type; sign; name

					if( g_strv_length (str_array) != 3 )
					{
						*error = _("invalid csv format");
						retval = FALSE;
						DB( g_print(" + error %s\n", *error) );
					}
					else
					{
						DB( g_print(" + read %s : %s : %s\n", str_array[0], str_array[1], str_array[2]) );

						fullcatname = NULL;
						if( g_str_has_prefix(str_array[0], "1") )
						{
							fullcatname = g_strdup(str_array[2]);
							g_free(lastcatname);
							lastcatname = g_strdup(str_array[2]);
							
							type = g_str_has_prefix(str_array[1], "+") ? GF_INCOME : 0;
							
							DB( g_print(" + type = %d\n", type) );
							
						}
						else
						if( g_str_has_prefix(str_array[0], "2") )
						{
							fullcatname = g_strdup_printf("%s:%s", lastcatname, str_array[2]);
						}

						DB( g_print(" + fullcatname %s\n", fullcatname) );

						item = da_cat_append_ifnew_by_fullname(fullcatname, FALSE);

						DB( g_print(" + item %d\n", item) );

						if( item != NULL)
						{
							DB( g_print(" + assign flags: '%c'\n", type) );
							
							item->flags |= type;
							
						}

						g_free(fullcatname);
						g_strfreev (str_array);
					}				
				}
				
			}
			g_free(tmpstr);

		}
		g_io_channel_unref (io);
	
		
	}

	g_free(lastcatname);

	return retval;
}

static gint category_save_csv_compare_func(Category *a, Category *b)
{
gint ka, kb, retval = 0;


	if(a->parent == 0 && b->parent == a->key)
		retval = -1;
	else
	if(b->parent == 0 && a->parent == b->key)
		retval = 1;
	else
	{
		ka = a->parent != 0 ? a->parent : a->key;
		kb = b->parent != 0 ? b->parent : b->key;
		retval = ka - kb;
	}	


	#if MYDEBUG == 1
	gchar *str;	
	
	if(retval < 0)
		str = "a < b";
	else
	if(retval ==0)
		str = "a = b";
	else
	if(retval > 0)
		str = "a > b";

	DB( g_print("compare a=%2d:%2d to b=%2d:%2d :: %d [%s]\n", a->key, a->parent, b->key, b->parent, retval, str ) );
	#endif

	return retval;
}


gboolean
category_save_csv(gchar *filename, gchar **error)
{
gboolean retval = FALSE;
GIOChannel *io;
gchar *outstr;
GList *list;


	io = g_io_channel_new_file(filename, "w", NULL);
	if(io != NULL)
	{

		list = g_hash_table_get_values(GLOBALS->h_cat);

		list = g_list_sort(list, (GCompareFunc)category_save_csv_compare_func);

		while (list != NULL)
		{
		Category *item = list->data;

			if(item->key != 0)
			{
			gchar lvel, type;
			
				if( item->parent == 0)
				{			
					lvel = '1';
					type = (item->flags & GF_INCOME) ? '+' : '-';
				}
				else
				{
					lvel = '2';
					type = ' ';
				}

				outstr = g_strdup_printf("%c;%c;%s\n", lvel, type, item->name);

				DB( g_print(" + export %s\n", outstr) );

				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				g_free(outstr);
			}
			list = g_list_next(list);
		}

		retval = TRUE;

		g_list_free(list);

		g_io_channel_unref (io);
	}


	return retval;
}


/**
 * category_find_preset:
 * 
 * find a user language compatible file for category preset
 * 
 * Return value: a pathname to the file or NULL
 *
 */
gchar *category_find_preset(gchar **lang)
{
gchar **langs;
gchar *filename;
gboolean exists;
gint i;

	DB( g_printf("** category_find_preset **\n") );

	langs = (gchar **)g_get_language_names ();

	DB( g_print(" -> %d languages detected\n", g_strv_length(langs)) );
	
	for(i=0;i<g_strv_length(langs);i++)
	{
		DB( g_print(" -> %d '%s'\n", i, langs[i]) );
		filename = g_strdup_printf("hb-categories-%s.csv", langs[i]);	
		gchar *pathfilename = g_build_filename(homebank_app_get_datas_dir(), filename, NULL);
		exists = g_file_test(pathfilename, G_FILE_TEST_EXISTS);
		DB( g_printf(" -> '%s' exists=%d\n", pathfilename, exists) );
		if(exists)
		{
			g_free(filename);
			*lang = langs[i];
			return pathfilename;
		}
		g_free(filename);
		g_free(pathfilename);
	}

	DB( g_print("return NULL\n") );
	
	*lang = NULL;
	return NULL;
}


