/*                          tea_funx.c  -  description
                             -------------------
    begin                : Mon Dec 12 2003
    copyright            : (C) 2003-2007 by Peter 'Roxton' Semiletov
    email                : peter.semiletov@gmail.com
 ***************************************************************************/

/*
Copyright (C) 2002 Paolo Maggi
Copyright (C) 2000  Kh. Naba Kumar Singh
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

#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gi18n.h>
#include <stdlib.h> // Needed for strtol
#include <limits.h> // Needed for strtol
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


#ifdef ZZIPLIB_SUPPORTED
#include "zzip/lib.h"
#endif


#include "tea_defs.h"
#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "tea_config.h"
#include "image_browser.h"

#include "rox_strings.h"
#include "interface.h"
#include "tea_funx.h"
#include "tea_proj.h"
#include "tea_gtk_utils.h"
#include "callbacks.h"


//Copyright (C) 2002 Paolo Maggi
gchar* get_time (const gchar *format)
{
   if (! format)
      return NULL;

  gchar *out = NULL;
  gchar *out_utf8 = NULL;
  time_t clock;
  struct tm *now;
  size_t out_length = 0;
  gchar *locale_format;

  if (strlen (format) == 0)
     return NULL;

  locale_format = g_locale_from_utf8 (format, -1, NULL, NULL, NULL);

  if (! locale_format)
     return NULL;

  clock = time (NULL);
  now = localtime (&clock);

  do
    {
     out_length += 255;
     out = g_realloc (out, out_length);
    }
  while (strftime (out, out_length, locale_format, now) == 0);

  g_free (locale_format);

  if (g_utf8_validate (out, -1, NULL))
     out_utf8 = out;
  else
      {
       out_utf8 = g_locale_to_utf8 (out, -1, NULL, NULL, NULL);
       g_free (out);
      }

  return out_utf8;
}


//from Anjuta ::
//    utilities.c
//  Copyright (C) 2000  Kh. Naba Kumar Singh
#define FILE_BUFFER_SIZE 65535
gboolean copy_file (const gchar *src, const gchar *dest)
{
  FILE *input_fp, *output_fp;
  gchar buffer[FILE_BUFFER_SIZE];
  gint bytes_read, bytes_written;
  gboolean error = TRUE;

  input_fp = fopen (src, "rb");
  if (! input_fp)
     return FALSE;

  output_fp = fopen (dest, "wb");
  if (! output_fp)
     {
      fclose (input_fp);
      return TRUE;
     }

  for (;;)
      {
       bytes_read = fread (buffer, 1, FILE_BUFFER_SIZE, input_fp);
       if (bytes_read != FILE_BUFFER_SIZE && ferror (input_fp))
          {
           error = FALSE;
           break;
          }

       if (bytes_read)
         {
          bytes_written = fwrite (buffer, 1, bytes_read, output_fp);
          if (bytes_read != bytes_written)
            {
             error = FALSE;
             break;
            }
         }

       if (bytes_read != FILE_BUFFER_SIZE && feof (input_fp))
          break;
       }

  fclose (input_fp);
  fclose (output_fp);

  return error;
}
//


static gint noname_name_counter = -1;

gchar* get_noname_name (void)
{
  ++noname_name_counter;
  if (noname_name_counter >= G_MAXINT)
     noname_name_counter = 0;
  return g_strdup_printf (_("noname_%d"), noname_name_counter);
}


int get_value (int total, int perc)
{
  return (int) (total / 100) * perc;
}


double get_percent (double total, double value)
{
  return (value / total) * 100;
}


void handle_file (const gchar *filename, const gchar *charset, gint mode, gboolean kwas)
{
  if (! filename)
     return;

  gchar *cmd = NULL;

  gint i = get_n_page_by_filename (filename);

  if (i != -1)
     {
      gtk_notebook_set_current_page (notebook1, i);
      return;
     }

  if (! g_file_test (filename, G_FILE_TEST_EXISTS))
     return;

  if (is_image (filename))
     {
      if (mode != 0)
         {
          cmd = g_strdup_printf (confile.ext_pic_editor, filename);
          system (cmd);
          g_free (cmd);
          return;
         }

      if (confile.use_ext_image_viewer)
         {
          cmd = g_strdup_printf (confile.ext_pic_editor, filename);
          system (cmd);
          g_free(cmd);
          return;
         }
      else
          image_viewer_create (filename);

      return;
     }

  open_file_std (filename, charset);
}


void handle_file_enc (const gchar *filename, const gchar *enc)
{
  if (! filename)
     return;

  gchar *cmd = NULL;

  gint i = get_n_page_by_filename (filename);

  if (i != -1)
     {
      gtk_notebook_set_current_page (notebook1, i);
      return;
     }

  if (! g_file_test (filename, G_FILE_TEST_EXISTS))
     return;

  open_file_std (filename, enc);
}


gboolean is_tex (const gchar *filename)
{
  return is_ext (filename, ".tex", NULL);
}


gboolean is_po (const gchar *f)
{
  return is_ext (f, ".pot", ".po", NULL);
}


void create_empty_file (const gchar *filename, const gchar *first_line)
{
  FILE *out = fopen (filename, "w");

  if (! out)
     return;

  if (first_line)
     fprintf (out, "%s", first_line);

  fclose (out);
}


gint get_file_size (const gchar *filename)
{
  struct stat s;
  stat (filename, &s);
  return s.st_size;
}


//check extension without the dot
gboolean check_ext_wo_dot (const gchar *filename, const gchar *ext)
{
  if (! filename || ! ext)
     return FALSE;

  gboolean r = FALSE;
  gchar *f = g_utf8_strdown (filename, -1);
  gchar *full_ext = g_strconcat (".", ext, NULL);

  r = g_str_has_suffix (f, full_ext);
  g_free (full_ext);
  g_free (f);
  return r;
}


gboolean check_ext (const gchar *filename, const gchar *ext)
{
  if (! filename || ! ext)
     return FALSE;

  gboolean r = FALSE;
  gchar *f = g_utf8_strdown (filename, -1);
  r = g_str_has_suffix (f, ext);
  g_free (f);
  return r;
}


gboolean is_markup (const gchar *f)
{
  return is_ext (f, ".html", ".htm", ".xml", ".xhtml", ".sgm", ".shtml", ".sgml", ".docbook", ".ts", NULL);
}


gboolean is_c (const gchar *f)
{
  return is_ext (f, ".c", ".h", ".cpp", ".hh", ".cxx", ".cc", ".cp", ".c++", NULL);
}


gboolean is_pascal (const gchar *f)
{
  return is_ext (f, ".pas", ".pp", ".dpr", NULL);
}


gchar* get_hl_name (const gchar *file_name)
{
  if (! file_name)
     return NULL;

  gchar *b;

  if (is_markup (file_name))
     return g_strdup (HL_MARKUP);

  if (is_c (file_name))
     return g_strdup (HL_C);

  if (is_pascal (file_name))
     return g_strdup (HL_PASCAL);

  if (check_ext (file_name, ".py"))
     return g_strdup (HL_PYTHON);

  if (check_ext (file_name, ".php"))
     return g_strdup (HL_PHP);

  if (check_ext (file_name, ".sh"))
     return g_strdup (HL_BASH);

  if (is_po (file_name))
     return g_strdup (HL_PO);

  if (is_tex (file_name))
     return g_strdup (HL_TEX);

  if (confile.do_det_scripts_by_content)
     {
      b = str_file_read (file_name);
      if (b)
         {
          if ((strstr (b, "#!/bin/bash")) || (strstr (b, "#!/bin/sh")))
             {
              g_free (b);
              return g_strdup (HL_BASH);
             }
          g_free (b);
         }
     }

  return g_strdup (HL_NONE);
}


gchar* get_l_filename (const gchar *filename)
{
  if (! filename)
     return NULL;

  gsize bytes_read;
  gsize bytes_written;
  return g_filename_from_utf8 (filename, -1, &bytes_read, &bytes_written, NULL);
}


gchar* get_8_filename (const gchar *filename)
{
  if (! filename)
     return NULL;

  gsize bytes_read;
  gsize bytes_written;

  return g_filename_to_utf8 (filename, -1, &bytes_read, &bytes_written, NULL);
}


gchar* get_lang_name (void)
{
  gchar *l = g_getenv ("LANG");
  if (! l)
     return g_strdup ("UTF-8");

  if (strcmp (l, "UTF-8") == 0)
     return g_strdup ("UTF-8");

  return g_strndup (g_getenv ("LANG"), 2);
}


gchar* find_good_browser (void)
{
   gchar *t = g_find_program_in_path ("konqueror");
   if (! t)
      t = g_find_program_in_path ("opera");
      if (! t)
         t = g_find_program_in_path ("firefox");
         if (! t)
            t = g_find_program_in_path ("mozilla");
            if (! t)
                t = g_find_program_in_path ("galeon");
                   if (! t)
                      t = g_find_program_in_path ("yelp");

   return t;
}


gchar* compose_browser_cmd (const gchar *filename)
{
  if (! filename)
     return NULL;

  gchar *t;
  gchar *b;

  if (confile.use_def_doc_browser)
     {
      if (confile.cmd_def_doc_browser)
         t = rep_all_s (confile.cmd_def_doc_browser, filename);
      return t;
     }

  b = find_good_browser ();

  if (! b)
     return NULL;

  t = g_strconcat (b, " ", filename, " &", NULL);

  g_free (b);
  return t;
}


gchar* get_tea_doc_compose_name (const gchar *f)
{
  return g_strconcat (TEA_DOC_DIR, "/", f, NULL);
}


gchar* find_doc_index_name (void)
{
  gchar *l = get_lang_name ();
  gchar *f = g_strconcat (TEA_DOC_DIR, G_DIR_SEPARATOR_S, l, G_DIR_SEPARATOR_S, "index.html", NULL);

  if (! g_file_test (f, G_FILE_TEST_EXISTS))
     {
      g_free (f);
      f = g_strconcat (TEA_DOC_DIR, G_DIR_SEPARATOR_S, "en", G_DIR_SEPARATOR_S, "index.html", NULL);
      g_free (l);
      return f;
     }

  g_free (l);
  return f;
}


void run_doc_in_browser (void)
{
  gchar *f = find_doc_index_name();
  gchar *t = compose_browser_cmd(f);
  if (t)
     system (t);

  g_free (f);
  g_free (t);
}


gchar* get_clipboard_text (void)
{
  GtkClipboard *c = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (! c)
     return NULL;

  if (! gtk_clipboard_wait_is_text_available (c))
     return NULL;

  return gtk_clipboard_wait_for_text (c);
}


void insert_link (const gchar *a_filename)
{
  if (! a_filename || ! cur_text_doc)
     return;

  if (! g_file_test (a_filename, G_FILE_TEST_EXISTS))
     return;

  gchar *filename = create_relative_link (cur_text_doc->file_name_local, a_filename);

  if (! filename)
      return;

  gchar *z =  g_strdup_printf ("<a href=\"%s\"></a>", filename);

  doc_insert_at_cursor (cur_text_doc, z);
  doc_move_cursor_backw_middle_tags (cur_text_doc);

  g_free (filename);
  g_free (z);
}


void handle_file_ide (const gchar *filename, gint line)
{
  if (! filename || ! cur_tea_project)
     return;

  gchar *cmd = file_combine_path (cur_tea_project->dir_source, filename);
  gint i = get_n_page_by_filename (cmd);

  if (i != -1)
     {
      gtk_notebook_set_current_page (notebook1, i);
      if (get_page_text ())
          doc_select_line (cur_text_doc, line);
      g_free (cmd);
      return;
     }

  if (! g_file_test (cmd, G_FILE_TEST_EXISTS))
     {
      g_free (cmd);
      return;
     }
  else
      {
       cur_text_doc = open_file_std (cmd, "UTF-8");
       doc_select_line (cur_text_doc, line);
       g_free (cmd);
      }
}


gboolean parse_error_line (const gchar *line, gchar **filename, gint *lineno)
{
  if (! line)
      return FALSE;

  if (! strstr (line, ":"))
      return FALSE;

  gchar **a = g_strsplit (line, ":", -1);

  if (a[0])
     if (a[1])
        if (a[2])
           if (strstr (a[2], "error") || strstr (a[2], "warning"))
              {
               *filename = g_strstrip (g_strdup (a[0]));
               *lineno = strtol (a[1], NULL, 10);
               g_strfreev (a);
               return TRUE;
              }

   g_strfreev (a);
   return FALSE;
}


void clipboard_put_text (const gchar *s)
{
  GtkClipboard *c = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (c && s)
     gtk_clipboard_set_text (c, s, -1);
}


GList* read_dir_to_glist (const gchar *path)
{
  if (! path)
     return NULL;

  GDir *dir = g_dir_open (path, 0, NULL);
  if (! dir)
     return NULL;

  GList *l = NULL;
  gchar *f = NULL;

  while (f = g_dir_read_name (dir))
        l = g_list_prepend (l, g_strconcat (path, G_DIR_SEPARATOR_S, f, NULL));

  g_dir_close (dir);
  return g_list_reverse (l);
}


GList* read_dir_files (const gchar *path)
{
  if (! path)
      return NULL;

  GList *l = NULL;

  GDir *d = g_dir_open (path, 0, NULL);
  gchar *t;

  while (t = g_dir_read_name (d))
         if (! g_file_test (t, G_FILE_TEST_IS_DIR))
             l = g_list_prepend (l, g_strdup (t));

  g_dir_close (d);
  return g_list_reverse (l);
}

/*
gint get_markup_mode (void)
returns an ID of the markup mode
*/
gint get_markup_mode (void)
{
  if (g_utf8_collate (confile.default_markup_mode, "HTML") == 0)
     return MM_HTML;
  if (g_utf8_collate (confile.default_markup_mode, "XHTML") == 0)
     return MM_XHTML;
  if (g_utf8_collate (confile.default_markup_mode, "Wikipedia") == 0)
     return MM_WIKI;
  if (g_utf8_collate (confile.default_markup_mode, "LaTeX") == 0)
     return MM_TEX;
  if (g_utf8_collate (confile.default_markup_mode, "Docbook") == 0)
     return MM_DOCBOOK;

  return MM_HTML;
}


gboolean is_ext (const gchar *filename, gchar *ext1, ...)
{
  if (! filename)
     return FALSE;

  va_list args;
  gchar *t;
  gboolean r = FALSE;

  va_start (args, ext1);
  t = ext1;

  while (t)
        {
         r = check_ext (filename, t);
         if (r)
            break;

         t = va_arg (args, gchar*);
        }

  va_end (args);

  return r;
}


gchar* run_process_and_wait (const gchar *command)
{
  if (! command)
     return NULL;

  gchar *standard_output = NULL;
  gchar *standard_error = NULL;
  gint exit_status;
  gchar *x = NULL;

  if (command)
     if (g_spawn_command_line_sync (command, &standard_output,
                                    &standard_error, &exit_status, NULL))
       {
        g_free (standard_error);
        x = locale_to_utf8 (standard_output);
        g_free (standard_output);
       }

  return x;
}


void delete_dir (const gchar *path)
{
  if (! path)
     return;

  if (g_utf8_collate (path, G_DIR_SEPARATOR_S) == 0)
     return;

  gchar *cm = g_strconcat ("rm -r -f ", path, NULL);
  system (cm);
  g_free (cm);
}


gchar* filename_from_xuri (const gchar *uri)
{
  gchar *p = NULL;
  gint mode = 0;

  /*
  0 - no uri
  1 - file:/
  2 - file:///
  */

  p = strstr (uri, "file:///");
  if (p)
     mode = 2;
  else
      {
       p = strstr (uri, "file:/");
       if (p)
          mode = 1;
      }

  if (mode == 0)
     return g_strdup (uri);

  if (mode == 1)
     p+=5;
  else
      p+=7;

  return g_strdup (p);
}


gchar* create_relative_link (const gchar *doc_filename, const gchar *img_filename)
{
  if (! doc_filename || ! img_filename)
     return NULL;

  gchar *dir_doc_filename = g_path_get_dirname (doc_filename);
  gchar *dir_img_filename = g_path_get_dirname (img_filename);

  gint dir_doc_filename_len = strlen (dir_doc_filename);
  gint dir_img_filename_len = strlen (dir_img_filename);

  //if the directories are the same, return the pure filename

  if (strcmp (dir_doc_filename, dir_img_filename) == 0)
     {
      g_free (dir_doc_filename);
      g_free (dir_img_filename);
      return g_path_get_basename (img_filename);
     }

  //if img_filename is in the upper dir than doc_filename, we return the pure filename
  if (strcmp (dir_img_filename, dir_doc_filename) < 0)
     {
      g_free (dir_doc_filename);
      g_free (dir_img_filename);
      return g_path_get_basename (img_filename);
     }

  //if img_filename is in the lower dir than doc_filename, we return the relative filename
  if (strcmp (dir_doc_filename, dir_img_filename) < 0)
  if (strstr (dir_img_filename, dir_doc_filename))
     {
      gchar *rel_path = dir_img_filename + dir_doc_filename_len;
      gchar *pure_filename = g_path_get_basename (img_filename);
      gchar *x = g_strconcat (".", rel_path, G_DIR_SEPARATOR_S, pure_filename, NULL);
      g_free (pure_filename);
      g_free (dir_doc_filename);
      g_free (dir_img_filename);

      return g_strdup (x);
     }

   return g_path_get_basename (img_filename);
}


gchar* create_full_path (const gchar *filename, const gchar *basedir)
{
  if (! filename && ! basedir)
     return NULL;

  if (g_path_is_absolute (filename))
     return g_strdup (filename);

  gchar *x = filename;
  if (x[0] == '.')
     x++;

  if (x[0] == G_DIR_SEPARATOR)
     x++;

  gchar *p = g_strconcat (basedir, G_DIR_SEPARATOR_S, x, NULL);
  return p;
}


gboolean is_ext_arr (const gchar *filename, const gchar *exts)
{
  if (! filename || ! exts)
     return FALSE;

  gboolean r = FALSE;

  gchar **arr = g_strsplit (exts, ";", -1);

  if (! arr)
     return FALSE;

  gint c = -1;

  while (arr[c++])
      {
       r = check_ext_wo_dot (filename, arr [c]);
       if (r)
          break;
      }

  g_strfreev (arr);

  return r;
}


void add_image_format (GdkPixbufFormat *data, GList **list)
{
  if (! data)
     return;

  gchar** arr = gdk_pixbuf_format_get_extensions (data);
  if (! arr)
     return;

  gchar *extensions = g_strjoinv (";", arr);
  *list = g_list_prepend (*list, extensions);
  g_strfreev (arr);
}


GList* image_formats_fill (void)
{
  GSList *sl_formats = gdk_pixbuf_get_formats ();
  GList *result = NULL;
  g_slist_foreach (sl_formats, add_image_format, &result);
  g_slist_free (sl_formats);
  return result;
}


gboolean is_image (const gchar *filename)
{
  if (! gl_image_formats || ! filename)
     return FALSE;

  gboolean r = FALSE;

  GList *t = g_list_first (gl_image_formats);
  while (t)
        {
         r = is_ext_arr (filename, t->data);
         if (r)
             break;

         t = g_list_next (t);
        }

  return r;
}


gboolean is_writable (const gchar *filename)
{
  if (access (filename, W_OK) != 0)
     return FALSE;
  else
      return TRUE;
}


gboolean is_readable (const gchar *filename)
{
  if (access (filename, R_OK) != 0)
     return FALSE;
  else
      return TRUE;
}


gboolean has_ext (const gchar *filename)
{
  return (strstr (++filename, "."));
}


gchar* get_build_info (void)
{
   return g_strdup_printf (_("(built on %s with GTK %d.%d.%d and GLib %d.%d.%d)"),
                          __DATE__, GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION,
                          GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
}


gchar* file_replace_path (const gchar *filename, const gchar *new_dir)
{
  if (! filename || ! new_dir)
     return NULL;

  gchar *name = g_strrstr (filename, G_DIR_SEPARATOR_S);
  if (! name)
     return NULL;

  if (g_str_has_suffix  (new_dir, G_DIR_SEPARATOR_S))
     return g_strconcat (new_dir, name, NULL);
  else
     return g_strconcat (new_dir, name, G_DIR_SEPARATOR_S, NULL);

}


gchar* file_combine_path (const gchar *dir, const gchar *filename)
{
  if (! filename || ! dir)
     return NULL;

  if (g_str_has_suffix (dir, G_DIR_SEPARATOR_S))
     return g_strconcat (dir, filename, NULL);
  else
     return g_strconcat (dir, G_DIR_SEPARATOR_S, filename, NULL);
}


gchar* add_slash_if_need (const gchar *dir)
{
  if (! dir)
     return NULL;

  if (g_str_has_suffix  (dir, G_DIR_SEPARATOR_S))
     return g_strdup (dir);
  else
     return g_strconcat (dir, G_DIR_SEPARATOR_S, NULL);
}


void insert_image (const gchar *a_filename, gint width, gint height, gboolean read_xy_from_file)
{
  if (! get_page_text ())
     return;

  if (! cur_text_doc->b_saved)
     return;

  if (! a_filename)
     return;

  if (! g_file_test (a_filename, G_FILE_TEST_EXISTS))
     return;

  if (read_xy_from_file)
     gdk_pixbuf_get_file_info (a_filename, &width, &height);

  gchar *filename = create_relative_link (cur_text_doc->file_name_local, a_filename);

  if (! filename)
      return;

  gint mm = get_markup_mode ();
  gchar *t;

  gchar *temp = g_malloc (7777);

  if (mm == MM_HTML)
      g_snprintf (temp,
                  7777,
                  "<img src=\"%s\" border=\"0\" alt=\"\" width=\"%d\" height=\"%d\">",
                  filename, width, height);
  else
  if (mm == MM_XHTML)
     g_snprintf (temp,
                 7777,
                 "<img src=\"%s\" border=\"0\" alt=\"\" width=\"%d\" height=\"%d\" />",
                 filename, width, height);
  else
      if (mm == MM_DOCBOOK)
          g_snprintf (temp,
                      7777,
                      "<mediaobject><imageobject>\n<imagedata fileref=\"%s\"/>\n</imageobject></mediaobject>",
                      filename);
  else
  if (mm == MM_WIKI)
     g_snprintf (temp,
                 7777,
                 "[[Image:%s|thumb|alternative text]]",
                 filename);
  else
  if (mm == MM_TEX)
     g_snprintf (temp,
                 7777,
                 "\\includegraphics{%s}",
                 filename);

  doc_insert_at_cursor (cur_text_doc, temp);

  g_free (temp);
  g_free (filename);
}


gboolean check_prefix (const gchar *filename, const gchar *ext)
{
  if (! filename || ! ext)
     return FALSE;

  gboolean r = FALSE;
  gchar *f = g_utf8_strdown (filename, -1);
  r = g_str_has_prefix (f, ext);
  g_free (f);
  return r;
}


gboolean is_prefixed (const gchar *filename, gchar *prefix1, ...)
{
  if (! filename)
     return FALSE;

  va_list args;
  gchar *t;
  gboolean r = FALSE;

  va_start (args, prefix1);
  t = prefix1;

  while (t)
        {
         r = check_prefix (filename, t);
         if (r)
            break;

         t = va_arg (args, gchar*);
        }

  va_end (args);

  return r;
}


gchar* path_get_last_element (const gchar *s)
{
  if (! s)
     return NULL;

  gchar *n = g_malloc (strlen (s));
  if (g_str_has_suffix (s, G_DIR_SEPARATOR_S))
      n = g_utf8_strncpy (n, s, g_utf8_strlen (s, -1) -1);
  else
      {
       g_free (n);
       n = g_strdup (s);
      }

  GList *l = glist_from_string_sep (n, G_DIR_SEPARATOR_S);
  if (! l)
      return NULL;

  GList *t = g_list_last (l);

  gchar *x = g_strdup (t->data);
  glist_strings_free (l);
  g_free (n);
  return x;
}


//n.p. Led Zeppelin "Immigrants Song"
gchar* path_up (const gchar *filename)
{
   gint c = 0;
   gint i = 0;

   while (filename[i++])
         if (filename[i] == G_DIR_SEPARATOR)
            c = i;

   if (c == 0)
      return g_strdup (G_DIR_SEPARATOR_S);
   else
       return g_strndup (filename, c);
}


gint get_desktop_name (void)
{
  gint res = DSK_UNKNOWN;

  if (g_getenv ("KDE_FULL_SESSION"))
     res = DSK_KDE;
  else
      if (g_getenv ("GNOME_DESKTOP_SESSION_ID"))
         res = DSK_GNOME;

  if (res != DSK_UNKNOWN)
     return res;

  gchar *wm = gdk_x11_screen_get_window_manager_name (gdk_screen_get_default ());

  if (wm)
     {
       if (strstr (wm, "kwin"))
          res = DSK_KDE;
       else
           if (strstr (wm, "metacity"))
              res = DSK_GNOME;
              else
                  if (strstr (wm, "xfwm4"))
                     res = DSK_XFCE;
      }
}


void read_dir_files_rec_inner (const gchar *path)
{
  GDir *d = g_dir_open (path, 0, NULL);
  if (! d)
      return;

  gchar *t;
  gchar *x;

  while (t = g_dir_read_name (d))
         {
          x = g_strconcat (path, G_DIR_SEPARATOR_S, t, NULL);
          if (g_file_test (x, G_FILE_TEST_IS_DIR) && ! g_file_test (x, G_FILE_TEST_IS_SYMLINK))
              read_dir_files_rec_inner (x);
          else
              gl_found_files = g_list_prepend (gl_found_files, x);
         }

  g_dir_close (d);
}


GList* read_dir_files_rec (const gchar *path)
{
  glist_strings_free (gl_found_files);
  gl_found_files = NULL;

  read_dir_files_rec_inner (path);
  return g_list_reverse (gl_found_files);
}


GList* read_dir_files_rec_pat (const gchar *path, const gchar *pattern)
{
  if (! path || ! pattern)
     return NULL;

  GList *ffiles = read_dir_files_rec (path);
  GList *files = filter_list_pattern (ffiles, pattern, TRUE);
  return files;
}


void zip_create_archive2 (const gchar *filename)
{
#ifdef LIBZIP_SUPPORTED

  if (! filename && file_exists (filename))
     return;

  gchar *archive_name = g_strconcat (filename, ".zip", NULL);

  dbm (archive_name);
  gint errorp;
  struct zip *archive = zip_open (archive_name, ZIP_CREATE, &errorp);
  if (! archive)
     {
      dbm ("! archive");
      return;
     }

  gchar *fname = g_path_get_basename (filename);
  struct zip_source *source = zip_source_file (archive, filename, 1, -1);

  if (source)
     zip_add (archive, fname, source);

  g_free (fname);
  g_free (archive_name);
  zip_close (archive);

#endif
}


gboolean parse_grep_line (const gchar *line, gchar **filename, gint *lineno)
{
  if (! line)
      return FALSE;

  if (! strstr (line, ":"))
      return FALSE;

  //dbm (line);

  gchar **a = g_strsplit (line, ":", -1);

  if (a[0])
     if (a[1])
        {
         *filename = g_strstrip (g_strdup (a[0]));
         *lineno = strtol (a[1], NULL, 10);
         g_strfreev (a);
         return TRUE;
        }

   g_strfreev (a);
   return FALSE;
}


void zip_create_archive (const gchar *filename)
{
#ifdef ZZIPLIB_SUPPORTED

  gchar *archive_name = g_strconcat (filename, ".zip", NULL);
  gchar *buf = str_file_read (filename);

  //ZZIP_DIR* dir = zzip_dir_creat (archive_name, O_WRONLY | O_CREAT);
/*
   ZZIP_DIR* dir = zzip_createdir (archive_name, 0755, dir);
   zzip_file_mkdir (dir, "/", 0755);
  if (dir)
     {
        dbm (archive_name);
      ZZIP_FILE* fp = zzip_file_creat (dir, filename, O_WRONLY);
      if (fp)
         {
          gchar *buf = str_file_read (filename);
          zzip_write (fp, buf, strlen (buf));
          zzip_file_close(fp);
          g_free (buf);
         }
     zzip_dir_close(dir);
    }

   g_free (archive_name);
*/

#ifndef zzip_savefile
#define zzip_savefile 0
#endif

  ZZIP_DIR* zip = zzip_dir_creat (archive_name, O_WRONLY | O_CREAT | O_TRUNC | S_IWGRP);
  zzip_file_mkdir (zip, filename, 0755);
  ZZIP_FILE* file = zzip_file_creat (zip, filename, 0644);
  zzip_write (file, buf, strlen (buf));
  zzip_close (file); file = 0;
  zzip_closedir (zip);
  zip = 0;
  g_free (buf);

#endif
}
