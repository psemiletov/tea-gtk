/***************************************************************************
                          tea_findfiles.c  -  description
                             -------------------
    begin                :
    copyright            : (C) 2006-2007 by Peter 'Roxton' Semiletov
    email                : peter.semiletov@gmail.com
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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <stdlib.h> // needed for system

#include "tea_defs.h"
#include "tea_text_document.h"
#include "interface.h"
#include "tea_gtk_utils.h"
#include "tea_config.h"
#include "tea_funx.h"
#include "rox_strings.h"
#include "tea_findfiles_mult.h"
#include "tea_tree_view.h" // Needed for tv_get_selected_single


static void find_in_files_free (t_find_in_files_mult *instance)
{
  dbm ("find_in_files_free");
  g_free (instance);
}


static gboolean on_find_in_files_delete_event (GtkWidget *widget, GdkEvent *event,
                                               gpointer user_data)
{
  find_in_files_free (user_data);
  return FALSE;
}


static gboolean on_find_in_files_key_press_event (GtkWidget *widget,
                                                  GdkEventKey *event,
                                                  gpointer user_data)
{
  if (event->keyval == GDK_KEY_Escape)
    {
     find_in_files_free (user_data);
     gtk_widget_destroy (widget);
     return TRUE;
    }

  return FALSE;
}


static gboolean pb_bounce (gpointer data)
{
  gtk_progress_bar_pulse (data);
  return TRUE;
}


static void pb_start (t_find_in_files_mult *p)
{
  p->id_pb = g_timeout_add (500, pb_bounce, p->pb_progress_bar);
  gtk_widget_show (p->pb_progress_bar);
  gtk_progress_bar_pulse (p->pb_progress_bar);
}


static void pb_stop (t_find_in_files_mult *p)
{
  gtk_progress_bar_set_fraction (p->pb_progress_bar, 0.0);
  g_source_remove (p->id_pb);
  gtk_widget_hide (p->pb_progress_bar);
}


static gint button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  t_find_in_files_mult *p = user_data;

  if (event->button == 1)
     if (event->type == GDK_2BUTTON_PRESS)
        {
         gchar *s = tv_get_selected_single (p->tv_found_files);
         if (s)
            {
             gchar *l = get_l_filename (s);
             execute_recent_item (l);
             g_free (l);
             g_free (s);
             return TRUE;
            }
         }
  return FALSE;
}


static GList* find_in_files (t_find_in_files_mult *instance, GList *files)
{
  if (! instance || ! files)
     return NULL;

  GList *result = NULL;

  GList *l = g_list_first (files);

  gchar *enc = g_strdup (combo_get_value (instance->cmb_charset, confile.gl_save_charsets));

  gchar *text_to_find = gtk_entry_get_text (instance->ent_text_to_find);

  gchar *t = NULL;

  while (l)
        {
         while (gtk_events_pending ())
               gtk_main_iteration ();

         if (is_office (l->data))
            t = strinfile_office (l->data, text_to_find);
         else
             t = strinfile_charset (l->data, text_to_find, enc);

         if (t)
           result = g_list_prepend (result, t);

         l = g_list_next (l);
        }

  g_free (enc);

  return result;
}


static void on_button_copy_to_dir (GtkWidget *wid, gpointer data)
{
  t_find_in_files_mult *instance = data;

  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Select a directory"), NULL,                                                             GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,                                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  gchar *filename = NULL;

  if (gtk_dialog_run (dialog) == GTK_RESPONSE_ACCEPT)
     filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  else
      {
       gtk_widget_destroy (dialog);
       return;
      }

  gtk_widget_destroy (dialog);

  GList *l = tv_get_list_data (instance->tv_found_files);
  gchar *x = NULL;
  gchar *f = NULL;

  gchar *cm = NULL;
  while (l)
        {
         x = str_before_sep (l->data, ",");
         f = get_l_filename (x);

         cm = g_strconcat ("cp --reply=yes ", f, " ", filename, NULL);

         system (cm);

         g_free (x);
         g_free (cm);
         g_free (f);

         l = g_list_next (l);
        }
}


gboolean do_search (gpointer data)
{
  t_find_in_files_mult *instance = data;

  gchar *dir = gtk_entry_get_text (instance->ent_dir_where_to_find);
  if (! dir)
      return FALSE;

  gl_found_files = NULL;

  pb_start (instance);

  GList *ffiles = read_dir_files_rec_pat (dir, gtk_entry_get_text (instance->ent_pattern));

  dbm ("Created the list of files");

  gtk_widget_show (instance->ffstatusbar);

  dbm ("start search within files ");

  GList *lx = find_in_files (instance, ffiles);
  tv_fill_with_glist (instance->tv_found_files, lx);
  glist_strings_free (lx);

  glist_strings_free (ffiles);

  dbm ("end search within files ");

  pb_stop (instance);

  return FALSE;
}


static void on_button_find (GtkWidget *wid, gpointer data)
{
  g_idle_add (do_search, data);
}


static void on_button_savesession (GtkWidget *wid, gpointer data)
{
  t_find_in_files_mult *instance = data;

  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("File save"), NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), confile.sessions_dir);

  gchar *filename = NULL;

  if (gtk_dialog_run (dialog) == GTK_RESPONSE_ACCEPT)
     filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  else
      {
       gtk_widget_destroy (dialog);
       return;
      }

  gtk_widget_destroy (dialog);

  GList *l = tv_get_list_data (instance->tv_found_files);
  GList *save = NULL;
  while (l)
        {
         save = g_list_prepend (save, get_l_filename (l->data));
         l = g_list_next (l);
        }

  glist_save_to_file (save, filename);
  glist_strings_free (save);
  reload_sessions ();
}


static void on_button_copy_filename (GtkWidget *wid, gpointer data)
{
  t_find_in_files_mult *instance = data;

  gchar *line = tv_get_selected_single (instance->tv_found_files);
  gchar **a = g_strsplit (line, ",", -1);
  clipboard_put_text (a[0]);
  g_strfreev (a);
  g_free (line);
}


static void on_button_list_clear (GtkWidget *wid, gpointer data)
{
  t_find_in_files_mult *instance = data;
  tv_clear (instance->tv_found_files);
}


GtkWidget* create_findfiles_mult (void)
{
  GtkWidget *frame_and_strings;
  GtkWidget *frame_not_strings;
  GtkWidget *frame_or_strings;

  t_find_in_files_mult *instance = g_malloc (sizeof (t_find_in_files_mult));
  gl_found_files = NULL;

  GtkWidget *window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window1), _("Find in files"));

  GtkWidget *vbox_main = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox_main);
  gtk_container_add (GTK_CONTAINER (window1), vbox_main);

  GtkWidget *hbox_top = gtk_hbox_new (TRUE, UI_PACKFACTOR);
  gtk_widget_show (hbox_top);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_top, TRUE, TRUE, UI_PACKFACTOR);

  GtkWidget *vbox_left = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox_left);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_left, TRUE, TRUE, UI_PACKFACTOR);

  GtkWidget *vbox_right = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox_right);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_right, FALSE, TRUE, UI_PACKFACTOR);

  instance->ent_text_to_find = tea_text_entry (vbox_left, _("Text to find"), "Linux");
  instance->cmb_charset = gtk_combo_box_new_with_entry ();
  fill_combo_with_glist (instance->cmb_charset, confile.gl_save_charsets);
  gtk_widget_show (instance->cmb_charset);
  gtk_box_pack_start (GTK_BOX (vbox_left), instance->cmb_charset, FALSE, FALSE, UI_PACKFACTOR);
  gtk_combo_box_set_active (instance->cmb_charset, 0);

  instance->ent_dir_where_to_find = tea_dir_selector (vbox_left, _("And where to find?"), g_get_home_dir ());
  instance->ent_pattern = tea_text_entry (vbox_left, _("File pattern"), "*.txt");

  GtkWidget *bt_find = tea_button_at_box (vbox_left, _("Find"), on_button_find, instance);
  GtkWidget *bt_copy_to_dir = tea_button_at_box (vbox_left, _("Copy to the directory"), bt_copy_to_dir, instance);
  GtkWidget *bt_savesession = tea_button_at_box (vbox_left, _("Save as a session"), on_button_savesession, instance);
  GtkWidget *bt_copyfilename = tea_button_at_box (vbox_left, _("Copy a filename"), on_button_copy_filename, instance);
  GtkWidget *bt_list_clear = tea_button_at_box (vbox_left, _("Clear a list"), on_button_list_clear, instance);

  instance->tv_found_files = tv_create_framed (vbox_right, _("Found files"), GTK_SELECTION_SINGLE);

  g_signal_connect ((gpointer) (instance->tv_found_files), "button_press_event",
                     /*(GtkSignalFunc)*/ button_press_event, instance);

  instance->ffstatusbar = gtk_statusbar_new ();
  gtk_widget_show (instance->ffstatusbar);
  gtk_box_pack_start (GTK_BOX (vbox_main), instance->ffstatusbar, FALSE, FALSE, UI_PACKFACTOR);

  //b.p. Scorn - Whine - Strand

  instance->pb_progress_bar = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (instance->ffstatusbar), instance->pb_progress_bar, FALSE, FALSE, UI_PACKFACTOR);
  gtk_progress_bar_set_pulse_step (instance->pb_progress_bar, 0.05);
  gtk_widget_show (instance->ffstatusbar);

  gtk_window_resize (window1, get_value (confile.screen_w, 65), get_value (confile.screen_h, 65));

  g_signal_connect (window1, "key_press_event",
                    G_CALLBACK (on_find_in_files_key_press_event),
                    instance);

  g_signal_connect (window1, "delete_event",
                    G_CALLBACK (on_find_in_files_delete_event),
                    instance);

  gtk_window_set_position (window1, GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_widget_show (window1);

  return window1;
}
