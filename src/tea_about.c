/***************************************************************************
                          tea_about.c  -  description
                             -------------------
    begin                : august 2004
    copyleft            : 2004-2007 by Peter Semiletov
    email                : tea@list.ru
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************-************************************/

#include <libintl.h>
#include <locale.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "tea_text_document.h" // Needed for log_to_memo
#include "callbacks.h"
#include "interface.h"
#include "tea_defs.h"
#include "tea_gtk_utils.h"
#include "tea_funx.h" // Needed for get_tea_doc_compose_name
#include "rox_strings.h" // Needed for get_gchar_array_size
#include "tea_about.h"


static gboolean cb_wnd_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  return FALSE;
}


GtkWidget* create_wnd_about (void)
{
  GtkWidget *wnd_about;
  GtkWidget *vbox1;
  GtkWidget *notebook1;
  GtkWidget *l_logo;
  GtkWidget *scrolledwindow1;
  GtkWidget *tv_code;
  GtkWidget *l_code;
  GtkWidget *scrolledwindow4;
  GtkWidget *tv_thanx;
  GtkWidget *l_thanx;
  GtkWidget *scrolledwindow2;
  GtkWidget *tv_translators;
  GtkWidget *l_translators;
  GtkWidget *scrolledwindow3;
  GtkWidget *tv_rpms;
  GtkWidget *l_rpm;
  GtkWidget *bt_close;

  gchar *f = get_tea_doc_compose_name ("AUTHORS");
  gchar *s = str_file_read (f);

  if (! s)
     {
      log_to_memo (_("%s is missing! I cannot show you the About window :("), f, LM_ERROR);
      g_free (f);
      return NULL;
     }

  gchar **ad;
  if (s)
     ad = g_strsplit (s, "##", -1);

  g_free (f);
  g_free (s);

  wnd_about = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (wnd_about), _("About"));

  vbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (wnd_about), vbox1);

  GtkWidget *image = gtk_image_new_from_file (TEA_PIX_MAINLOGO);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vbox1), image, FALSE, FALSE, UI_PACKFACTOR);

  notebook1 = gtk_notebook_new();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox1), notebook1, TRUE, TRUE, UI_PACKFACTOR);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow1);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  tv_code = gtk_text_view_new();
  gtk_widget_show (tv_code);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), tv_code);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv_code), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv_code), GTK_WRAP_WORD);

  l_code = gtk_label_new (_("Code"));
  gtk_widget_show (l_code);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), l_code);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow4);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  tv_thanx = gtk_text_view_new();
  gtk_widget_show (tv_thanx);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), tv_thanx);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv_thanx), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv_thanx), GTK_WRAP_WORD);

  l_thanx = gtk_label_new (_("Acknowledgements"));
  gtk_widget_show (l_thanx);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), l_thanx);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  tv_translators = gtk_text_view_new ();
  gtk_widget_show (tv_translators);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), tv_translators);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv_translators), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv_translators), GTK_WRAP_WORD);

  l_translators = gtk_label_new (_("Translators"));
  gtk_widget_show (l_translators);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), l_translators);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  tv_rpms = gtk_text_view_new ();
  gtk_widget_show (tv_rpms);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), tv_rpms);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv_rpms), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv_rpms), GTK_WRAP_WORD);

  l_rpm = gtk_label_new (_("Packages"));
  gtk_widget_show (l_rpm);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), l_rpm);

  bt_close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_widget_show (bt_close);
  gtk_box_pack_start (GTK_BOX (vbox1), bt_close, FALSE, FALSE, UI_PACKFACTOR);

 // GTK_WIDGET_SET_FLAGS (bt_close, GTK_CAN_DEFAULT);
  gtk_widget_set_can_default (bt_close, TRUE);

//  g_signal_connect (bt_close, "clicked", G_CALLBACK (cb_on_bt_close), wnd_about);

  g_signal_connect_swapped (bt_close, "clicked", G_CALLBACK (cb_on_bt_close), wnd_about);
  
  gtk_widget_grab_focus (bt_close);
  gtk_widget_grab_default (bt_close);

  gtk_window_set_modal (wnd_about, TRUE);

  if (get_gchar_array_size (ad) == 4)
     {
      gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv_code)), g_strchug (ad[0]), -1);
      gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv_rpms)), g_strchug (ad[2]), -1);
      gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv_translators)), g_strchug (ad[1]), -1);
      gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv_thanx)), g_strchug (ad[3]), -1);
     }

  gtk_window_set_position (GTK_WINDOW (wnd_about), GTK_WIN_POS_CENTER);

  g_signal_connect ((gpointer) wnd_about, "destroy",
                    G_CALLBACK (cb_wnd_destroy_event),
                    NULL);

  g_strfreev (ad);

  g_signal_connect (G_OBJECT (wnd_about), "key_press_event", G_CALLBACK (win_key_handler), wnd_about);

  gtk_window_resize (wnd_about, 410, 450);
  gtk_widget_show (wnd_about);
  return wnd_about;
}
