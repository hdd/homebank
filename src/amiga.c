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
#include "amiga.h"

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

// Memory storage:
//amiga is big endian so ABCD value will be stored ABCD in memory
//x86 is little endian (packed from right to left) so ABCD value will be stored DCBA in memory

// Memory alignment:
//amiga is 2 bytes
//x86 (linux) is 4 bytes

// Amiga Date
//date stamp ds_day is the number of days since Jan 1, 1978;
//GDate Jan 1, 1978 is 722085 in julian representation

//HomeBank Amiga v3.0 file
#define AM_SIZE_BASE		64
#define AM_SIZE_ACCOUNT		122
#define AM_SIZE_PAYEE		42
#define AM_SIZE_GROUP		140
#define AM_SIZE_ARCHIVE		56
#define AM_SIZE_OPERATION	68

#define DATE_DECAY	722085

/* prototypes */
void import_from_amiga_real(gchar *buffer);
gdouble amiga_double(gchar *ptr, gint offset);
guint32 amiga_ulong(gchar *ptr, gint offset);
guint16 amiga_uword(gchar *ptr, gint offset);




/*
**
*/
void import_from_amiga(gchar *filename)
{
struct stat st;
gchar *buffer;

#ifdef WORDS_BIGENDIAN
	#warning WORDS_BIGENDIAN
#endif

	//get file infos (size)
	if( g_stat(filename, &st) == 0)
	{
		DB( printf(" %s: %d\n", filename, (int)st.st_size) );

		//allocate memory and read the amiga file
		buffer = g_malloc(st.st_size+8);
		if( buffer != NULL )
		{

			if( g_file_get_contents (filename, &buffer, NULL, NULL) )
			{
			gint ver, rev;

				ver = buffer[2];
				rev = buffer[3];
				DB( printf(" amiga file v%d.%d\n", ver, rev) );

				if(ver==3 && rev==0)
				{
					import_from_amiga_real(buffer);
				}
				else
				{
					homebank_message_dialog(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_ERROR,
						_("Import HomeBank Amiga File"),
						_("Wrong Amiga file version %d.%d\n( must be > 3.0 )"),
						 ver, rev);
				}
			}

			//finally free the memory
			g_free(buffer);
		}
	}
}

/*
** do the real import
*/
void import_from_amiga_real(gchar *buffer)
{
struct Base base;
gchar *ptr;
gint i, j;

	DB( printf(" your system: byteorder=%s_endian, memalign=%d\n", G_BYTE_ORDER == G_LITTLE_ENDIAN ? "little":"big", G_MEM_ALIGN ) );
	DB( printf(" amiga was  : byteorder=big_endian, memalign=2\n\n") );

	DB( printf(" ubyte=%d uword=%d ulong=%d double=%d\n", sizeof(UBYTE), sizeof(UWORD), sizeof(ULONG), sizeof(double) ) );
	DB( printf(" gint=%d, glong=%d, gdouble=%d\n\n", sizeof(gint), sizeof(glong), sizeof(gdouble)) );

	DB( printf(" taille header: %d\n", sizeof(struct Base)) );



	ptr = buffer;

	//convert base
	base.id = amiga_ulong(ptr, 0);
	base.nbacc = ptr[4];
	base.okeuro = ptr[5];
	base.nbpay = amiga_uword(ptr, 6);
	base.nbgrp = amiga_uword(ptr, 8);
	base.nbbud = amiga_uword(ptr, 10);
	base.nbarc = amiga_uword(ptr, 12);
	base.nbope = amiga_ulong(ptr, 14);
	memcpy(&base.ccgrp, ptr+18, 45);

	GLOBALS->title = g_convert (ptr+20, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
	GLOBALS->car_category = ptr[18];
	GLOBALS->auto_nbdays = 	ptr[19];

	DB( printf(" wallet: %s, %d, %d\n", GLOBALS->title, GLOBALS->car_category, GLOBALS->auto_nbdays) );

	ptr += AM_SIZE_BASE;
	GLOBALS->change++;

	//debug
	/*
	base.nbgrp = 0;
	base.nbbud = 0;
	base.nbarc = 0;
	base.nbope = 0;
	*/

	//accounts
	DB( g_printf("nbacc %d\n", base.nbacc) );
	for(i=0;i<base.nbacc;i++)
	{
	Account *entry;

		entry = da_account_malloc();

		entry->key = ptr[0];
		entry->flags = ptr[1];
		//entry->name = g_locale_to_utf8(ptr+2, -1, NULL, NULL, &error);
		entry->name = g_convert (ptr+2, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
		entry->number = g_convert (ptr+2+64, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
		entry->bankname = g_convert (ptr+2+32, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
		entry->initial = amiga_double(ptr, 98);
		entry->minimum = amiga_double(ptr, 98+8);
		entry->cheque1 = amiga_ulong(ptr, 98+8+8);
		entry->cheque2 = amiga_ulong(ptr, 98+8+8+4);

		DB( printf(" account: %d, %s = %s\n", entry->key, ptr+2, entry->name) );

		//acc->acc_Flags |= AF_ADDED;

		//add our entry pointer to the glist
		//g_printf("account: %d, inserting %s\n", i, acc->acc_Name);

		GLOBALS->acc_list = g_list_append(GLOBALS->acc_list, entry);

		ptr += AM_SIZE_ACCOUNT;
		GLOBALS->change++;
	}


	//category
	DB( g_printf("nbcat %d\n", base.nbgrp) );
	for(i=0;i<base.nbgrp;i++)
	{
	Category *entry;
	gint parentid;
	gboolean budget;

		entry = da_category_malloc();

		entry->key = amiga_uword(ptr, 0);
		entry->flags = ptr[2];

		/*
		#if MYDEBUG == 1
		if(i >= 48)
		{
			g_print("******\n=> k:%d f:%d p:%d %s (", entry->key, entry->flags, ptr[3], ptr+4);
			if(entry->flags & GF_SUB) g_print(" sub");
			if(entry->flags & GF_INCOME) g_print(" inc");
			if(entry->flags & GF_CUSTOM) g_print(" cust");
			g_print(")\n");
		}
		#endif
		*/

		// look here !!

		if(!(entry->flags & GF_SUB))
		{
			parentid = entry->key;
			//DB( g_print("\n----------------\nnew parent %d\n", parentid) );

		}

		if(entry->flags & GF_SUB)
			entry->parent = parentid;
		else
			entry->parent = 0;

		entry->name = g_convert (ptr+4, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

		//budget
		// months are 0->11 custom is at 12
		budget = FALSE;
		for(j=0;j<12;j++)
		{
			entry->budget[j+1] = amiga_double(ptr, 36 + (j*8));
			//if(entry->budget[j]) DB( g_print("%d:%s :: %d %f\n", i, entry->name, j, entry->budget[j]) );
			if(entry->budget[j+1]) budget = TRUE;
		}
		entry->budget[0] = amiga_double(ptr, 36 + (12*8));
		if(entry->budget[0]) budget = TRUE;

		if(budget == TRUE)
			entry->flags |= GF_BUDGET;


		//debug
		/*
		#if MYDEBUG == 1
		if(!(entry->flags & GF_SUB))
		{
			g_printf("%3d import cat    :: %2d %2d %s (%c)\n", i+1,
				entry->key, entry->parent, entry->name, entry->flags & GF_INCOME ? '+':'-');
		}
		else
		{
			g_printf("%3d import subcat :: + %2d %2d %s (%c)\n", i+1,
				entry->key, entry->parent, entry->name, entry->flags & GF_INCOME ? '+':'-');
		}
		#endif
		*/

		GLOBALS->cat_list = g_list_append(GLOBALS->cat_list, entry);

		ptr += AM_SIZE_GROUP;
		GLOBALS->change++;
	}

	//payee
	DB( g_printf("nbpay %d at %d\n", base.nbpay, (ptr-buffer)) );
	for(i=0;i<base.nbpay;i++)
	{
	Payee *entry;

		entry = da_payee_malloc();
		entry->key = amiga_uword(ptr, 0);
		//entry->pay_Flags = ptr[2];

		entry->name = g_convert (ptr+4, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

		//g_printf("payee: %d, inserting %s (%d)\n", i, entry->pay_Name, (ptr-buffer));

		GLOBALS->pay_list = g_list_append(GLOBALS->pay_list, entry);

		ptr += AM_SIZE_PAYEE;
		GLOBALS->change++;
	}

	//archives
	DB( g_printf("nbarc %d\n", base.nbarc) );
	for(i=0;i<base.nbarc;i++)
	{
	Archive *entry;

		entry = da_archive_malloc();
		entry->amount		= amiga_double(ptr, 0);
		entry->account		= ptr[8];
		entry->dst_account	= ptr[9];
		entry->paymode		= ptr[10];
		entry->flags		= ptr[11];
		entry->payee		= amiga_uword(ptr, 12);
		entry->category		= amiga_uword(ptr, 14);
		entry->wording		= g_convert (ptr+16, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

		//add of standing order
		//if(entry->arc_Mode >= 4) entry->arc_Mode++;

		entry->nextdate = DATE_DECAY + amiga_ulong(ptr, 48);
		entry->every = ptr[48+4];
		entry->unit = ptr[48+5];
		entry->limit = ptr[48+6];

		GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, entry);

		ptr += AM_SIZE_ARCHIVE;
		GLOBALS->change++;
	}

	//operations
	DB( g_printf("** Operations: %d\n", base.nbope) );



	for(i=0;i<base.nbope;i++)
	{
	Operation *entry;

		//entry = &ope;
		entry = da_operation_malloc();

		entry->date = DATE_DECAY + amiga_ulong(ptr, 0);
		entry->amount = amiga_double(ptr, 4);
		entry->account = ptr[12];
		entry->dst_account = ptr[13];
		entry->paymode = ptr[14];
		entry->flags = ptr[15];
		entry->payee = amiga_uword(ptr, 16);
		entry->category = amiga_uword(ptr, 18);
		entry->wording	= g_convert (ptr+20, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
		entry->info		= g_convert (ptr+52, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

		//add of standing order
		//if(entry->ope_Mode >= 4) entry->ope_Mode++;

		//DB( g_printf(" + %d, inserting :: %d %s %.2f\n", i, entry->date, entry->wording, entry->amount) );

		GLOBALS->ope_list = g_list_append(GLOBALS->ope_list, entry);

		ptr += AM_SIZE_OPERATION;
		GLOBALS->change++;
	}




}


/*
** get amiga DOUBLE (big endian)
*/
gdouble amiga_double(gchar *ptr, gint offset)
{
gdouble value, *dptr;

	ptr += offset;

	//hack method used as not found howto with GLib
	if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
	{
	guint8 temp[8];
	gint i;

		// reorder bytes 01234567 to 76543210
		for(i=0;i<8;i++)
		{
			temp[7-i] = ptr[i];
			//g_printf("%d %x => %d %x\n", 7-i, temp[7-i] , i, ptr[i]);
		}

		dptr = (gdouble *)temp;
		value = *dptr;
	}
	else
	{
		dptr = (gdouble *)ptr;
		value = *dptr;
	}

	//g_print("result: %f\n", value);

	return value;
}


/*
** get amiga ULONG (big endian)
*/
guint32 amiga_ulong(gchar *ptr, gint offset)
{
guint32	*valueptr;
guint32 value;

	ptr += offset;

	//hack method...
	//guint32 toto = (ptr[0]<<24 | ptr[1]<<16 | ptr[2]<<8 | ptr[3]);

	//glib method ;-)
	valueptr = (guint32 *)ptr;
	value = GUINT32_FROM_BE(*valueptr);

	//g_printf("%x,%x,%x,%x = %d (0x%x) == %d (0x%x)\n", ptr[0],ptr[1],ptr[2],ptr[3], value, value, toto, toto);

	return value;
}

/*
** get amiga UWORD (big endian)
*/
guint16 amiga_uword(gchar *ptr, gint offset)
{
guint16 *valueptr;
guint16 value;

	ptr += offset;
	//hack method
	//value = ptr[0]<<8 | ptr[1];
	valueptr = (guint16 *)ptr;
	value = GUINT16_FROM_BE(*valueptr);

	return value;
}

