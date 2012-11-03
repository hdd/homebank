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

#include "wiz_import.h"
#include "list_operation.h"

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

struct wizimport_data
{
	GtkWidget	*window;

	GtkWidget	*filechooser;
	GtkWidget	*notebook;
	GtkWidget	*user_info;

	GtkWidget	*PO_acc;
	GtkWidget	*NB_decay;
	
	GtkWidget	*imported_ope;
	GtkWidget	*duplicat_ope;
	
	GtkWidget	*last_info;
	
	GtkWidget	*BT_cancel, *BT_backward, *BT_forward, *BT_apply, *BT_close;

	gchar		*filename;
	guint		filetype;

	guint		imported;
	guint		total;

	gboolean	valid;
	guint		step;
	guint		maxstep;

	GList		*ope_imp_list;
	
	//struct	Base base;
	//UBYTE	tmppass[12];
	//BOOL	check;
	//ULONG	change;
};


void wizimport_update(GtkWidget *widget, gpointer user_data);


/* ----------------- */
//todo move that somewhere else

/*
** try to determine the file type (if supported for import by homebank
**
**
*/
gint homebank_alienfile_recognize(gchar *filename)
{
GIOChannel *io;
gint i, retval = FILETYPE_UNKNOW;
gchar *tmpstr;
gint io_stat;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};
					
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		for(i=0;i<1;i++)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != "")
				{
					DB( g_print(" line %4d: %s\n", i, tmpstr) );

					/* is it OFX ? */
					if( g_strstr_len(tmpstr, 10, "OFX") != NULL)
					{
						DB( g_print(" type is OFX\n") );
						retval = FILETYPE_OFX;
					}
					/* is it csv homebank ? */
					else
					{
					gboolean hbcsv;
					
						hbcsv = hb_string_csv_valid(tmpstr, 7, csvtype);
						
						DB( g_print(" hbcsv %d\n", hbcsv) );
						
						if( hbcsv == TRUE )
						{
							DB( g_print(" type is CSV homebank\n") );
							retval = FILETYPE_CSV_HB;
						}
						

					}
	
					g_free(tmpstr);
				}
			}

		}
		g_io_channel_unref (io);
	}

	return retval;
}

GList *homebank_csv_import(gchar *filename, guint accnum)
{
GIOChannel *io;
GList *list = NULL;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};

	DB( g_print("(import) homebank csv\n") );


	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr, *txt;
	gint io_stat;
	gboolean valid;
	gint count = 0;
	gint error = 0;

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != "")
				{
				gchar **str_array;
				Operation *newope = da_operation_malloc();

					hb_string_strip_crlf(tmpstr);

					/* control validity here */
					valid = hb_string_csv_valid(tmpstr, 7, csvtype);
				
					 DB( g_print("valid %d, '%s'\n", valid, tmpstr) );
				
					if( !valid )
					{
						error++;
					}
					else
					{
						count++;
					
						str_array = g_strsplit (tmpstr, ";", 7);
						// 0:date; 1:paymode; 2:info; 3:payee, 4:wording; 5:amount; 6:category

						newope->date		= hb_date_get_julian_parse(str_array[0]);
						newope->paymode     = atoi(str_array[1]);
						newope->info        = g_strdup(str_array[2]);
						newope->payee       = defpayee_glist_exists(GLOBALS->pay_list, str_array[3]);
						newope->wording     = g_strdup(str_array[4]);
						newope->amount      = g_ascii_strtod(str_array[5],NULL);
						newope->category    = defcategory_glist_exists(GLOBALS->cat_list, str_array[6]);
						
						
						newope->account     = accnum;
						newope->dst_account = accnum;

						newope->flags |= OF_ADDED;

						if( newope->amount > 0)
							newope->flags |= OF_INCOME;

						DB( g_print(" storing %s : %s : %s :%s : %s : %s : %s\n", 
							str_array[0], str_array[1], str_array[2],
							str_array[3], str_array[4], str_array[5],
							str_array[6], str_array[7]
							) );

						list = g_list_append(list, newope);

						g_strfreev (str_array);
					}
				}
				g_free(tmpstr);
			}

		}
		g_io_channel_unref (io);

	/*
		homebank_message_dialog(data->window, error > 0 ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
			_("Operation CSV import result"),
			_("%d operation(s) inserted\n%d error(s) in the file"),
			count, error);
		*/ 
	}


	return list;
}

/* ----------------- */
void        wizimport_selchange                  (GtkWidget *widget,
                                            gpointer        user_data)
{

struct wizimport_data *data;

	DB( g_print("(wizimport) selchange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	
	if(data->filename) 
		g_free( data->filename );
	data->filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(data->filechooser));
	DB( g_print(" filename -> %s\n", data->filename) );

	data->valid = FALSE;	
	if( data->filename != NULL )
	{
		data->filetype = homebank_alienfile_recognize(data->filename);
		switch(data->filetype)
		{
			case FILETYPE_UNKNOW:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("Unknow/Invalid file..."));
				break;
			case FILETYPE_OFX:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("OFX file recognized !"));
				break;
			case FILETYPE_CSV_HB:
				gtk_label_set_text(GTK_LABEL(data->user_info), _("CSV operation file recognized !"));
				data->valid = TRUE;
				break;
		
		
		}
	}

	wizimport_update(widget, NULL);

                                            
}

/*
 * find duplicate operations
 *
 *
 */

void wizimport_find_duplicate_operations(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
GtkWidget *view;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *tmplist, *implist;
Operation *item;
guint32 mindate;
guint decay, accnum;

	DB( g_print("(wizimport) find duplicate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* 1: get import min bound date */
	tmplist = g_list_first(data->ope_imp_list);
	item = tmplist->data;
	mindate = item->date;
	decay = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_decay));

	accnum = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));

	tmplist = g_list_first(GLOBALS->ope_list);
	while (tmplist != NULL)
	{
	Operation *ope = tmplist->data;

		if( ope->account == accnum && ope->date >= mindate )
		{
			DB( g_print("should check here %d: %s\n", ope->date, ope->wording) );

			implist = g_list_first(data->ope_imp_list);
			while (implist != NULL)
			{
			Operation *impope = implist->data;

				if( (impope->amount == ope->amount) && (ope->date <= impope->date+decay) && (ope->date >= impope->date-decay) )
				{
					DB( g_print(" found %d: %s\n", impope->date, impope->wording) );

					impope->same = g_list_append(impope->same, ope);
				}

				implist = g_list_next(implist);
			}
		}

		tmplist = g_list_next(tmplist);
	}


}


void wizimport_fill_imp_operations(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
GtkWidget *view;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *tmplist;

	DB( g_print("(wizimport) fill\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	view = data->imported_ope;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	tmplist = g_list_first(data->ope_imp_list);
	while (tmplist != NULL)
	{
	Operation *item = tmplist->data;

		/* append to our treeview */
    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

    		//g_print(" populate: %s\n", ope->ope_Word);

    		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, item,
				LST_OPE_IMPTOGGLE, item->same == NULL ? TRUE : FALSE,
				-1);

		DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, item->same) );

		tmplist = g_list_next(tmplist);
	}

  gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */

  g_object_unref(model);



}

void wizimport_fillsame(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model, *newmodel;
GtkTreeIter			 iter, newiter;
GList *tmplist;
GtkWidget *view;


	DB( g_print("(wizimport) fillsame\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	view = data->duplicat_ope;
	newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(newmodel));



	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Operation *item = tmplist->data;
	
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &item, -1);

		if( item->same != NULL )
		{
			tmplist = g_list_first(item->same);
			while (tmplist != NULL)
			{
			Operation *tmp = tmplist->data;

				/* append to our treeview */
		    	gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

	    		gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
					LST_DSPOPE_DATAS, tmp,
					-1);

				DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, item->same) );

				tmplist = g_list_next(tmplist);
			}
		}

	}


}


/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
gboolean result;

	DB( g_print("(wizimport) cancel\n") );

	//gtk_widget_destroy(GLOBALS->mainwindow);

	g_signal_emit_by_name(window, "delete-event", NULL, &result);
	gtk_widget_destroy(window);

	//return FALSE;
}



/*
 *
 */
void wizimport_close(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
gboolean result;

	DB( g_print("(wizimport) close\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_signal_emit_by_name(window, "delete-event", NULL, &result);
	gtk_widget_destroy(window);


}

/*
 *
 */
void wizimport_forward(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
gint filetype;
guint accnum;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(wizimport) forward :: %d\n", data->step) );

	if( data->step < data->maxstep )
	{
		switch( data->step )
		{
			case 3:
				//ensure the imported operation list is empty
				da_operation_destroy(data->ope_imp_list);
				data->ope_imp_list = NULL;

				accnum = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));

				switch(data->filetype)
				{
					case FILETYPE_OFX:
						//todo
						data->ope_imp_list = NULL;
						break;
					case FILETYPE_CSV_HB:
						data->ope_imp_list = homebank_csv_import(data->filename, accnum);
						break;
				}
								
				//sort by date
				data->ope_imp_list = da_operation_sort(data->ope_imp_list);

				wizimport_find_duplicate_operations(widget, NULL);
				
				wizimport_fill_imp_operations(widget, NULL);
			
				
				break;					
		
			case 4:
			{	gchar *txt;
		
					txt = g_strdup_printf(_("%d operation(s) were imported.\n%d operation(s) were rejected."), data->imported, data->total-data->imported);
		
					gtk_label_set_text(GTK_LABEL(data->last_info), txt);
	
					g_free(txt);
			
				break;
			}
		}

		data->step++;
		gtk_notebook_next_page(GTK_NOTEBOOK(data->notebook));
	}

	wizimport_update(widget, NULL);
}


/*
 *
 */
void wizimport_backward(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(wizimport) backward :: %d\n", data->step) );

		switch( data->step )
		{
			case 3:

				DB( g_print("valid to true\n") );

				data->valid = TRUE;
				break;
				
		}



	if( data->step > 1 )
	{

		data->step--;
		gtk_notebook_prev_page(GTK_NOTEBOOK(data->notebook));


	}


	wizimport_update(widget, NULL);
}

/*
 *
 */
void wizimport_apply(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
guint accnum;

	DB( g_print("(wizimport) apply\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	//accnum = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc));
	
	//all ok, usr want to import
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));

	data->imported = 0;
	data->total = 0;

	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Operation *item;
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DSPOPE_DATAS, &item,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
		{
			operation_add(item, NULL, item->account);
			data->imported++;
		}

		/* Make iter point to the next row in the list store */
		data->total++;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	wizimport_forward(widget, NULL);

	/* todo optimize this */
	if(data->imported > 0)
	{
		GLOBALS->change += data->imported;
		wallet_compute_balances(GLOBALS->mainwindow, NULL);
		wallet_update(GLOBALS->mainwindow, (gpointer)UF_TITLE+UF_SENSITIVE+UF_BALANCE);
	}	

}

/*
 *
 */
void wizimport_update(GtkWidget *widget, gpointer user_data)
{
struct wizimport_data *data;
gboolean sensitive;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(wizimport) update\n") );
	DB( g_print(" -> %d\n", data->step) );

	sensitive = data->step == 1 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->BT_backward, sensitive);

	sensitive = data->step == data->maxstep ? FALSE : TRUE;
	if(data->step==2 && data->valid==FALSE) sensitive = FALSE;
	gtk_widget_set_sensitive(data->BT_forward, sensitive);

	// last user action
	if( data->step == data->maxstep-1)
	{
		gtk_widget_hide(data->BT_forward);
		gtk_widget_show(data->BT_apply);
	}
	// summary (last page)
	else if( data->step == data->maxstep )
	{
		gtk_widget_hide(data->BT_cancel);
		gtk_widget_hide(data->BT_backward);
		gtk_widget_hide(data->BT_forward);
		gtk_widget_hide(data->BT_apply);
		gtk_widget_show(data->BT_close);
	}	
	else
	{
		gtk_widget_show(data->BT_forward);
		gtk_widget_hide(data->BT_apply);
	}

}

/*
**
*/
void wizimport_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	wizimport_fillsame(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


/*
**
*/
gboolean wizimport_cleanup(struct wizimport_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(wizimport) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		DB( g_print(" we always update our glist\n") );

		//wizimport_get(data->ST_owner, NULL);
		GLOBALS->change++;
	}
	
	
	return doupdate;
}

/*
**
*/
void wizimport_setup(struct wizimport_data *data)
{
	DB( g_print("(wizimport) setup\n") );

	data->filename = NULL;
	data->step = 1;
	data->valid = FALSE;
	data->maxstep = gtk_notebook_get_n_pages(GTK_NOTEBOOK(data->notebook));

	data->ope_imp_list = NULL;

#if MYDEBUG==1
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (data->filechooser), "/home/max/dev/homebank/data");
#endif

	//make_popcategory_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->cat_list);

	//wizimport_set(data->ST_owner, NULL);

	make_popaccount_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->acc_list);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), 0);

	wizimport_update(data->window, NULL);

}


/*
**
*/
gboolean wizimport_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct wizimport_data *data = user_data;
gboolean retval = FALSE;

	DB( g_print("(wizimport) dispose\n") );

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_print(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

	g_free( data->filename );

	da_operation_destroy(data->ope_imp_list);

	g_free(user_data);


	//delete-event TRUE abort/FALSE destroy
	return FALSE;
}


// the window creation
GtkWidget *create_wizimport_window (void)
{
struct wizimport_data *data;
GtkWidget *window, *mainvbox;
GtkWidget *hbox, *vbox, *widget, *notebook, *sw,*bbox, *button, *label;
//GdkPixbuf *icon;
GtkFileFilter *filter;

	data = g_malloc0(sizeof(struct wizimport_data));
	if(!data) return NULL;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	/*
      window = gtk_dialog_new_with_buttons ("Wallet creation assistant",
				//GTK_WINDOW (do_widget),
				NULL,
				0,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
				GTK_STOCK_GO_BACK,
				GTK_RESPONSE_REJECT,
				GTK_STOCK_GO_FORWARD,
				GTK_RESPONSE_ACCEPT,
				NULL);
		*/

	data->window = window;


	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print("(wizimport) window=%x, inst_data=%x\n", window, data) );


	//icon = gtk_window_get_icon(GLOBALS->mainwindow);
	//gtk_window_set_icon(window, icon);

	gtk_window_set_title(GTK_WINDOW(window), _("Operations import wizard"));

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

	//notebook
	notebook = gtk_notebook_new();
    gtk_box_pack_start (GTK_BOX (mainvbox), notebook, TRUE, TRUE, HB_BOX_SPACING);
	data->notebook = notebook;

	DB( g_print(" data=%x, notebook=%x\n", &data, notebook) );

    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
    //gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);

	//page 0
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Operation import wizard</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	label = make_label(_(
	
		"The operation import wizard will insert the operations\n" \
		"from a file (csv homebank specific format only supported for now)\n" \
		"into your wallet under your control at each step.\n\n" \
		"Duplicate existing operations will be found and unselected by default\n" \
		"for import and can of course be reselected by the user.\n\n" \
		"(NB: OFX/QFX/QIF format support will come in the next release !)"
	), 0.0, 0.5);
   
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);




	//page 1
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);




	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Select a file to import</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	/*
	widget = gtk_file_chooser_button_new (_("Select a file"),
                                        GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, HB_BOX_SPACING);
	*/

	widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
    data->filechooser = widget;
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, HB_BOX_SPACING);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.csv");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(data->filechooser), filter);

	label = gtk_label_new("");
	data->user_info = label;
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, HB_BOX_SPACING);


	//preview widget
	/*
	label = gtk_label_new("preview");
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(data->filechooser), label);
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(data->filechooser), TRUE);
	*/

	//page 2
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);

	label = make_label(_("1) Select the account to import operation to."), 0.0, 1.0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	label = make_label(_(
	"2) Each operation to be imported will be checked for\n" \
	"an existing duplicate on amount and date criterion.\n\n" \
	"For the date matching, a decay can be set in number of days.\n" \
	"- 0 means an exact match.\n" \
	"- other choice will lead to consider operations as a duplicate\n" \
	"if the date is plus or minus the number of day you select.\n"), 0.0, 1.0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	widget = gtk_hseparator_new();
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, HB_BOX_SPACING);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Import options</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, HB_BOX_SPACING);

	label = make_label(_("A_ccount:"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);
	widget = make_popaccount(label);
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, HB_BOX_SPACING);
	data->PO_acc = widget;

	hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, HB_BOX_SPACING);

	label = make_label(_("Duplicate date decay:"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);
	widget = make_numeric(label, 0.0, 14.0);
	data->NB_decay = widget;
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, HB_BOX_SPACING);
	label = make_label(_("days"), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, HB_BOX_SPACING);



	//page 3
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Operations to import</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	
	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	widget = create_list_import_operation();
	data->imported_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Possible duplicate of selected operations to import</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	widget = create_list_operation();
	data->duplicat_ope = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	//gtk_widget_set_size_request(sw, -1, 50);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);


	//page 4
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);

	label = make_label(NULL, 0.0, 1.0);
	gtk_label_set_markup (GTK_LABEL(label), _("<b>Summary</b>"));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	label = make_label(NULL, 0.0, 1.0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	data->last_info = label;

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, NULL);



	//widget = gtk_hseparator_new();
	//gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, HB_BOX_SPACING);

	//buttons
	bbox = gtk_hbutton_box_new();
	gtk_box_pack_end (GTK_BOX(mainvbox), bbox, FALSE, FALSE, 0);

	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (bbox), HB_BOX_SPACING);

	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	data->BT_cancel = button;

	button = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	data->BT_backward = button;

 	button = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	data->BT_forward = button;

 	button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	data->BT_apply = button;

 	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	data->BT_close = button;

	//connect all our signals
    g_signal_connect (window, "delete-event", G_CALLBACK (wizimport_dispose), (gpointer)data);

    g_signal_connect (G_OBJECT (data->filechooser), "selection-changed", G_CALLBACK (wizimport_selchange), (gpointer)data);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope)), "changed", G_CALLBACK (wizimport_selection), NULL);


    g_signal_connect (G_OBJECT (data->BT_cancel)  , "clicked", G_CALLBACK (destroy), NULL);
    g_signal_connect (G_OBJECT (data->BT_backward), "clicked", G_CALLBACK (wizimport_backward), NULL);
    g_signal_connect (G_OBJECT (data->BT_forward) , "clicked", G_CALLBACK (wizimport_forward) , NULL);
    g_signal_connect (G_OBJECT (data->BT_apply)   , "clicked", G_CALLBACK (wizimport_apply) , NULL);
    g_signal_connect (G_OBJECT (data->BT_close)   , "clicked", G_CALLBACK (wizimport_close) , NULL);


	//setup, init and show window
	wizimport_setup(data);
	//wizimport_update(data.LV_arc, NULL);

	gtk_window_resize(GTK_WINDOW(window), 580, 400);

	gtk_widget_show_all (window);

	gtk_widget_hide(data->BT_apply);
	gtk_widget_hide(data->BT_close);

	//wait for the user
	//gint result = gtk_dialog_run (GTK_DIALOG (window));

	// cleanup and destroy
	//wizimport_cleanup(&data, result);
	//gtk_widget_destroy (window);

	return window;
}
