/***************************************************************************
                          tea_kwas.c  -  description
                             -------------------
    begin                : 11.06.2004 - it was a really good day
    copyright            : (C) 2004-2007 by Peter 'Roxton' Semiletov
    email                : peter.semiletov@gmail.com
 ***************************************************************************/
/*
portions of code from:
Michele Garoche <michele.garoche@easyconnect.fr>

** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** filesops.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
**                Michael Terry <mterry@fastmail.fm>
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
#include <glib/gi18n.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


#include "tea_defs.h"
#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "tea_funx.h"
#include "rox_strings.h"
#include "interface.h"
#include "tea_config.h"
#include "tea_kwas.h"
#include "tea_gtk_utils.h"
#include "image_browser.h"

#ifdef CHAI_MODE

#include <libgnomevfs/gnome-vfs.h>

#endif


enum
{
  COL_TEXT = 0,
  COL_ACCESS,
  COL_SIZE,
  COL_DATETIME,
  NUM_COLS
};


typedef struct
              {
               struct stat *fstat;
               gchar *path;
               gchar *access;
               gchar *file_size;
               gchar *file_time;
              }
               t_flist_item;


//from Beaver's an Early AdVanced EditoR fileop.c
/* Return the rwx permissions of a file in a string */
gchar* get_file_mode (struct stat *Stats)
{
  gchar Mode[10];

  g_snprintf (Mode, 10, "%c%c%c%c%c%c%c%c%c",
	      (Stats->st_mode & S_IRUSR) ? 'r' : '-',
	      (Stats->st_mode & S_IWUSR) ? 'w' : '-',
	      (Stats->st_mode & S_IXUSR) ? 'x' : '-',
	      (Stats->st_mode & S_IRGRP) ? 'r' : '-',
	      (Stats->st_mode & S_IWGRP) ? 'w' : '-',
	      (Stats->st_mode & S_IXGRP) ? 'x' : '-',
	      (Stats->st_mode & S_IROTH) ? 'r' : '-',
	      (Stats->st_mode & S_IWOTH) ? 'w' : '-',
	      (Stats->st_mode & S_IXOTH) ? 'x' : '-');
  return g_strdup (Mode);
}


static void kwas_navigate (t_kwas_win *w, const gchar *path)
{
  if (w->focused_panel)
     {
      gchar *dir = filename_from_xuri (path);
      g_free (w->focused_panel->path);
      w->focused_panel->path = dir;
      fill_filelist (w->focused_panel, dir);
     }
}


static void on_mni_kwas_imageplane (GtkButton *button,
                                    gpointer user_data)
{
  t_kwas_win *w = user_data;
  t_image_plane *plane = create_image_plane (w->focused_panel->path);
}


static void on_mni_kwas_open_bookmark (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  kwas_navigate (user_data, gtk_widget_get_name (menuitem));
}


static void kwas_bookmarks_menu_update (t_kwas_win *w)
{
  if (! gl_tea_kwas_bookmarks)
     return;

  glist_strings_free (gl_tea_kwas_bookmarks);
  gl_tea_kwas_bookmarks = load_file_to_glist (confile.tea_kwas_bookmarks);

  gtk_widget_destroy (w->mni_bookmarks_menu);
  w->mni_bookmarks_menu = new_menu_submenu (w->mni_bookmarks);

  GList *p = g_list_first (gl_tea_kwas_bookmarks);

  while (p)
       {
        if (p->data)
           if (strlen (p->data) > 1)
              mni_temp = new_menu_item_with_udata (p->data, w->mni_bookmarks_menu, on_mni_kwas_open_bookmark, w);

        p = g_list_next (p);
       }
}


static gboolean on_ent_nav_key_press_event (GtkWidget *widget,
                                            GdkEventKey *event,
                                            gpointer user_data)
{
  t_kwas_win *w = user_data;
  if (! w)
      return FALSE;

  if (event->keyval != GDK_KEY_Return)
     return FALSE;

  gchar *s = gtk_entry_get_text (w->cm_entry);

  if (access (s, R_OK) == 0)
    {
     g_free (w->focused_panel->path);
     w->focused_panel->path = g_strdup (s);
     fill_filelist (w->focused_panel, s);
    }

  return TRUE;
}


static void on_execute_button (GtkButton *button,
                               gpointer user_data)
{
  t_kwas_win *w = user_data;
  if (! w)
      return;

  gchar *s = gtk_entry_get_text (w->cm_entry);
  if (! s)
     return;

  if (access (s, R_OK) == 0)
      kwas_navigate (w, s);
}


static void on_mni_kwas_insert_link (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  if (! get_page_text ())
     return;

  if (! cur_text_doc->b_saved)
     return;

  gchar *f = kwas_get_current_filename (user_data);
  if (! f)
     return;

  insert_link (f);
  g_free (f);
}


static void on_mni_kwas_insert_image (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  if (! get_page_text ())
     return;

  if (! cur_text_doc->b_saved)
     return;

  gchar *f = kwas_get_current_filename (user_data);
  if (! f)
     return;

  insert_image (f, 0, 0, TRUE);
  g_free (f);
}


static void on_mni_kwas_run_with (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  t_kwas_win *w = user_data;
  gchar *f = kwas_get_current_filename (w);

  if (! f)
     return;

  gchar *t = g_strconcat (gtk_entry_get_text (w->cm_entry), " ", f, "&", NULL);
  system (t);

  g_free (f);
  g_free (t);
}


static void on_mni_kwas_copy_filename (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  GtkClipboard *c = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (! c)
     return;

  gchar *f = kwas_get_current_filename (user_data);

  if (! f)
     return;

  gtk_clipboard_set_text (c, f, -1);

  g_free (f);
}


//I am not sure - maybe better to move it into destroy_event?
static gboolean on_kwas_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  free_kwas_window (user_data);
  return FALSE;
}


static void kwas_log_to_memo (t_kwas_win *win, const gchar *m1, const gchar* m2, gint mode)
{
  if (! win->log_memo)
     return;

  GtkTextBuffer *b = gtk_text_view_get_buffer (win->log_memo);
  GtkTextIter it;

  if ((++win->log_to_memo_counter) == confile.logmemo_lines_max)
     {
      win->log_to_memo_counter = 0;
      GtkTextIter itstart, itend;
      gtk_text_buffer_get_bounds (b, &itstart, &itend);
      gtk_text_buffer_delete (b, &itstart, &itend);
     }

  if (confile.msg_counter == G_MAXINT)
     confile.msg_counter = 0;

  gchar *prefix = g_strdup_printf  ("(%d) ", ++win->msg_counter);
  gchar *st;

  if (! m2)
     {
      st = g_strconcat (prefix, m1, "\n", NULL);

      gtk_text_buffer_get_iter_at_offset (b, &it, 0);
      gtk_text_buffer_place_cursor (b, &it);

      if (mode == LM_NORMAL)
          gtk_text_buffer_insert_with_tags_by_name (b, &it, st, -1, "lm_normal", NULL);
      else
          if (mode == LM_ERROR)
             gtk_text_buffer_insert_with_tags_by_name (b, &it, st, -1, "lm_error", NULL);
      else
          if (mode == LM_ADVICE)
             gtk_text_buffer_insert_with_tags_by_name (b, &it, st, -1, "lm_advice", NULL);
      else
          if (mode == LM_GREET)
             gtk_text_buffer_insert_with_tags_by_name (b, &it, st, -1, "lm_greet", NULL);

      g_free (st);
      g_free (prefix);
      logmemo_set_pos (win->log_memo, 0);

      return;
     }

  gulong len = strlen (m1) + strlen (m2) + 2024;
  gchar *message = g_malloc (len);

  g_snprintf (message, len, m1, m2);
  gchar *s = g_strconcat (prefix, message, "\n", NULL);

  gtk_text_buffer_insert_at_cursor (b, s, -1);

  g_free (message);
  g_free (s);
  g_free (prefix);
  logmemo_set_pos (win->log_memo, 0);

  return;
}


static void on_mni_kwas_refresh (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  t_kwas_win *w = user_data;

  if (! w)
     return;

  if (! w->focused_panel)
     return;

  fill_filelist (w->focused_panel, w->focused_panel->path);
}


static void on_mni_kwas_goto_homedir (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  t_kwas_win *w = user_data;

  if (! w)
     return;

  if (! w->focused_panel)
     return;

  fill_filelist (w->focused_panel, g_get_home_dir ());
}


static void on_mni_kwas_get_file_info (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  gchar *f = kwas_get_current_filename (user_data);

  if (! f)
     return;

  gchar *t = g_strconcat ("file ", f, " > ", confile.tmp_file, NULL);
  system (t);
  gchar *s = get_tmp_file_string ();
  if (s)
     kwas_log_to_memo (user_data, "%s", g_strstrip (s), LM_NORMAL);

  g_free (s);
  g_free (f);
  g_free (t);
}


static struct stat* get_stat (const gchar *filename)
{
  struct stat *s = g_malloc (sizeof (struct stat));
  stat (filename, s);
  return s;
}


static void statusbar_message (t_kwas_win *w, const gchar *s)
{
  if (! w->status_bar)
     return;

  if (s)
     gtk_statusbar_push (w->status_bar,
                         gtk_statusbar_get_context_id (w->status_bar, s),
                         s);
}


static void wnd_caption (t_kwas_win *w, gchar *s)
{
  if (! w->window)
     return;

  if (s)
     gtk_window_set_title (w->window, s);
}


static void filelist_add_new_item (t_kwas_panel *p, const gchar *s)
{
  if (! s)
     return;

  GtkTreeModel *model;
  GtkTreeIter newrow;
  t_flist_item *t = g_hash_table_lookup (p->hash_filelist, s);
  if (! t)
     return;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (p->tree_view));

  gtk_list_store_append (GTK_LIST_STORE (model), &newrow);

  gtk_list_store_set (GTK_LIST_STORE (model), &newrow, COL_TEXT, s, COL_ACCESS, t->access,
                         COL_SIZE, t->file_size, COL_DATETIME, t->file_time, -1);

}


static t_flist_item* fli_new (const gchar *path, const gchar *filename)
{
  t_flist_item* tfl = g_malloc (sizeof (t_flist_item));

  tfl->path = g_strconcat (path, G_DIR_SEPARATOR_S, filename, NULL);
  tfl->fstat = get_stat (tfl->path);

#ifndef CHAI_MODE

#if defined(DARWIN)
  if (sizeof (off_t) == 8)
      tfl->file_size = g_strdup_printf ("%-10llu", tfl->fstat->st_size);
  else
      tfl->file_size = g_strdup_printf ("%-10lu"), (tfl->fstat->st_size);
#else

  gint size = tfl->fstat->st_size;

  if (size < 1024)
     tfl->file_size = g_strdup_printf ("%-10d", tfl->fstat->st_size);
  else
     tfl->file_size = g_strdup_printf (_("%-10d Kb"), (tfl->fstat->st_size / 1024));
#endif

#else

  GnomeVFSFileInfo *info;
  GnomeVFSResult res;
  info = gnome_vfs_file_info_new ();
  res = gnome_vfs_get_file_info (tfl->path, info, (GNOME_VFS_FILE_INFO_DEFAULT | GNOME_VFS_FILE_INFO_FOLLOW_LINKS));

  if (res == GNOME_VFS_OK)
     tfl->file_size = gnome_vfs_format_file_size_for_display (info->size);
  else
      tfl->file_size = g_strdup ("?");

  gnome_vfs_file_info_unref (info);

#endif

  struct tm *lt = localtime (&tfl->fstat->st_mtime);

  tfl->file_time = g_strdup (asctime (lt));
  tfl->access = get_file_mode (tfl->fstat);

  return tfl;
}


static void fli_free (t_flist_item* fli)
{
   g_free (fli->path);
   g_free (fli->file_size);
   g_free (fli->file_time);
   g_free (fli->fstat);
   g_free (fli->access);
   g_free (fli);
}


void fill_filelist (t_kwas_panel *p, const gchar *path)
{
  if (! p->tree_view)
     return;

  GtkListStore *liststore = gtk_tree_view_get_model (GTK_TREE_VIEW (p->tree_view));

  if (liststore)
     gtk_list_store_clear (liststore);

  GDir *dir = g_dir_open (path, 0, NULL);

  if (! dir)
     return;

  GList *l_dirs = NULL;
  GList *l_files = NULL;

  gchar *t;
  gchar *z;
  gchar *x;
  gchar *y;

  t_flist_item *tfl;

  if (p->hash_filelist)
     g_hash_table_destroy (p->hash_filelist);

  p->hash_filelist = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, fli_free);

  while (t = g_dir_read_name (dir))
        {
         if (! confile.show_hidden_files)
         if (t[0] == '.')
            continue;

         z = g_strconcat (path, G_DIR_SEPARATOR_S, t, NULL); //full localized path

         y = get_8_filename (t); //filename in UTF-8
         if (! y)
            continue;

         if (g_file_test (z, G_FILE_TEST_IS_DIR))
            {
             x = g_strconcat (G_DIR_SEPARATOR_S, y, NULL); //dirname in utf-8

             l_dirs = g_list_prepend (l_dirs, x);
             g_hash_table_insert (p->hash_filelist,
                                  g_strdup (x),
                                  fli_new (path, t));

             g_free (y);
            }
         else
             {
              l_files = g_list_prepend (l_files, y);
              g_hash_table_insert  (p->hash_filelist,
                                    g_strdup (y),
                                    fli_new (path, t));
              }

        g_free (z);
       }

  l_dirs = g_list_prepend (sort_list_case_insensetive (l_dirs), g_strdup (".."));
  g_hash_table_insert (p->hash_filelist,
                       g_strdup (".."),
                       fli_new (path, ".."));

  GList *tl = g_list_first (l_dirs);

  while (tl)
        {
         filelist_add_new_item (p, tl->data);
         tl = g_list_next (tl);
        }

  tl = g_list_first (sort_list_case_insensetive (l_files));
  while (tl)
        {
         filelist_add_new_item (p, tl->data);
         tl = g_list_next (tl);
        }

  g_dir_close (dir);

  glist_strings_free (l_dirs);
  glist_strings_free (l_files);

  wnd_caption (p->kwas_win, p->path);
  statusbar_message (p->kwas_win, p->path);

  gtk_widget_grab_focus (p->tree_view);
}


static void dir_up (t_kwas_panel *p)
{
  gchar *x = g_strrstr (p->path, G_DIR_SEPARATOR_S);
  gchar *sel_dir = NULL;
  if (x)
     sel_dir = g_strdup (x);

  gchar *f = path_up (p->path);
  g_free (p->path);
  p->path  = f;
  fill_filelist (p, p->path);
  p->pos = 1;

  tv_sel_by_name (p->tree_view, sel_dir);
}


static void handle_enter (t_kwas_panel *p, const gchar *f)
{
  t_flist_item *ti = g_hash_table_lookup (p->hash_filelist, f);
  t_kwas_win *w = p->kwas_win;

  if (! ti)
     return;

  if (strstr (f, ".."))
     {
      dir_up (p);
      return;
     }

  if (S_ISDIR (ti->fstat->st_mode))
     {
      if (access (ti->path, R_OK) == -1)
         return;

      p->path = g_strdup (ti->path);
      fill_filelist (p, p->path);
      return;
     }

  if (S_ISREG (ti->fstat->st_mode))
     {
      handle_file (ti->path, combo_get_value (w->cb_charset, confile.iconv_encs), 0, TRUE);
      return;
     }
}


static gboolean on_treeview_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  t_kwas_panel *p = user_data;
  t_kwas_panel *op = p->opposite_panel;

  if (event->keyval == GDK_KEY_Tab)
     {
      gtk_widget_grab_focus (op->tree_view);
      return TRUE;
     }

  if (event->keyval == GDK_KEY_BackSpace)
     {
      dir_up (user_data);
      return TRUE;
     }

  if (event->keyval == GDK_KEY_Return)
     {
      GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
      GtkTreeModel *model;
      GtkTreeIter iter;

      if (gtk_tree_selection_get_selected (sl, &model, &iter))
         {
          gchar *name;

          gtk_tree_model_get (model, &iter, COL_TEXT, &name, -1);

          if (! name)
             return FALSE;

          handle_enter (user_data, name);

          g_free (name);
          return TRUE;
         }
     }

  return FALSE;
}


static void cell_data_func (GtkTreeViewColumn *col,
                            GtkCellRenderer *renderer,
                            GtkTreeModel *model,
                            GtkTreeIter *iter,
                            gpointer user_data)
{
  t_kwas_panel *p = user_data;

  if (! p)
     return;

  if (! p->hash_filelist)
     return;

  gchar *c1;
  gchar *c2;
  gchar *c3;
  gchar *c4;
  gchar *st;

  gtk_tree_model_get (model, iter, COL_TEXT, &c1, -1);

  t_flist_item *t = g_hash_table_lookup (p->hash_filelist, c1);

  if (! t)
     return;

  if (S_ISDIR (t->fstat->st_mode))
      g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
  else
      g_object_set (renderer, "weight", PANGO_WEIGHT_NORMAL, "weight-set", TRUE, NULL);

}


static gint widget_get_y (GtkWidget *widget, const gchar *s)
{
  PangoLayout *l;
  gint result = -1;
  l = gtk_widget_create_pango_layout (widget, s);
  if (l)
     {
      pango_layout_get_pixel_size(l, NULL, &result);
      g_object_unref (G_OBJECT (l));
     }
  return result;
}


static gint button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if (event->button == 1)
     if (event->type == GDK_2BUTTON_PRESS)
        {
         GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
         GtkTreeModel *model;
         GtkTreeIter iter;

         if (gtk_tree_selection_get_selected (sl, &model, &iter))
            {
             gchar *name;
             gtk_tree_model_get (model, &iter, COL_TEXT, &name, -1);

             if (! name)
               return FALSE;

             handle_enter (user_data, name);

             g_free (name);
             return TRUE;
            }
        }
  return FALSE;
}


void free_kwas_panel (t_kwas_panel *p)
{
   g_hash_table_destroy (p->hash_filelist);
   g_free (p->path);
   g_free (p);
   p = NULL;
}


void free_kwas_window (t_kwas_win *w)
{
  free_kwas_panel (w->p1);
  free_kwas_panel (w->p2);
}


static gboolean on_panel_focus (GtkWidget *widget,
                                GtkDirectionType arg1,
                                gpointer user_data)
{
  t_kwas_panel *p = user_data;
  t_kwas_win *w = p->kwas_win;
  w->focused_panel = p;
  statusbar_message (w, p->path);

  return FALSE;
}


t_kwas_panel* create_filenav_panel (t_kwas_win *w)
{
  t_kwas_panel *p = g_malloc (sizeof (t_kwas_panel));
  p->hash_filelist = NULL;
  p->kwas_win = w;
  p->path = NULL;

  gchar *fn = g_malloc (4096);
  gchar *cur_dir = g_get_current_dir ();

  if (cur_dir)
  if (realpath (cur_dir, fn))
     p->path = fn;

  g_free (cur_dir);

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *col;
  GtkTreeSelection *sel;
  GtkListStore *list_store;

  list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING); /* NUM_COLS = 1 */
  p->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  renderer = gtk_cell_renderer_text_new ();

  g_object_set (renderer, "height", widget_get_y (GTK_WIDGET (p->tree_view), "ZORROTEST") + 5, NULL);

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_TEXT);
  gtk_tree_view_column_set_cell_data_func (col, renderer, cell_data_func, p, NULL);
  gtk_tree_view_column_set_title (col, _("Names: "));
  gtk_tree_view_append_column (GTK_TREE_VIEW (p->tree_view), col);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_ACCESS);
  gtk_tree_view_column_set_title (col, _("Access: "));
  gtk_tree_view_append_column (GTK_TREE_VIEW (p->tree_view), col);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_SIZE);
  gtk_tree_view_column_set_title (col, _("Size: "));
  gtk_tree_view_append_column (GTK_TREE_VIEW (p->tree_view), col);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_DATETIME);
  gtk_tree_view_column_set_title (col, _("Date and time: "));
  gtk_tree_view_append_column (GTK_TREE_VIEW (p->tree_view), col);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (p->tree_view));

  gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
  g_signal_connect ((gpointer) p->tree_view, "key_press_event",
                    G_CALLBACK (on_treeview_key_press_event),
                    p);

  g_signal_connect ((gpointer) p->tree_view, "button_press_event",
                     (GCallback) button_press_event, p);

  g_signal_connect ((gpointer) p->tree_view, "focus-in-event",
                    G_CALLBACK (on_panel_focus),
                    p);

  fill_filelist (p, p->path);

  return p;
}


gchar* kwas_get_current_filename (t_kwas_win *win)
{
  t_kwas_panel *p = win->focused_panel;
  if (! p)
     return NULL;

  GtkTreeSelection *sl = gtk_tree_view_get_selection (GTK_TREE_VIEW (p->tree_view));
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (sl, &model, &iter))
     {
      gchar *name;

      gtk_tree_model_get (model, &iter, COL_TEXT, &name, -1);

      if (! name)
         return FALSE;

      t_flist_item *ti = g_hash_table_lookup (p->hash_filelist, name);

      if (! ti)
         return NULL;

      if (S_ISREG (ti->fstat->st_mode))
         {
          g_free (name);
          return g_strdup (ti->path);
         }

      g_free (name);
      return NULL;
     }
}


t_kwas_win* create_kwas_window (void)
{
  t_kwas_win *w = g_malloc (sizeof (t_kwas_win));
  w->focused_panel = NULL;

  GtkAccelGroup *a = gtk_accel_group_new ();
  GtkWidget *vpaned1;
  GtkWidget *vbox1;
  GtkWidget *mn_kwas_menu;
  GtkWidget *mn_kwas_nav_menu;
  GtkWidget *mn_kwas_ins_menu;

  GtkWidget *item1;
  GtkWidget *item2;
  GtkWidget *tb_toolbar;
  GtkWidget *hpaned1;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *entry1;
  GtkWidget *scrolledwindow2;
  GtkWidget *viewport2;
  GtkWidget *entry2;
  GtkWidget *vbox2;
  GtkWidget *scrolledwindow3;
  GtkWidget *tv_memo;
  GtkWidget *hbox1;
  GtkWidget *combo1;
  GtkWidget *combo_entry1;
  GtkWidget *bt_go;
  gint width;
  gint height;

  w->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (w->window), _("Kwas"));

  g_signal_connect (G_OBJECT (w->window), "key_press_event", G_CALLBACK (win_key_handler), w->window);

  w->status_bar = NULL;

  vpaned1 = gtk_vpaned_new ();
  gtk_widget_show (vpaned1);
  gtk_container_add (GTK_CONTAINER (w->window), vpaned1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_paned_pack1 (GTK_PANED (vpaned1), vbox1, FALSE, TRUE);

  mn_kwas_menu = gtk_menu_bar_new ();
  gtk_widget_show (mn_kwas_menu);
  gtk_box_pack_start (GTK_BOX (vbox1), mn_kwas_menu, FALSE, FALSE, UI_PACKFACTOR);

  mni_temp = new_menu_item (_("Actions"), mn_kwas_menu, NULL);
  kwas_mn_functions = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (kwas_mn_functions);

  mni_temp = new_menu_item_with_udata (_("Refresh Kwas"), kwas_mn_functions, on_mni_kwas_refresh, w);
  gtk_widget_add_accelerator (mni_temp, "activate", accel_group,
                              GDK_KEY_R, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

  mni_temp = new_menu_item_with_udata (_("Get file info"), kwas_mn_functions, on_mni_kwas_get_file_info, w);

  mni_temp = new_menu_item_with_udata (_("Run with..."), kwas_mn_functions, on_mni_kwas_run_with, w);
  mni_temp = new_menu_item_with_udata (_("Copy current file name"), kwas_mn_functions, on_mni_kwas_copy_filename, w);

  mni_temp = new_menu_item (_("Navigate"), mn_kwas_menu, NULL);
  mn_kwas_nav_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mn_kwas_nav_menu);

  mni_temp = new_menu_item_with_udata (_("Go to the home directory"), mn_kwas_nav_menu, on_mni_kwas_goto_homedir, w);

  w->mni_bookmarks = new_menu_item (_("Bookmarks"), mn_kwas_menu, NULL);
  w->mni_bookmarks_menu = new_menu_submenu (w->mni_bookmarks);

  kwas_bookmarks_menu_update (w);

  mni_temp = new_menu_item (_("Ins to editor"), mn_kwas_menu, NULL);
  mn_kwas_ins_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mn_kwas_ins_menu);
  mni_temp = new_menu_item_with_udata (_("Insert image"), mn_kwas_ins_menu, on_mni_kwas_insert_image, w);
  mni_temp = new_menu_item_with_udata (_("Insert link"), mn_kwas_ins_menu, on_mni_kwas_insert_link, w);

  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_top);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_top, FALSE, FALSE, UI_PACKFACTOR);

  w->cb_charset = gtk_combo_box_text_new ();
  fill_combo_with_glist (w->cb_charset, confile.iconv_encs);

  gint i = find_str_index (confile.iconv_encs, confile.default_charset);
  if (i == -1)
     i = 0;

  gtk_combo_box_set_active (w->cb_charset, i);
  gtk_widget_show (w->cb_charset);
  gtk_box_pack_start (GTK_BOX (hbox_top), w->cb_charset, FALSE, FALSE, UI_PACKFACTOR);

  tb_toolbar = gtk_toolbar_new ();
  gtk_widget_show (tb_toolbar);
  gtk_box_pack_start (GTK_BOX (hbox_top), tb_toolbar, FALSE, FALSE, UI_PACKFACTOR);
  gtk_toolbar_set_style (GTK_TOOLBAR (tb_toolbar), GTK_TOOLBAR_BOTH);

  hpaned1 = gtk_hpaned_new ();
  gtk_widget_show (hpaned1);
  gtk_box_pack_start (GTK_BOX (vbox1), hpaned1, TRUE, TRUE, UI_PACKFACTOR);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow1, FALSE, TRUE);

  w->p1 = create_filenav_panel (w);
  gtk_widget_show (w->p1->tree_view);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), w->p1->tree_view);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_paned_pack2 (GTK_PANED (hpaned1), scrolledwindow2, TRUE, TRUE);

  w->p2 = create_filenav_panel (w);
  gtk_widget_show (w->p2->tree_view);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), w->p2->tree_view);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_paned_pack2 (GTK_PANED (vpaned1), vbox2, TRUE, TRUE);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow3, TRUE, TRUE, UI_PACKFACTOR);

  w->log_memo = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (w->log_memo), GTK_WRAP_WORD);
  gtk_widget_show (w->log_memo);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), w->log_memo);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, UI_PACKFACTOR);

  w->cm_entry = gtk_entry_new ();
  gtk_widget_show (w->cm_entry);
  gtk_box_pack_start (GTK_BOX (hbox1), w->cm_entry, TRUE, TRUE, UI_PACKFACTOR);

  bt_go = gtk_button_new_with_label (_("Go to"));
  gtk_widget_show (bt_go);
  gtk_box_pack_start (GTK_BOX (hbox1), bt_go, FALSE, FALSE, UI_PACKFACTOR);
  g_signal_connect ((gpointer) bt_go, "clicked",
                    G_CALLBACK (on_execute_button),
                    w);

  GtkWidget *imgplane_button = gtk_button_new_with_label ("Imageplane");
  gtk_widget_show (imgplane_button);
  gtk_box_pack_start (GTK_BOX (hbox1), imgplane_button, FALSE, FALSE, UI_PACKFACTOR);
  g_signal_connect ((gpointer) imgplane_button, "clicked",
                    G_CALLBACK (on_mni_kwas_imageplane),
                    w);


  w->status_bar = gtk_statusbar_new ();
  gtk_widget_show (w->status_bar);
  gtk_box_pack_start (GTK_BOX (vbox2), w->status_bar, FALSE, FALSE, UI_PACKFACTOR);

  w->p1->opposite_panel = w->p2;
  w->p2->opposite_panel = w->p1;
  w->focused_panel = w->p1;

  width = get_value (confile.screen_w, 80);
  height = get_value (confile.screen_h, 80);

  gtk_window_resize (GTK_WINDOW (w->window), width, height);

  gtk_paned_set_position (vpaned1, get_value (height, 85));
  gtk_paned_set_position (hpaned1, get_value (width, 50));

  GtkTextBuffer *buf = gtk_text_view_get_buffer (w->log_memo);

  gtk_text_buffer_create_tag (buf, "lm_greet",
                              "foreground", "#6c0606", NULL);

  gtk_text_buffer_create_tag (buf, "lm_error",
                              "foreground", "red", NULL);

  gtk_text_buffer_create_tag (buf, "lm_normal",
                              "foreground", "black", NULL);

  gtk_text_buffer_create_tag (buf, "lm_advice",
                              "foreground", "navy", NULL);

  g_signal_connect (w->window, "delete_event",
                    G_CALLBACK (on_kwas_window_delete_event),
                    w);

  g_signal_connect ((gpointer) w->cm_entry, "key_press_event",
                    G_CALLBACK (on_ent_nav_key_press_event),
                    w);

  gtk_window_add_accel_group (GTK_WINDOW (w->window), a);
  gtk_widget_show (w->window);

  gtk_widget_grab_focus (w->p1->tree_view);

  w->log_to_memo_counter = 0;
  w->msg_counter = 0;
  return w;
}
