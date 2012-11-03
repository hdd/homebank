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
#include "da_account.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

Account *
da_acc_clone(Account *src_item)
{
Account *new_item = g_memdup(src_item, sizeof(Account));

	DB( g_print("da_acc_clone\n") );
	if(new_item)
	{
		//duplicate the string
		new_item->name		= g_strdup(src_item->name);
		new_item->number	= g_strdup(src_item->number);
		new_item->bankname	= g_strdup(src_item->bankname);
	}
	return new_item;
}


void 
da_acc_free(Account *item)
{
	DB( g_print("da_acc_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item->number);
		g_free(item->bankname);
		g_free(item);
	}
}


Account *
da_acc_malloc(void)
{
	DB( g_print("da_acc_malloc\n") );
	return g_malloc0(sizeof(Account));
}


void
da_acc_destroy(void)
{
	DB( g_print("da_acc_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_acc);
}


void
da_acc_new(void)
{
	DB( g_print("da_acc_new\n") );
	GLOBALS->h_acc = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_acc_free);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_acc_max_key_ghfunc(gpointer key, Account *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_acc_name_grfunc(gpointer key, Account *item, gchar *name)
{
	if( name && item->name )
	{
		if(!strcasecmp(name, item->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_acc_length:
 *
 * Return value: the number of elements
 */
guint
da_acc_length(void)
{
	return g_hash_table_size(GLOBALS->h_acc);
}


/**
 * da_acc_remove:
 * 
 * remove an account from the GHashTable
 * 
 * Return value: TRUE if the key was found and removed
 *
 */
gboolean
da_acc_remove(guint32 key)
{
	DB( g_print("da_acc_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_acc, &key);
}

/**
 * da_acc_insert:
 * 
 * insert an account into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_acc_insert(Account *item)
{
guint32 *new_key;

	DB( g_print("da_acc_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_acc, new_key, item);	

	return TRUE;
}


/**
 * da_acc_append:
 * 
 * insert an account into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_acc_append(Account *item)
{
Account *existitem;
guint32 *new_key;

	DB( g_print("da_acc_append\n") );

	/* ensure no duplicate */
	//g_strstrip(item->name);
	if(item->name != NULL)
	{
		existitem = da_acc_get_by_name( item->name );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_acc_get_max_key() + 1;
			item->key = *new_key;
			item->pos = da_acc_length() + 1;

			DB( g_print(" -> insert id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_acc, new_key, item);	
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->name, item->key) );

	return FALSE;
}

/**
 * da_acc_get_max_key:
 * 
 * Get the biggest key from the GHashTable
 * 
 * Return value: the biggest key value
 *
 */
guint32
da_acc_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_acc, (GHFunc)da_acc_max_key_ghfunc, &max_key);
	return max_key;
}




/**
 * da_acc_get_by_name:
 * 
 * Get an account structure by its name
 * 
 * Return value: Account * or NULL if not found
 *
 */
Account *
da_acc_get_by_name(gchar *name)
{
	DB( g_print("da_acc_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_acc, (GHRFunc)da_acc_name_grfunc, name);
}



/**
 * da_acc_get:
 * 
 * Get an account structure by key
 * 
 * Return value: Account * or NULL if not found
 *
 */
Account *
da_acc_get(guint32 key)
{
	DB( g_print("da_acc_get_account\n") );

	return g_hash_table_lookup(GLOBALS->h_acc, &key);
}





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#if MYDEBUG

static void
da_acc_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Account *item = value;

	DB( g_print(" %d :: %s\n", *id, item->name) );

}

void
da_acc_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_acc, da_acc_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif

