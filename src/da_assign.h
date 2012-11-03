/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2010 Maxime DOYEN
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

#ifndef __DA_ASSIGN_H__
#define __DA_ASSIGN_H__


typedef struct _assign		Assign;


struct _assign
{
	guint32   	key;
	gchar   	*name;
	gboolean	exact;
	guint32		payee;
	guint32		category;
};

void 
da_asg_free(Assign *item);
Assign *da_asg_malloc(void);

void da_asg_destroy(void);
void da_asg_new(void);

guint		da_asg_length(void);
gboolean	da_asg_create_none(void);
gboolean	da_asg_remove(guint32 key);
gboolean	da_asg_insert(Assign *asg);
gboolean	da_asg_append(Assign *asg);
guint32		da_asg_get_max_key(void);
Assign		*da_asg_get_by_name(gchar *name);
Assign		*da_asg_get(guint32 key);

#endif

