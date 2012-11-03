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
#include "da_transaction.h"
#include "hb_transaction.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void
da_operation_free(Operation *item)
{
	if(item != NULL)
	{
		if(item->wording != NULL)
			g_free(item->wording);
		if(item->info != NULL)
			g_free(item->info);
		if(item->tags != NULL)
		{
			DB( g_print(" -> item->tags %x\n", item->tags) );

			g_free(item->tags);
		}

		if(item->same != NULL)
			g_list_free(item->same);

		g_free(item);
	}
}


Operation *
da_operation_malloc(void)
{
	return g_malloc0(sizeof(Operation));
}

Operation *da_operation_clone(Operation *src_item)
{
Operation *new_item = g_memdup(src_item, sizeof(Operation));
guint count;

	DB( g_print("da_operation_clone\n") );

	if(new_item)
	{
		//duplicate the string
		new_item->wording = g_strdup(src_item->wording);
		new_item->info = g_strdup(src_item->info);

		//duplicate tags		
		new_item->tags = NULL;
		count = transaction_count_tags(src_item);
		if(count > 0)
			new_item->tags = g_memdup(src_item->tags, count*sizeof(guint32));

		DB( g_print(" clone %x -> %x\n", src_item, new_item) );

	}
	return new_item;
}

GList *
da_operation_new(void)
{
	return NULL;
}


void
da_operation_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Operation *item = tmplist->data;
		da_operation_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static gint da_operation_compare_func(Operation *a, Operation *b)
{
	return ((gint)a->date - b->date);
}

GList *da_operation_sort(GList *list)
{
	return( g_list_sort(list, (GCompareFunc)da_operation_compare_func));
}

gboolean
da_operation_append(Operation *item)
{
	GLOBALS->ope_list = g_list_append(GLOBALS->ope_list, item);

	// append the memo if new
	if( item->wording != NULL )
	{
		if( g_hash_table_lookup(GLOBALS->h_memo, item->wording) == NULL )
		{
			g_hash_table_insert(GLOBALS->h_memo, g_strdup(item->wording), NULL);	
		}
	}

	return TRUE;
}



