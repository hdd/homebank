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
#include "da_payee.h"

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
da_pay_free(Payee *item)
{
	DB( g_print("da_pay_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item);
	}
}


Payee *
da_pay_malloc(void)
{
	DB( g_print("da_pay_malloc\n") );
	return g_malloc0(sizeof(Payee));
}


void
da_pay_destroy(void)
{
	DB( g_print("da_pay_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_pay);
}


void
da_pay_new(void)
{
Payee *item;

	DB( g_print("da_pay_new\n") );
	GLOBALS->h_pay = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_pay_free);

	// insert our 'no payee'
	item = da_pay_malloc();
	item->name = g_strdup("");	
	da_pay_insert(item);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_pay_max_key_ghfunc(gpointer key, Payee *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_pay_name_grfunc(gpointer key, Payee *item, gchar *name)
{
	if( name && item->name )
	{
		if(!strcasecmp(name, item->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_pay_length:
 *
 * Return value: the number of elements
 */
guint
da_pay_length(void)
{
	return g_hash_table_size(GLOBALS->h_pay);
}

/*
gboolean
da_pay_create_none(void)
{
Payee *pay;
guint32 *new_key;

	DB( g_print("da_pay_insert none\n") );

	pay = da_pay_malloc();
	new_key = g_new0(guint32, 1);
	*new_key = 0;
	pay->key = 0;
	pay->name = g_strdup("");	

	DB( g_print(" -> insert id: %d\n", *new_key) );

	g_hash_table_insert(GLOBALS->h_pay, new_key, pay);	


	return TRUE;
}
*/


/**
 * da_pay_remove:
 * 
 * remove an payee from the GHashTable
 * 
 * Return value: TRUE if the key was found and removed
 *
 */
gboolean
da_pay_remove(guint32 key)
{
	DB( g_print("da_pay_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_pay, &key);
}

/**
 * da_pay_insert:
 * 
 * insert an payee into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_pay_insert(Payee *item)
{
guint32 *new_key;

	DB( g_print("da_pay_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_pay, new_key, item);	

	return TRUE;
}


/**
 * da_pay_append:
 * 
 * append a new payee into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_pay_append(Payee *item)
{
Payee *existitem;
guint32 *new_key;

	DB( g_print("da_pay_append\n") );

	/* ensure no duplicate */
	//g_strstrip(item->name);
	if( item->name != NULL )
	{
		existitem = da_pay_get_by_name( item->name );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_pay_get_max_key() + 1;
			item->key = *new_key;

			DB( g_print(" -> append id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_pay, new_key, item);	
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->name, item->key) );

	return FALSE;
}

/**
 * da_pay_get_max_key:
 * 
 * Get the biggest key from the GHashTable
 * 
 * Return value: the biggest key value
 *
 */
guint32
da_pay_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_pay, (GHFunc)da_pay_max_key_ghfunc, &max_key);
	return max_key;
}




/**
 * da_pay_get_by_name:
 * 
 * Get an payee structure by its name
 * 
 * Return value: Payee * or NULL if not found
 *
 */
Payee *
da_pay_get_by_name(gchar *name)
{
	DB( g_print("da_pay_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_pay, (GHRFunc)da_pay_name_grfunc, name);
}



/**
 * da_pay_get:
 * 
 * Get an payee structure by key
 * 
 * Return value: Payee * or NULL if not found
 *
 */
Payee *
da_pay_get(guint32 key)
{
	DB( g_print("da_pay_get_payee\n") );

	return g_hash_table_lookup(GLOBALS->h_pay, &key);
}





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#if MYDEBUG

static void
da_pay_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Payee *item = value;

	DB( g_print(" %d :: %s\n", *id, item->name) );

}

void
da_pay_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_pay, da_pay_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif

