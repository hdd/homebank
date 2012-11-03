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

/* wallet/account update flags */
enum
{
	UF_TITLE     = 1 << 0,
	UF_SENSITIVE = 1 << 1,
	UF_BALANCE   = 1 << 2,
	UF_VISUAL    = 1 << 3
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
	PAYMODE_CARD,
	PAYMODE_CHEQUE,
	PAYMODE_CASH,
	PAYMODE_BANKTRANSFERT,
	PAYMODE_PERSTRANSFERT,
	NUM_PAYMODE_MAX
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
	//FILTER_TEXT,
	FILTER_MAX
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

/* list display account (wallet) */
enum
{
	LST_DSPACC_DATAS,
	LST_DSPACC_NAME,	/* fake column */
	LST_DSPACC_NUMBER,	/* fake column */
	LST_DSPACC_BANK,
	LST_DSPACC_TODAY,
	LST_DSPACC_FUTURE,
	NUM_LST_DSPACC
};

/* list display operation (account) */
enum
{
	LST_DSPOPE_DATAS,
	LST_DSPOPE_STATUS,	/* fake column */
	LST_DSPOPE_DATE,	/* fake column */
	LST_DSPOPE_INFO,	/* fake column */
	LST_DSPOPE_PAYEE,	/* fake column */
	LST_DSPOPE_WORDING,	/* fake column */
	LST_DSPOPE_EXPENSE,	/* fake column */
	LST_DSPOPE_INCOME,	/* fake column */
	LST_DSPOPE_CATEGORY,	/* fake column */
	NUM_LST_DSPOPE
};

/* list define account (defaccount) */
enum
{
	LST_DEFACC_TOGGLE,
	LST_DEFACC_DATAS,
	LST_DEFACC_OLDPOS,
	NUM_LST_DEFACC
};

/* list define payee (defpayee) */
enum
{
	LST_DEFPAY_TOGGLE,
	LST_DEFPAY_DATAS,
	LST_DEFPAY_OLDPOS,
	NUM_LST_DEFPAY
};

/* list define category (defcategory) */
enum
{
	LST_DEFCAT_TOGGLE,
	LST_DEFCAT_DATAS,
	LST_DEFCAT_OLDPOS,
	NUM_LST_DEFCAT
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

enum
{
	FILETYPE_UNKNOW,
	FILETYPE_AMIGA_HB,
	FILETYPE_CSV_HB,
	FILETYPE_OFX,
	NUM_FILETYPE
};

#define LST_OPE_IMPTOGGLE 2

