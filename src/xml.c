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
struct HomeBank *GLOBALS;
struct Preferences *PREFS;

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

typedef struct _ParseContext ParseContext;
struct _ParseContext
{
  ParseState state;
  ParseState prev_state;

  GtkUIManager *self;

  GNode *current;

  guint merge_id;
};

static void
start_element_handler (GMarkupParseContext *context,
		       const gchar         *element_name,
		       const gchar        **attribute_names,
		       const gchar        **attribute_values,
		       gpointer             user_data,
		       GError             **error)
{
//ParseContext *ctx = user_data;
//GtkUIManager *self = ctx->self;
gint i, j;

	DB( g_printf("** element: %s\n", element_name) );


	switch(element_name[0])
	{
		//todo: add version control here

		case 'a':
		{
			if(!strcmp (element_name, "account"))
			{
			Account *entry = da_account_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_printf(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"     )) { entry->key = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"   )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->name = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "number"  )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->number = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "bankname")) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->bankname = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "initial" )) { entry->initial = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "minimum" )) { entry->minimum = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "cheque1" )) { entry->cheque1 = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "cheque2" )) { entry->cheque2 = atoi(attribute_values[i]); }
				}

				//all attribute loaded: append
				GLOBALS->acc_list = g_list_append(GLOBALS->acc_list, entry);

			}
		}
		break;

		case 'p':
		{
			if(!strcmp (element_name, "pay"))
			{
			Payee *entry = da_payee_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_printf(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "key"  )) { entry->key = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags")) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "name" )) { entry->name = g_strdup(attribute_values[i]); }
				}

				//all attribute loaded: append
				GLOBALS->pay_list = g_list_append(GLOBALS->pay_list, entry);

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
			Category *entry = da_category_malloc();
			gboolean budget;

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_printf(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

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
				GLOBALS->cat_list = g_list_append(GLOBALS->cat_list, entry);

			}
		}
		break;

		case 'f':
		{
			if(!strcmp (element_name, "fav"))
			{
			Archive *entry = da_archive_malloc();

				for (i = 0; attribute_names[i] != NULL; i++)
				{
					DB( g_printf(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "account"    )) { entry->account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "dst_account")) { entry->dst_account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "payee"      )) { entry->payee = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "category"   )) { entry->category = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->wording = g_strdup(attribute_values[i]); }
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
					DB( g_printf(" att=%s val=%s\n", attribute_names[i], attribute_values[i]) );

					     if(!strcmp (attribute_names[i], "date"       )) { entry->date = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
					else if(!strcmp (attribute_names[i], "account"    )) { entry->account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "dst_account")) { entry->dst_account = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "payee"      )) { entry->payee = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "category"   )) { entry->category = atoi(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->wording = g_strdup(attribute_values[i]); }
					else if(!strcmp (attribute_names[i], "info"       )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != "") entry->info = g_strdup(attribute_values[i]); }

				}

				//all attribute loaded: append
				GLOBALS->ope_list = g_list_append(GLOBALS->ope_list, entry);

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

	//DB( g_printf(" eeh: %s\n", element_name) );


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
void homebank_load_xml(gchar *filename)
{
gchar *buffer;
gsize length;
GError *error = NULL;
ParseContext ctx = { 0 };
GMarkupParseContext *context;
gboolean rc;

	if (!g_file_get_contents (filename, &buffer, &length, &error))
	{
		//g_message ("%s", error->message);
		g_error_free (error);
	}
	else
	{
		/*
		ctx.state = STATE_START;
		ctx.self = self;
		ctx.current = NULL;
		ctx.merge_id = gtk_ui_manager_new_merge_id (self);
		*/

		// ctx is user_data
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
	}

}




/*
** XML properties save
*/
void homebank_prop_save_xml(FILE *fp)
{
gchar *tmpstr;

	tmpstr = g_markup_printf_escaped("<properties title=\"%s\" car_category=\"%d\" auto_nbdays=\"%d\"/>\n",
		GLOBALS->title,
		GLOBALS->car_category,
		GLOBALS->auto_nbdays
	);

	fprintf(fp, tmpstr);
	g_free(tmpstr);

}


/*
** XML account save
*/
void homebank_acc_save_xml(FILE *fp)
{
GList *list;
char buf1[G_ASCII_DTOSTR_BUF_SIZE];
char buf2[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	//we must use this, as fprintf use locale decimal settings and not '.'
	//char buf[G_ASCII_DTOSTR_BUF_SIZE];
	//fprintf (out, "value=%s\n", g_ascii_dtostr (buf, sizeof (buf), value));

	list = g_list_first(GLOBALS->acc_list);
	while (list != NULL)
	{
	Account *item = list->data;

		item->flags &= (AF_BUDGET|AF_CLOSED);

		tmpstr = g_markup_printf_escaped("<account key=\"%d\" flags=\"%d\" name=\"%s\"\n"
			" number=\"%s\" bankname=\"%s\" initial=\"%s\" minimum=\"%s\"\n"
			" cheque1=\"%d\" cheque2=\"%d\"/>\n",
			item->key,
			item->flags,
			item->name == NULL ? "" : item->name,
			item->number == NULL ? "" : item->number,
			item->bankname == NULL ? "" : item->bankname,
			g_ascii_dtostr (buf1, sizeof (buf1), item->initial),
			g_ascii_dtostr (buf2, sizeof (buf2), item->minimum),
			item->cheque1,
			item->cheque2
		);

		fprintf(fp, tmpstr);
		g_free(tmpstr);
	
		list = g_list_next(list);
	}
}

/*
** XML payee save
*/
void homebank_pay_save_xml(FILE *fp)
{
GList *list;
gchar *tmpstr;

	list = g_list_first(GLOBALS->pay_list);
	while (list != NULL)
	{
	Payee *item = list->data;

		if(item->key != 0)
		{
			tmpstr = g_markup_printf_escaped("<pay key=\"%d\" flags=\"%d\" name=\"%s\"/>\n",
				item->key,
				item->flags,
				item->name
			);
			
			fprintf(fp, tmpstr);
			g_free(tmpstr);

		}
		list = g_list_next(list);
	}
}

/*
** XML category save
*/
void homebank_cat_save_xml(FILE *fp)
{
GList *list;
guint i;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	list = g_list_first(GLOBALS->cat_list);
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

			fprintf(fp, tmpstr);
			g_free(tmpstr);

			for(i=0;i<=12;i++)
			{
				if(item->budget[i] != 0)
					fprintf(fp, " b%d=\"%s\"", i, g_ascii_dtostr (buf, sizeof (buf), item->budget[i]));
			}

			fprintf(fp, "/>\n");

		}
		list = g_list_next(list);
	}
}

/*
** XML archive save
*/
void homebank_arc_save_xml(FILE *fp)
{
GList *list;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = list->data;

		fprintf(fp, "<fav amount=\"%s\" account=\"%d\" dst_account=\"%d\"\n",
			g_ascii_dtostr (buf, sizeof (buf), item->amount),
			item->account,
			item->dst_account
		);

		fprintf(fp, " paymode=\"%d\" flags=\"%d\" payee=\"%d\" category=\"%d\"\n",
			item->paymode,
			item->flags,
			item->payee,
			item->category
		);

		tmpstr = g_markup_printf_escaped(" wording=\"%s\" nextdate=\"%d\" every=\"%d\" unit=\"%d\" limit=\"%d\"/>\n",
			item->wording == NULL ? "" : item->wording,
			item->nextdate,
			item->every,
			item->unit,
			item->limit
		);

		fprintf(fp, tmpstr);
		g_free(tmpstr);

		list = g_list_next(list);
	}
}

/*
** XML operation save
*/
void homebank_ope_save_xml(FILE *fp)
{
GList *list;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *tmpstr;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Operation *item = list->data;

		item->flags &= (OF_VALID|OF_INCOME|OF_REMIND);

		tmpstr = g_markup_printf_escaped(
			"<ope date=\"%d\" amount=\"%s\" account=\"%d\" dst_account=\"%d\" paymode=\"%d\" flags=\"%d\" payee=\"%d\" category=\"%d\" wording=\"%s\" info=\"%s\"/>\n",
			item->date,
			g_ascii_dtostr (buf, sizeof (buf), item->amount),
			item->account,
			item->dst_account,
			item->paymode,
			item->flags,
			item->payee,
			item->category,
			item->wording == NULL ? "" : item->wording,
			item->info == NULL ? "" : item->info
		);

		fprintf(fp, tmpstr);
		g_free(tmpstr);

		list = g_list_next(list);
	}
}

/*
** XML save homebank file: wallet
*/
void homebank_save_xml(gchar *filename)
{
FILE *fp;

	if ((fp = fopen(filename, "w")) == NULL)
	{
		g_message("file error on: %s", filename);
	}
	else
	{
		fputs("<?xml version=\"1.0\"?>\n", fp);
		fputs("<homebank v=\"" FILE_VERSION "\">\n", fp);

		homebank_prop_save_xml(fp);
		homebank_acc_save_xml(fp);
		homebank_pay_save_xml(fp);
		homebank_cat_save_xml(fp);
		homebank_arc_save_xml(fp);
		homebank_ope_save_xml(fp);

		fputs("</homebank>\n", fp);
		fclose(fp);
	}
}
