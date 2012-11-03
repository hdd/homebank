/* HomeBank -- Free easy personal accounting for all !
 * Copyright (C) 1995-2006 Maxime DOYEN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "homebank.h"
#include "data_access.h"

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
/* Account */

Account *da_account_malloc(void)
{
	return g_malloc0(sizeof(Account));
}

Account *da_account_clone(Account *src_item)
{
Account *new_item = g_memdup(src_item, sizeof(Account));

	if(new_item)
	{
		//duplicate the string
		new_item->name		= g_strdup(src_item->name);
		new_item->number	= g_strdup(src_item->number);
		new_item->bankname	= g_strdup(src_item->bankname);
	}
	return new_item;
}

void da_account_free(Account *item)
{
	if(item != NULL)
	{
		g_free(item->name);
		g_free(item->number);
		g_free(item->bankname);
		g_free(item);
	}
}

void da_account_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Account *item = tmplist->data;
		da_account_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


/* = = = = = = = = = = = = = = = = = = = = */
/* Payee */

Payee *da_payee_malloc(void)
{
	return g_malloc0(sizeof(Payee));
}

Payee *da_payee_clone(Payee *src_item)
{
Payee *new_item = g_memdup(src_item, sizeof(Payee));

	if(new_item)
	{
		//duplicate the string
		new_item->name = g_strdup(src_item->name);
	}
	return new_item;
}

void da_payee_free(Payee *item)
{
	if(item != NULL)
	{
		if(item->name != NULL)
			g_free(item->name);

		g_free(item);
	}
}

void da_payee_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Payee *item = tmplist->data;
		da_payee_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


/* = = = = = = = = = = = = = = = = = = = = */
/* Category */

Category *da_category_malloc(void)
{
	return g_malloc0(sizeof(Category));
}

Category *da_category_clone(Category *src_item)
{
Category *new_item = g_memdup(src_item, sizeof(Category));

	if(new_item)
	{
		//duplicate the string
		new_item->name = g_strdup(src_item->name);
	}
	return new_item;
}

void da_category_free(Category *item)
{
	if(item != NULL)
	{
		if(item->name != NULL)
			g_free(item->name);

		g_free(item);
	}
}

void da_category_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Category *item = tmplist->data;
		da_category_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


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


/* = = = = = = = = = = = = = = = = = = = = */
/* Operation */

Operation *da_operation_malloc(void)
{
	return g_malloc0(sizeof(Operation));
}

Operation *da_operation_clone(Operation *src_item)
{
Operation *new_item = g_memdup(src_item, sizeof(Operation));

	if(new_item)
	{
		//duplicate the string
		new_item->wording = g_strdup(src_item->wording);
		new_item->info = g_strdup(src_item->info);
	}
	return new_item;
}

void da_operation_free(Operation *item)
{
	if(item != NULL)
	{
		if(item->wording != NULL)
			g_free(item->wording);
		if(item->info != NULL)
			g_free(item->info);
		if(item->same != NULL)
			g_list_free(item->same);

		g_free(item);
	}
}

void da_operation_destroy(GList *list)
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

gint da_operation_compare_func(Operation *a, Operation *b)
{
	return ((gint)a->date - b->date);
}

GList *da_operation_sort(GList *list)
{
	return( g_list_sort(list, (GCompareFunc)da_operation_compare_func));
}

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
		if(flt->acc != NULL)
			g_free(flt->acc);
		if(flt->pay != NULL)
			g_free(flt->pay);
		if(flt->cat != NULL)
			g_free(flt->cat);

		g_free(flt);
	}
}

/* = = = = = = = = = = = = = = = = = = = = */
/* CarCost */

CarCost *da_carcost_malloc(void)
{
	return g_malloc0(sizeof(CarCost));
}

void da_carcost_free(CarCost *item)
{
	if(item != NULL)
	{
		g_free(item);
	}
}

void da_carcost_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	CarCost *item = tmplist->data;
		da_carcost_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}

/* = = = = = = = = = = = = = = = = = = = = */
/* View populate */

void populate_view_acc(GtkWidget *dst_view, GList *src_list, gboolean clone)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(dst_view));
	i=0; list = g_list_first(src_list);
	while (list != NULL)
	{
	Account *item;

		if(clone == TRUE)
			item = da_account_clone(list->data);
		else
			item = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFACC_DATAS, item, 	//data struct
			LST_DEFACC_OLDPOS, i,  		//oldpos
			-1);

		//DB( g_printf(" populate_view_acc: %d %08x, %d\n", i, list->data, clone) );

		i++; list = g_list_next(list);
	}
}

void populate_view_pay(GtkWidget *dst_view, GList *src_list, gboolean clone)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(dst_view));
	i=0; list = g_list_first(src_list);
	while (list != NULL)
	{
	Payee *item;

		if(clone == TRUE)
			item = da_payee_clone(list->data);
		else
			item = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFPAY_DATAS, item, 	//data struct
			LST_DEFPAY_OLDPOS, i,  		//oldpos
			-1);

		//DB( g_printf(" populate_view_pay: %d %08x, %d\n", i, list->data, clone) );

		i++; list = g_list_next(list);
	}
}

void populate_view_cat(GtkWidget *dst_view, GList *src_list, gboolean clone)
{
GtkTreeModel *model;
GtkTreeIter  toplevel, child;
GList *list;
gint i;

	//insert all glist item into treeview
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(dst_view));
	i=0; list = g_list_first(src_list);
	while (list != NULL)
	{
	Category *item;

		if(clone == TRUE)
			item = da_category_clone(list->data);
		else
			item = list->data;

		if(!(item->flags & GF_SUB))
		{
			// category
			gtk_tree_store_append(GTK_TREE_STORE(model), &toplevel, NULL);
			gtk_tree_store_set(GTK_TREE_STORE(model), &toplevel,
				LST_DEFCAT_DATAS, item,	//data struct
				LST_DEFCAT_OLDPOS, i,   //oldpos
				-1);
		}
		else
		{
			// subcategory
			gtk_tree_store_append(GTK_TREE_STORE(model), &child, &toplevel);
			gtk_tree_store_set(GTK_TREE_STORE(model), &child,
				LST_DEFCAT_DATAS, item,	//data struct
				LST_DEFCAT_OLDPOS, i,   //oldpos
				-1);
		}

		//DB( g_printf(" populate_view_cat: %d %08x, %d\n", i, list->data, clone) );

		i++; list = g_list_next(list);
	}
}




/* for category */
/*
void hb_glist_populate_treeview_tree(GList *srclist, GtkWidget *treeview, gint column1_id, gint column2_id)
{
GtkTreeModel *model;
GtkTreeIter  toplevel, child;
GList *list;
gint i;

	//insert all glist item into treeview
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	i=0; list = g_list_first(srclist);
	while (list != NULL)
	{
	Category *item = da_category_clone(list->data);

		if(item->flags & GF_SUB)
		{
			//insert subcategory
			gtk_tree_store_append(GTK_TREE_STORE(model), &child, &toplevel);
			gtk_tree_store_set(GTK_TREE_STORE(model), &child,
				column1_id, item,	//data struct
				column2_id, i,		//oldpos
				-1);
		}
		else
		{
			//insert category
			gtk_tree_store_append(GTK_TREE_STORE(model), &toplevel, NULL);
			gtk_tree_store_set(GTK_TREE_STORE(model), &toplevel,
				column1_id, item,	//data struct
				column2_id, i,		//oldpos
				-1);
		}

		DB( g_printf(" populate_treeview: %d, %s\n", i, item->name ) );

		i++; list = g_list_next(list);
	}
}
*/

/* for category */
/*
void hb_glist_populate_treeview_tree_noclone(GList *srclist, GtkWidget *treeview, gint column1_id, gint column2_id)
{
GtkTreeModel *model;
GtkTreeIter  toplevel, child;
GList *list;
gint i;

	//insert all glist item into treeview
	if(treeview)
	{
		model  = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		i=0; list = g_list_first(srclist);
		while (list != NULL)
		{
		Category *item = list->data;

			if(item->flags & GF_SUB)
			{
				//insert subcategory
				gtk_tree_store_append(GTK_TREE_STORE(model), &child, &toplevel);
				gtk_tree_store_set(GTK_TREE_STORE(model), &child,
					column1_id, item,	//data struct
					column2_id, i,		//oldpos
					-1);
			}
			else
			{
				//insert category
				gtk_tree_store_append(GTK_TREE_STORE(model), &toplevel, NULL);
				gtk_tree_store_set(GTK_TREE_STORE(model), &toplevel,
					column1_id, item,	//data struct
					column2_id, i,		//oldpos
					-1);
			}

			DB( g_printf(" populate_treeview: %d, %s\n", i, item->name ) );

			i++; list = g_list_next(list);
		}
	}
}
*/

//void hb_glist_free_data(gpointer data, gpointer user_data) { /*DB( g_printf(" **glist free data %08x\n", data) );*/  g_free(data); }
/*
void hb_glist_free_with_datas(GList *list)
{

	//DB( g_printf(" ** freeing list at %08x\n", list) );
	//DB( g_printf(" **list %d\n", g_list_length(list)) );

	if(list != NULL)
	{
		g_list_foreach(list, hb_glist_free_data, NULL);
		g_list_free(list);
		list = NULL;
	}

	//DB( g_printf(" **list %d\n", g_list_length(list)) );

}
*/
/*
GList *hb_glist_clone_list(GList *srclist, gint datalength)
{
GList *list, *clonelist = NULL;
gpointer *ptr;

	list = g_list_first(srclist);
	while (list != NULL)
	{
		ptr = g_malloc(datalength);
		memcpy(ptr, list->data, datalength);
		clonelist = g_list_append(clonelist, ptr);

		//DB( g_printf(" clone_list (%d): %08x -> %08x\n", datalength, list->data, ptr) );


		list = g_list_next(list);
	}
	return clonelist;
}


void hb_glist_populate_treeview(GList *srclist, GtkWidget *treeview, gint column1_id, gint column2_id)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	//insert all glist item into treeview
	model  = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	i=0; list = g_list_first(srclist);
	while (list != NULL)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			column1_id, list->data,	//data struct
			column2_id, i,		//oldpos
			-1);

		//DB( g_printf(" populate_treeview: %d %08x\n", i, list->data) );

		i++; list = g_list_next(list);
	}
}
*/
