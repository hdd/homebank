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

#include "widgets.h"

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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


enum
{
	LST_PAYMODE_PIXBUF,
	LST_PAYMODE_LABEL,
	NUM_LST_PAYMODE
};

GdkPixbuf *paymode_icons[NUM_PAYMODE_MAX];

char *paymode_pixbuf_names[NUM_PAYMODE_MAX] =
{
	"0none.svg",
	"creditcard.svg",
	"cheque.svg",
	"cash.svg" ,
	"banktransfert.svg",
	"personaltransfert.svg"
};

char *paymode_label_names[NUM_PAYMODE_MAX] =
{
	N_("(none)"),
	N_("Credit card"),
	N_("Cheque"),
	N_("Cash"),
	N_("Bank transfer"),
	N_("Internal transfer")

/*	"none", "credit card", "standing order", "cheque", "withdrawal of cash", "transfer", "internal transfer",
	"deposit of cheque", "deposit of cash" */
};

/*
	facture cb / credit card
	prelevement	/ standing order
	cheque / cheque
	retrait espece / withdrawal of cash
	virement / transfer
	virement compte / internal transfer
	dépôt chéques / deposit of cheque
	dépôt espece / deposit of cash
	autre
*/

void load_paymode_icons(void)
{
//GError        *error = NULL;
gchar *pathfilename;
guint i;

	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		pathfilename = g_build_filename(homebank_app_get_pixmaps_dir(), paymode_pixbuf_names[i], NULL);

		DB( g_print("loading %s\n", pathfilename) );

		paymode_icons[i] = gdk_pixbuf_new_from_file_at_size(pathfilename, 22, 22, NULL);
		g_free (pathfilename);
	}
}


void free_paymode_icons(void)
{
guint i;

	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		if(paymode_icons[i] != NULL)
			g_object_unref(paymode_icons[i]);
	}
}

void
gimp_label_set_attributes (GtkLabel *label,
                           ...)
{
  PangoAttribute *attr  = NULL;
  PangoAttrList  *attrs;
  va_list         args;

  //g_return_if_fail (GTK_IS_LABEL (label));

  attrs = pango_attr_list_new ();

  va_start (args, label);

  do
    {
      PangoAttrType   attr_type = va_arg (args, PangoAttrType);

      switch (attr_type)
        {
        case PANGO_ATTR_LANGUAGE:
          attr = pango_attr_language_new (va_arg (args, PangoLanguage *));
          break;

        case PANGO_ATTR_FAMILY:
          attr = pango_attr_family_new (va_arg (args, const gchar *));
          break;

        case PANGO_ATTR_STYLE:
          attr = pango_attr_style_new (va_arg (args, PangoStyle));
          break;

        case PANGO_ATTR_WEIGHT:
          attr = pango_attr_weight_new (va_arg (args, PangoWeight));
          break;

        case PANGO_ATTR_VARIANT:
          attr = pango_attr_variant_new (va_arg (args, PangoVariant));
          break;

        case PANGO_ATTR_STRETCH:
          attr = pango_attr_stretch_new (va_arg (args, PangoStretch));
          break;

        case PANGO_ATTR_SIZE:
          attr = pango_attr_size_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_FONT_DESC:
          attr = pango_attr_font_desc_new (va_arg (args,
                                                   const PangoFontDescription *));
          break;

        case PANGO_ATTR_FOREGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_foreground_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_BACKGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_background_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_UNDERLINE:
          attr = pango_attr_underline_new (va_arg (args, PangoUnderline));
          break;

        case PANGO_ATTR_STRIKETHROUGH:
          attr = pango_attr_underline_new (va_arg (args, gboolean));
          break;

        case PANGO_ATTR_RISE:
          attr = pango_attr_rise_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_SCALE:
          attr = pango_attr_scale_new (va_arg (args, gdouble));
          break;

        default:
          g_warning ("%s: invalid PangoAttribute type %d",
                     G_STRFUNC, attr_type);
        case -1:
        case PANGO_ATTR_INVALID:
          attr = NULL;
          break;
        }

      if (attr)
        {
          attr->start_index = 0;
          attr->end_index   = -1;
          pango_attr_list_insert (attrs, attr);
        }
    }
  while (attr);

  va_end (args);

  gtk_label_set_attributes (label, attrs);
  pango_attr_list_unref (attrs);
}

/*
**
*/
GtkWidget *make_label(char *str, gfloat xalign, gfloat yalign)
{
GtkWidget *label;

	label = gtk_label_new_with_mnemonic (str);
	gtk_misc_set_alignment (GTK_MISC (label), xalign, yalign);

	return label;
}

/*
**
*/
GtkWidget *make_text(gfloat xalign)
{
GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE(entry), FALSE);
	g_object_set(entry, "xalign", xalign, NULL);

	//entry = gtk_label_new(NULL);
	//gtk_misc_set_padding (entry, 4, 2);
	//gtk_misc_set_alignment(entry, xalign, 0.5);
	return entry;
}

/*
**
*/
GtkWidget *make_string(GtkWidget *label)
{
GtkWidget *entry;

	entry = gtk_entry_new ();

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);

	return entry;
}

/*
**
*/
GtkWidget *make_memo_entry(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *entry;
GtkEntryCompletion *completion;
GList *list;

	store = gtk_list_store_new (1, G_TYPE_STRING);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
    gtk_entry_completion_set_text_column (completion, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_completion (GTK_ENTRY (entry), completion);

	g_object_unref(store);

	//populate
	//gtk_list_store_clear (GTK_LIST_STORE(store));

	list = g_hash_table_get_keys(GLOBALS->h_memo);
	while (list != NULL)
	{
	GtkTreeIter  iter;

		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (GTK_LIST_STORE(store), &iter, 0, list->data, -1);

		list = g_list_next(list);
	}

	g_list_free(list);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);

	return entry;
}




/*
**
*/
GtkWidget *make_string_maxlength(GtkWidget *label, guint max_length)
{
GtkWidget *entry;

	entry = make_string(label);
	gtk_entry_set_max_length(GTK_ENTRY(entry), max_length);

	return entry;
}
/*
**
*/
GtkWidget *make_amount(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -G_MAXDOUBLE, G_MAXDOUBLE, 0.1, 1.0, 0.0);
	spinner = gtk_spin_button_new (adj, 1.0, 2);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}


GtkWidget *make_euro(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -G_MAXDOUBLE, G_MAXDOUBLE, 0.1, 1.0, 0.0);
	spinner = gtk_spin_button_new (adj, 1.0, 6);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}

/*
**
*/
GtkWidget *make_numeric(GtkWidget *label, gdouble min, gdouble max)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, min, max, 1.0, 10.0, 0.0);
	spinner = gtk_spin_button_new (adj, 0, 0);
	//gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}

/*
**
*/
GtkWidget *make_long(GtkWidget *label)
{
GtkWidget *spinner;

	spinner = make_numeric(label, 0.0, G_MAXINT);
	return spinner;
}

/*
**
*/
GtkWidget *make_year(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 1.0, 3000, 1.0, 10.0, 0.0);
	spinner = gtk_spin_button_new (adj, 0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}


/*
**
*/
GtkWidget *make_cycle(GtkWidget *label, gchar **items)
{
GtkWidget *combobox;
guint i;

	combobox = gtk_combo_box_new_text();

	for (i = 0; items[i] != NULL; i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), _(items[i]));
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}

/*
**
*/
GtkWidget *make_radio(GtkWidget *label, gchar **items)
{
GtkWidget *box, *button;
//GSList *group;
guint i;

	box = gtk_hbox_new (FALSE, 0);

    button = gtk_radio_button_new_with_label (NULL, _(items[0]));
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
	for (i = 1; items[i] != NULL; i++)
	{
		button = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (button), _(items[i]));
	    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
	}
	return box;
}


/*
**
*/
guint make_poparchive_populate(GtkComboBox *combobox, GList *srclist)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;

	//insert all glist item into treeview
	model  = gtk_combo_box_get_model(combobox);
	gtk_list_store_clear(GTK_LIST_STORE(model));

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, "----", -1);

	i=0; list = g_list_first(srclist);
	while (list != NULL)
	{
	Archive *entry = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, entry->wording, -1);

		//DB( g_printf(" populate_treeview: %d %08x\n", i, list->data) );

		i++; list = g_list_next(list);
	}

	return i;
}


GtkWidget *make_poparchive(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *combobox;
GtkCellRenderer    *renderer;

	//store
	store = gtk_list_store_new (1, G_TYPE_STRING);
	combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL(store));
	g_object_unref(store);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}














/*
**
*/
/* for catgeory */
/*
guint make_popcategory_populate(GtkComboBox *combobox, GList *srclist)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint i;
gchar *category_str;

	//insert all glist item into treeview
	model  = gtk_combo_box_get_model(combobox);
	gtk_list_store_clear(GTK_LIST_STORE(model));
	i=0; list = g_list_first(srclist);
	while (list != NULL)
	{
	Category *entry = list->data;
	gchar *parent;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);

		if(entry->flags & GF_SUB)
		{
			//insert subcategory
			category_str = g_strdup_printf("+ %s", entry->name);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, category_str, -1);
			g_free(category_str);
		}
		else
		{
			//insert category
			parent = entry->name;
			gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, (entry->name==NULL ? (gchar *)_("(none)") : (gchar*)entry->name), -1);
		}

		//DB( g_printf(" populate_treeview: %d %08x\n", i, list->data) );

		i++; list = g_list_next(list);
	}

	return i;
}


GtkWidget *make_popcategory(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *combobox;
GtkCellRenderer    *renderer;

	//store
	store = gtk_list_store_new (1, G_TYPE_STRING);
	combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL(store));
	g_object_unref(store);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}
*/

/*
** Make a paymode combobox widget
*/
GtkWidget *make_paymode(GtkWidget *label)
{
GtkListStore  *store;
GtkTreeIter    iter;
GtkWidget *combobox;
GtkCellRenderer    *renderer;
guint i;

	//store
	store = gtk_list_store_new (
		NUM_LST_PAYMODE,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING
		);

	//combobox
	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	//column 1
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "pixbuf", LST_PAYMODE_PIXBUF);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "text", LST_PAYMODE_LABEL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	//populate our combobox model
	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			LST_PAYMODE_PIXBUF, paymode_icons[i],
			LST_PAYMODE_LABEL, _(paymode_label_names[i]),
			-1);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}
