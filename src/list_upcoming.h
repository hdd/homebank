/* HomeBank -- Free easy personal accounting for all !
 * Copyright (C) 1995-2007 Maxime DOYEN
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

#ifndef __LIST_UPCOMING__H__
#define __LIST_UPCOMING__H__

GtkWidget *create_list_upcoming(void);

enum
{
	LST_DSPUPC_DATAS,
	LST_DSPUPC_PAYEE,
	LST_DSPUPC_WORDING,
	LST_DSPUPC_AMOUNT,
	LST_DSPUPC_NEXTON,
	LST_DSPUPC_REMAINING,
	NUM_LST_DSPUPC
};

#endif /* __LIST_UPCOMING__H__ */
