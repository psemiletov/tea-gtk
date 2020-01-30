/***************************************************************************
                          tea_tree_view.c  -  description
                             -------------------
    begin                : 2005, 2006
    dev. time            : 2005-2013 by Peter Semiletov
    email                : tea@list.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <libintl.h>
#include <locale.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "tea_defs.h"
#include "tea_funx.h" // Needed for is_image
#include "tea_tree_view.h"


static GList *tv_temp_list = NULL;
static GList *tv_temp_list_img = NULL;

static void tv_simple_cb (GtkTreeModel *model,
                          GtkTreePath *path,
                          GtkTreeIter *iter,
                          gpointer data)
{
  gchar *name;
  gtk_tree_model_get (model, iter, COL_TEXT, &name, -1);
  tv_temp_list = g_list_prepend (tv_temp_list, name);
}


GList* tv_get_selected_multiply (GtkTreeView *tree_view)
{
  if (tv_temp_list)
     g_list_free (tv_temp_list);

  tv_temp_list = NULL;

  GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

  if (gtk_tree_selection_get_mode (sl) != GTK_SELECTION_MULTIPLE)
     return NULL;

  gtk_tree_selection_selected_foreach (sl, tv_simple_cb, NULL);

  return g_list_reverse (tv_temp_list);
}


static void tv_add_new_item (GtkTreeView *tree_view, gchar *s)
{
  if (! tree_view || ! s)
     return;

  GtkTreeModel *model;
  GtkTreeIter newrow;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
  gtk_list_store_append (GTK_LIST_STORE (model), &newrow);
  gtk_list_store_set (GTK_LIST_STORE (model), &newrow, COL_TEXT, s, -1);
}


void tv_fill_with_glist (GtkTreeView *tree_view, GList *l)
{
  if (! tree_view || !l)
     return;

  GtkListStore *liststore = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  if (liststore)
     gtk_list_store_clear (liststore);

  GList *t = g_list_first (l);
  while (t)
       {
        tv_add_new_item (tree_view, t->data);
        t = g_list_next (t);
       }
}


gchar* tv_get_selected_single (GtkTreeView *tree_view)
{
  if (! tree_view)
     return NULL;

  GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  if (! sl)
     return NULL;

  if (gtk_tree_selection_get_mode (sl) != GTK_SELECTION_SINGLE)
     return NULL;

  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name = NULL;

  if (gtk_tree_selection_get_selected (sl, &model, &iter))
      gtk_tree_model_get (model, &iter, COL_TEXT, &name, -1);

  return name;
}


GtkTreeView* tv_create (GtkSelectionMode mode)
{
  GtkTreeView *tree_view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *col;
  GtkTreeSelection *sel;
  GtkListStore *list_store;

  list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  renderer = gtk_cell_renderer_text_new();

  col = gtk_tree_view_column_new();

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_TEXT);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), col);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_selection_set_mode (sel, mode);
  gtk_tree_view_set_headers_visible (tree_view, FALSE);

  return tree_view;
}


GtkTreeView* tv_create_framed (GtkContainer *c, const gchar *caption, GtkSelectionMode mode)
{
  GtkWidget *wnd = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (wnd);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *f = gtk_frame_new (caption);
  gtk_widget_show (f);

  GtkTreeView *tv = tv_create (mode);
  gtk_widget_show (tv);

  gtk_container_add (c, f);

  //gtk_scrolled_window_add_with_viewport (wnd, tv);
  gtk_container_add (wnd, tv);

  gtk_container_add (f, wnd);

  gtk_tree_view_set_vadjustment (tv, gtk_scrolled_window_get_vadjustment (wnd));
  gtk_tree_view_set_hadjustment (tv, gtk_scrolled_window_get_hadjustment (wnd));

  return tv;
}


static gboolean cb_listitems (GtkTreeModel *model,
                              GtkTreePath *path,
                              GtkTreeIter *iter,
                              gpointer data)
{
  gchar *name;
  gtk_tree_model_get (model, iter, COL_TEXT, &name, -1);
  tv_temp_list = g_list_prepend (tv_temp_list, name);

  return FALSE;
}


GList* tv_get_list_data (GtkTreeView *tree_view)
{
  if (tv_temp_list)
     g_list_free (tv_temp_list);

  tv_temp_list = NULL;

  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
  gtk_tree_model_foreach (GTK_TREE_MODEL (model), cb_listitems, NULL);

  return g_list_reverse (tv_temp_list);
}


void tv_add_new_item_img (GtkTreeView *tree_view, const gchar *filename, GdkPixbuf *pixbuf)
{
  if (! tree_view || ! filename || ! pixbuf)
     return;

  GtkTreeModel *model;
  GtkTreeIter newrow;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
  gtk_list_store_append (GTK_LIST_STORE (model), &newrow);
  gtk_list_store_set (GTK_LIST_STORE (model), &newrow, COL_IMG_CELL, pixbuf, COL_TEXT_CELL, filename, -1);
}


void tv_fill_with_glist_img (GtkTreeView *tree_view, GList *l)
{
  if (! tree_view || ! l)
     return;

  GtkListStore *liststore = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  if (liststore)
     gtk_list_store_clear (liststore);

  GList *t = g_list_first (l);
  while (t)
        {
         GdkPixbuf *p = NULL;
         if (is_image (t->data))
            {
             p = gdk_pixbuf_new_from_file_at_size (t->data, 64, 64, NULL);
             tv_add_new_item_img (tree_view, t->data, p);
            }

         t = g_list_next (t);
        }
}


GtkTreeView* tv_create_img (GtkSelectionMode mode)
{
  GtkTreeView *tree_view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *col;
  GtkTreeSelection *sel;
  GtkListStore *list_store;

  list_store = gtk_list_store_new (NUM_COLS_IMG, GDK_TYPE_PIXBUF, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));

  renderer = gtk_cell_renderer_pixbuf_new ();

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "pixbuf", COL_IMG_CELL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), col);

  renderer = gtk_cell_renderer_text_new ();
  col = gtk_tree_view_column_new();

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_TEXT_CELL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), col);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_selection_set_mode (sel, mode);
  gtk_tree_view_set_headers_visible (tree_view, FALSE);

  return tree_view;
}


GtkTreeView* tv_create_framed_img (GtkContainer *c, const gchar *caption, GtkSelectionMode mode)
{
  GtkWidget *wnd = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (wnd);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wnd), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  GtkWidget *f = gtk_frame_new (caption);
  gtk_widget_show (f);

  GtkTreeView *tv = tv_create_img (mode);
  gtk_widget_show (tv);

  gtk_container_add (c, f);

  gtk_container_add (wnd, tv);

  gtk_container_add (f, wnd);

  gtk_tree_view_set_vadjustment (tv, gtk_scrolled_window_get_vadjustment (wnd));
  gtk_tree_view_set_hadjustment (tv, gtk_scrolled_window_get_hadjustment (wnd));

  return tv;
}


GtkTreeView* tv_create_tree (GtkContainer *c, GtkSelectionMode mode)
{
  GtkWidget *wnd = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (wnd);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wnd), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  GtkTreeView *tv = tv_create_img (mode);
  gtk_widget_show (tv);

  gtk_container_add (wnd, tv);

  gtk_container_add (c, wnd);

  gtk_tree_view_set_vadjustment (tv, gtk_scrolled_window_get_vadjustment (wnd));
  gtk_tree_view_set_hadjustment (tv, gtk_scrolled_window_get_hadjustment (wnd));

  return tv;
}


GtkTreeView* tv_create_non_framed (GtkContainer *c, GtkSelectionMode mode)
{
  GtkWidget *wnd = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (wnd);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkTreeView *tv = tv_create (mode);
  gtk_widget_show (tv);
  gtk_container_add (wnd, tv);

  //gtk_container_add (c, wnd);
  
  gtk_box_pack_start (c, wnd, TRUE, TRUE, UI_PACKFACTOR);

  gtk_tree_view_set_vadjustment (tv, gtk_scrolled_window_get_vadjustment (wnd));
  gtk_tree_view_set_hadjustment (tv, gtk_scrolled_window_get_hadjustment (wnd));
  
  return tv;
}


void tv_clear (GtkTreeView *tv)
{
  if (! tv)
     return;

  GtkTreeModel *model = gtk_tree_view_get_model (tv);
  if (! model)
     return;

  GtkTreeIter iter;

  if (! gtk_tree_model_get_iter_first (model, &iter))
     return;

  while (gtk_list_store_remove (model, &iter))
        ;
}


void tv_sel_by_name (GtkTreeView *tree_view, const gchar *name)
{
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  GtkTreeIter iter;
  gboolean valid = gtk_tree_model_get_iter_first (model, &iter);

  while (valid)
        {
         gchar *item_name = NULL;
         gtk_tree_model_get (model, &iter, COL_TEXT, &item_name, -1);
         if (g_utf8_collate (name, item_name) == 0)
            {
             GtkTreePath *path =  gtk_tree_model_get_path (model, &iter);
             if (path)
                {
                 gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
                 gtk_tree_path_free (path);
                }
            }

         valid = gtk_tree_model_iter_next (model, &iter);
        }
}


void model_add_new_item (GtkTreeModel *model, const gchar *s)
{
  if (! model || ! s)
     return;

//is s already in the store? If yes, then return
  GtkTreeIter iter;
  gboolean valid = gtk_tree_model_get_iter_first (model, &iter);

  while (valid)
        {
         gchar *item_name = NULL;
         gtk_tree_model_get (model, &iter, COL_TEXT, &item_name, -1);
         if (g_utf8_collate (s, item_name) == 0)
            return;

         valid = gtk_tree_model_iter_next (model, &iter);
        }
//
  GtkTreeIter newrow;
  gtk_list_store_append (GTK_LIST_STORE (model), &newrow);
  gtk_list_store_set (GTK_LIST_STORE (model), &newrow, COL_TEXT, s, -1);
}



void model_add_new_item_nocheck (GtkTreeModel *model, const gchar *s)
{
  if (! model || ! s)
     return;

  GtkTreeIter newrow;
  gtk_list_store_append (GTK_LIST_STORE (model), &newrow);
  gtk_list_store_set (GTK_LIST_STORE (model), &newrow, COL_TEXT, s, -1);
}



void model_fill_with_glist (GtkTreeModel *model, GList *l)
{
  if (! model || ! l)
     return;

  GList *t = g_list_first (l);
  while (t)
       {
        model_add_new_item_nocheck (model, t->data);
        t = g_list_next (t);
       }
}


void model_fill_from_file (GtkTreeModel *model, const gchar *filename)
{
  GList *l = load_file_to_glist (filename);
  model_fill_with_glist (model, l);
  glist_strings_free (l);
}


GList* model_get_list_data (GtkTreeModel *model)
{
  if (tv_temp_list)
     g_list_free (tv_temp_list);

  tv_temp_list = NULL;

  gtk_tree_model_foreach (GTK_TREE_MODEL (model), cb_listitems, NULL);
  return g_list_reverse (tv_temp_list);
}


void model_save_to_file (GtkTreeModel *model, const gchar *filename)
{
  GList *l = model_get_list_data (model);
  glist_save_to_file (l, filename);
}


static gint button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if (event->button == 1)
     if (event->type == GDK_2BUTTON_PRESS)
        {
         gtk_dialog_response (user_data, GTK_RESPONSE_ACCEPT);
         return TRUE;
        }

  return FALSE;
}


gchar* request_item_from_tvlist (GList *list)
{
  GtkWidget *dialog;

  dialog = gtk_dialog_new_with_buttons (_("Choose"),
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_ACCEPT,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_REJECT,
                                        NULL);

//  GtkTreeView *tv = tv_create_non_framed (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), GTK_SELECTION_SINGLE);
  GtkTreeView *tv = tv_create_non_framed (GTK_CONTAINER (gtk_dialog_get_content_area (dialog)), GTK_SELECTION_SINGLE);

  tv_fill_with_glist (tv, list);

  g_signal_connect (tv, "button_press_event", button_press_event, dialog);

  gtk_widget_show_all (dialog);

  gtk_window_resize (dialog, 384, 512);

  gchar *x = NULL;

  if ((gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT))
      x = tv_get_selected_single (tv);

  gtk_widget_destroy (dialog);

  return x;
}
