/***************************************************************************
                          free_desktop_support.c  -  description
                             -------------------
    begin                : 09.02.2005
    copyleft            : 2005-2013 by Peter Semiletov
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


#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h> // needed for unlink

#include <glib.h>

#include "tea_defs.h"
#include "rox_strings.h" // Needed for glist_save_to_file
#include "free_desktop_support.h"


void free_desktop_create_shortcut (void)
{
  gchar *desktop_file = g_strconcat (g_getenv ("HOME"), "/Desktop/teagtk.desktop", NULL);
  gchar *icon_file = g_strconcat ("Icon=", TEA_PIX_MAINICON, NULL);

  GList *l = NULL;

  l = g_list_append (l, "[Desktop Entry]");
  l = g_list_append (l, "Comment=punk is not dead");
  l = g_list_append (l, "Comment[ru]=punk is not dead");
  l = g_list_append (l, "Encoding=UTF-8");
  l = g_list_append (l, TEA_RUN_CMD);
  l = g_list_append (l, "GenericName=TEA - a mighty text editor");
  l = g_list_append (l, "GenericName[ru]=TEA - a mighty text editor");
  l = g_list_append (l, "GenericName[fr]=TEA - un fabuleux éditeur de texte");
  l = g_list_append (l, icon_file);
  l = g_list_append (l, "MimeType=text/html;text/plain");
  l = g_list_append (l, "Name=teagtk");
  l = g_list_append (l, "Name[ru]=teagtk");
  l = g_list_append (l, "Name[fr]=teagtk");
  l = g_list_append (l, "Path=");
  //l = g_list_append (l, "Categories=TextEditor;WordProcessor;GTK;");
  l = g_list_append (l, "StartupNotify=true");
  l = g_list_append (l, "Terminal=false");
  l = g_list_append (l, "TerminalOptions=");
  l = g_list_append (l, "Type=Application");
  l = g_list_append (l, "X-DCOP-ServiceType=");
  l = g_list_append (l, "X-KDE-SubstituteUID=false");
  l = g_list_append (l, "X-KDE-Username=");

  glist_save_to_file (l, desktop_file);

  g_list_free (l);
  g_free (icon_file);
  g_free (desktop_file);
}


void free_desktop_delete_shortcut (void)
{
  if (! g_getenv ("HOME"))
     return;

  gchar *desktop_file = g_strconcat (g_getenv ("HOME"), "/Desktop/teagtk.desktop", NULL);
  unlink (desktop_file);
  g_free (desktop_file);
}
