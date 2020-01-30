/***************************************************************************
                          tea_fr.c  -  description
                             -------------------
    begin                :
    copyright            : (C) 2004-2006 by Peter 'Roxton' Semiletov
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

#include "tea_fr.h"
#include "tea_defs.h"
#include "tea_text_document.h"
#include "interface.h"
#include "tea_gtk_utils.h"
#include "tea_funx.h" // Needed for get_clipboard_text


static void on_button_find (GtkWidget *wid, gpointer data)
{
  if (! get_page_text ())
     return;

  gchar *t = gtk_entry_get_text (ent_text_to_find);
  if (! t)
      return;

  if (gtk_toggle_button_get_active (&chb_case_insensetive->toggle_button))
      doc_search_f_ncase (cur_text_doc, t);
  else
      doc_search_f (cur_text_doc, t);
}


void on_button_find_next_f (GtkWidget *wid, gpointer data)
{
  if (! get_page_text ())
     return;

  if (gtk_toggle_button_get_active (&chb_case_insensetive->toggle_button))
     doc_search_f_next_ncase (cur_text_doc);
  else
      doc_search_f_next (cur_text_doc);
}


void on_button_replace (GtkWidget *wid, gpointer data)
{
  if (! get_page_text ())
     return;

  gchar *t = gtk_entry_get_text (ent_text_to_replace);
  if (! t)
      return;

  if (doc_has_selection (cur_text_doc))
     doc_rep_sel (cur_text_doc, t);
}


void on_button_jmp_top (GtkWidget *wid, gpointer data)
{
  if (! get_page_text ())
     return;
  editor_set_pos (cur_text_doc, 0);
}


GtkWidget* create_fr (void)
{
  GtkWidget *window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window1), _("Search and Replace window"));

  vbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);

  hbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, UI_PACKFACTOR);

  ent_text_to_find = tea_text_entry (hbox1, _("Text to find"), NULL);

  gchar *ft = get_clipboard_text ();

  if (ft)
      gtk_entry_set_text (ent_text_to_find, g_strstrip (ft));

  g_free (ft);

  ent_text_to_replace = tea_text_entry (hbox1, _("Replace with"), NULL);

  hbox3 = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, UI_PACKFACTOR);

  chb_case_insensetive = gtk_check_button_new_with_label (_("Case insensitive"));
  gtk_widget_show (chb_case_insensetive);
  gtk_box_pack_start (GTK_BOX (hbox3), chb_case_insensetive, FALSE, FALSE, UI_PACKFACTOR);

  //n.p. Fugees - La-Fu-Gee

  GtkWidget *hbox4 = gtk_hbox_new (TRUE, UI_PACKFACTOR);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox4, TRUE, TRUE, UI_PACKFACTOR);

  GtkWidget *bt_replace = gtk_button_new_with_label (_("Replace"));
  gtk_widget_show (bt_replace);
  gtk_box_pack_start (GTK_BOX (hbox4), bt_replace, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_replace, "clicked",
                    G_CALLBACK (on_button_replace),
                    NULL);

  GtkWidget *bt_forward = gtk_button_new_with_label (_("Find next"));
  gtk_widget_show (bt_forward);
  gtk_box_pack_start (GTK_BOX (hbox4), bt_forward, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_forward, "clicked",
                    G_CALLBACK (on_button_find_next_f),
                    NULL);

  GtkWidget *bt_find = gtk_button_new_with_label (_("Find"));
  gtk_widget_show (bt_find);
  gtk_box_pack_start (GTK_BOX (hbox4), bt_find, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_find, "clicked",
                    G_CALLBACK (on_button_find),
                    NULL);

  GtkWidget *bt_jmp_top = gtk_button_new_with_label (_("Jump to top"));
  gtk_widget_show (bt_jmp_top);
  gtk_box_pack_start (GTK_BOX (hbox4), bt_jmp_top, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_jmp_top, "clicked",
                    G_CALLBACK (on_button_jmp_top),
                    NULL);

  gtk_window_set_focus (window1, ent_text_to_find);

  gtk_window_set_modal (window1, TRUE);

  g_signal_connect (G_OBJECT (window1), "key_press_event", G_CALLBACK (win_key_handler), window1);

  gtk_window_set_transient_for (window1, tea_main_window);
  gtk_window_set_position (window1, GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_widget_show (window1);

  return window1;
}
