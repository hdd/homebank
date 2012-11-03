/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2011 Maxime DOYEN
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

void ui_gtk_entry_set_text(GtkWidget *widget, gchar *text)
{
	DB( g_print(" set text to '%s'\n", text) );
	
	if( text != NULL )
		gtk_entry_set_text(GTK_ENTRY(widget), text);
	else
		gtk_entry_set_text(GTK_ENTRY(widget), "");
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
	//gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
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
GtkWidget *make_scale(GtkWidget *label)
{
GtkWidget *scale;

	scale = gtk_hscale_new_with_range(GTK_CHART_MINBARW, GTK_CHART_MAXBARW, 1.0);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	gtk_range_set_value(GTK_RANGE(scale), GTK_CHART_BARW);
	
	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), scale);

	return scale;
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

	box = gtk_vbox_new (FALSE, 0);

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

	i=0;
	list = g_list_first(srclist);
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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * free_combobox_icons:
 *
 * generic function to free combobox icons
 *
 */
static void free_combobox_icons(GdkPixbuf **storage, gint max)
{
guint i;

	for(i=0;i<max;i++)
	{
		if(storage[i] != NULL)
			g_object_unref(storage[i]);
	}
}

/**
 * load_combobox_icons:
 *
 * generic function to load combobox icons
 *
 */
static void load_combobox_icons(gchar **filenames, GdkPixbuf **storage, gint max)
{
//GError        *error = NULL;
GtkWidget *cellview;
guint i;

	cellview = gtk_cell_view_new ();
	
	for(i=0;i<max;i++)
	{
		storage[i] = gtk_widget_render_icon (cellview, filenames[i], GTK_ICON_SIZE_BUTTON, NULL);
		#if MYDEBUG == 1
		if( !storage[i] )
			g_print("cannot found a private stock %s\n", filenames[i]);
		#endif
	}

	gtk_widget_destroy (cellview);
}

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
	"pm-none",
	"pm-ccard",
	"pm-check",
	"pm-cash" ,
	"pm-transfer",
	"pm-intransfer",
	"pm-dcard",
	"pm-standingorder",
	"pm-epayment",
	"pm-deposit",
	"pm-fifee"
};

char *paymode_label_names[NUM_PAYMODE_MAX] =
{
	N_("(none)"),
	N_("Credit card"),
	N_("Check"),
	N_("Cash"),
	N_("Transfer"),
	N_("Internal transfer"),
	N_("Debit card"),
	N_("Standing order"),
	N_("Electronic payment"),
	N_("Deposit"),
	N_("FI fee")
};

/*
	http://www.direct.gov.uk/en/MoneyTaxAndBenefits/ManagingMoney/BankAccountsAndBankingProducts/DG_10035158

0	aucun              / none
1	facture cb         / credit card
2	cheque             / check
3   retrait espece     / withdrawal of cash
4	virement           / transfer
5	virement compte    / internal transfer
6	(carte de paiement / debit card) 
7	prelevement	       / standing order/repeating payment
8	télépaiement       / electronic payment
9	dépôt              / deposit
10	frais bancaires    / FI fee
	autre
		
*/




void load_paymode_icons(void)
{
	load_combobox_icons(paymode_pixbuf_names, paymode_icons, NUM_PAYMODE_MAX);
}


void free_paymode_icons(void)
{
	free_combobox_icons(paymode_icons, NUM_PAYMODE_MAX);
}


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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#define NUM_NAINEX_MAX 3

enum
{
	LST_NAINEX_PIXBUF,
	LST_NAINEX_LABEL,
	NUM_LST_NAINEX
};

GdkPixbuf *nainex_icons[NUM_NAINEX_MAX];

char *nainex_pixbuf_names[NUM_NAINEX_MAX] =
{
	"flt-inactive",
	"flt-include",
	"flt-exclude",
};

char *nainex_label_names[NUM_NAINEX_MAX] =
{
	N_("Inactive"),
	N_("Include"),
	N_("Exclude")
};

void load_nainex_icons(void)
{
	load_combobox_icons(nainex_pixbuf_names, nainex_icons, NUM_NAINEX_MAX);
}


void free_nainex_icons(void)
{
	free_combobox_icons(nainex_icons, NUM_NAINEX_MAX);
}


/*
** Make a nainex combobox widget
*/
GtkWidget *make_nainex(GtkWidget *label)
{
GtkListStore  *store;
GtkTreeIter    iter;
GtkWidget *combobox;
GtkCellRenderer    *renderer;
guint i;

	//store
	store = gtk_list_store_new (
		NUM_LST_NAINEX,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING
		);

	//combobox
	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	//column 1
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "pixbuf", LST_NAINEX_PIXBUF);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "text", LST_NAINEX_LABEL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	//populate our combobox model
	for(i=0;i<NUM_NAINEX_MAX;i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			LST_NAINEX_PIXBUF, nainex_icons[i],
			LST_NAINEX_LABEL, _(nainex_label_names[i]),
			-1);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}

