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

/* wallet/account/import update flags */
enum
{
	UF_TITLE     = 1 << 0,
	UF_SENSITIVE = 1 << 1,
	UF_BALANCE   = 1 << 2,
	UF_VISUAL    = 1 << 3
};

/*
** filter options
*/
enum
{
	FILTER_DATE,
	FILTER_STATUS,
	FILTER_PAYMODE,
	FILTER_AMOUNT,
	FILTER_ACCOUNT,
	FILTER_CATEGORY,
	FILTER_PAYEE,
	FILTER_TEXT,
	FILTER_MAX
};

/*
** list pixbuf (account/operation)
*/
enum
{
	LST_PIXBUF_ADD,
	LST_PIXBUF_EDIT,
	LST_PIXBUF_REMIND,
	LST_PIXBUF_VALID,
	LST_PIXBUF_AUTO,
	LST_PIXBUF_WARNING,
	NUM_LST_PIXBUF
};	

/*
** paymode pixbuf
*/
enum
{
	PAYMODE_NONE,
	PAYMODE_CCARD,
	PAYMODE_CHECK,
	PAYMODE_CASH,
	PAYMODE_XFER,
	PAYMODE_INTXFER,
	/* 4.1 new payments here */
	PAYMODE_DCARD,
	PAYMODE_REPEATPMT,
	PAYMODE_EPAYMENT,
	PAYMODE_DEPOSIT,
	PAYMODE_FEE,
//	PAYMODE_,
	NUM_PAYMODE_MAX
};



/*
** operation edit type
*/
enum
{
	OPERATION_EDIT_ADD,
	OPERATION_EDIT_INHERIT,
	OPERATION_EDIT_MODIFY
};

/*
** toolbar item type
*/
enum
{
	TOOLBAR_SEPARATOR,
	TOOLBAR_BUTTON,
	TOOLBAR_TOGGLE
};

/*
** automated unit
*/
enum
{
	AUTO_UNIT_DAY,
	AUTO_UNIT_WEEK,
	AUTO_UNIT_MONTH,
	AUTO_UNIT_YEAR
};


/* list display operation (dsp_account) */
enum
{
	LST_DSPOPE_DATAS,
	LST_DSPOPE_STATUS,	/* fake column */
	LST_DSPOPE_DATE,	/* fake column */
	LST_DSPOPE_INFO,	/* fake column */
	LST_DSPOPE_PAYEE,	/* fake column */
	LST_DSPOPE_WORDING,	/* fake column */
	LST_DSPOPE_AMOUNT,	/* fake column */
	LST_DSPOPE_EXPENSE,	/* fake column */
	LST_DSPOPE_INCOME,	/* fake column */
	LST_DSPOPE_CATEGORY,	/* fake column */
	LST_DSPOPE_TAGS,	/* fake column */
	/* here we insert accoutn column, only used for detail */
	LST_DSPOPE_ACCOUNT,
	NUM_LST_DSPOPE
};


/* list define archive (defarchive) */
enum
{
	LST_DEFARC_DATAS,
	LST_DEFARC_OLDPOS,
	LST_DEFARC_AUTO,
	NUM_LST_DEFARC
};

/* csv format validator */
enum
{
	CSV_STRING,
	CSV_DATE,
	CSV_INT,
	CSV_DOUBLE
};


#define LST_OPE_IMPTOGGLE 2

