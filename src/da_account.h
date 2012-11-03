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

#ifndef __DA_ACCOUNT_H__
#define __DA_ACCOUNT_H__


typedef struct _account		Account;


struct _account
{
	guint32		key;
	gushort		flags;
	gushort		type;		//future use
//	guint32		kcur;		//future use
	gchar		*name;
	gchar		*number;
	gchar		*bankname;
	gdouble		initial;
	gdouble		minimum;
	guint32		cheque1;
	guint32		cheque2;
	//currency ?
	//note ?
	
// non persitent datas
	GtkWindow	*window;	//dsp_account opened
	guint32		pos;			//position in list
	gboolean	filter;
	
	// import datas
	gboolean	imported;
	guint32		imp_key;
	gchar		*imp_name;
};

#define AF_BUDGET	(1<<0)
#define AF_CLOSED	(1<<1)
#define AF_ADDED	(1<<2)
#define AF_CHANGED	(1<<3)

enum
{
	ACC_TYPE_NONE       = 0,
	ACC_TYPE_BANK       = 1,	//Banque
	ACC_TYPE_CASH       = 2,	//Espèce
	ACC_TYPE_ASSET      = 3,	//Actif (avoir)
	ACC_TYPE_CREDITCARD = 4,	//Carte crédit
	ACC_TYPE_LIABILITY  = 5,	//Passif (dettes)
//	ACC_TYPE_STOCK      = 6,	//Actions
//	ACC_TYPE_MUTUALFUND = 7,	//Fond de placement
//	ACC_TYPE_INCOME     = 8,	//Revenus
//	ACC_TYPE_EXPENSE    = 9,	//Dépenses
//	ACC_TYPE_EQUITY     = 10,	//Capitaux propres
//	ACC_TYPE_,
};



Account *da_acc_clone(Account *src_item);
Account *da_acc_malloc(void);
void da_acc_free(Account *item);
Account *da_acc_malloc(void);

void da_acc_destroy(void);
void da_acc_new(void);

guint		da_acc_length(void);
gboolean	da_acc_create_none(void);
gboolean	da_acc_remove(guint32 key);
gboolean	da_acc_insert(Account *acc);
gboolean	da_acc_append(Account *item);
guint32		da_acc_get_max_key(void);
Account		*da_acc_get_by_name(gchar *name);
Account		*da_acc_get_by_imp_name(gchar *name);
Account		*da_acc_get(guint32 key);

#endif

