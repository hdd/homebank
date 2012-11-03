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
#include "da_other.h"

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



/* = = = = = = = = = = = = = = = = = = = = */
/* Archive */

Archive *da_archive_malloc(void)
{
	return g_malloc0(sizeof(Archive));
}

Archive *da_archive_clone(Archive *src_item)
{
Archive *new_item = g_memdup(src_item, sizeof(Archive));

	if(new_item)
	{
		//duplicate the string
		new_item->wording = g_strdup(src_item->wording);
	}
	return new_item;
}

void da_archive_free(Archive *item)
{
	if(item != NULL)
	{
		if(item->wording != NULL)
			g_free(item->wording);

		g_free(item);
	}
}

void da_archive_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Archive *item = tmplist->data;
		da_archive_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}

static gint da_archive_glist_compare_func(Archive *a, Archive *b)
{
	return (g_utf8_collate(a->wording, b->wording));
}


GList *da_archive_sort(GList *list)
{
	return g_list_sort(list, (GCompareFunc)da_archive_glist_compare_func);
}
	



/* = = = = = = = = = = = = = = = = = = = = */
/* Operation */


/* = = = = = = = = = = = = = = = = = = = = */
/* Filter */

Filter *da_filter_malloc(void)
{
	return g_malloc0(sizeof(Filter));
}

void da_filter_free(Filter *flt)
{
	if(flt != NULL)
	{
		g_free(flt->wording);
		g_free(flt->info);
		g_free(flt->tag);
		g_free(flt);
	}
}

/* = = = = = = = = = = = = = = = = = = = = */
/* CarCost */

CarCost *da_vehiclecost_malloc(void)
{
	return g_malloc0(sizeof(CarCost));
}

void da_vehiclecost_free(CarCost *item)
{
	if(item != NULL)
	{
		g_free(item);
	}
}

void da_vehiclecost_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	CarCost *item = tmplist->data;
		da_vehiclecost_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


