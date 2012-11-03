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

#ifndef __HB_CATEGORY_DATA_H__
#define __HB_CATEGORY_DATA_H__


typedef struct _category		Category;


struct _category
{
	guint		key;
	guint		parent;
	gushort		flags;
	gchar		*name;
	gdouble		budget[13];	//0:is same value, 1 ..12 are months
	gboolean	filter;
	gboolean	imported;
};

#define GF_SUB		(1<<0)
#define GF_INCOME	(1<<1)
#define GF_CUSTOM	(1<<2)
#define GF_BUDGET	(1<<3)

Category *da_cat_clone(Category *src_item);
void da_cat_free(Category *item);
Category *da_cat_malloc(void);
void da_cat_destroy(void);
void da_cat_new(void);

guint da_cat_length(void);
guint32 da_cat_remove(guint32 key);
gboolean da_cat_insert(Category *acc);
gboolean da_cat_append(Category *cat);
guint32 da_cat_get_max_key(void);
gchar *da_cat_get_fullname(Category *cat);

guint32 da_cat_get_key_by_name(gchar *name);
Category *da_cat_get_by_name(gchar *name);
Category *da_cat_get(guint32 key);
Category *da_cat_get_by_fullname(gchar *fullname);
Category *da_cat_append_ifnew_by_fullname(gchar *fullname, gboolean imported);

#endif

