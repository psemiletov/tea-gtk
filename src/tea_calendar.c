/***************************************************************************
                          tea_calendar.c  -  description
                             -------------------
    begin                : 04.02.2005
    copylft            : 2005-2006 by Semiletov
    email                : tea@list.ru
 ***************************************************************************/
/*
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
*/

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
#include <unistd.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <glib/gi18n.h>

#include "tea_defs.h"
#include "tea_gtk_utils.h"
#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "tea_config.h"
#include "tea_calendar.h"



static GDate *date1;
static GDate *date2;


void calendar_init (void)
{
  date1 = NULL;
  date2 = NULL;
  wnd_calendar = NULL;
}


void calendar_done (void)
{
  if (date2)
     g_date_free (date2);
  if (date1)
     g_date_free (date1);
}


static void cb_on_dateinsert (GtkButton *button,
                              gpointer user_data)
{
  guint year;
  guint month;
  guint day;
  gchar d1[256];
  GDate *dt;

  gtk_calendar_get_date (GTK_CALENDAR (user_data), &year, &month, &day);

  dt = g_date_new_dmy (day, month, year);

  g_date_strftime (d1, sizeof (d1), "%x", dt);

  if (get_page_text ())
     doc_insert_at_cursor (cur_text_doc, d1);

  g_date_free (dt);
}


static void cb_on_insert_julian (GtkButton *button,
                                 gpointer user_data)
{
  guint year;
  guint month;
  guint day;

  gtk_calendar_get_date (GTK_CALENDAR (user_data), &year, &month, &day);

  GDate *dt = g_date_new_dmy (day, month, year);
  gchar *d = g_strdup_printf ("%d", g_date_get_julian (dt));

  if (get_page_text())
     doc_insert_at_cursor (cur_text_doc, d);

  g_free (d);
  g_date_free (dt);
}


static void cb_on_insert_day_of_year (GtkButton *button,
                                      gpointer user_data)
{
  guint year;
  guint month;
  guint day;

  gtk_calendar_get_date (GTK_CALENDAR (user_data), &year, &month, &day);

  GDate *dt = g_date_new_dmy (day, month, year);
  gchar *d = g_strdup_printf ("%d", g_date_get_day_of_year (dt));

  if (get_page_text())
     doc_insert_at_cursor (cur_text_doc, d);

  g_free (d);
  g_date_free (dt);
}


static void cb_on_date1_button_click (GtkButton *button,
                                      gpointer user_data)
{
  guint year;
  guint month;
  guint day;

  gtk_calendar_get_date (GTK_CALENDAR (user_data), &year, &month, &day);
  if (date1)
     g_date_free (date1);
  date1 = g_date_new_dmy (day, month, year);
}


static void cb_on_date2_button_click (GtkButton *button,
                                      gpointer user_data)
{
  guint year;
  guint month;
  guint day;

  gtk_calendar_get_date (GTK_CALENDAR (user_data), &year, &month, &day);
  if (date2)
     g_date_free (date2);
  date2 = g_date_new_dmy (day, month, year);
}


static void cb_on_calc_button_click (GtkButton *button,
                                     gpointer user_data)
{
  if (! date1 && ! date2)
     return;

  gchar d1[256];
  gchar d2[256];

  g_date_strftime (d1, sizeof (d1), "%x", date1);
  g_date_strftime (d2, sizeof (d2), "%x", date2);

  gchar *days = g_strdup_printf (_("Days between %s and %s = %d"), d1, d2, g_date_days_between (date2, date1));

  log_to_memo (days, NULL, LM_NORMAL);
  g_free (days);
}


static gint destroy_event (GtkWidget *widget, GdkEventAny *event, gpointer data)
{
  gtk_widget_destroy (wnd_calendar);
  wnd_calendar = NULL;
  return TRUE;
}


GtkWidget* make_calendar_window (void)
{
  GtkWidget *wnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect ((gpointer) (wnd), "destroy",
                    (gpointer) (destroy_event), wnd);

  gtk_window_set_position (wnd, GTK_WIN_POS_CENTER);

  gtk_window_set_title (GTK_WINDOW (wnd), _("Calendar"));

  GtkWidget *vbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (wnd), vbox1);

  GtkWidget *fr_diff = gtk_frame_new (_("Days between"));
  gtk_widget_show (fr_diff);
  gtk_container_add (GTK_CONTAINER (vbox1), fr_diff);

  GtkWidget *hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (fr_diff), hbox1);

  GtkWidget *cal = gtk_calendar_new ();
  gtk_widget_show (cal);

  GtkWidget *bt_date1 = gtk_button_new_with_label (_("date1"));
  gtk_widget_show (bt_date1);
  gtk_container_add (GTK_CONTAINER (hbox1), bt_date1);
  g_signal_connect (bt_date1, "clicked", G_CALLBACK (cb_on_date1_button_click), cal);

  GtkWidget *bt_date2 = gtk_button_new_with_label (_("date2"));
  gtk_widget_show (bt_date2);
  gtk_container_add (GTK_CONTAINER (hbox1), bt_date2);
  g_signal_connect (bt_date2, "clicked", G_CALLBACK (cb_on_date2_button_click), cal);

  GtkWidget *bt_calc = gtk_button_new_with_label (_("Calculate"));
  gtk_widget_show (bt_calc);
  gtk_container_add (GTK_CONTAINER (hbox1), bt_calc);
  g_signal_connect (bt_calc, "clicked", G_CALLBACK (cb_on_calc_button_click), NULL);

  gtk_container_add (GTK_CONTAINER (vbox1), cal);

  GtkWidget *bt_insert = gtk_button_new_with_label (_("Insert as a date"));
  gtk_widget_show (bt_insert);
  gtk_container_add (GTK_CONTAINER (vbox1), bt_insert);
  g_signal_connect (bt_insert, "clicked", G_CALLBACK (cb_on_dateinsert), cal);

  bt_insert = gtk_button_new_with_label (_("Insert as a julian day"));
  gtk_widget_show (bt_insert);
  gtk_container_add (GTK_CONTAINER (vbox1), bt_insert);
  g_signal_connect (bt_insert, "clicked", G_CALLBACK (cb_on_insert_julian), cal);

  bt_insert = gtk_button_new_with_label (_("Insert as a day of year"));
  gtk_widget_show (bt_insert);
  gtk_container_add (GTK_CONTAINER (vbox1), bt_insert);
  g_signal_connect (bt_insert, "clicked", G_CALLBACK (cb_on_insert_day_of_year), cal);

  g_signal_connect (G_OBJECT (wnd), "key_press_event", G_CALLBACK (win_key_handler), wnd);

  gtk_widget_show (wnd);
  return wnd;
}
