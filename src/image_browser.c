/***************************************************************************
                          image_browser.c  -  description
                             -------------------
    begin                : 11.02.2005
    copyleft            : 2005-2013 Peter 'Roxton' Semiletov, <peter.semiletov@gmail.com>
                          (c)2006 Michele Garoche, <michele.garoche@easyconnect.fr>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <glib.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h> // for GdkScreen

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib/gi18n.h>

#include "tea_defs.h"
#include "rox_strings.h"
#include "tea_config.h"
#include "image_browser.h"
#include "tea_gtk_utils.h"
#include "interface.h"
#include "md5-util.h"
#include "tea_tree_view.h"
#include "tea_funx.h" //Needed for insert_image



static void set_wallpaper_gnome (t_image_viewer *instance, const gchar *align)
{
  gchar *cm = g_strconcat ("gconftool-2 -s /desktop/gnome/background/picture_filename --type string ", instance->filename, NULL);
  system (cm);
  g_free (cm);

  cm = g_strconcat ("gconftool-2 -s /desktop/gnome/background/picture_options --type string ", align, NULL);
  system (cm);
  g_free (cm);
}


static void image_viewer_size_normalize (t_image_viewer *instance)
{
  gint pixw = gdk_pixbuf_get_width (gtk_image_get_pixbuf (instance->image));
  gint pixh = gdk_pixbuf_get_height (gtk_image_get_pixbuf (instance->image));

  gtk_window_resize (instance->wnd_image,
                     gdk_pixbuf_get_width (gtk_image_get_pixbuf (instance->image)),
                     gdk_pixbuf_get_height (gtk_image_get_pixbuf (instance->image)));
}


static void image_viewer_set_as_wallpaper_kde (t_image_viewer *instance, gint mode)
{
  if (! instance)
     return;

  gchar *cm = g_strdup_printf ("dcop kdesktop KBackgroundIface setWallpaper %s %d", instance->filename, mode);
  system (cm);
  g_free (cm);
}


static void image_viewer_toggle_fullscreen (t_image_viewer *instance)
{
  instance->fullscreen = ! instance->fullscreen;

  if (instance->fullscreen)
     gtk_window_fullscreen (instance->wnd_image);
  else
      {
       gtk_window_unfullscreen (instance->wnd_image);
       image_viewer_size_normalize (instance);
       #ifndef DARWIN
       gtk_window_move (instance->wnd_image, 1, 1);
       #endif
      }
}


static void image_load_next (t_image_viewer *instance, gboolean next)
{
  if (! instance)
     return;

  gpointer t = NULL;

  if (next)
     t = g_list_next (instance->files_list_current);
  else
     t = g_list_previous (instance->files_list_current);

  if (t)
     instance->files_list_current = t;
  else
      return;

  GdkPixbuf *p = gdk_pixbuf_new_from_file (instance->files_list_current->data, NULL);
  instance->filename = ch_str (instance->filename, instance->files_list_current->data);

  if (gdk_pixbuf_get_width (p) >= gdk_screen_width () || gdk_pixbuf_get_width (p) >= gdk_screen_height ())
     {
      GdkPixbuf *newpb = gdk_pixbuf_new_from_file_at_size (instance->filename, 512, -1, NULL);
      gtk_image_set_from_pixbuf (instance->image, newpb);
      g_object_unref (newpb);
     }
  else
     gtk_image_set_from_pixbuf (instance->image, p);

  g_object_unref (p);

  image_viewer_size_normalize (instance);

  gchar *title = g_path_get_basename (instance->filename);
  gtk_window_set_title (instance->wnd_image, title);
  g_free (title);
}

static void image_zoom_to_fit_screen (t_image_viewer *instance)
{
  if (! instance)
     return;

/* Needs to take into account a provision for top and bottom panels
(tiny ones: 2 * 25 pixels + Apple's one: 24 pixels
 + the window frame = 24 pixels + the menu = 24 pixels, total = 122 pixels)
 Seems to need 5 pixels more */
#ifdef DARWIN
  GdkPixbuf *p = gdk_pixbuf_new_from_file_at_size (instance->filename, gdk_screen_width (), gdk_screen_height () - 122, NULL);
#else
  GdkPixbuf *p = gdk_pixbuf_new_from_file_at_size (instance->filename, gdk_screen_width (), gdk_screen_height (), NULL);
#endif
  if (! p)
     return;

  gtk_image_set_from_pixbuf (instance->image, p);
  g_object_unref (p);
  image_viewer_size_normalize (instance);
}


static void image_zoom_in (t_image_viewer *instance, gint pixels)
{
  if (! instance)
     return;

  GdkPixbuf *x = gtk_image_get_pixbuf (instance->image);

  /* Need to test if there is an error, aka the pixbuf cannot get the
   width or the height of the image when previously incrementing
  beyond the available viewport. */
  if (! x)
     return;

  gint w = gdk_pixbuf_get_width (x);
  gint h = gdk_pixbuf_get_height (x);
  
 /* Scaling with h = -1 does not work (at least GTK 2.6 on Mac),
  so force the minimum width to 10 pixels to take decorations
  into account, rescaling the height proportionnally.
  Test also that the size does not exceed the visible screen limits,
  minus the panels.
  Since w is defined and ratio is preserved, it works as expected. */

 /* Needs to calculate the minimum resulting scaling: 10 - 5 = 5 */
  if (w <= 5 || h <= 5)
     {
      gdouble scale_min_w = (gdouble) ((gdouble) 5.0 / (gdouble) w);
      gdouble scale_min_h =  (gdouble) ((gdouble) 5.0 / (gdouble) h);
      gdouble min_scale = MAX (scale_min_w, scale_min_h);
      w = (gint) ((gdouble) w * min_scale);
      h = (gint) ((gdouble) h * min_scale);
     }

  /* Needs to test the max visible width and height,
    then scale to fit the screen, minus a provision for top and
    bottom panels (tiny ones: 2 * 25 pixels) + the Apple's X11 top menu
    (24 pixels) + the window frame (24 pixels) + the menu = 24 pixels,
    total= 122 pixels, and
    9 pixels for the close button, preserving aspect ratio before
    applying the desired scaling. */

   /* Get the size of the screen */
   GdkScreen *screen = gtk_widget_get_screen (instance->wnd_image);
   if (! screen)
      return;

  gint screen_width_visible = gdk_screen_get_width (screen) - 9;
  gint screen_height_visible = gdk_screen_get_height (screen) - 122;

  if (w > (screen_width_visible - pixels) || h > (screen_height_visible - pixels))
    {
     gdouble scale_max_w = (gdouble) (screen_width_visible - pixels) / (gdouble) w;
     gdouble scale_max_h = (gdouble) (screen_height_visible - pixels) / (gdouble) h;
     gdouble max_scale = MIN (scale_max_w, scale_max_h);
     w = (gint) ((gdouble) w * max_scale);
     h = (gint) ((gdouble) h * max_scale);
    }

  GdkPixbuf *p = NULL;

#if ((GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >= 8))
  p = gdk_pixbuf_new_from_file_at_size (instance->filename, w + pixels, -1, NULL);
#else
  p = gdk_pixbuf_new_from_file_at_size (instance->filename, w + pixels, h + pixels, NULL);
#endif

  if (! p)
     return;

  gtk_image_set_from_pixbuf (instance->image, p);
  g_object_unref (p);
  image_viewer_size_normalize (instance);
}


static void image_zoom_out (t_image_viewer *instance, gint pixels)
{
  if (! instance)
     return;

  GdkPixbuf *x = gtk_image_get_pixbuf (instance->image);

  /* Need to test if there is an error, aka the pixbuf cannot get the
  width or the height of the image when previously decrementing
  beyond 0 pixel. */
  if (x)
     {
      gint w = 0;
      gint h = 0;

      w = gdk_pixbuf_get_width (x);
      h = gdk_pixbuf_get_height (x);

      /* Scaling with h = -1 does not work (at least GTK 2.6 on Mac),
      so force the minimum width to 9 pixels to take decorations
      into account, rescaling the height proportionnally.
      Test also that the size does not exceed the visible screen limits,
      minus the panels.
      Since w is defined and ratio is preserved, it works as expected. */

      /* Needs to calculate the minimum resulting scaling: 9 + 1 = 10 */
      if (w <= 10 || h <= 10)
         {
          gdouble scale_min_w = (gdouble) ((gdouble) 10.0 / (gdouble) w);
          gdouble scale_min_h =  (gdouble) ((gdouble) 10.0 / (gdouble) h);
          gdouble min_scale = MAX (scale_min_w, scale_min_h);
          w = (gint) ((gdouble) w * min_scale);
          h = (gint) ((gdouble) h * min_scale);
         }

      /* Needs to test the max visible width and height,
      then scale to fit the screen, minus a provision for top and bottom
      panels (tiny ones: 2 * 25 pixels) + the Apple's X11 top menu (24 pixels)
      + the window frame (24 pixels) + the menu (24 pixels) - total= 122 pixels,
      preserving aspect ratio before applying the desired scaling. */

       /* Get the size of the screen */
       GdkScreen *screen = gtk_widget_get_screen (instance->wnd_image);
       if (! screen)
           return;

       gint screen_width_visible = gdk_screen_get_width (screen);
       gint screen_height_visible = gdk_screen_get_height (screen) - 122;

       if (w > screen_width_visible || h > screen_height_visible)
           {
            gdouble scale_max_w = (gdouble) (screen_width_visible) / (gdouble) w;
            gdouble scale_max_h = (gdouble) (screen_height_visible) / (gdouble) h;
            gdouble max_scale = MIN (scale_max_w, scale_max_h);
            w = (gint) ((gdouble) w * max_scale);
            h = (gint) ((gdouble) h * max_scale);
            }

       GdkPixbuf *p = gdk_pixbuf_new_from_file_at_size (instance->filename, w - pixels, h - pixels, NULL);

       // Again need to test if there is an error
       if (p)
          {
           gtk_image_set_from_pixbuf (instance->image, p);
           g_object_unref (p);
           image_viewer_size_normalize (instance);
           }
       }
}


static void image_zoom_actual_size (t_image_viewer *instance)
{
  if (! instance)
     return;

  GdkPixbuf *p = gdk_pixbuf_new_from_file (instance->filename, NULL);
  if (! p )
     return;

  gtk_image_set_from_pixbuf (instance->image, p);
  g_object_unref (p);
  image_viewer_size_normalize (instance);
}


static void image_zoom_multiply (t_image_viewer *instance, gint multiplier)
{
  if (! instance)
     return;

  GdkPixbuf *x = gtk_image_get_pixbuf (instance->image);

  /* Need to test if there is an error, aka the pixbuf cannot get the
  width or the height of the image when previously incrementing beyond
  the available viewport. */
  if (! x)
    return;

  gint w = 0;
  gint h = 0;
  w = gdk_pixbuf_get_width (x);
  h = gdk_pixbuf_get_height (x);

  /* Scaling with h = -1 does not work (at least GTK 2.6 on Mac),
  so force the minimum width to 10 pixels to take decorations
  into account, rescaling the height proportionnally.
  Test also that the size does not exceed the visible screen limits,
  minus the panels.
  Since w is defined and ratio is preserved, it works as expected. */

   /* Needs to calculate the minimum resulting scaling: 10 / 2 = 5 */
   if (w <= 5 || h <= 5)
      {
       gdouble scale_min_w = (gdouble) ((gdouble) 5.0 / (gdouble) w);
       gdouble scale_min_h =  (gdouble) ((gdouble) 5.0 / (gdouble) h);
       gdouble min_scale = MAX (scale_min_w, scale_min_h);
       w = (gint) ((gdouble) w * min_scale);
       h = (gint) ((gdouble) h * min_scale);
       }

    /* Needs to test the max visible width and height,
   then scale to fit the screen, minus a provision for top and bottom
   panels (tiny ones: 2 * 25 pixels) + the Apple's X11 top menu (24 pixels)
   + the window frame (24 pixels) + the menu (24 pixels) - total= 122 pixels,
   preserving aspect ratio before applying the desired scaling. */

  GdkScreen *screen = gtk_widget_get_screen (instance->wnd_image);
  if (! screen)
     return;

  gint screen_width_visible = gdk_screen_get_width (screen);
  gint screen_height_visible = gdk_screen_get_height (screen) - 122;


  if (w > (screen_width_visible / multiplier) || h > (screen_height_visible / multiplier))
     {
      gdouble scale_max_w = ((gdouble) (screen_width_visible) / (gdouble) multiplier) / (gdouble) w;
      gdouble scale_max_h = ((gdouble) (screen_height_visible) / (gdouble) multiplier) / (gdouble) h;
      gdouble max_scale = MIN (scale_max_w, scale_max_h);
      w = (gint) ((gdouble) w * max_scale);
      h = (gint) ((gdouble) h * max_scale);
     }

   GdkPixbuf *p = gdk_pixbuf_new_from_file_at_size (instance->filename, (gint) ((gdouble) w * (gdouble) multiplier),
                                                    (gint) ((gdouble) h * (gdouble) multiplier), NULL);

   if (p)
      {
       gtk_image_set_from_pixbuf (instance->image, p);
       g_object_unref (p);
      }

  image_viewer_size_normalize (instance);
}


static void image_zoom_div (t_image_viewer *instance, gdouble divider)
{
  if (! instance)
     return;

  GdkPixbuf *x = gtk_image_get_pixbuf (instance->image);

  /* Need to test if there is an error, aka the pixbuf cannot get the
  width or the height of the image when previously decrementing beyond
  0 pixel. */

  if (x)
     {
      gint w=0;
      gint h=0;
      w = gdk_pixbuf_get_width (x);
      h = gdk_pixbuf_get_height (x);

      /* Scaling with h = -1 does not work (at least GTK 2.6 on Mac),
      so force the minimum width to 10 pixels to take decorations
      into account, rescaling the height proportionnally.
      Test also that the size does not exceed the visible screen limits,
      minus the panels.
      Since w is defined and ratio is preserved, it works as expected. */

      /* Needs to calculate the minimum resulting scaling: 14 * 0,75 = 10 */
      if (w <= 14 || h <= 14)
         {
          gdouble scale_min_w = (gdouble) ((gdouble) 14.0 / (gdouble) w);
          gdouble scale_min_h =  (gdouble) ((gdouble) 14.0 / (gdouble) h);
          gdouble min_scale = MAX (scale_min_w, scale_min_h);
          w = (gint) ((gdouble) w * min_scale);
          h = (gint) ((gdouble) h * min_scale);
         }

       /* Needs to test the max visible width and height,
       then scale to fit the screen, minus a provision for top and bottom
       panels (tiny ones: 2 * 25 pixels) + the Apple's X11 top menu (24 pixels)
       + the window frame (24 pixels) + the menu (24 pixels) - total= 122 pixels,
       preserving aspect ratio before applying the desired scaling. */

       GdkScreen *screen = gtk_widget_get_screen (instance->wnd_image);
       if (! screen)
          return;

       gint screen_width_visible = gdk_screen_get_width (screen);
       gint screen_height_visible = gdk_screen_get_height (screen) - 122;


       if (w > (screen_width_visible * divider) || h > (screen_height_visible * divider))
          {
           gdouble scale_max_w = ((gdouble) (screen_width_visible) * (gdouble) divider) / (gdouble) w;
           gdouble scale_max_h = ((gdouble) (screen_height_visible) * (gdouble) divider) / (gdouble) h;
           gdouble max_scale = MIN (scale_max_w, scale_max_h);
           w = (gint) ((gdouble) w * max_scale);
           h = (gint) ((gdouble) h * max_scale);
          }

       GdkPixbuf *p = gdk_pixbuf_new_from_file_at_size (instance->filename,
                                                        (gint) ((gdouble) w * (gdouble) divider),
                                                        (gint) ((gdouble) h * (gdouble) divider), NULL);

       if (p)
          {
           gtk_image_set_from_pixbuf (instance->image, p);
           g_object_unref (p);
           image_viewer_size_normalize (instance);
          }
      }
}


void image_plane_free (t_image_plane *instance)
{
  glist_strings_free (instance->gl_thumbs);
  g_free (instance->directory);
  g_free (instance);
}

/*
other values for GNOME:

"wallpaper" - tiled
"stretched"
"none"
*/

static void on_mni_wallp_centered (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  if (get_desktop_name () == DSK_KDE)
      image_viewer_set_as_wallpaper_kde (user_data, 1);
  else
      if (get_desktop_name () == DSK_GNOME)
         set_wallpaper_gnome (user_data, "centered");
}


static void on_mni_wallp_scaled (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  if (get_desktop_name () == DSK_KDE)
     image_viewer_set_as_wallpaper_kde (user_data, 6);
  else
      if (get_desktop_name () == DSK_GNOME)
         set_wallpaper_gnome (user_data, "scaled");
}


static void on_mni_zoom_in (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  image_zoom_in (user_data, 5);
}


static void on_mni_zoom_out (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  image_zoom_out (user_data, 1);
}


static void on_mni_zoom_to_fit_screen (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  image_zoom_to_fit_screen (user_data);
}


static void on_mni_zoom_actual_size (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  image_zoom_actual_size (user_data);
}


static void on_mni_zoom_mult (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  image_zoom_multiply (user_data, 2);
}


static void on_mni_zoom_div (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  image_zoom_div (user_data, 0.75);
}


static void on_mni_toggle_fullscreen (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  image_viewer_toggle_fullscreen (user_data);
}


static void on_mni_ins_img (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  t_image_viewer *p = user_data;
  insert_image (p->filename, 0, 0, TRUE);
}


static void on_mni_ins_img_with_dimensions (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
  t_image_viewer *instance = user_data;
  insert_image (instance->filename, gdk_pixbuf_get_width (gtk_image_get_pixbuf (instance->image)),
                gdk_pixbuf_get_height (gtk_image_get_pixbuf (instance->image)), FALSE);
}


static void on_mni_copy_pixbuf (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  GtkClipboard *c = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (! c)
     return NULL;

  t_image_viewer *instance = user_data;
  gtk_clipboard_set_image (c, gtk_image_get_pixbuf (instance->image));
}


static void on_mni_load_next (GtkMenuItem *menuitem, gpointer user_data)
{
  image_load_next (user_data, TRUE);
}

static void on_mni_load_previous (GtkMenuItem *menuitem, gpointer user_data)
{
  image_load_next (user_data, FALSE);
}

static void create_menu (GtkWidget *menu, t_image_viewer *instance)
{
  GtkWidget *mni = new_menu_tof (menu);
  mni = new_menu_item_with_udata (gettext("Slow zoom in (+)"), menu, on_mni_zoom_in, instance);
  mni = new_menu_item_with_udata (gettext("Slow zoom out (-)"), menu, on_mni_zoom_out, instance);
  mni = new_menu_item_with_udata (gettext("Actual size (=)"), menu, on_mni_zoom_actual_size, instance);
  mni = new_menu_item_with_udata (gettext("Zoom to fit screen (S)"), menu, on_mni_zoom_to_fit_screen, instance);
  mni = new_menu_item_with_udata (gettext("Fast zoom in (*)"), menu, on_mni_zoom_mult, instance);
  mni = new_menu_item_with_udata (gettext("Fast zoom out (/)"), menu, on_mni_zoom_div, instance);
  mni = new_menu_sep (menu);

#ifndef DARWIN
  mni = new_menu_item_with_udata (gettext("Set image as wallpaper centered"), menu, on_mni_wallp_centered, instance);
  mni = new_menu_item_with_udata (gettext("Set image as wallpaper scaled"), menu, on_mni_wallp_scaled, instance);
#endif

  mni = new_menu_item_with_udata (gettext("Toggle fullscreen (F)"), menu, on_mni_toggle_fullscreen, instance);
  mni = new_menu_item_with_udata (gettext("Insert this image into the current document (Ins or I)"), menu, on_mni_ins_img, instance);
  mni = new_menu_item_with_udata (gettext("Insert this image into the current document //with the current size (D)"), menu, on_mni_ins_img_with_dimensions, instance);

  mni = new_menu_item_with_udata (gettext("Copy this image to clipboard"), menu, on_mni_copy_pixbuf, instance);

  mni = new_menu_item_with_udata (gettext("Load next image (PgDown)"), menu, on_mni_load_next, instance);
  mni = new_menu_item_with_udata (gettext("Load previous image (PgUp)"), menu, on_mni_load_previous, instance);
}


static void do_popup_menu (GtkWidget *widget, GdkEventButton *event, t_image_viewer *instance)
{
  GtkWidget *menu = gtk_menu_new ();
  create_menu (menu, instance);
  //FIXME: to destroy or not to destroy? That is the question.
  //g_signal_connect (menu, "deactivate", G_CALLBACK (gtk_widget_destroy), NULL);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                  0, gtk_get_current_event_time ());
}

//now playing: Slipknot - IWOA - IWOA
static gboolean on_viewer_button_press_event_handler (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
    {
      do_popup_menu (widget, event, user_data);
      return TRUE;
    }

  return FALSE;
}


static gboolean on_viewer_key_press_event (GtkWidget *widget,
                                           GdkEventKey *event,
                                           gpointer user_data)
{
  t_image_viewer *instance = user_data;

  if (event->keyval == GDK_KEY_Escape)
    {
     image_viewer_free (user_data);
     gtk_widget_destroy (widget);
     return TRUE;
    }

  if (event->keyval == GDK_KEY_KP_Add)
     image_zoom_in (instance, 5);

  if (event->keyval == GDK_KEY_KP_Subtract)
    image_zoom_out (instance, 1);

  if (event->keyval == GDK_KEY_KP_Multiply)
     image_zoom_multiply (instance, 2);

  if (event->keyval == GDK_KEY_KP_Divide)
     image_zoom_div (instance, 0.75);

  if (event->keyval == GDK_KEY_KP_Equal)
     image_zoom_actual_size (instance);

  if (event->keyval == GDK_KEY_Page_Down)
     image_load_next (instance, TRUE);

  if (event->keyval == GDK_KEY_Page_Up)
     image_load_next (instance, FALSE);

  if (event->keyval == GDK_KEY_F || event->keyval == GDK_KEY_f)
     image_viewer_toggle_fullscreen (instance);

  if (event->keyval == GDK_KEY_S || event->keyval == GDK_KEY_s)
     image_zoom_to_fit_screen (instance);

/* No INS key on Apple keyboard, so provide an alternative key */
  if (event->keyval == GDK_KEY_Insert || event->keyval == GDK_KEY_I || event->keyval == GDK_KEY_i)
     insert_image (instance->filename, 0, 0, TRUE);

  if (event->keyval == GDK_KEY_D || event->keyval == GDK_KEY_d)
     insert_image (instance->filename, gdk_pixbuf_get_width (gtk_image_get_pixbuf (instance->image)),
                   gdk_pixbuf_get_height (gtk_image_get_pixbuf (instance->image)), FALSE);

  return FALSE;
}


static gint button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  t_image_plane *instance = user_data;

  if (event->button == 1)
     if (event->type == GDK_2BUTTON_PRESS)
        {
         GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
         GtkTreeModel *model;
         GtkTreeIter iter;

         if (gtk_tree_selection_get_selected (sl, &model, &iter))
            {
             gchar *name;
             gtk_tree_model_get (model, &iter, COL_TEXT_CELL, &name, -1);

             if (! name)
                return FALSE;

             gchar *f = g_strconcat (instance->directory, G_DIR_SEPARATOR_S, name, NULL);

             if (gtk_toggle_button_get_active (&cb_imgplane_mode_sw->toggle_button))
                insert_image (f, 0, 0, TRUE);
             else
                 {
                  t_image_viewer *v = image_viewer_create (f);
                 }

             g_free (f);
             g_free (name);
             return TRUE;
            }
        }

  return FALSE;
}


static gboolean on_image_viewer_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  image_viewer_free (user_data);
  return FALSE;
}


t_image_viewer* image_viewer_create (const gchar *filename)
{
  if (! filename)
     return NULL;

  t_image_viewer *instance =  g_malloc (sizeof (t_image_viewer));
  instance->filename = g_strdup (filename);
  instance->wnd_image = create_wnd_imgviewer (instance);

#ifndef DARWIN

  g_signal_connect (instance->wnd_image, "button_press_event",
                    G_CALLBACK (on_viewer_button_press_event_handler),
                    instance);
#endif

  g_signal_connect (instance->wnd_image, "key_press_event",
                    G_CALLBACK (on_viewer_key_press_event),
                    instance);

  g_signal_connect (instance->wnd_image, "delete_event",
                    G_CALLBACK (on_image_viewer_delete_event),
                    instance);

////
  gchar *dir = g_path_get_dirname (instance->filename);
  GList *x = read_dir_to_glist (dir);
  g_free (dir);
  if (! x)
     return instance;

  instance->files_list = glist_copy_with_filter (x, is_image);

  instance->files_list_current = g_list_first (instance->files_list);

  glist_strings_free (x);
  return instance;
}


void image_viewer_free (t_image_viewer *instance)
{
  glist_strings_free (instance->files_list);
  g_free (instance->filename);
  g_free (instance);
}


GtkWidget* create_wnd_imgviewer (t_image_viewer *instance)
{
  if (! instance)
     return NULL;

  if (! g_file_test (instance->filename, G_FILE_TEST_EXISTS))
     return NULL;

  GtkWidget *wnd_imgviewer = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *vbox = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox);

  gtk_container_add (GTK_CONTAINER (wnd_imgviewer), vbox);

  instance->event_box = gtk_event_box_new ();
  instance->fullscreen = FALSE;

#ifdef DARWIN

  GtkWidget *menu = gtk_menu_bar_new ();
  gtk_widget_show (menu);

  GtkWidget *mt = new_menu_item (gettext("Functions"), menu, NULL);
  GtkWidget *mtm = new_menu_submenu (mt);

  create_menu (mtm, instance);
  gtk_box_pack_start (GTK_BOX (vbox), menu, FALSE, FALSE, UI_PACKFACTOR);

#endif

  gtk_widget_show (instance->event_box);
  gtk_box_pack_start (GTK_BOX (vbox), instance->event_box, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *image = gtk_image_new_from_file (instance->filename);

  GdkPixbuf *pb = gtk_image_get_pixbuf (image);
  if (gdk_pixbuf_get_width (pb) >= gdk_screen_width () || gdk_pixbuf_get_width (pb) >= gdk_screen_height ())
     {
      GdkPixbuf *newpb = gdk_pixbuf_new_from_file_at_size (instance->filename, 512, -1, NULL);
      gtk_image_set_from_pixbuf (image, newpb);
      g_object_unref (newpb);
     }

  gtk_container_add (GTK_CONTAINER (instance->event_box), image);
  gtk_widget_show (image);
  instance->image = image;

  gtk_widget_show (wnd_imgviewer);
  gchar *t = g_path_get_basename (instance->filename);
  gtk_window_set_title (wnd_imgviewer, t);
  g_free (t);

  return wnd_imgviewer;
}

static gboolean on_imageplane_key_press_event (GtkWidget *widget,
                                               GdkEventKey *event,
                                               gpointer user_data)
{
  if (event->keyval == GDK_KEY_Escape)
    {
     image_plane_free (user_data);
     gtk_widget_destroy (widget);
     return TRUE;
    }

  return FALSE;
}


static gboolean on_imageplane_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  image_plane_free (user_data);
  return FALSE;
}


t_image_plane* create_image_plane (const gchar *dir)
{
  t_image_plane *instance = g_malloc (sizeof (t_image_plane));
  instance->gl_thumbs = NULL;

  instance->directory = g_strdup (dir);
  instance->wnd_imageplane = create_image_browser (instance);

  g_signal_connect (instance->wnd_imageplane, "key_press_event",
                    G_CALLBACK (on_imageplane_key_press_event),
                    instance);

  g_signal_connect (instance->wnd_imageplane, "delete_event",
                    G_CALLBACK (on_imageplane_delete_event),
                    instance);
}


GtkWidget* create_image_browser (t_image_plane *instance)
{
  if (! instance)
     return NULL;

  GtkWidget *tvb;
  gchar *filename;
  gchar *utf;

  GdkPixbuf *pixbuf;
  guchar digest[16];
  gchar *thumb_filename;
  gchar *md5sum = NULL;
  gchar *uri = NULL;
  gdouble val;

  GtkWidget *wnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (wnd, "Imageplane");

//  g_signal_connect (G_OBJECT (wnd), "key_press_event", G_CALLBACK (win_key_handler), wnd);

  //gtk_window_set_transient_for (wnd, tea_main_window);
  gtk_window_set_default_size (wnd, -1, gdk_screen_height ());
  gtk_window_move (wnd, 1, 1);
  gtk_widget_show (wnd);

  GtkWidget *vbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (wnd), vbox1);

  cb_imgplane_mode_sw = tea_checkbox (vbox1, gettext("Insert tags"), confile.implane_ins_image);

  /*
  gtk_tooltips_set_tip (tooltips,
                        cb_imgplane_mode_sw,
                        gettext("Insert the tag rather than view image"),
                        NULL);
*/
  
  gtk_widget_set_tooltip_text (cb_imgplane_mode_sw, gettext("Insert the tag rather than view image"));       
  
  instance->tree_view = tv_create_tree (vbox1, GTK_SELECTION_SINGLE);

  instance->pb = gtk_progress_bar_new ();
  gtk_widget_show (instance->pb);
  gtk_box_pack_start (vbox1, instance->pb, FALSE, FALSE, UI_PACKFACTOR);
  image_browser_update (instance);

  g_signal_connect ((gpointer) (instance->tree_view), "button_press_event",
                   /*(GtkSignalFunc)*/ button_press_event, instance);

  return wnd;
}


void image_browser_update (t_image_plane *instance)
{
  if (! instance)
     return NULL;

  gchar *filename;
  gchar *utf;

  GdkPixbuf *pixbuf;
  guchar digest[16];
  gchar *thumb_filename;
  gchar *md5sum = NULL;
  gchar *uri = NULL;
  gdouble val;

  glist_strings_free (instance->gl_thumbs);
  instance->gl_thumbs = read_dir_to_glist (instance->directory);
  gint len;
  gint c = 0;
  if (instance->gl_thumbs)
     {
      GtkListStore *liststore = gtk_tree_view_get_model (GTK_TREE_VIEW (instance->tree_view));

      if (liststore)
        gtk_list_store_clear (liststore);

      len = g_list_length (instance->gl_thumbs) - 1;

      GList *t = g_list_first (instance->gl_thumbs);
      while (t)
           {
            if (is_image (t->data))
               {
                while (gtk_events_pending ())
                    gtk_main_iteration ();

                val = ((gdouble)++c / (gdouble)(len - 0.999));
                if (val > 1.0)
                   val = 0.0;
                gtk_progress_bar_set_fraction (instance->pb, val);

                uri = g_strconcat ("file://", t->data, NULL);
                md5_get_digest (uri, strlen (uri), digest);

                md5sum = md5_digest_to_text (digest);

                if (md5sum)
                  {
                   thumb_filename = g_strconcat (confile.dir_thumbs, G_DIR_SEPARATOR_S, md5sum, ".png", NULL);
                   if (g_file_test (thumb_filename, G_FILE_TEST_EXISTS))
                       pixbuf = gdk_pixbuf_new_from_file (thumb_filename, NULL);
                   else
                       pixbuf = gdk_pixbuf_new_from_file_at_size (t->data, confile.thumb_width, confile.thumb_height, NULL);
                  }
                else
                    pixbuf = gdk_pixbuf_new_from_file_at_size (t->data, confile.thumb_width, confile.thumb_height, NULL);

                g_free (uri);
                g_free (md5sum);
                g_free (thumb_filename);

                filename = g_path_get_basename (t->data);
                utf = get_8_filename (filename);

                tv_add_new_item_img (instance->tree_view, utf, pixbuf);

                g_free (filename);
                g_free (utf);
               }

             t = g_list_next (t);
            }
     }

  gtk_progress_bar_set_fraction (instance->pb, 0);
}
