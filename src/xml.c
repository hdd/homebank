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


#include "homebank.h"

#include "da_transaction.h"
#include "hb_transaction.h"
#include "xml.h"

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
extern struct Preferences *PREFS;

/*
typedef enum
{
  STATE_START,
  STATE_ROOT,
  STATE_MENU,
  STATE_TOOLBAR,
  STATE_MENUITEM,
  STATE_TOOLITEM,
  STATE_ACCELERATOR,
  STATE_END
} ParseState;
*/

typedef struct _ParseContext ParseContext;
struct _ParseContext
{
/*
  ParseState state;
  ParseState prev_state;

  GtkUIManager *self;

  GNode *current;

  guint merge_id;
*/
	gdouble	version;

};

static void
start_element_handler (GMarkupParseContext *context,
		       const gchar         *element_name,
		       const gchar        **attribute_names,
		       const gchar        **attribute_values,
		       gpointer             user_data,
		       GError             **error)
{
ParseContext *ctx = user_data;
//GtkUIManager *self = ctx->self;
gint i, j;

	DB( g_print("** start element: %s\n", element_name) );


	switch(element_name[0])
	{
		//get file version
		/*		
		case 'h':
		{
			if(!strcmp (element_name, "homebank"))
			{
			     if(!strcmp (attribute_names[0], "v" ))
			     {
					version = g_ascii_strtod(attribute_values[0], NULL);
			     	DB( g_print(" version %f\n", version) );
			     }
			
			}
		}
		*/

		case 'a':
		{
			if(!strcmp (element_name, "account"))
			{
			Account *entry = da_acc_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"     )) { entry->key   = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"   )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "pos"     )) { entry->pos   = atoi(attribute_values[i]); }
					//todo: for stock account
					//else if(!strcmp (attribute_names[i], "type"    )) { entry->type = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->name = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "number"  )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->number = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "bankname")) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->bankname = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "initial" )) { entry->initial = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "minimum" )) { entry->minimum = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "cheque1" )) { entry->cheque1 = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "cheque2" )) { entry->cheque2 = atoi(attribute_values[i]); }
				
				}

				//version upgrade: type was added in 0.2
				//todo: for stock account
				/*				
				if(version <= 0.1)
				{
					entry->type = ACC_TYPE_BANK;
					DB( g_print(" acctype forced to BANK\n") );
				}
			*/

				DB( g_print(" version %f\n", ctx->version) );

				//upgrade to v0.2 file
				// we must change account reference by making a +1 to its index references
				if( ctx->version == 0.1 )
				{
					entry->key++;
					entry->pos = entry->key;
				}

				//all attribute loaded: append
				da_acc_insert(entry);
			}
		}
		break;

		case 'p':
		{
			if(!strcmp (element_name, "pay"))
			{
			Payee *entry = da_pay_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"  )) { entry->key = atoi(attribute_values[i]); }
					//else if(!strcmp (attribute_names[i], "flags")) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name" )) { entry->name = g_strdup(attribute_values[i]); }
				}

				//all attribute loaded: append
				da_pay_insert(entry);

			}
			else if(!strcmp (element_name, "properties"))
			{
				for (i = 0; attribute_names[i] != NULL; i++)
				{
					     if(!strcmp (attribute_names[i], "title"       )) { g_free(GLOBALS->title); GLOBALS->title = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "car_category")) { GLOBALS->car_category = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "auto_nbdays" )) { GLOBALS->auto_nbdays = atoi(attribute_values[i]); }
				}
			}
		}
		break;

		case 'c':
		{
			if(!strcmp (element_name, "cat"))
			{
			Category *entry = da_cat_malloc();
			gboolean budget;

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					//DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"   )) { entry->key = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "parent")) { entry->parent = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags" )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name"  )) { entry->name = g_strdup(attribute_values[i]); }

					budget = FALSE;
					for(j=0;j<=12;j++)
					{
					gchar *tmpname;

						tmpname = g_strdup_printf ("b%d", j);
						if(!(strcmp (attribute_names[i], tmpname))) { entry->budget[j] = g_ascii_strtod(attribute_values[i], NULL); }
						g_free(tmpname);

						if(entry->budget[j]) budget = TRUE;
					}
					if(budget == TRUE)
						entry->flags |= GF_BUDGET;

				}

				//all attribute loaded: append
				da_cat_insert( entry);

			}
		}
		break;

		case 't':
		{
			if(!strcmp (element_name, "tags"))
			{
			Tag *entry = da_tag_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"  )) { entry->key = atoi(attribute_values[i]); }
					//else if(!strcmp (attribute_names[i], "flags")) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name" )) { entry->name = g_strdup(attribute_values[i]); }
				}

				//all attribute loaded: append
				da_tag_insert(entry);

			}
		}

		case 'f':
		{
			if(!strcmp (element_name, "fav"))
			{
			Archive *entry = da_archive_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					//DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "account"    )) { entry->account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "dst_account")) { entry->dst_account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "payee"      )) { entry->payee = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "category"   )) { entry->category = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->wording = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "nextdate"   )) { entry->nextdate = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "every"      )) { entry->every = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "unit"       )) { entry->unit = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "limit"      )) { entry->limit = atoi(attribute_values[i]); }

				}

				//all attribute loaded: append
				GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, entry);

			}
		}
		break;

		case 'o':
		{
			if(!strcmp (element_name, "ope"))
			{
			Operation *entry = da_operation_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					//DB( g_print(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "date"       )) { entry->date = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "account"    )) { entry->account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "dst_account")) { entry->dst_account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "payee"      )) { entry->payee = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "category"   )) { entry->category = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->wording = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "info"       )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->info = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "tags"       ))
					{
						if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL)
						{
							transaction_set_tags(entry, attribute_values[i]);
						}
					}
				}

				//bugfix 303886
				if(entry->category < 0)
					entry->category = 0;
				
				//all attribute loaded: append
				da_operation_append(entry);				
			}
		}
		break;



	}

}

/*
static void
end_element_handler (GMarkupParseContext *context,
		     const gchar         *element_name,
		     gpointer             user_data,
		     GError             **error)
{
  ParseContext *ctx = user_data;

	//DB( g_print("-- end element: %s\n", element_name) );


}
*/

static GMarkupParser hb_parser = {
  start_element_handler,
  NULL,	//end_element_handler,
  NULL, //text_handler,
  NULL,
  NULL  //cleanup
};

/*
** XML load homebank file: wallet
*/
gint homebank_load_xml(gchar *filename)
{
gint retval;
gchar *buffer;
gsize length;
GError *error = NULL;
ParseContext ctx = { 0 };
GMarkupParseContext *context;
gboolean rc;

	retval = XML_OK;
	if (!g_file_get_contents (filename, &buffer, &length, &error))
	{
		//g_message ("%s", error->message);
		retval = XML_LOAD_ERROR;
		g_error_free (error);
	}
	else
	{
	gchar *v_buffer;
	gdouble version;

		/* v3.4 add :: prevent load of future file version */
		v_buffer = g_strstr_len(buffer, 50, "<homebank v=");
		if( v_buffer == NULL )
			return XML_FILE_ERROR;

		DB( g_print("id line: %s", v_buffer) );

		version = g_ascii_strtod(v_buffer+13, NULL);	/* a little hacky, but works ! */
		if( version == 0.0 )
			version = 0.1;

		ctx.version = version;

		if( version > FILE_VERSION )
		{
			DB( g_print("failed: version %f is not supported (max is %f)\n", version, FILE_VERSION) );
			return XML_VERSION_ERROR;
		}
		else
		{		
			DB( g_print("ok file version is: %.1f\n", version) );
		
			context = g_markup_parse_context_new (&hb_parser, 0, &ctx, NULL);

			error = NULL;
			rc = g_markup_parse_context_parse (context, buffer, length, &error);

			if( error )
				g_print("failed: %s\n", error->message);

			if( rc == FALSE )
			{
				error = NULL;
				g_markup_parse_context_end_parse(context, &error);

				if( error )
					g_print("failed: %s\n", error->message);
			}

			g_markup_parse_context_free (context);


			g_free (buffer);

			//upgrade to v0.2 file
			// we must change account reference by making a +1 to its index references
			if( version == 0.1 )
			{
			GList *list;

				list = g_list_first(GLOBALS->ope_list);
				while (list != NULL)
				{
				Operation *entry = list->data;
					entry->account++;
					entry->dst_account++;
					list = g_list_next(list);
				}

				list = g_list_first(GLOBALS->arc_list);
				while (list != NULL)
				{
				Archive *entry = list->data;
					entry->account++;
					entry->dst_account++;
					list = g_list_next(list);
				}

			}

			//upgrade to v0.3 file
			// we must assume categories exists : bugs 303886, 303738
			if( version == 0.2 )
			{
			Category *cat;
			GList *list;

				list = g_list_first(GLOBALS->ope_list);
				while (list != NULL)
				{
				Operation *entry = list->data;

					cat = da_cat_get(entry->category);
					if(cat == NULL)
					{
						DB( g_print(" !! fixing cat for ope: %d is unknow\n", entry->category) );
						entry->category = 0;
					}

					list = g_list_next(list);
				}

				list = g_list_first(GLOBALS->arc_list);
				while (list != NULL)
				{
				Archive *entry = list->data;

					cat = da_cat_get(entry->category);
					if(cat == NULL)
					{
						DB( g_print(" !! fixing cat for arc: %d is unknow\n", entry->category) );
						entry->category = 0;
					}

					list = g_list_next(list);
				}

			}



		}
	}

	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/*
** XML properties save
*/
static void homebank_prop_save_xml(GIOChannel *io)
{
gchar *tmpstr;
gchar *title;

	title = GLOBALS->title == NULL ? "" : GLOBALS->title;

	tmpstr = g_markup_printf_escaped("<properties title=\"%s\" car_category=\"%d\" auto_nbdays=\"%d\"/>\n",
		title,
		GLOBALS->car_category,
		GLOBALS->auto_nbdays
	);

	g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
	g_free(tmpstr);

}


/*
** XML account save
*/
static void homebank_acc_save_xml(GIOChannel *io)
{
GList *list;
char buf1[G_ASCII_DTOSTR_BUF_SIZE];
char buf2[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	//we must use this, as fprintf use locale decimal settings and not '.'
	//char buf[G_ASCII_DTOSTR_BUF_SIZE];
	//fprintf (out, "value=%s\n", g_ascii_dtostr (buf, sizeof (buf), value));

	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		item->flags &= (AF_BUDGET|AF_CLOSED);

		//todo: for stock account
		//tmpstr = g_markup_printf_escaped("<account key=\"%d\" flags=\"%d\" type=\"%d\" name=\"%s\"\n"
		tmpstr = g_markup_printf_escaped("<account key=\"%d\" flags=\"%d\" pos=\"%d\" name=\"%s\""
			" number=\"%s\" bankname=\"%s\" initial=\"%s\" minimum=\"%s\""
			" cheque1=\"%d\" cheque2=\"%d\"/>\n",
			item->key,
			item->flags,
			item->pos,
			//todo: for stock account
			//item->type,
			item->name == NULL ? "" : item->name,
			item->number == NULL ? "" : item->number,
			item->bankname == NULL ? "" : item->bankname,
			g_ascii_dtostr (buf1, sizeof (buf1), item->initial),
			g_ascii_dtostr (buf2, sizeof (buf2), item->minimum),
			item->cheque1,
			item->cheque2
		);

		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);
	
		list = g_list_next(list);
	}
	g_list_free(list);
}

/*
** XML payee save
*/
static void homebank_pay_save_xml(GIOChannel *io)
{
GList *list;
gchar *tmpstr;

	list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if(item->key != 0)
		{
			tmpstr = g_markup_printf_escaped("<pay key=\"%d\" name=\"%s\"/>\n",
				item->key,
				item->name
			);
			
			g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
			g_free(tmpstr);

		}
		list = g_list_next(list);
	}
	g_list_free(list);
}

/*
** XML category save
*/
static void homebank_cat_save_xml(GIOChannel *io)
{
GList *list;
guint i;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if(item->key != 0)
		{
			tmpstr = g_markup_printf_escaped("<cat key=\"%d\" parent=\"%d\" flags=\"%d\" name=\"%s\"",
				item->key,
				item->parent,
				item->flags,
				item->name
			);

			g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
			g_free(tmpstr);

			for(i=0;i<=12;i++)
			{
				if(item->budget[i] != 0)
				{
					tmpstr = g_strdup_printf(" b%d=\"%s\"", i, g_ascii_dtostr (buf, sizeof (buf), item->budget[i]));
					g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
					g_free(tmpstr);
				}
			}

			g_io_channel_write_chars(io, "/>\n", -1, NULL, NULL);

		}
		list = g_list_next(list);
	}
}

/*
** XML tag save
*/
static void homebank_tag_save_xml(GIOChannel *io)
{
GList *list;
gchar *tmpstr;

	list = g_hash_table_get_values(GLOBALS->h_tag);
	while (list != NULL)
	{
	Tag *item = list->data;

		if(item->key != 0)
		{
			tmpstr = g_markup_printf_escaped("<tag key=\"%d\" name=\"%s\"/>\n",
				item->key,
				item->name
			);
			
			g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
			g_free(tmpstr);

		}
		list = g_list_next(list);
	}
	g_list_free(list);
}

/*
** XML archive save
*/
static void homebank_arc_save_xml(GIOChannel *io)
{
GList *list;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = list->data;

		tmpstr = g_strdup_printf("<fav amount=\"%s\" account=\"%d\" dst_account=\"%d\"",
			g_ascii_dtostr (buf, sizeof (buf), item->amount),
			item->account,
			item->dst_account
		);
		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);


		tmpstr = g_strdup_printf(" paymode=\"%d\" flags=\"%d\" payee=\"%d\" category=\"%d\"",
			item->paymode,
			item->flags,
			item->payee,
			item->category
		);
		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);


		tmpstr = g_markup_printf_escaped(" wording=\"%s\" nextdate=\"%d\" every=\"%d\" unit=\"%d\" limit=\"%d\"/>\n",
			item->wording == NULL ? "" : item->wording,
			item->nextdate,
			item->every,
			item->unit,
			item->limit
		);

		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);

		list = g_list_next(list);
	}
}

/*
** XML operation save
*/
static void homebank_ope_save_xml(GIOChannel *io)
{
GList *list;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;
gchar *tagstr;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *item = list->data;

		item->flags &= (OF_VALID|OF_INCOME|OF_REMIND);

		tagstr = transaction_get_tagstring(item);

		tmpstr = g_markup_printf_escaped(
			"<ope date=\"%d\" amount=\"%s\" account=\"%d\" dst_account=\"%d\" paymode=\"%d\" flags=\"%d\" "
			"payee=\"%d\" category=\"%d\" wording=\"%s\" info=\"%s\" tags=\"%s\"/>\n",
			item->date,
			g_ascii_dtostr (buf, sizeof (buf), item->amount),
			item->account,
			item->dst_account,
			item->paymode,
			item->flags,
			item->payee,
			item->category,
			item->wording == NULL ? "" : item->wording,
			item->info == NULL ? "" : item->info,
			item->tags == NULL ? "" : tagstr			
		);

		g_free(tagstr);

		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);

		list = g_list_next(list);
	}
}

/*
** XML save homebank file: wallet
*/
void homebank_save_xml(gchar *filename)
{
GIOChannel *io;
char buf1[G_ASCII_DTOSTR_BUF_SIZE];
gchar *outstr;

	io = g_io_channel_new_file(filename, "w", NULL);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
	}
	else
	{
		g_io_channel_write_chars(io, "<?xml version=\"1.0\"?>\n", -1, NULL, NULL);
		
		outstr = g_strdup_printf("<homebank v=\"%s\">\n", g_ascii_dtostr (buf1, sizeof (buf1), FILE_VERSION));
		g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
		g_free(outstr);

		homebank_prop_save_xml(io);
		homebank_acc_save_xml(io);
		homebank_pay_save_xml(io);
		homebank_cat_save_xml(io);
		homebank_tag_save_xml(io);
		homebank_arc_save_xml(io);
		homebank_ope_save_xml(io);

		g_io_channel_write_chars(io, "</homebank>\n", -1, NULL, NULL);

		g_io_channel_unref (io);
	}
}
