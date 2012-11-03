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

#include "hb_account.h"

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


/**
 * account_is_used:
 * 
 * controls if an account is used by any archive or transaction
 * 
 * Return value: TRUE if used, FALSE, otherwise
 */
gboolean
account_is_used(guint32 key)
{
GList *list;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if( key == entry->account || key == entry->dst_account)
			return TRUE;
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if( key == entry->account || key == entry->dst_account)
			return TRUE;
		list = g_list_next(list);
	}

	return FALSE;
}

void
account_move(guint32 key1, guint32 key2)
{
GList *list;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *entry = list->data;
		if(entry->account == key1)
			entry->account = key2;		
		if(entry->dst_account == key1)
			entry->dst_account = key2;		
		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if(entry->account == key1)
			entry->account = key2;		
		if(entry->dst_account == key1)
			entry->dst_account = key2;		
		list = g_list_next(list);
	}
}

gboolean
account_rename(Account *item, gchar *newname)
{
Account *existitem;
gchar *stripname;

	stripname = g_strdup(newname);
	g_strstrip(stripname);

	existitem = da_acc_get_by_name(stripname);
	if( existitem == NULL )
	{
		g_free(item->name);
		item->name = g_strdup(stripname);
		return TRUE;	
	}

	g_free(stripname);

	return FALSE;
}



