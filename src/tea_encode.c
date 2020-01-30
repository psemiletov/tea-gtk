/***************************************************************************
                          tea_encode.c  -  description
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
#include <string.h>


#include "tea_encode.h"
#include "tea_defs.h"
#include "tea_text_document.h"
#include "interface.h"
#include "tea_gtk_utils.h"
#include "tea_config.h"
#include "tea_funx.h"
#include "rox_strings.h"
#include "tea_tree_view.h" // Needed for tea_create_framed



static void mass_encode_tool_free (t_mass_encode_tool *instance)
{
  dbm ("mass_encode_tool_free");
  g_free (instance);
}


static gboolean on_mass_encode_tool_free_delete_event (GtkWidget *widget, GdkEvent *event,
                                                      gpointer user_data)
{
  mass_encode_tool_free (user_data);
  return FALSE;
}


static gboolean on_mass_encode_tool_key_press_event (GtkWidget *widget,
                                                     GdkEventKey *event,
                                                     gpointer user_data)
{
  if (event->keyval == GDK_KEY_Escape)
    {
     mass_encode_tool_free (user_data);
     gtk_widget_destroy (widget);
     return TRUE;
    }

  return FALSE;
}


static gboolean pb_bounce (gpointer data)
{
  t_mass_encode_tool *p = data;
  gtk_progress_bar_pulse (p->pb_progress_bar);
  return TRUE;
}


static void pb_start (t_mass_encode_tool *instance)
{
  instance->id_pb = g_timeout_add (100, instance->pb_progress_bar, NULL);
  gtk_widget_show (instance->pb_progress_bar);
  gtk_progress_bar_pulse (instance->pb_progress_bar);
}


static void pb_stop (t_mass_encode_tool *instance)
{
  gtk_progress_bar_set_fraction (instance->pb_progress_bar, 0.0);
  g_source_remove (instance->id_pb);
  gtk_widget_hide (instance->pb_progress_bar);
}


static void on_button_encode (GtkWidget *wid, gpointer data)
{
  t_mass_encode_tool *instance = data;

  gchar *dir = gtk_entry_get_text (instance->ent_dir_where_to_find);
  if (! dir)
      return;

  gchar *from = tv_get_selected_single (instance->tv_charsets_from);
  gchar *to = tv_get_selected_single (instance->tv_charsets_to);

  gboolean convert_eols = (gtk_toggle_button_get_active (&instance->bt_convert_eol->toggle_button));
  gint eol_mode;

  if (gtk_toggle_button_get_active (instance->bt_lf))
      eol_mode = eol_LF;
  if (gtk_toggle_button_get_active (instance->bt_crlf))
      eol_mode = eol_CRLF;
  if (gtk_toggle_button_get_active (instance->bt_cr))
      eol_mode = eol_CR;

  if (! from)
     {
      g_free (to);
      return;
     }

  if (! to)
     {
      g_free (from);
      return;
     }

  gl_found_files = NULL;

  gsize bytes_read;
  gsize bytes_written;
  gchar *buf;
  gchar *dest;
  gchar *z;

  GError *error = NULL;
  GList *l;

  pb_start (instance);

  GList *lf = read_dir_files_rec_pat (dir, gtk_entry_get_text (instance->ent_pattern));

  if (lf)
     {
      l = g_list_first (lf);
      while (l)
            {
             error = NULL;
             buf = str_file_read (l->data);
             if (buf)
                {
                 dest = g_convert (buf, strlen (buf), to, from, &bytes_read, &bytes_written, &error);
                 if (dest)
                    {
                     if (convert_eols)
                        {
                         if (eol_mode == eol_LF)
                            z = str_eol_to_lf (dest);
                         if (eol_mode == eol_CRLF)
                            z = str_eol_to_crlf (dest);
                         if (eol_mode == eol_CR)
                            z = str_eol_to_cr (dest);

                         save_string_to_file (l->data, z);

                         g_free (dest);
                         g_free (z);
                        }
                     else
                         {
                          save_string_to_file (l->data, dest);
                          g_free (dest);
                         }
                    }
                 g_free (buf);
                }
             l = g_list_next (l);
            }

      glist_strings_free (lf);
     }

  pb_stop (instance);

  gtk_widget_show (instance->ffstatusbar);
  g_free (to);
  g_free (from);
}


GtkWidget* create_wnd_encode (void)
{
  t_mass_encode_tool *instance = g_malloc (sizeof (t_mass_encode_tool));

  GtkWidget *window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window1), _("Encode files"));

  GtkWidget *vbox_main = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox_main);
  gtk_container_add (GTK_CONTAINER (window1), vbox_main);

  GtkWidget *box_top = gtk_hbox_new (TRUE, UI_PACKFACTOR);
  gtk_widget_show (box_top);
  gtk_container_add (GTK_CONTAINER (vbox_main), box_top);

  GtkWidget *box_left = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (box_left);
  gtk_box_pack_start (GTK_BOX (box_top), box_left, FALSE, FALSE, UI_PACKFACTOR);

  instance->ent_dir_where_to_find = tea_dir_selector (box_left, _("Directory"), g_get_home_dir ());
  instance->ent_pattern = tea_text_entry (box_left, _("File pattern"), "*.txt");

  instance->bt_convert_eol = gtk_check_button_new_with_label (_("Convert ends of lines"));
  gtk_widget_show (instance->bt_convert_eol);
  gtk_box_pack_start (GTK_BOX (box_left), instance->bt_convert_eol, FALSE, FALSE, UI_PACKFACTOR);

  instance->bt_lf = gtk_radio_button_new_with_label (NULL, "LF");
  instance->bt_crlf = gtk_radio_button_new_with_label (gtk_radio_button_get_group (instance->bt_lf), "CR/LF");
  instance->bt_cr = gtk_radio_button_new_with_label (gtk_radio_button_get_group (instance->bt_lf), "CR");

  gtk_widget_show (instance->bt_lf);
  gtk_box_pack_start (GTK_BOX (box_left), instance->bt_lf, FALSE, FALSE, UI_PACKFACTOR);
  gtk_widget_show (instance->bt_crlf);
  gtk_box_pack_start (GTK_BOX (box_left), instance->bt_crlf, FALSE, FALSE, UI_PACKFACTOR);
  gtk_widget_show (instance->bt_cr);
  gtk_box_pack_start (GTK_BOX (box_left), instance->bt_cr, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *bt_encode = gtk_button_new_with_label (_("Find and convert"));
  gtk_widget_show (bt_encode);
  gtk_box_pack_start (GTK_BOX (box_left), bt_encode, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_encode, "clicked",
                    G_CALLBACK (on_button_encode),
                    instance);

  GtkWidget *box_right = gtk_hbox_new (TRUE, UI_PACKFACTOR);
  gtk_widget_show (box_right);
  gtk_box_pack_start (GTK_BOX (box_top), box_right, TRUE, TRUE, UI_PACKFACTOR);

  instance->tv_charsets_from = tv_create_framed (box_right, _("From: "), GTK_SELECTION_SINGLE);
  tv_fill_with_glist (instance->tv_charsets_from, gl_enc_available);

  instance->tv_charsets_to = tv_create_framed (box_right, _("To: "), GTK_SELECTION_SINGLE);
  tv_fill_with_glist (instance->tv_charsets_to, gl_enc_available);

  instance->ffstatusbar = gtk_statusbar_new ();
  gtk_widget_show (instance->ffstatusbar);
  gtk_box_pack_start (GTK_BOX (vbox_main), instance->ffstatusbar, FALSE, FALSE, UI_PACKFACTOR);

  //b.p. Scorn - Whine - Strand

  instance->pb_progress_bar = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (instance->ffstatusbar), instance->pb_progress_bar, FALSE, FALSE, UI_PACKFACTOR);
  gtk_progress_bar_set_pulse_step (instance->pb_progress_bar, 0.05);
  gtk_widget_show (instance->ffstatusbar);

  gtk_window_resize (window1, get_value (confile.screen_w, 50), get_value (confile.screen_h, 60));

  g_signal_connect (window1, "key_press_event",
                    G_CALLBACK (on_mass_encode_tool_key_press_event),
                    instance);

  g_signal_connect (window1, "delete_event",
                    G_CALLBACK (on_mass_encode_tool_free_delete_event),
                    instance);

  gtk_window_set_position (window1, GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_widget_show (window1);

  return window1;
}
