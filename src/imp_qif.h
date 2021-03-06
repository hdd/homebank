/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2010 Maxime DOYEN
 *
 *	This file is part of HomeBank.
 *
 *	HomeBank is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	HomeBank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

typedef struct _QifContext QifContext;
struct _QifContext
{
	GList *q_acc;
	GList *q_cat;
	GList *q_pay;
	GList *q_tra;

};



typedef struct _qif_tran	QIF_Tran;
struct _qif_tran
{
	gchar		*date;
	gdouble		amount;
	gboolean	validated;
	gchar		*info;
	gchar		*payee;
	gchar		*memo;
	gchar		*category;
	gchar		*account;
};


enum QIF_Type
{
	QIF_NONE,
	QIF_HEADER,
	QIF_ACCOUNT,
	QIF_CATEGORY,
	QIF_CLASS,
	QIF_MEMORIZED,
	QIF_TRANSACTION,
	QIF_SECURITY,
	QIF_PRICES
};


GList *account_import_qif(gchar *filename, ImportContext *ictx);
gdouble
hb_qif_parser_get_amount(gchar *string);

void test_qif_export (void);

