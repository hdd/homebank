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

#include "homebank.h"

#include "hb_transaction.h"

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

guint
transaction_count_tags(Operation *ope)
{
guint count = 0;
guint32 *ptr = ope->tags;

	if( ope->tags == NULL )
		return 0;

	while(*ptr++ != 0 && count < 32)
		count++;

	return count;
}

guint
transaction_set_tags(Operation *ope, const gchar *tagstring)
{
gchar **str_array;
guint count, i;
Tag *tag;

	DB( g_print("(transaction_set_tags)\n") );

	str_array = g_strsplit (tagstring, " ", 0);
	count = g_strv_length( str_array );
	
	g_free(ope->tags);
	ope->tags = NULL;

	DB( g_print(" -> reset storage %x\n", ope->tags) );

	
	if( count > 0 )
	{
		
		ope->tags = g_new0(guint32, count + 1);

		DB( g_print(" -> storage %x\n", ope->tags) );

		for(i=0;i<count;i++)
		{
			tag = da_tag_get_by_name(str_array[i]);
			if(tag == NULL)
			{
			Tag *newtag = da_tag_malloc();
			
				newtag->name = g_strdup(str_array[i]);
				da_tag_append(newtag);
				tag = da_tag_get_by_name(str_array[i]);
			}
	
			DB( g_print(" -> storing %d=>%s at tags pos %d\n", tag->key, tag->name, i) );
	
			ope->tags[i] = tag->key;
		}
	}

	//hex_dump(ope->tags, sizeof(guint32*)*count+1);

	g_strfreev (str_array);

	return count;
}

gchar *
transaction_get_tagstring(Operation *ope)
{
guint count, i;
gchar **str_array;
gchar *tagstring;
Tag *tag;

	DB( g_print("transaction_get_tagstring\n") );

	DB( g_print(" -> tags at=%x\n", ope->tags) );

	if( ope->tags == NULL )
	{
	
		return NULL;
	}
	else
	{	
		count = transaction_count_tags(ope);

		DB( g_print(" -> tags at=%x, nbtags=%d\n", ope->tags, count) );

		str_array = g_new0(gchar*, count+1);	

		DB( g_print(" -> str_array at %x\n", str_array) );

		//hex_dump(ope->tags, sizeof(guint32*)*(count+1));

		for(i=0;i<count;i++)
		{
			DB( g_print(" -> try to get tag %d\n", ope->tags[i]) );
			
			tag = da_tag_get(ope->tags[i]);
			if( tag )
			{
				DB( g_print(" -> get %s at %d\n", tag->name, i) );
				str_array[i] = tag->name;
			}
			else
				str_array[i] = NULL;
			

		}

		tagstring = g_strjoinv(" ", str_array);

		g_free (str_array);

	}

	return tagstring;
}





