/*  HomeBank -- Free, easy, personal catounting for everyone.
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
#include "da_category.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

Category *
da_cat_clone(Category *src_item)
{
Category *new_item = g_memdup(src_item, sizeof(Category));

	DB( g_print("da_cat_clone\n") );
	if(new_item)
	{
		//duplicate the string
		new_item->name		= g_strdup(src_item->name);
	}
	return new_item;
}


void 
da_cat_free(Category *item)
{
	DB( g_print("da_cat_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item);
	}
}


Category *
da_cat_malloc(void)
{
	DB( g_print("da_cat_malloc\n") );
	return g_malloc0(sizeof(Category));
}


void
da_cat_destroy(void)
{
	DB( g_print("da_cat_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_cat);
}


void
da_cat_new(void)
{
Category *item;

	DB( g_print("da_cat_new\n") );
	GLOBALS->h_cat = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_cat_free);
	
	// insert our 'no category'
	item = da_cat_malloc();
	item->name = g_strdup("");	
	da_cat_insert(item);	
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * da_cat_length:
 *
 * Return value: the number of elements
 */
guint
da_cat_length(void)
{
	return g_hash_table_size(GLOBALS->h_cat);
}



/**
 * da_cat_remove_grfunc:
 * 
 * GRFunc to get the max id
 * 
 * Return value: TRUE if the key/value must be removed
 *
 */
static gboolean
da_cat_remove_grfunc(gpointer key, Category *cat, guint32 *remkey)
{
	if(cat->key == *remkey || cat->parent == *remkey)
		return TRUE;

	return FALSE;
}


/**
 * da_cat_remove:
 * 
 * remove a category from the GHashTable
 * 
 * Return value: TRUE if the key was found and removed
 *
 */
guint
da_cat_remove(guint32 key)
{
	DB( g_print("da_cat_remove %d\n", key) );

	return g_hash_table_foreach_remove(GLOBALS->h_cat, (GHRFunc)da_cat_remove_grfunc, &key);
}

/**
 * da_cat_insert:
 * 
 * insert a category into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_cat_insert(Category *item)
{
guint32 *new_key;

	DB( g_print("da_cat_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_cat, new_key, item);	

	return TRUE;
}


/**
 * da_cat_append:
 * 
 * append a payee into the GHashTable
 * 
 * Return value: TRUE if inserted
 *
 */
gboolean
da_cat_append(Category *cat)
{
Category *existitem;
guint32 *new_key;
gchar *fullname;

	DB( g_print("da_cat_append\n") );

	if( cat->name != NULL)
	{
	
		fullname = da_cat_get_fullname(cat);
		existitem = da_cat_get_by_fullname( fullname );
		g_free(fullname);

		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_cat_get_max_key() + 1;
			cat->key = *new_key;

			DB( g_print(" -> insert id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_cat, new_key, cat);	
			return TRUE;
		}
		
	}

	DB( g_print(" -> %s already exist: %d\n", cat->name, new_key) );

	return FALSE;
}


/**
 * da_cat_max_key_ghfunc:
 * 
 * GHFunc for biggest key
 *
 */
static void
da_cat_max_key_ghfunc(gpointer key, Category *cat, guint32 *max_key)
{

	*max_key = MAX(*max_key, cat->key);
}

/**
 * da_cat_get_max_key:
 * 
 * Get the biggest key from the GHashTable
 * 
 * Return value: the biggest key value
 *
 */
guint32
da_cat_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)da_cat_max_key_ghfunc, &max_key);
	return max_key;
}

/**
 * da_cat_get_fullname:
 * 
 * Get category the fullname 'xxxx:yyyyy'
 * 
 * Return value: the category fullname (free it with g_free)
 *
 */
gchar *
da_cat_get_fullname(Category *cat)
{
Category *parent;

	if( cat->parent == 0)	
		return g_strdup(cat->name);
	else
	{
		parent = da_cat_get(cat->parent);
		if( parent )
		{
			return g_strdup_printf("%s:%s", parent->name, cat->name);
		}
	}

	return NULL;
}


/**
 * da_cat_name_grfunc:
 * 
 * GRFunc to get the max id
 * 
 * Return value: TRUE if the key/value pair match our name
 *
 */
static gboolean
da_cat_name_grfunc(gpointer key, Category *cat, gchar *name)
{

//	DB( g_print("%s == %s\n", name, cat->name) );
	if( name && cat->name)
	{
		if(!strcasecmp(name, cat->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_cat_get_key_by_name:
 * 
 * Get a category key by its name
 * 
 * Return value: the payee key or -1 if not found
 *
 */
guint32
da_cat_get_key_by_name(gchar *name)
{
Category *cat;

	DB( g_print("da_cat_get_id_by_name\n") );

	cat = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_name_grfunc, name);
	if( cat == NULL)
		return -1;

	return cat->key;
}

/**
 * da_cat_get_by_name:
 * 
 * Get a category structure by its name
 * 
 * Return value: Category * or NULL if not found
 *
 */
Category *
da_cat_get_by_name(gchar *name)
{
	DB( g_print("da_cat_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_name_grfunc, name);
}


/* fullname i.e. car:refuel */
struct fullcatcontext
{
	guint	parent;
	gchar	*name;
};


static gboolean
da_cat_fullname_grfunc(gpointer key, Category *item, struct fullcatcontext *ctx)
{

	//DB( g_print("'%s' == '%s'\n", ctx->name, item->name) );
	if( item->parent == ctx->parent )
	{
		if(!strcasecmp(ctx->name, item->name))
			return TRUE;
	}
	return FALSE;
}

Category *
da_cat_get_by_fullname(gchar *fullname)
{
struct fullcatcontext ctx;
gchar **typestr;
Category *item = NULL;

	DB( g_print("da_cat_get_by_fullname\n") );

	//g_strstrip(fullname);
	typestr = g_strsplit(fullname, ":", 2);
	if( g_strv_length(typestr) == 2 )
	{
		DB( g_print(" try to find parent '%s'\n", typestr[0]) );


		Category *parent = 	g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_name_grfunc, typestr[0]);
		if( parent != NULL )
		{
			ctx.parent = parent->key;
			ctx.name = typestr[1];

			DB( g_print(" searching %d '%s'\n", ctx.parent, ctx.name) );
			
			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
		}
	}
	else
	{
		ctx.parent = 0;
		ctx.name = fullname;

		item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
	}

	g_strfreev(typestr);

	DB( g_print(" return value %d\n", item) );

	return item;
}


/**
 * da_cat_append_ifnew_by_fullname:
 * 
 * append a category if it is new by fullname
 * 
 * Return value: 
 *
 */
Category *
da_cat_append_ifnew_by_fullname(gchar *fullname, gboolean imported)
{
struct fullcatcontext ctx;
gchar **typestr;
Category *newcat, *item, *retcat = NULL;
guint32 *new_key;


	DB( g_print("da_cat_append_ifnew_by_fullname\n") );

	DB( g_print(" -> fullname: '%s' %d\n", fullname, strlen(fullname)) );

	if( strlen(fullname) > 0 )
	{
		//g_strstrip(fullname);
		typestr = g_strsplit(fullname, ":", 2);
	
		/* if we have a subcategory : aaaa:bbb */
		if( g_strv_length(typestr) == 2 )
		{
			ctx.parent = 0;
			ctx.name = typestr[0];
			DB( g_print(" try to find the parent:'%s'\n", typestr[0]) );

			Category *parent = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( parent == NULL )
			{
				DB( g_print(" -> not found\n") );

				// append a new category
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->name = g_strdup(typestr[0]);
				newcat->imported = imported;
			
				parent = newcat;
			
				DB( g_print(" -> insert cat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);	
			}

			ctx.parent = parent->key;
			ctx.name = typestr[1];
			DB( g_print(" searching %d '%s'\n", ctx.parent, ctx.name) );
		
			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( item == NULL )
			{
				// append a new subcategory
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->parent = parent->key;
				newcat->name = g_strdup(typestr[1]);
				newcat->imported = imported;

				newcat->flags |= GF_SUB;

				DB( g_print(" -> insert subcat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);	
			
				retcat = newcat;
			}
			else
				retcat = item;
		}
		/* this a single category : aaaa */
		else
		{
			ctx.parent = 0;
			ctx.name = typestr[0];
			DB( g_print(" searching %d '%s'\n", ctx.parent, ctx.name) );

			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( item == NULL )
			{
				// append a new category
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->name = g_strdup(typestr[0]);
				newcat->imported = imported;

				DB( g_print(" -> insert cat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);	
			
				retcat = newcat;
			}
			else
				retcat = item;

		}

		g_strfreev(typestr);
	}

	return retcat;
}



/**
 * da_cat_get:
 * 
 * Get a category structure by key
 * 
 * Return value: Category * or NULL if not found
 *
 */
Category *
da_cat_get(guint32 key)
{
	DB( g_print("da_cat_get_payee\n") );

	return g_hash_table_lookup(GLOBALS->h_cat, &key);
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#if MYDEBUG

static void
da_cat_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Category *cat = value;

	DB( g_print(" %d :: %s (parent=%d\n", *id, cat->name, cat->parent) );

}

void
da_cat_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_cat, da_cat_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif

