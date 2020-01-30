/***************************************************************************
                          tea_gtk_utils.c  -  description
                             -------------------
    begin                : aug. 2004
    copyleft            : 2004-2013 by Peter Semiletov
    email                : tea@list.ru
 ***************************************************************************/
//  Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
//  Copyright (C) 2000, 2002 Chema Celorio, Paolo Maggi
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
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tea_defs.h"
#include "tea_text_document.h" // Needed for t_note_page struct
#include "tea_config.h"
#include "tea_gtk_utils.h"


GtkWidget* tea_text_entry (GtkWidget *container, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_label_set_use_markup (x, TRUE);

  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);
  x = gtk_entry_new ();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (value)
     gtk_entry_set_text (x, value);

  return x;
}


static void cb_select_color (GtkColorButton *widget, gpointer user_data)
{
  GdkColor color;
  gtk_color_button_get_color (widget, &color);
  gchar *t = g_strdup_printf ("#%02x%02x%02x", color.red / 256, color.green / 256, color.blue / 256);
  gtk_entry_set_text (GTK_ENTRY (user_data), t);
  g_free (t);
}


GtkWidget* tea_color_selector (GtkWidget *container, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

  x = gtk_entry_new ();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (value)
     gtk_entry_set_text (x, value);

  GdkColor color;
  gdk_color_parse (value, &color);

  GtkWidget *bt = gtk_color_button_new_with_color (&color);
  gtk_widget_show (bt);
  gtk_box_pack_start (b, bt, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect (bt, "color-set", G_CALLBACK (cb_select_color), x);

  return x;
}


GtkWidget* tea_checkbox (GtkWidget *container, const gchar *caption, gboolean value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkCheckButton *x = gtk_check_button_new_with_label (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);
  gtk_toggle_button_set_active (&x->toggle_button, value);

  return x;
}


GtkWidget* tea_spinbutton (GtkWidget *container, const gchar *caption, gdouble value)
{
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  gtk_box_pack_start (container, b, FALSE, FALSE, UI_PACKFACTOR);


  GtkSpinButton *x = gtk_spin_button_new_with_range (0.0d, G_MAXDOUBLE, 1.0d);
  gtk_spin_button_set_value (x, value);
  
  
   gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

   GtkWidget *l = gtk_label_new (caption);
   gtk_box_pack_start (b, l, FALSE, FALSE, UI_PACKFACTOR);
  
  gtk_widget_show_all (b); 
  
  return x;
}


GtkWidget* tea_scale_widget (GtkWidget *container, const gchar *caption, gdouble min, gdouble max, gdouble value)
{
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);


   GtkWidget *l = gtk_label_new (caption);
   gtk_box_pack_start (b, l, FALSE, FALSE, UI_PACKFACTOR);
  
  
  GtkScale *x = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, min, max, 1.0d);
  gtk_range_set_value (x, value);
   
  
   gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  
  gtk_widget_show_all (b); 
  
  return x;
}


void logmemo_set_pos (GtkTextView *v, guint pos)
{
  GtkTextMark *mark;
  GtkTextIter it;
  GtkTextBuffer *b = gtk_text_view_get_buffer (v);
  gtk_text_buffer_get_iter_at_offset (b, &it, pos);
  gtk_text_buffer_place_cursor (b, &it);
  gtk_text_view_scroll_to_iter (v, &it, 0.0, FALSE, 0.0, 0.0);
}


static void cb_select_dir (GtkWidget *w)
{
  gchar *dir;
  gchar *t;
  GtkEntry *e = w;
  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Select a directory"),
                                                   NULL,
                                                   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  if (g_file_test (gtk_entry_get_text (e), G_FILE_TEST_EXISTS))
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                           gtk_entry_get_text (e));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
     dir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
     gtk_entry_set_text (e, dir);
     g_free (dir);
    }

  gtk_widget_destroy (dialog);
}


GtkWidget* tea_dir_selector (GtkWidget *container, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL); 
  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

  x = gtk_entry_new ();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (value)
     gtk_entry_set_text (x, value);

  GtkWidget *bt = tea_stock_button (GTK_STOCK_OPEN);
  gtk_widget_show (bt);
  gtk_box_pack_start (b, bt, FALSE, FALSE, UI_PACKFACTOR);
  g_signal_connect (bt, "clicked", G_CALLBACK (cb_select_dir), x);

  return x;
}


//returns the combo
GtkWidget* tea_combo_combo (GtkWidget *container, GList *strings, const gchar *caption,
                            gint value, gpointer on_changed, gpointer user_data)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);
  
  x = gtk_combo_box_text_new();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (strings)
    fill_combo_with_glist (x, strings);

  if (value != -1)
     gtk_combo_box_set_active (x, value);

  if (on_changed)
      g_signal_connect (x, "changed", on_changed, user_data);

  return x;
}


//returns the text entry
GtkWidget* tea_combo (GtkWidget *container, GList *strings, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);
  
  x = gtk_combo_box_new_with_entry();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (strings)
    fill_combo_with_glist (x, strings);

  GtkEntry *e = gtk_bin_get_child (x);
  
  if (value)
     gtk_entry_set_text (e, value);

  return e;
}


//from Gedit ::gedit_view.c
// * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
// * Copyright (C) 2000, 2002 Chema Celorio, Paolo Maggi
//mod. by roxton
void widget_set_colors (GtkWidget *w, GdkColor *background, GdkColor *text,
                        GdkColor *selection, GdkColor *sel_text)
{
  if (background)
     gtk_widget_modify_base (GTK_WIDGET (w), GTK_STATE_NORMAL, background);

  if (text)
     gtk_widget_modify_text (GTK_WIDGET (w), GTK_STATE_NORMAL, text);

  if (selection)
    {
     gtk_widget_modify_base (GTK_WIDGET (w), GTK_STATE_SELECTED, selection);
     gtk_widget_modify_base (GTK_WIDGET (w), GTK_STATE_ACTIVE, selection);
    }

  if (sel_text)
    {
     gtk_widget_modify_text (GTK_WIDGET (w), GTK_STATE_SELECTED, sel_text);
     gtk_widget_modify_text (GTK_WIDGET (w), GTK_STATE_ACTIVE, sel_text);
    }
}


gboolean dlg_question (const gchar *caption, const gchar *prompt, const gchar *val)
{
   gboolean result = FALSE;
   GtkWidget *dialog = gtk_dialog_new_with_buttons (caption,
                                                    NULL,
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_STOCK_NO,
                                                    GTK_RESPONSE_REJECT,
                                                    GTK_STOCK_YES,
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);
  gchar *s;
  if (val)
     s = g_strdup_printf (prompt, val);
  else
      s = g_strdup (prompt);

  //GtkWidget *hbox = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  
  GtkWidget *label = gtk_label_new (s);
  gtk_label_set_line_wrap (label, TRUE);
  GtkWidget *im = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_container_add (hbox, im);
  gtk_container_add (hbox, label);
  gtk_widget_show_all (hbox);
  gtk_container_add (gtk_dialog_get_content_area (dialog), hbox);
  
  if (gtk_dialog_run (dialog) == GTK_RESPONSE_ACCEPT)
     result = TRUE;

  gtk_widget_destroy (dialog);
  g_free (s);

  return result;
}


void dlg_info (const gchar *caption, const gchar *prompt, const gchar *val)
{
  GtkWidget *dialog = gtk_dialog_new_with_buttons (caption,
                                                   NULL,
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_STOCK_OK,
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);
  gchar *s;
  if (val)
     s = g_strdup_printf (prompt, val);
  else
      s = g_strdup (prompt);

//  GtkWidget *hbox = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  GtkWidget *label = gtk_label_new (s);
  gtk_label_set_line_wrap (label, TRUE);
  GtkWidget *im = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
  gtk_container_add (hbox, im);
  gtk_container_add (hbox, label);
  gtk_widget_show_all (hbox);
  gtk_container_add (gtk_dialog_get_content_area (dialog), hbox);

  gtk_dialog_run (dialog);
  gtk_widget_destroy (dialog);
  g_free (s);
}


GtkWidget* tea_button_at_box (GtkWidget *container, const gchar *caption, gpointer callback, gpointer user_data)
{
  if (! container)
     return NULL;

  GtkWidget *b = gtk_button_new_with_label (caption);
  gtk_widget_show (b);

  gtk_box_pack_start (GTK_BOX (container),
                      b,
                      FALSE,
                      FALSE,
                      UI_PACKFACTOR);

  if (callback)
      g_signal_connect ((gpointer) b, "clicked", G_CALLBACK (callback), user_data);

  return b;
}


void cb_on_bt_close (gpointer *w)
{
//  window_destroy (w);
  g_print ("cb_on_bt_close\n ");
  gtk_widget_destroy (w);
}


static void find_menuitem_cb (GtkWidget *widget, gpointer data)
{
  if (! widget)
     return;

  gchar *s = gtk_widget_get_name (widget);

  if (g_utf8_collate (data, s) == 0)
     {
      mni_temp = widget;
      return;
     }

  if (GTK_IS_MENU_ITEM (widget))
     find_menuitem (GTK_CONTAINER (gtk_menu_item_get_submenu (widget)), data);
}


GtkWidget* find_menuitem (GtkContainer *widget, const gchar *widget_name)
{
  if (widget)
     gtk_container_foreach (widget, find_menuitem_cb, widget_name);

  return mni_temp;
}


GtkFileFilter* tea_file_filter_with_patterns (const gchar *name, gchar *pattern1, ...)
{
  va_list args;
  gchar *t;

  GtkFileFilter *f = gtk_file_filter_new ();
  gtk_file_filter_set_name (f, name);

  va_start (args, pattern1);
  t = pattern1;
  while (t)
        {
         gtk_file_filter_add_pattern (f, t);
         t = va_arg (args, gchar*);
        }
  va_end (args);

  return f;
}


GtkWidget* daisho_add_del (gpointer *f1, gpointer f2)
{
//  GtkWidget *vb = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *vb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR);  
  gtk_widget_show (vb);

  GtkWidget *b1 = gtk_button_new_from_stock (GTK_STOCK_ADD);
  GtkWidget *b2 = gtk_button_new_from_stock (GTK_STOCK_DELETE);

  if (f1)
     g_signal_connect (b1, "clicked", G_CALLBACK (f1), NULL);

  if (f2)
     g_signal_connect (b2, "clicked", G_CALLBACK (f2), NULL);

  gtk_widget_show (b1);
  gtk_widget_show (b2);

  gtk_box_pack_start (vb, b1, FALSE, FALSE, UI_PACKFACTOR);
  gtk_box_pack_start (vb, b2, FALSE, FALSE, UI_PACKFACTOR);

  return vb;
}


GtkWidget* new_menu_item_with_udata (const gchar *label, GtkWidget *parent, gpointer f, gpointer data)
{
  GtkWidget *item = gtk_menu_item_new_with_label (label);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (parent), item);
  gtk_widget_set_name (item, label);

  if (f)
     g_signal_connect ((gpointer) item, "activate", G_CALLBACK (f), data);

  return item;
}


GtkWidget* new_menu_item (const gchar *label, GtkWidget *parent, gpointer f)
{
  GtkWidget *item = gtk_menu_item_new_with_label (label);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (parent), item);
  gtk_widget_set_name (item, label);

  if (f)
     g_signal_connect ((gpointer) item, "activate", G_CALLBACK (f), NULL);

  return item;
}


GtkWidget* new_menu_sep (GtkWidget *parent)
{
  GtkWidget *mni_sep = gtk_menu_item_new();
  gtk_widget_show (mni_sep);
  gtk_container_add (GTK_CONTAINER (parent), mni_sep);
  gtk_widget_set_name (mni_sep, MENUITEMUTIL);
  gtk_widget_set_sensitive (mni_sep, FALSE);
  return mni_sep;
}


GtkWidget* new_menu_tof (GtkWidget *parent)
{
  GtkWidget *mni_tof = gtk_tearoff_menu_item_new();
  gtk_widget_set_name (mni_tof, MENUITEMUTIL);
  gtk_widget_show (mni_tof);
  gtk_container_add (GTK_CONTAINER (parent), mni_tof);
}


GtkWidget* new_menu_submenu (GtkWidget *parent)
{
  GtkWidget *mni_menu = gtk_menu_new();
  gtk_widget_set_name (mni_menu, MENUITEMUTIL);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (parent), mni_menu);
  return mni_menu;
}


gboolean win_key_handler (GtkWidget *widget, GdkEventKey *event, GtkWidget *w)
{
  if (event->keyval == GDK_KEY_Escape)
    {
     gtk_widget_destroy (w);
     return TRUE;
    }
  return FALSE;
}


gboolean dlg_key_handler (GtkWidget *widget, GdkEventKey *event, GtkWidget *w)
{
  if (event->keyval == GDK_KEY_Escape)
    {
     gtk_dialog_response (GTK_DIALOG (widget), GTK_RESPONSE_REJECT);
     return TRUE;
    }
  return FALSE;
}

//from Bluefish:: bf_lib.c and gtk_easy
/**
 * window_destroy:
 * @windowname: a #GtkWidget pointer to the window
 *
 * Remove grab and signals and then destroy window
 *
 * Return value: void
 **/
void window_destroy (GtkWidget *windowname)
     {
      g_signal_handlers_destroy (G_OBJECT(windowname));
      gtk_grab_remove (windowname);
      gtk_widget_destroy (windowname);
     }
////////

void set_cursor_blink (gboolean value)
{
  g_object_set (G_OBJECT ( gtk_settings_get_default ()),
               "gtk-cursor-blink",
                value,
                NULL);
}


GtkWidget* tea_gtk_toolbar_prepend_space (GtkToolbar *toolbar)
{
/*  return gtk_toolbar_prepend_element (toolbar,
                                      GTK_TOOLBAR_CHILD_SPACE,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);*/

   return 0;
}


void fill_combo_with_glist (GtkComboBox *combo, GList *l)
{
  if (! combo || ! l)
     return;

  GList *t = g_list_first (l);
  while (t && t->data)
        {
         //if (strlen (t->data) > 1)
         //gtk_combo_box_prepend_text (combo, t->data);
        // gtk_combo_box_append_text (combo, t->data);
	 gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), NULL, t->data);

         t = g_list_next (t);
        }
}


GtkWidget* tea_font_button (GtkWidget *container, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (TRUE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

  x = gtk_font_button_new ();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, TRUE, UI_PACKFACTOR);

  if (value)
     gtk_font_button_set_font_name (x, value);

  return x;
}


GtkWidget* tea_stock_button (const gchar *stock_id)
{
  GtkWidget *x = gtk_button_new();
  GtkWidget *im = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (x), im);
  gtk_widget_show_all (x);
  return x;
}


gchar* dlg_question_char (const gchar *prompt, const gchar *defval)
{
  gchar *result = NULL;
  GtkWidget *dialog = gtk_dialog_new_with_buttons (NULL,
                                                   NULL,
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_STOCK_CANCEL,
                                                   GTK_RESPONSE_REJECT,
                                                   GTK_STOCK_OK,
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

  //GtkWidget *hbox = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR);  

  GtkWidget *im = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_container_add (gtk_dialog_get_content_area (dialog), im);

  gtk_widget_show (im);

  GtkWidget *ent = tea_text_entry (gtk_dialog_get_content_area (dialog), prompt, defval);

  if (gtk_dialog_run (dialog) == GTK_RESPONSE_ACCEPT)
     if (gtk_entry_get_text (ent))
         result = g_strdup (gtk_entry_get_text (ent));

  gtk_widget_destroy (dialog);

  return result;
}


static build_menu_from_ht_cb (gpointer key,
                              gpointer value,
                              gpointer user_data)
{
  t_ppair *t = user_data;
  new_menu_item (key, t->a, t->b);
}


void build_menu_from_ht (GHashTable *hash_table, gpointer menu, gpointer callback)
{
   if (! ht_entities && ! hash_table)
       return;

   t_ppair t;
   t.a = menu;
   t.b = callback;

   g_hash_table_foreach (hash_table,
                         build_menu_from_ht_cb,
                         &t);
}


void build_menu_from_glist (GList *list, gpointer menu, gpointer callback)
{
   if (! list)
       return;

   GList *p = g_list_first (list);
   while (p)
        {
         if (strlen (p->data) >= 1)
            mni_temp = new_menu_item (p->data, menu, callback);
         p = g_list_next (p);
        }
}


//current music: Radiohead - OK Computer - Paranoid Andriod
void build_menu_wudata_from_glist (GList *list, gpointer menu, gpointer callback, gpointer udata)
{
  if (! list)
     return;
  GList *p = g_list_first (list);
  while (p)
       {
        if (p->data)
        if (strlen (p->data) >= 1)
           mni_temp = new_menu_item_with_udata (p->data, menu, callback, udata);
        p = g_list_next (p);
       }
}


void tea_widget_show (GtkWidget *w, gboolean visible)
{
  if (visible)
     gtk_widget_show (w);
   else
       gtk_widget_hide (w);
}


GtkWidget* tea_color_selector_simple (GtkWidget *container, const gchar *caption)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR);  
  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *bt = gtk_color_button_new ();
  gtk_widget_show (bt);
  gtk_box_pack_start (b, bt, FALSE, FALSE, UI_PACKFACTOR);

  return bt;
}


gchar* get_text_buf (gpointer text_buffer)
{
  if (! text_buffer)
     return NULL;

  GtkTextIter itstart, itend;
  gtk_text_buffer_get_bounds (text_buffer, &itstart, &itend);

  return gtk_text_buffer_get_text (text_buffer, &itstart, &itend, FALSE);
}


static gboolean on_shortcut_entry_keypress (GtkWidget *widget,
                                            GdkEventKey *event,
                                            gpointer data)
{
  GString *s = g_string_sized_new (64);

  if (event->state & GDK_CONTROL_MASK)
     s = g_string_append (s, "Ctrl ");

  if (event->state & GDK_SHIFT_MASK)
     s = g_string_append (s, "Shift ");

  if (event->state & GDK_MOD1_MASK)
     s = g_string_append (s, "Alt ");

  gchar *key = gdk_keyval_name (event->keyval);
  s = g_string_append (s, key);
  gchar *val = g_string_free (s, FALSE);

  gtk_entry_set_text (data, val);
  g_free (val);

  return TRUE;
}


GtkWidget* tea_shortcut_entry (void)
{
  GtkWidget *x = gtk_entry_new ();
  gtk_widget_show (x);

  g_signal_connect (x, "key-press-event",
                    G_CALLBACK (on_shortcut_entry_keypress), x);

  return x;
}


GtkWidget* tea_text_entry_hotkey (GtkWidget *container, const gchar *caption, const gchar *value)
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_label_set_use_markup (x, TRUE);

  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);
  x = tea_shortcut_entry ();
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, TRUE, TRUE, UI_PACKFACTOR);

  if (value)
     gtk_entry_set_text (x, value);

  return x;
}


static void on_bt_choose (GtkButton *button,
                          gpointer user_data)
{
  gchar *x = request_item_from_tvlist (user_data);

  if (x)
     {
      gtk_button_set_label (button, x);
      g_free (x);
     }
}


GtkWidget* tea_button_list_items (
                                  GtkWidget *container,
                                  GList *list,
                                  const gchar *caption,
                                  const gchar *defval
                                  )
{
  //GtkWidget *b = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR);  
  gtk_widget_show (b);
  gtk_box_pack_start (container, b, FALSE, TRUE, UI_PACKFACTOR);

  GtkWidget *x = gtk_label_new (caption);
  gtk_widget_show (x);
  gtk_box_pack_start (b, x, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *bt = gtk_button_new_with_label (defval);
  gtk_widget_show (bt);

  gtk_box_pack_start (b, bt, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect (bt, "clicked", G_CALLBACK (on_bt_choose), list);

  return bt;
}
