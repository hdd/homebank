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

#ifndef __HOMEBANK_AMIGA_H__
#define __HOMEBANK_AMIGA_H__

#include <gtk/gtk.h>
//#include <types.h>

typedef guint8 UBYTE;
typedef guint16 UWORD;
typedef guint32 ULONG;

/****************************************************************************/
/* Datas structures definition                                              */
/****************************************************************************/
/*
typedef struct _Account Account;
typedef struct _Payee Payee;
typedef struct _Group Group;
typedef struct _Archive Archive;
*/

struct Base				/* 64 bytes */
{
	ULONG	id;				/* identifier: 'HB'+version+revision */
	UBYTE	nbacc;			/* accounts count */
	UBYTE	okeuro;			/* new used in v1.9 for euro converted ok */
	UWORD	nbpay;			/* v3 payee count */
	UWORD	nbgrp;			/* groups count */
	UWORD	nbbud;			/* v3 budget count */
	UWORD	nbarc;			/* archives count */
	ULONG	nbope;			/* count operations */
	UBYTE	ccgrp;			/* default car cost group */
	UBYTE	autoinsert;		/* archive day insert view limit */
	UBYTE	name[32];		/* description */
	UBYTE	password[12];	/* password */
};

struct _Account		/* 122 bytes */
{
	UBYTE	acc_Id;
	UBYTE	acc_Flags;
	UBYTE	acc_Name[32];		/* nom du compte/name of the account */
	UBYTE	acc_Bank[32];		/* etablissement financier/financial institution */
	UBYTE	acc_Number[32];		/* N° compte/account n° */
	double	acc_Initial;		/* initial balance */
	double	acc_Minimum;		/* minimum balance */
	ULONG	acc_Cheque1;
	ULONG	acc_Cheque2;
};

#define AF_BUDGET	(1<<0)
#define AF_CLOSED	(1<<1)
#define AF_ADDED	(1<<2)
#define AF_CHANGED	(1<<3)

struct _Payee 		/* 46 bytes */
{
	UWORD	pay_Id;
	/*UBYTE	pay_Flags;*/			/* not used */
	/*UBYTE	pad1; */
	UBYTE	pay_Name[32];		/* the payee name */
	/*UWORD	pay_LastGroup;*/		/* last asociated group */
	/*ULONG	pay_LastDate;*/		/* last payed date */
};

struct _Group		/* 140 bytes */
{
	UWORD	grp_Id;
	UBYTE	grp_Flags;			/* type: revenus/depenses */
	UBYTE	pad1;
	UBYTE	grp_Name[32];		/* the group name */
	double	grp_Budget[13];		/* budget: jan, ... , dec, custom */
};

#define GF_SUB		(1<<0)
#define GF_INCOME	(1<<1)
#define GF_CUSTOM	(1<<2)

/** not used ?? */
/*
struct _Budget
{
	UWORD	bud_Id;
	UBYTE	bud_Flags;			// type: revenus/depenses
	UBYTE	pad1;
	UWORD	bug_Group;
	UWORD	bud_Year;			// year
	double	bud_Value[13];		// budget: jan, ... , dec, custom
};
*/

#define BF_CUSTOM	(1<<2)

struct _Archive			/* 56 bytes */
{
	double	arc_Amount;
	UBYTE	arc_Account;
	UBYTE	arc_To;
	UBYTE	arc_Mode;		/* paymode */
	UBYTE	arc_Flags;		/* see OF_FLAGS BELOW */
	UWORD	arc_Payee;		/* v3 payee */
	UWORD	arc_Group;		/*  */
	UBYTE	arc_Word[32];

	ULONG	arc_Next;		/* date of next */
	UBYTE	arc_Every;		/* unit delay */
	UBYTE	arc_Unit;		/* day/week/month/year */
	UBYTE	arc_Limit;		/* v3 */
	UBYTE	pad1;
};

struct _Operation		/* 68 bytes */
{
	ULONG	ope_Date;
	double	ope_Amount;
	UBYTE	ope_Account;
	UBYTE	ope_To;
	UBYTE	ope_Mode;		/* paymode */
	UBYTE	ope_Flags;		/* see OF_FLAGS BELOW */
	UWORD	ope_Payee;		/* v3 payee */
	UWORD	ope_Group;		/*  */
	UBYTE	ope_Word[32];
	UBYTE	ope_Info[16];
};


void import_from_amiga(gchar *filename);


#endif /* __HOMEBANK_AMIGA_H__ */
