/***************************************************************************
                          tea_text_document.c  -  description
                             -------------------
    begin                : Mon Dec 1 2003
    copyleft            : 2003-2013 by Peter Semiletov,
                           Michele Garoche
    email                : tea@list.ru
 ***************************************************************************/

/*
Copyright (C) 1998 Olivier Sessink and Chris Mazuc
Copyright (C) 1999-2003 Olivier Sessink
(c)Oskar Swida <swida@aragorn.pb.bialystok.pl>
Copyright (C) 1999-2001  David A Knight
Copyright (C)2004 Tarot Osuji <tarot@sdf.lonestar.org>
 *  Copyright (C) 2001 Mikael Hermansson<tyan@linux.se>
 *  Copyright (C) 2003 - Gustavo Giraldez <gustavo.giraldez@gmx.net>
Copyright (C) 2002 Paolo Maggi
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

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <glib/gi18n.h>
#include <pango/pango-layout.h> // for widget_get_string
#include <unistd.h> // for readlink, unlink


#include "tea_defs.h"
#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "tea_config.h"
#include "rox_strings.h" // Needed for str_in_glist
#include "callbacks.h" // Needed for scan_links
#include "interface.h"
#include "tea_funx.h"
#include "tea_gtk_utils.h"
#include "tea_enc.h" // Needed for detect_charset_ja


#ifdef HAVE_LIBASPELL

#include "aspell.h"

#endif


gpointer create_tags_table (void)
{
  GtkTextTag *tag_comment = gtk_text_tag_new ("comment");
  GtkTextTag *tag_identifier = gtk_text_tag_new ("identifier");
  GtkTextTag *tag_digit = gtk_text_tag_new ("digit");
  GtkTextTag *tag_string = gtk_text_tag_new ("string");
  GtkTextTag *tag_html_tag = gtk_text_tag_new ("html_tag");
  GtkTextTag *tag_preprocessor = gtk_text_tag_new ("preprocessor");
  GtkTextTag *tag_type = gtk_text_tag_new ("type");
  GtkTextTag *tag_spell_err = gtk_text_tag_new ("spell_err");
  GtkTextTag *tag_paint = gtk_text_tag_new ("paint");
  GtkTextTag *tag_current_line = gtk_text_tag_new ("current_line");

  g_object_set (G_OBJECT (tag_current_line), "foreground", confile.tag_current_line_fg, "background", confile.tag_current_line_bg, NULL);
  g_object_set (G_OBJECT (tag_paint), "foreground", confile.tag_color_paint_fg, "background", confile.tag_color_paint_bg, NULL);
  g_object_set (G_OBJECT (tag_spell_err), "foreground", confile.tag_spellcheck, NULL);

#if PANGO_VERSION_CHECK(1,4,0)

 g_object_set (G_OBJECT (tag_spell_err), "underline", PANGO_UNDERLINE_ERROR, NULL);

#else

  g_object_set (G_OBJECT (tag_spell_err), "underline", PANGO_UNDERLINE_SINGLE, NULL);

#endif

  g_object_set (G_OBJECT (tag_comment), "foreground", confile.tag_comment, "font", confile.tag_comment_font, NULL);
  g_object_set (G_OBJECT (tag_identifier), "foreground", confile.tag_identifier, "font", confile.tag_identifier_font, NULL);
  g_object_set (G_OBJECT (tag_digit), "foreground", confile.tag_digit, "font", confile.tag_digit_font, NULL);
  g_object_set (G_OBJECT (tag_string), "foreground", confile.tag_string, "font", confile.tag_string_font, NULL);
  g_object_set (G_OBJECT (tag_html_tag), "foreground", confile.tag_html_tag, "font", confile.tag_html_tag_font, NULL);
  g_object_set (G_OBJECT (tag_preprocessor), "foreground", confile.tag_preprocessor, "font", confile.tag_preprocessor_font, NULL);
  g_object_set (G_OBJECT (tag_type), "foreground", confile.tag_type, "font", confile.tag_type_font, NULL);

//#ifdef CHAI_MODE
//  GtkSourceTagTable *tags_table = gtk_source_tag_table_new ();
//#else
  GtkTextTagTable *tags_table = gtk_text_tag_table_new ();
//#endif

  gtk_text_tag_table_add (tags_table, tag_paint);
  gtk_text_tag_table_add (tags_table, tag_spell_err);
  gtk_text_tag_table_add (tags_table, tag_comment);
  gtk_text_tag_table_add (tags_table, tag_identifier);
  gtk_text_tag_table_add (tags_table, tag_digit);
  gtk_text_tag_table_add (tags_table, tag_string);
  gtk_text_tag_table_add (tags_table, tag_html_tag);
  gtk_text_tag_table_add (tags_table, tag_preprocessor);
  gtk_text_tag_table_add (tags_table, tag_type);
  gtk_text_tag_table_add (tags_table, tag_current_line);

  return tags_table;
}


void remove_tags (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter itstart, itend;
  gtk_text_buffer_get_bounds (doc->text_buffer, &itstart, &itend);
  gtk_text_buffer_remove_all_tags (doc->text_buffer, &itstart, &itend);
}




gboolean find_slash (gunichar ch, gpointer user_data)
{
  if (ch == '/')
     return TRUE;
  else
      return FALSE;
}


gboolean find_q (gunichar ch, gpointer user_data)
{
  if (ch == '"')
     return TRUE;
  else
      return FALSE;
}


gboolean find_q2 (gunichar ch, gpointer user_data)
{
  if (ch == '\'')
     return TRUE;
  else
      return FALSE;
}


gboolean find_ast (gunichar ch, gpointer user_data)
{
  if (ch == '*')
     return TRUE;
  else
      return FALSE;
}


static gboolean is_word_number (const gchar *s)
{
  return (g_unichar_isdigit (g_utf8_get_char (s)));
}


void do_hl_c (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

   //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text = gtk_text_iter_get_slice (&a, &iter);
                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (c_t_keywords, text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (c_t_types, text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "type"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                        g_free (text);
                       }
                     }
              }
          }
        }

  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {
             case '/':
                      {
                       b = iter;
                       if (gtk_text_iter_forward_char (&b))
                          if (gtk_text_iter_get_char (&b) == '*')
                             {
                              c = iter;
                              b = c;

                              do
                                {
                                 if (gtk_text_iter_forward_find_char (&iter, find_slash, NULL, NULL))
                                    {
                                     b = iter;
                                     if (gtk_text_iter_backward_char (&b))
                                        if (gtk_text_iter_get_char (&b) == '*')
                                           {
                                            gtk_text_buffer_remove_all_tags (doc->text_buffer, &c, &iter);
                                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &c, &iter);
                                           }
                                     }
                                 }
                               while ( gtk_text_iter_get_char (&b) != '*' );
                              }
                          else
                              {
                               b = iter;
                               if (gtk_text_iter_forward_char (&b))
                                  if (gtk_text_iter_get_char (&b) == '/')
                                     {
                                      a = iter;
                                      gtk_text_iter_forward_line (&iter);
                                      gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                                      gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                                     }
                              }
                       break;
                      }

             case '#':
                      {
                       a = iter;
                       if (gtk_text_iter_forward_line (&iter))
                          if (gtk_text_iter_backward_char (&iter))
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "preprocessor"), &a, &iter);
                       break;
                      }

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                             {
                              gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                             }
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                              {
                               gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                               gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                              }
                        break;
                       }
        }
  }
  while (gtk_text_iter_forward_char (&iter));
}


void do_hl_php (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

  //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text = gtk_text_iter_get_slice (&a, &iter);
                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (php_t_keywords, text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                        g_free (text);
                       }
                    }
              }
          }
        }

  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {
             case '#':
                      {
                       a = iter;
                       if (gtk_text_iter_forward_line (&iter))
                          if (gtk_text_iter_backward_char (&iter))
                             {
                              gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                             }
                       break;
                      }

             case '/':
                      {
                       b = iter;
                       if (gtk_text_iter_forward_char (&b))
                          if (gtk_text_iter_get_char (&b) == '*')
                             {
                              c = iter;
                              b = c;

                              do
                                {
                                 if (gtk_text_iter_forward_find_char (&iter, find_slash, NULL, NULL))
                                    {
                                     b = iter;
                                     if (gtk_text_iter_backward_char (&b))
                                        if (gtk_text_iter_get_char (&b) == '*')
                                           {
                                            gtk_text_buffer_remove_all_tags (doc->text_buffer, &c, &iter);
                                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &c, &iter);
                                           }
                                     }
                                 }
                               while ( gtk_text_iter_get_char (&b) != '*' );
                              }
                          else
                              {
                               b = iter;
                               if (gtk_text_iter_forward_char (&b))
                                  if (gtk_text_iter_get_char (&b) == '/')
                                     {
                                      a = iter;
                                      gtk_text_iter_forward_line (&iter);
                                      gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                                      gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                                     }
                              }
                       break;
                      }

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                        break;
                       }
        }
  }
  while ( gtk_text_iter_forward_char (&iter));
}


void do_hl_bash (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

  //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text = gtk_text_iter_get_slice (&a, &iter);
                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (bash_t_keywords, text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                         g_free (text);
                       }
                    }
               }
          }
  }
  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {
             case '#':
                     {
                      a = iter;
                      if (gtk_text_iter_forward_line (&iter))
                         if (gtk_text_iter_backward_char (&iter))
                            {
                             gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                            }
                      break;
                     }

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                            {
                             gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                            }
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                               gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                        break;
                       }

        }
  }
  while (gtk_text_iter_forward_char (&iter));
}


void do_hl_po (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

  //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text = gtk_text_iter_get_slice (&a, &iter);
                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (po_t_keywords, text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                         g_free (text);
                        }
                   }
              }
          }
  }
  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {
             case '#':
                      {
                       a = iter;
                       if (gtk_text_iter_forward_line (&iter))
                          if (gtk_text_iter_backward_char (&iter))
                             {
                              gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                             }
                       break;
                      }

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                            {
                             gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                            }
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                               gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                        break;
                       }
        }
    }
    while ( gtk_text_iter_forward_char (&iter) );
}


void do_hl_pascal (t_note_page *doc)
{
  if (! doc)
      return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;
  gchar *text2;
  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

   //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text2 = gtk_text_iter_get_slice (&a, &iter);
                    text = g_utf8_strdown (text2, -1);

                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            g_free (text2);
                            continue;
                           }

                         if (g_hash_table_lookup (pas_t_keywords, text) != NULL)
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            g_free (text2);
                            continue;
                           }

                         if (g_hash_table_lookup (pas_t_types, text) != NULL)
                            {
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "type"), &a, &iter);
                             g_free (text);
                             g_free (text2);
                             continue;
                            }
                         g_free (text);
                   }
                }
              }
          }
  }
  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                        break;
                       }

             case '/':
                       {
                        b = iter;
                        if (gtk_text_iter_forward_char (&b))
                           {
                            b = iter;
                            if (gtk_text_iter_forward_char (&b))
                               if (gtk_text_iter_get_char (&b) == '/')
                                  {
                                   a = iter;
                                   gtk_text_iter_forward_line (&iter);
                                      gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                                      gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                                  }
                            }
                        break;
                       }
             }
    }
    while ( gtk_text_iter_forward_char (&iter) );
}


//n.p. Scorn - Vae Solis - Suck And Eat You
void do_python_hl (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;
  GtkTextIter c;
  GtkTextIter d;

  remove_tags (doc);

  gchar *text;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
     {
      a = iter;
      c = iter;
     }

   //keywords
  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
            if (gtk_text_iter_get_char (&b) != '_')
               a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                if (gtk_text_iter_get_char (&iter) != '_')
                   {
                    text = gtk_text_iter_get_slice (&a, &iter);
                    if (text)
                       {
                        if (g_unichar_isdigit (g_utf8_get_char (text)))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "digit"), &a, &iter);
                            g_free (text);
                            continue;
                           }

                        if (g_hash_table_lookup (py_t_keywords, text) != NULL)
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "identifier"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                        g_free (text);
                       }
                     }
                 }
          }
  }
  while (gtk_text_iter_forward_char (&iter));

  //end keywords

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);
  a = iter;
  c = iter;

  do
    {
     a = iter;

     switch (gtk_text_iter_get_char (&iter))
            {
             case '#':
                      {
                       a = iter;
                       if (gtk_text_iter_forward_line (&iter))
                          if (gtk_text_iter_backward_char (&iter))
                             {
                              gtk_text_buffer_remove_all_tags (doc->text_buffer, &a, &iter);
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "comment"), &a, &iter);
                             }
                       break;
                      }

             case '"':
                      {
                       if (gtk_text_iter_forward_find_char (&iter, find_q, NULL, NULL))
                          if (gtk_text_iter_forward_char (&iter))
                             gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                       break;
                      }

             case '\'':
                       {
                        if (gtk_text_iter_forward_find_char (&iter, find_q2, NULL, NULL))
                           if (gtk_text_iter_forward_char (&iter))
                              gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "string"), &a, &iter);
                        break;
                       }
        }
  }
  while ( gtk_text_iter_forward_char (&iter) );
}


void apply_hl (t_note_page *doc)
{
  if (! doc)
     return;
/*
  gchar *hm = get_hl_name (doc->file_name_utf8);
  doc->hl_mode = ch_str (doc->hl_mode, hm);
  g_free (hm);

  if (g_utf8_collate (doc->hl_mode, HL_MARKUP) == 0)
     apply_html_hl (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_C) == 0)
     do_hl_c (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_PHP) == 0)
     do_hl_php (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_PASCAL) == 0)
     do_hl_pascal (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_PYTHON) == 0)
     do_python_hl (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_NONE) == 0)
     remove_tags (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_BASH) == 0)
     do_hl_bash (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_PO) == 0)
     do_hl_po (doc);
  else
  if (g_utf8_collate (doc->hl_mode, HL_TEX) == 0)
      do_hl_tex (doc);*/
}


typedef struct
              {
               gboolean start;
               gboolean end;
              } t_s_switcher;


static gboolean find_tag_end (gunichar ch, gpointer user_data)
{
  if (ch == '>')
     return TRUE;
  else
      return FALSE;
}


void hide_error_marks (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter itstart, itend;
  gtk_text_buffer_get_bounds (doc->text_buffer, &itstart, &itend);
  gtk_text_buffer_remove_tag (doc->text_buffer, get_tag_by_name (doc, "spell_err"), &itstart, &itend);
}

//n.p. Public Image Limited "Another"
void do_hl_spell_check (t_note_page *doc, const gchar *lang)
{
  if (! doc || ! lang)
     return;

#ifdef HAVE_LIBASPELL

  spell_checker_ch_lang (lang);

#endif

  GtkTextIter iter;
  GtkTextIter a;
  GtkTextIter b;

  gchar *p = NULL;

  hide_error_marks (doc);
  gchar *text;
  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &iter, 0);

  if (gtk_text_iter_starts_word (&iter))
      a = iter;

  do
    {
     if (gtk_text_iter_starts_word (&iter))
        {
         b = iter;
         if (gtk_text_iter_backward_char (&b))
             a = iter;

         if (gtk_text_iter_forward_word_end (&iter))
            if (gtk_text_iter_ends_word (&iter))
               {
                text = gtk_text_iter_get_slice (&a, &iter);

                if (text)
                   {
                    if (g_utf8_strlen (text, -1) > 1)
                       {
                        #if defined (HAVE_LIBASPELL) || defined (ENCHANT_SUPPORTED)
                        if (! spell_checker_check (text))
                           {
                            gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "spell_err"), &a, &iter);
                            g_free (text);
                            continue;
                           }
                        #endif
                        }
                    g_free (text);
                   }
               }
          }
        }
  while (gtk_text_iter_forward_char (&iter));
}


void do_errors_hl (GtkTextView *text_view)
{
  GtkTextIter iter;
  GtkTextIter a;
  gchar *filename = NULL;
  gint lineno = 0;

  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (text_view);

  gchar *text;

  gint c = gtk_text_buffer_get_line_count (text_buffer);
  gint i;

  gtk_text_buffer_get_start_iter (text_buffer, &iter);

  for (i = 0; i <= c; i++)
      {
       gtk_text_iter_set_line (&iter, i);
       a = iter;

       if (gtk_text_iter_forward_to_line_end (&a))
          {
           text = gtk_text_iter_get_text (&iter, &a);

           if (parse_error_line (text, &filename, &lineno))
              {
               gtk_text_buffer_remove_all_tags (text_buffer, &iter, &a);
               gtk_text_buffer_apply_tag_by_name (text_buffer, "lm_error", &iter, &a);
               g_free (filename);
              }

           g_free (text);
          }
       }
}


void set_lm_colors (void)
{
  g_object_set (G_OBJECT (tag_lm_error), "foreground", confile.tag_color_lm_error_fg,
                "background", confile.tag_color_lm_error_bg, NULL);
}


void hl_line_under_cursor (t_note_page *doc)
{
  if (! confile.hl_current_line)
     return;

  if (! doc)
     return;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  GtkTextIter it_line1_start = iter;
  GtkTextIter it_line1_end = iter;

  gtk_text_view_backward_display_line_start (doc->text_view, &it_line1_start);
  gtk_text_view_forward_display_line_end (doc->text_view, &it_line1_end);

  if (! gtk_text_iter_has_tag (&iter, get_tag_by_name (doc, "current_line")))
     {
      GtkTextIter start;
      GtkTextIter end;

      gtk_text_buffer_get_bounds (doc->text_buffer, &start, &end);
      gtk_text_buffer_remove_tag (doc->text_buffer, get_tag_by_name (doc, "current_line"), &start, &end);
      gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "current_line"), &it_line1_start, &it_line1_end);
    }
}


void hl_line_n (t_note_page *doc, gint line)
{
  if (! doc)
     return;

  GtkTextIter iter;

  gtk_text_buffer_get_iter_at_line_index (doc->text_buffer, &iter, line, 0);

  GtkTextIter it_line_end = iter;

  if (gtk_text_iter_forward_line (&it_line_end))
     gtk_text_iter_backward_char (&it_line_end);

  gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "paint"), &iter, &it_line_end);
}


gboolean on_editor_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  t_note_page *page = data;

  gchar *t;
  gchar *s;
  GtkTextIter itstart;
  GtkTextIter itend;

  GList *l;

  guint32 k = gdk_keyval_to_unicode (event->keyval);

#ifdef SOUNDS_MODE
  if (g_unichar_isalnum (k) || g_unichar_ispunct (k) || g_unichar_isspace (k))
      sounds_click ();
#endif

  if (confile.autoclose_tags)
     {
      if (k == '>')
         {
          current_tag_close (page, 0);
          return TRUE;
         }
       return FALSE;
     }


  if (confile.do_autorep && ht_autoreplace)
     {
      t = NULL;
      if (g_unichar_ispunct (k) || g_unichar_isspace (k))
         t = doc_get_word_at_left (page, &itstart, &itend);
      if (! t)
         return FALSE;

      s = g_hash_table_lookup (ht_autoreplace, t);
      if (s)
         {
          gtk_text_buffer_delete (page->text_buffer, &itstart, &itend);
          gtk_text_buffer_insert (page->text_buffer, &itstart, s, -1);
         }

      g_free (t);
      return FALSE;
     }

  if ((event->keyval == GDK_KEY_Return) && (confile.use_auto_indent))
     {
      indent_real (page->text_view);
      return TRUE;
     }

#ifdef LEGACY_MODE
/*   if (confile.homeend && (event->state == 0))
      if ((event->keyval == GDK_KEY_End) || (event->keyval == GDK_KEY_Home))
        {
         doc_smarthomeend (page, event);
         return TRUE;
        }

   if (event->keyval == GDK_KEY_Tab && confile.ins_spaces_on_tab_press)
      {
       gchar *indentstring = g_strnfill (confile.tab_size, ' ');
       doc_insert_at_cursor (page, indentstring);
       g_free (indentstring);
       return TRUE;
      }*/
#endif


 return FALSE;
}


void event_after (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  if (event->type == GDK_KEY_PRESS)
     hl_line_under_cursor (user_data);
}


void on_button_close (GtkWidget *wid, gpointer data)
{
  page_del_by_index (find_index_by_page (data));
}

////////////////////
/*
from Leafpad::indent.c
(C)2004 Tarot Osuji <tarot@sdf.lonestar.org>
*/
static gchar* compute_indentation (gpointer buffer, gint line) // from gedit
{
  GtkTextIter start_iter, end_iter;
  gunichar ch;

  gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line);
  end_iter = start_iter;
  ch = gtk_text_iter_get_char (&end_iter);

  while (g_unichar_isspace (ch) && ch != '\n')
       {
        if (! gtk_text_iter_forward_char (&end_iter))
           break;
        ch = gtk_text_iter_get_char (&end_iter);
       }

  if (gtk_text_iter_equal (&start_iter, &end_iter))
     return NULL;

  return gtk_text_iter_get_text (&start_iter, &end_iter);
}


void indent_real (GtkWidget *text_view)
{
  GtkTextIter iter;
  gchar *ind, *str;

  gpointer buffer = gtk_text_view_get_buffer (text_view);

  gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
  ind = compute_indentation (buffer, gtk_text_iter_get_line (&iter));
  str = g_strconcat ("\n", ind, NULL);
  gtk_text_buffer_insert (buffer, &iter, str, -1);
  g_free (str);
  g_free (ind);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (text_view),
                                      gtk_text_buffer_get_insert (buffer));
}
///////////////////////////////


//from Bluefish::document.c
/**
 * doc_toggle_highlighting_cb:
 * @doc: a #Tdocument*
 * @unindent: #gboolean
 *
 * Indent the selected block in current document.
 * Set unindent to TRUE to unindent.
 *
 * Return value: void
 **/
void doc_indent_selection (t_note_page *doc, gboolean unindent)
{
  if (! doc)
     return;

  GtkTextIter itstart, itend;

  if (gtk_text_buffer_get_selection_bounds (doc->text_buffer, &itstart, &itend))
     {
      /* we have a selection, now we loop trough the characters, and for every newline
      we add or remove a tab, we set the end with a mark */
      GtkTextMark *end = gtk_text_buffer_create_mark (doc->text_buffer, NULL, &itend, TRUE);

      if (gtk_text_iter_get_line_offset (&itstart) > 0)
         gtk_text_iter_set_line_index (&itstart, 0);

      while (gtk_text_iter_compare (&itstart, &itend) < 0)
           {
            GtkTextMark *cur = gtk_text_buffer_create_mark (doc->text_buffer, NULL, &itstart, TRUE);
            if (unindent)
               {
                /* when unindenting we try to set itend to the end of the indenting step
                which might be a tab or 'tabsize' spaces, then we delete that part */
                gboolean cont = TRUE;
                gchar *buf = NULL;
                gunichar cchar = gtk_text_iter_get_char (&itstart);
                if (cchar == 9)
                   { /* 9 is ascii for tab */
                    itend = itstart;
                    cont = gtk_text_iter_forward_char (&itend);
                    buf = g_strdup ("\t");
                   }
                else
                    if (cchar == 32)
                       { /* 32 is ascii for space */
                        gchar *tmpstr;
                        gint i = 0;
                        itend = itstart;
                        gtk_text_iter_forward_chars (&itend, confile.tab_size);
                        tmpstr = gtk_text_buffer_get_text (doc->text_buffer, &itstart, &itend, FALSE);

                        while (cont && tmpstr[i] != '\0')
                              {
                               cont = (tmpstr[i++] == ' ');
                              }

                        if (cont)
                           buf = tmpstr;
                        else
                            g_free (tmpstr);
                        }
                     else
                         cont = FALSE;

                     if (cont)
                        {
                         gint offsetstart, offsetend;
                         offsetstart = gtk_text_iter_get_offset (&itstart);
                         offsetend = gtk_text_iter_get_offset (&itend);
                         gtk_text_buffer_delete(doc->text_buffer,&itstart,&itend);
                         }

                }
                 else
                      { /* indent */
                       gint offsetstart = gtk_text_iter_get_offset(&itstart);
                       gchar *indentstring;
                       gint indentlen;
                       if (confile.ins_spaces_on_tab_press)
                          {
                           indentstring = g_strnfill (confile.tab_size, ' ');
                           indentlen = confile.tab_size;
                          }
                       else
                           {
                            indentstring = g_strdup ("\t");
                            indentlen = 1;
                            }

                       gtk_text_buffer_insert (doc->text_buffer, &itstart, indentstring, indentlen);
                       g_free (indentstring);
                       }

                gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &itstart, cur);
                gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &itend, end);
                gtk_text_buffer_delete_mark (doc->text_buffer, cur);
                gtk_text_iter_forward_line(&itstart);
               }
         gtk_text_buffer_delete_mark(doc->text_buffer,end);

       }
       else
           {
            /* there is no selection, work on the current line */
            GtkTextIter iter;
            gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, gtk_text_buffer_get_insert (doc->text_buffer));
            gtk_text_iter_set_line_offset (&iter, 0);
            if (unindent)
               {
                gint deletelen = 0;
                gchar *tmpstr, *tmp2str;
                GtkTextIter itend = iter;
                gtk_text_iter_forward_chars (&itend, confile.tab_size);
                tmpstr = gtk_text_buffer_get_text (doc->text_buffer, &iter, &itend, FALSE);
                tmp2str = g_strnfill (confile.tab_size, ' ');
                if (tmpstr[0] == '\t')
                   deletelen = 1;
                else
                    if (tmpstr && strncmp (tmpstr, tmp2str, confile.tab_size) == 0)
                       deletelen = confile.tab_size;

                g_free (tmpstr);
                g_free(tmp2str);
                if (deletelen)
                   {
                    itend = iter;
                    gtk_text_iter_forward_chars (&itend, deletelen);
                    gtk_text_buffer_delete (doc->text_buffer, &iter, &itend);
                   }
                } else
                 { /* indent */
                  gchar *indentstring;
                  gint indentlen;
                  if (confile.ins_spaces_on_tab_press)
                     {
                      indentstring = g_strnfill (confile.tab_size, ' ');
                      indentlen = confile.tab_size;
                     }
                  else
                      {
                       indentstring = g_strdup ("\t");
                       indentlen = 1;
                      }

                gtk_text_buffer_insert (doc->text_buffer, &iter, indentstring, indentlen);
                g_free (indentstring);
               }
        }
}
//////////////////////


//from Bluefish:: gtk_easy.c, modified by roxton

/**
 * widget_get_string_size:
 * @widget: #GtkWidget* to put the string on
 * @string: #ghcar* with the string
 *
 * This function will calculate the width in pixels from the
 * string passed to it in string, using the font from widget
 *
 * Return value: #gint pixels
 */
gint widget_get_string_size (GtkWidget *w, const gchar *s)
{
  gint r = -1;

  PangoLayout *l = gtk_widget_create_pango_layout (w, s);
  if (l)
     {
      pango_layout_get_pixel_size (l, &r, NULL);
      g_object_unref (G_OBJECT (l));
     }

  return r;
}


//from Bluefish:: document.c
/**
 * This function is taken from gtksourceview
 * Copyright (C) 2001
 * Mikael Hermansson <tyan@linux.se>
 * Chris Phelps <chicane@reninet.com>
 */
static gint textview_calculate_real_tab_width (GtkWidget *textview, gint tab_size)
{
  if (tab_size <= 0)
     return 0;

  gint counter = 0;
  gint tab_width = 0;

  gchar *tab_string = g_malloc (tab_size + 1);
  while (counter < tab_size)
        {
         tab_string[counter++] = ' ';
        }

  tab_string[tab_size] = '\0';
  tab_width =  widget_get_string_size (textview, tab_string);
  g_free (tab_string);
  return tab_width;
}

/**
 * doc_set_tabsize:
 * @doc: a #Tdocument
 * @tabsize: a #gint with the tab size
 *
 * this function will set the textview from doc to use the tabsize
 * described by tabsize
 *
 * Return value: void
 **/
void doc_set_tabsize (t_note_page *doc, gint tabsize)
{
  if (! doc)
      return;

  gint pixels = textview_calculate_real_tab_width (GTK_WIDGET (doc->text_view), tabsize);
  PangoTabArray *tab_array = pango_tab_array_new (1, TRUE);
  pango_tab_array_set_tab (tab_array, 0, PANGO_TAB_LEFT, pixels);
  gtk_text_view_set_tabs (GTK_TEXT_VIEW (doc->text_view), tab_array);
  pango_tab_array_free(tab_array);
}


 //from Bluefish :: document.c
/* contributed by Oskar Swida <swida@aragorn.pb.bialystok.pl>, with help from the gedit source */
/*
static gboolean doc_textview_expose_event_lcb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GtkTextView *view = (GtkTextView*)widget;
  GdkRectangle rect;
  GdkWindow *win;
  GtkTextIter l_start, l_end,it;
  gint l_top1, l_top2;
  PangoLayout *l;
  gchar *pomstr;
  gint numlines, w, i;

  win = gtk_text_view_get_window(view, GTK_TEXT_WINDOW_LEFT);
  if (win != event->window)
     return FALSE;

  gtk_text_view_get_visible_rect (view, &rect);
  gtk_text_view_get_line_at_y (view, &l_start, rect.y, &l_top1);
  gtk_text_view_get_line_at_y (view, &l_end, rect.y + rect.height, &l_top2);
  l = gtk_widget_create_pango_layout (widget, "");

  numlines = gtk_text_buffer_get_line_count(gtk_text_view_get_buffer(view));
  pomstr = g_strdup_printf ("%d", MAX (99, numlines));
  pango_layout_set_text (l, pomstr, -1);
  g_free (pomstr);
  pango_layout_get_pixel_size (l, &w, NULL);
  gtk_text_view_set_border_window_size (view, GTK_TEXT_WINDOW_LEFT, w + 4);
  it = l_start;
  for (i = gtk_text_iter_get_line (&l_start); i <= gtk_text_iter_get_line (&l_end); i++)
      {
       gtk_text_iter_set_line (&it, i);
       gtk_text_view_get_line_yrange(view, &it, &w, NULL);
       gtk_text_view_buffer_to_window_coords (view, GTK_TEXT_WINDOW_LEFT, 0, w, NULL, &w);

       pomstr = g_strdup_printf ("%d", i + 1);
       pango_layout_set_text (l, pomstr, -1);

  gtk_paint_layout (widget_get_style (widget), win, GTK_WIDGET_STATE(widget), FALSE, 0, widget, 0, 2, l);
  g_free (pomstr);
      }

   g_object_unref (G_OBJECT(l));
   return TRUE;
}
*/
/**
 * document_set_line_numbers:
 * @doc: a #Tdocument*
 * @value: a #gboolean
 *
 * Show or hide linenumbers (at the left of the main GtkTextView).
 *
 * Return value: void
 **/


void document_set_line_numbers (t_note_page *doc, gboolean value)
{
  if (! doc)
     return;

  gtk_source_view_set_show_line_numbers (doc->text_view, value);
  doc->linenums = value;
}


//////////
gchar* get_full_fname (const gchar *fname, const gchar *linkname)
{
  if (! fname && ! linkname)
     return NULL;

  gchar *dir = g_path_get_dirname (fname);
  gchar *filename = create_full_path (linkname, dir);
  g_free (dir);
  return filename;
}


static void msg_wrong_encoding (const gchar *filename)
{
  log_to_memo (_("Sorry, but I cannot determine the charset of %s. Please try again and select it manually."), filename, LM_ERROR);
}


void do_backup (t_note_page *doc, gboolean do_check)
{
  if (do_check)
     if (! confile.do_backup)
        return;

  if (! g_file_test (doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  gchar *f = g_strconcat (doc->file_name_local, ".bak", NULL);
  text_doc_save_silent (doc, f);
  g_free (f);
}


gboolean text_doc_save (t_note_page *doc, const gchar *a_filename)
{
  if (! doc || ! a_filename)
     return FALSE;

  if (doc->readonly)
     {
      log_to_memo (_("This file is read-only"), NULL, LM_ERROR);
      return FALSE;
     }

  gboolean result = FALSE;

  if (g_file_test (a_filename, G_FILE_TEST_EXISTS))
  if (! is_writable (a_filename))
     {
      dlg_info (_("Warning!"), _("%s is not writable for you!"), a_filename);
      log_to_memo (_("%s is not writable for you!"), a_filename, LM_ERROR);
      return FALSE;
     }

  if (g_file_test (a_filename, G_FILE_TEST_IS_DIR))
     {
      log_to_memo (_("And how I can save this text file as a DIRECTORY?!"), NULL, LM_ERROR);
      return FALSE;
     }

  if (g_utf8_collate (doc->encoding, "UTF-8") == 0)
     {
      do_backup (doc, TRUE);
      result = doc_save_buffer_to_file (doc->text_buffer, a_filename);
     }
  else
      {
       do_backup (doc, TRUE);
       result = doc_save_buffer_to_file_iconv (doc->text_buffer, a_filename, doc->encoding);
      }

  if (g_utf8_collate (a_filename, confile.tea_rc) == 0)
     {
      confile_reload ();
      doc_update_all ();
      log_to_memo (_("config reloaded"), NULL, LM_NORMAL);
     }
  else
  if (g_utf8_collate (a_filename, confile.tea_hotkeys) == 0)
     {
      reload_hotkeys ();
      log_to_memo (_("hotkeys reloaded"), NULL, LM_NORMAL);
     }
  else
  if (g_utf8_collate (a_filename, confile.ext_programs) == 0)
     {
      reload_ext_programs ();
      log_to_memo (_("external programs list reloaded"), NULL, LM_NORMAL);
     }
  else
  if (g_utf8_collate (a_filename, confile.tea_autoreplace) == 0)
      reload_autoreplace ();
  else
  if (g_utf8_collate (a_filename, confile.bmx_file) == 0)
     {
      bmx_reload ();
      log_to_memo (_("%s reloaded"), confile.bmx_file, LM_NORMAL);
     }
  else
  if (g_utf8_collate (a_filename, confile.autosave_file) == 0)
     {
      reload_autosave ();
      log_to_memo (_("%s reloaded"), confile.autosave_file, LM_NORMAL);
     }

  markup_change_by_ext (doc->file_name_utf8);

  gtk_text_buffer_set_modified (doc->text_buffer, FALSE);

  return result;
}


gchar* doc_get_sel (t_note_page *doc)
{
  if (! doc)
     return NULL;

  GtkTextIter start;
  GtkTextIter end;

  if (gtk_text_buffer_get_selection_bounds (doc->text_buffer, &start, &end))
     return (gtk_text_buffer_get_text (doc->text_buffer, &start, &end, FALSE));
  else
      {
       gtk_text_buffer_get_bounds (doc->text_buffer, &start, &end);
       return (gtk_text_buffer_get_text (doc->text_buffer, &start, &end, FALSE));
      }
}


void doc_select_line (t_note_page *doc, gint line)
{
  if (! doc)
     return NULL;

  GtkTextIter itstart, itend;
  gtk_text_buffer_get_iter_at_line (doc->text_buffer, &itstart, line - 1);
  itend = itstart;
  if (gtk_text_iter_forward_to_line_end (&itend))
     {
      gtk_text_buffer_place_cursor (doc->text_buffer, &itstart);
      GtkTextMark *m = gtk_text_buffer_get_mark (doc->text_buffer, "insert");
      if (m)
         {
          gtk_text_view_scroll_to_mark (doc->text_view, m, 0.0, TRUE, 0.0, 0.0);
          gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &itend);
         }
     }
}


gboolean doc_save_buffer_to_file (gpointer text_buffer, const gchar *filename)
{
  if (! text_buffer && ! filename)
     return NULL;

  gchar *t = doc_get_buf (text_buffer);
  if (! t)
     return FALSE;

  gboolean r = save_string_to_file (filename, t);
  g_free (t);
  return r;
}


gboolean doc_save_buffer_to_file_iconv (gpointer text_buffer, const gchar *filename, const gchar *enc)
{
  if (! text_buffer && ! filename && ! enc)
     return NULL;

  gchar *buf = doc_get_buf (text_buffer);
  if (! buf)
     return FALSE;

  gboolean result = FALSE;
  gsize bytes_read;
  gsize bytes_written;

  gchar *t = g_convert (buf, -1, enc, "UTF-8", &bytes_read, &bytes_written, NULL);

  if (! t)
     {
      g_free (buf);
      return FALSE;
     }

  result = save_string_to_file (filename, t);

  g_free (buf);
  g_free (t);

  return result;
}


static const char *xpm_data[] =
                                {
                                 "6 6 2 1",
                                 "  c None",
                                 ". c #000000000000",
                                 ".    .",
                                 " .  . ",
                                 "  ..  ",
                                 "  ..  ",
                                 " .  . ",
                                 ".    ."
                                };


t_note_page* page_create_new (void)
{
  t_note_page *page = (t_note_page *) g_malloc (sizeof (t_note_page));

  page->toggle_images_visibility = FALSE;
  page->position = 1;
  page->readonly = FALSE;
  page->last_searched_text = NULL;
  page->hl_mode = g_strdup (HL_NONE);
  page->autosave = FALSE;
  page->end_of_line = g_strdup ("LF");

  page->text_buffer = 0;
  page->text_view = 0;

  dbm ("1");

  page->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (page->scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (page->scrolledwindow);

  page->text_view = gtk_source_view_new();
  page->text_buffer = gtk_text_view_get_buffer (page->text_view);

  if (page->text_view == NULL)
      dbm ("page->text_view is NULL");


  g_signal_connect (G_OBJECT(page->text_view), "event-after", G_CALLBACK (event_after), page);

  page->encoding = g_strdup (confile.def_filesave_charset);//g_strdup ("UTF-8");
  page->linenums = FALSE;

  gtk_widget_show (page->text_view);

  dbm ("3");


  if (confile.word_wrap == 1)
     gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (page->text_view), GTK_WRAP_WORD);
  else
      gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (page->text_view), GTK_WRAP_NONE);

  gtk_container_set_border_width (GTK_CONTAINER (page->text_view), 1);

  gtk_container_add (GTK_CONTAINER (page->scrolledwindow), page->text_view);

  page->tab_label = gtk_label_new (NULL);
  //GTK_WIDGET_UNSET_FLAGS (page->tab_label, GTK_CAN_FOCUS);
  gtk_widget_set_can_focus (page->tab_label, FALSE);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

  page->but = gtk_button_new ();
  page->dclicked_y = 0;

  GtkStyle *style = gtk_widget_get_style (tea_main_window);
  
  /*GdkPixmap *pixmap = gdk_pixmap_create_from_xpm_d (tea_main_window->window, NULL,
                                                    &style->bg[GTK_STATE_NORMAL],
                                                    (gchar **) xpm_data);

  GtkWidget *w_pixmap = gtk_image_new_from_pixmap (pixmap, NULL);*/
  
  GtkWidget *w_pixmap = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_SMALL_TOOLBAR);
  
  
  gtk_widget_show (w_pixmap);
  gtk_container_add (GTK_CONTAINER (page->but), w_pixmap);

  gtk_box_pack_start (GTK_BOX(hbox), page->tab_label, TRUE, FALSE, UI_PACKFACTOR);
  gtk_box_pack_start (GTK_BOX(hbox), page->but, FALSE, FALSE, UI_PACKFACTOR);
  gtk_widget_show (hbox);
  gtk_widget_show (page->but);
  gtk_widget_show (page->tab_label);

  g_signal_connect ((gpointer) page->but, "clicked",
                    G_CALLBACK (on_button_close),
                    page);


  dbm ("4");

  page->b_saved = FALSE;
  page->file_name_local = g_strdup ("noname");
  page->file_name_utf8 = g_strdup ("noname");

  dbm ("5");

 // page->text_buffer = gtk_source_buffer_new (create_tags_table());
  //page->text_buffer = gtk_source_buffer_new (NULL);


  dbm ("5.5");

//  dbm (page->text_buffer);

//  if (page->text_buffer == NULL)
  //    dbm ("(");


//  gtk_text_view_set_buffer (page->text_view, page->text_buffer);

  dbm ("6");


  g_signal_connect (page->text_view, "key-press-event",
                    G_CALLBACK (on_editor_keypress), page);

  dox = g_list_append (dox, page);
  gtk_notebook_append_page_menu (GTK_NOTEBOOK(notebook1), page->scrolledwindow, hbox, NULL);

#if ((GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >= 9))

  gtk_notebook_set_tab_reorderable (notebook1, hbox, FALSE);
  gtk_notebook_set_tab_detachable (notebook1, hbox, FALSE);

#endif

  if (confile.show_line_nums == 1)
     document_set_line_numbers (page, TRUE);

  set_dnd_accept_tab (page->text_view);

  g_signal_connect (page->text_buffer, "mark_set", G_CALLBACK (move_cursor_cb), page);
  g_signal_connect (page->text_buffer, "changed", G_CALLBACK (update_cursor_position), page);


  return page;
}


gpointer glist_find_by_index (GList *list, int i)
{
  return g_list_nth_data (list, i);
}


t_note_page* get_page (void)
{
  cur_text_doc = NULL;

  gint i = gtk_notebook_get_current_page (notebook1);
  if (i == -1)
    return NULL;
  else
      cur_text_doc = (t_note_page *) g_list_nth_data (dox, i);

  return cur_text_doc;
}


t_note_page* get_page_by_index (int i)
{
  return (t_note_page *) g_list_nth_data (dox, i);
}


void page_del_by_index (int i)
{
  t_note_page *page;

  if (i != -1)
     {
      page = glist_find_by_index (dox, i);
      dox = g_list_remove (dox, page);
      page_free (page);
      gtk_notebook_remove_page (notebook1, i);
      update_recent_list_menu (FALSE);
     }
}


t_note_page* doc_clear_new (void)
{
  t_note_page *doc = page_create_new ();

  g_free (doc->file_name_local);
  g_free (doc->file_name_utf8);

  doc->file_name_local = get_noname_name ();
  doc->file_name_utf8 = g_strdup (doc->file_name_local);

  gtk_label_set_label (doc->tab_label, doc->file_name_utf8);
  doc_apply_settings (doc);
  gtk_notebook_set_current_page (notebook1, g_list_length (dox) - 1);

  return doc;
}


t_note_page* doc_open_file (const gchar *a_filename, const gchar *a_charset)
{
  if (! a_filename)
     return NULL;

  gsize length;
  GError *error = NULL;
  gchar *buf;
  gchar *newlabel;
  gsize bytes_read;
  gchar *enc;
  gchar *t;
  gchar *filename;
  gboolean ronly = FALSE;

  t_note_page* page = NULL;

  gchar *utf8_name = get_8_filename (a_filename);
  if (! utf8_name)
     {
      log_to_memo (_("Cannot convert filename to UTF-8, abort"), NULL, LM_ERROR);
      return NULL;
     }

  if (! g_file_test (a_filename, G_FILE_TEST_EXISTS))
     {
      log_to_memo (_("%s is not exists!"), utf8_name, LM_ERROR);
      g_free (utf8_name);
      return NULL;
     }

  if (! is_readable (a_filename))
     {
      log_to_memo (_("You have NO access to %s"), utf8_name, LM_ERROR);
      g_free (utf8_name);
      return NULL;
     }

  filename = g_strdup (a_filename);

  if (g_file_test (a_filename, G_FILE_TEST_IS_SYMLINK))
     {
      g_free (filename);
      filename = g_malloc (2048);
      readlink (a_filename, filename, 2048);

      g_free (utf8_name);
      utf8_name = get_8_filename (filename);

      if (! is_readable (filename))
         {
          log_to_memo (_("You have NO access to %s"), utf8_name, LM_ERROR);
          g_free (filename);
          g_free (utf8_name);
          return NULL;
         }

      if (! g_file_test (filename, G_FILE_TEST_EXISTS))
         {
          log_to_memo (_("%s is not exists!"), utf8_name, LM_ERROR);
          g_free (filename);
          g_free (utf8_name);
          return NULL;
         }
      }

   if (is_format_readonly (utf8_name))
      {
       if (is_ext (filename, ".sxw", ".odt", NULL))
          buf = read_xml_text (filename, "content.xml", "text:p");
       else
           if (check_ext (filename, ".abw"))
               buf = read_abw_text (filename, "p");
       else
           if (check_ext (filename, ".kwd"))
              buf = read_xml_text (filename, "maindoc.xml", "TEXT");
       else
           if (is_rtf (utf8_name))
               buf = text_load_rtf (filename);
       else
           if (is_ext (utf8_name, ".gz", ".zip", ".bz2", NULL))
               buf = text_load_gzip (filename);
       else
           if (is_ext (utf8_name, ".doc", NULL))
              buf = text_load_msdoc (filename);

       length = strlen (buf);
       ronly = TRUE;
      }
   else //the format is not read only
       if (! g_file_get_contents (filename, &buf, &length, &error))
          {
           g_free (filename);
           g_free (utf8_name);
           return NULL;
          }

  error = NULL;
  page = doc_clear_new ();

  if (strstr (buf, "\r"))
     {
      if (strstr (buf, "\n"))
         page->end_of_line = ch_str (page->end_of_line, "CR/LF");
      else
         page->end_of_line = ch_str (page->end_of_line, "CR");
      }

  g_free (page->encoding);
  page->encoding = NULL;

  if (a_charset)
     page->encoding = g_strdup (a_charset);
  else
     page->encoding = g_strdup (confile.default_charset);

  gchar *tc = NULL;
  if (confile.det_charset_by_meta && is_ext (page->file_name_local, ".html", ".htm", ".xhtml", ".xml", NULL));
     {
      tc = get_charset_from_meta (buf);
      if (tc)
         {
          g_free (page->encoding);
          page->encoding = tc;
         }
     }

  if (! tc)
  if (g_utf8_collate (page->encoding, CHARSET_JAPANESE) == 0)
     {
      enc = detect_charset_ja (buf);
      page->encoding = ch_str (page->encoding, enc);
      g_free (enc);
     }

  if (! tc)
  if (g_utf8_collate (page->encoding, CHARSET_AUTODETECT) == 0)
     {
      enc = enc_guess (buf);
      if (enc)
         {
          page->encoding = ch_str (page->encoding, enc);
          g_free (enc);
         }
     else
         if (confile.enc_use_fallback)
           {
            page->encoding = ch_str (page->encoding, confile.fallback_charset);
            g_free (enc);
           }
         else //not fallback
             {
              g_free (buf);
              page_del_by_index (gtk_notebook_get_current_page ((GtkNotebook *) notebook1));
              msg_wrong_encoding (filename);
              g_free (utf8_name);
              g_free (filename);
              return NULL;
             }
     }

  if (ronly && ! check_ext (filename, ".zip"))
     {
      page->readonly = TRUE;
      g_free (page->encoding);
      page->encoding = g_strdup ("UTF-8");
     }

  if (g_utf8_collate (page->encoding, "UTF-8") != 0)
     {
      t = g_convert (buf, length, "UTF-8", page->encoding, &bytes_read, &length, NULL);
      g_free (buf);
      buf = NULL;
     }
  else
      t = buf;

  if (! t)
     {
      if (buf)
         g_free (buf);
      page_del_by_index (gtk_notebook_get_current_page ((GtkNotebook *) notebook1));
      msg_wrong_encoding (filename);
      g_free (filename);
      return NULL;
     }

  if (g_utf8_validate (t, length, NULL))
     {
      mpb_start ();
      gtk_text_view_set_editable (page->text_view, FALSE);

      gtk_source_buffer_begin_not_undoable_action (page->text_buffer);
      gtk_text_buffer_set_text (page->text_buffer, t, length);
      gtk_source_buffer_end_not_undoable_action (page->text_buffer);

      mpb_stop();
      gtk_text_view_set_editable (page->text_view, TRUE);
     }

  g_free (t);

  g_free (page->file_name_local);
  g_free (page->file_name_utf8);

  page->file_name_utf8 = g_strdup (utf8_name);
  page->file_name_local = g_strdup (filename);

  if (str_in_glist (gl_autosave, page->file_name_utf8))
     page->autosave = TRUE;

  page->b_saved = TRUE;
  newlabel = g_path_get_basename (page->file_name_utf8);
  gtk_label_set_label (page->tab_label, newlabel);

  doc_apply_settings (page);
  editor_set_pos (page, 0);

  log_to_memo (_("%s is opened"), page->file_name_utf8, LM_NORMAL);

  document_do_hl (page);


  if (confile.scan_for_links_on_doc_open == 1)
      scan_links();

  g_free (filename);
  g_free (utf8_name);
  g_free (newlabel);

  gtk_text_buffer_set_modified (page->text_buffer, FALSE);
  tabs_reload ();
  set_title (page);
  markup_change_by_ext (page->file_name_utf8);

  return page;
}


void doc_apply_settings (t_note_page *doc)
{
  if (! doc) return;

  PangoFontDescription *font_desc = pango_font_description_from_string (confile.editor_font);
  gtk_widget_modify_font (doc->text_view, font_desc);
  pango_font_description_free (font_desc);

  g_object_set (G_OBJECT (get_tag_by_name (doc, "spell_err")), "foreground", confile.tag_spellcheck, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "current_line")), "foreground", confile.tag_current_line_fg, "background", confile.tag_current_line_bg, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "paint")), "foreground", confile.tag_color_paint_fg, "background", confile.tag_color_paint_bg, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "comment")), "foreground", confile.tag_comment, "font", confile.tag_comment_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "identifier")), "foreground", confile.tag_identifier, "font", confile.tag_identifier_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "digit")), "foreground", confile.tag_digit, "font", confile.tag_digit_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "string")), "foreground", confile.tag_string, "font", confile.tag_string_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "html_tag")), "foreground", confile.tag_html_tag, "font", confile.tag_html_tag_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "preprocessor")), "foreground", confile.tag_preprocessor, "font", confile.tag_preprocessor_font, NULL);
  g_object_set (G_OBJECT (get_tag_by_name (doc, "type")), "foreground", confile.tag_type, "font", confile.tag_type_font, NULL);

  GdkColor background;
  GdkColor text;
  GdkColor selection;
  GdkColor sel_text;

  gdk_color_parse (confile.text_foreground, &text);
  gdk_color_parse (confile.text_background, &background);
  gdk_color_parse (confile.text_sel_background, &selection);
  gdk_color_parse (confile.text_sel_foreground, &sel_text);

  widget_set_colors (doc->text_view, &background, &text, &selection, &sel_text);



//#ifdef GTKSOURCEVIEW_SUPPORTED

  gtk_source_view_set_right_margin_position (doc->text_view, confile.margin_width);
  gtk_source_view_set_show_right_margin (doc->text_view, confile.margin_visible);
  gtk_source_view_set_insert_spaces_instead_of_tabs (doc->text_view, confile.ins_spaces_on_tab_press);
  gtk_source_view_set_tab_width (doc->text_view, confile.tab_size);
  gtk_source_view_set_smart_home_end (doc->text_view, confile.homeend);
  gtk_source_buffer_set_max_undo_levels (doc->text_buffer, confile.max_undo_levels);

  GtkSourceStyleScheme *shm = gtk_source_style_scheme_manager_get_scheme (gtk_source_style_scheme_manager_get_default (),
                                                                          confile.def_color_id);

  if (shm)
     gtk_source_buffer_set_style_scheme (doc->text_buffer, shm);

//#endif
}


void widget_apply_colors (GtkWidget *w)
{
  if (! w)
     return;

  GdkColor background;
  GdkColor text;
  GdkColor selection;
  GdkColor sel_text;

  gdk_color_parse (confile.text_foreground, &text);
  gdk_color_parse (confile.text_background, &background);
  gdk_color_parse (confile.text_sel_background, &selection);
  gdk_color_parse (confile.text_sel_foreground, &sel_text);

  widget_set_colors (w, &background, &text, &selection, &sel_text);
}


void page_free (t_note_page *page)
{
  if (! page)
    return;

  gchar *f = page->file_name_local;

  if (f)
    {
     if (page->autosave)
        text_doc_save (page, f);
     else
        {
         if (strcmp (f, confile.crapbook_file) != 0)
            if (page->b_saved && confile.prompt_on_not_saved && gtk_text_buffer_get_modified (page->text_buffer))
               if (dlg_question (_("Question"), _("%s is modified but not saved. Save it?"), page->file_name_utf8))
                  text_doc_save (page, f);
        }

     if (strcmp (f, confile.crapbook_file) == 0)
        doc_save_buffer_to_file (page->text_buffer, confile.crapbook_file);
     else
         if (g_file_test (f, G_FILE_TEST_EXISTS))
            add_recent_internal (page);
    }

  g_free (page->end_of_line);
  g_free (page->last_searched_text);
  g_free (page->encoding);
  g_free (page->hl_mode);
  g_free (page->file_name_local);
  g_free (page->file_name_utf8);
  g_free (page);

  tabs_reload ();
}


void doc_insert_at_cursor (t_note_page* doc, const gchar *text)
{
  if (! doc || ! text)
     return;

  gtk_text_buffer_begin_user_action (doc->text_buffer);
  gtk_text_buffer_insert_at_cursor (doc->text_buffer, text, -1);
  gtk_text_buffer_end_user_action (doc->text_buffer);
}


void doc_update_cb (gpointer data, gpointer user_data)
{
  if (data)
     doc_apply_settings ((t_note_page*) data);
}


void doc_update_all (void)
{
  g_list_foreach (dox, doc_update_cb, NULL);
  set_lm_colors();
  
  if (confile.do_show_main_toolbar)
     gtk_widget_show (tb_main_toolbar);
  else
      gtk_widget_hide (tb_main_toolbar);
}


gint editor_get_pos (t_note_page* doc)
{
  if (! doc && ! doc->text_buffer && ! doc->text_view)
     return 0;

  GtkTextMark *mark;
  GtkTextIter it;
  mark = gtk_text_buffer_get_mark (doc->text_buffer, "insert");
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &it, mark);
  return gtk_text_iter_get_offset (&it);
}


void tv_logmemo_set_pos (guint pos)
{
  GtkTextIter it;
  gtk_text_buffer_get_iter_at_offset (log_memo_textbuffer, &it, pos);
  gtk_text_buffer_place_cursor (log_memo_textbuffer, &it);
  gtk_text_view_scroll_to_iter (tv_logmemo, &it, 0.0, FALSE, 0.0, 0.0);
}

gboolean doc_reload_text (t_note_page *doc, const gchar *filename, const gchar *enc)
{
  gsize length;
  GError *error = NULL;
  gchar *buf;
  gchar *t;
  gsize bytes_read;
  gsize z;

  if (! g_file_get_contents (filename, &buf, &length, &error))
     return FALSE;


  if (g_utf8_collate (enc, "UTF-8") != 0)
     {
      z = length;
      error = NULL;
      t = g_convert (buf, z, "UTF-8", enc, &bytes_read, &length, NULL);
      g_free (buf);
     }
   else
       t = buf;

   if (! t)
      return FALSE;

   if (! g_utf8_validate (t, length, NULL))
      {
       g_free (t);
       return FALSE;
      }



   gtk_source_buffer_begin_not_undoable_action (doc->text_buffer);
   gtk_text_buffer_set_text (doc->text_buffer, t, length);
   gtk_source_buffer_end_not_undoable_action (doc->text_buffer);


   g_free (t);
   doc->b_saved = TRUE;
   gtk_text_buffer_set_modified (doc->text_buffer, FALSE);

  return TRUE;
}


gint get_n_page_by_filename (const gchar *filename)
{
  if (! filename)
     return -1;

  gint i = 0;
  t_note_page *t;

  GList *tl = g_list_first (dox);

  while (tl)
        {
         t = (t_note_page*) tl->data;
         if (strcmp (filename, t->file_name_utf8) == 0)
            return i;
         i++;

         tl = g_list_next (tl);
        }

  return -1;
}


static gint log_to_memo_counter = 0;

void log_to_memo (const gchar *m1, const gchar* m2, gint mode)
{
  if (! log_memo_textbuffer || ! m1)
     return;

  if ((++log_to_memo_counter) == confile.logmemo_lines_max)
     {
      log_to_memo_counter = 0;
      GtkTextIter itstart, itend;
      gtk_text_buffer_get_bounds (log_memo_textbuffer, &itstart, &itend);
      gtk_text_buffer_delete (log_memo_textbuffer, &itstart, &itend);
     }

  if (confile.msg_counter == G_MAXINT)
     confile.msg_counter = 0;

  gchar *prefix = g_strdup_printf  ("(%d) ", ++confile.msg_counter);
  gchar *st;

  if (! m2)
     {
      st = g_strconcat (prefix, m1, "\n", NULL);

      GtkTextIter it;
      gtk_text_buffer_get_iter_at_offset (log_memo_textbuffer, &it, 0);
      gtk_text_buffer_place_cursor (log_memo_textbuffer, &it);

      if (mode == LM_NORMAL)
          gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, st, -1, "lm_normal", NULL);
      else
          if (mode == LM_ERROR)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, st, -1, "lm_error", NULL);
      else
          if (mode == LM_ADVICE)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, st, -1, "lm_advice", NULL);
      else
          if (mode == LM_GREET)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, st, -1, "lm_greet", NULL);
      else
          if (mode == LM_BOLD)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, st, -1, "lm_bold", NULL);

      g_free (st);
      g_free (prefix);
      tv_logmemo_set_pos (0);
      return;
     }

  gulong len = strlen (m1) + strlen (m2) + 2024;
  gchar *message = g_malloc (len);

  g_snprintf (message, len, m1, m2);
  gchar *s = g_strconcat (prefix, message, "\n", NULL);

//  gtk_text_buffer_insert_at_cursor (log_memo_textbuffer, s, -1);

      GtkTextIter it;
      gtk_text_buffer_get_iter_at_offset (log_memo_textbuffer, &it, 0);
      gtk_text_buffer_place_cursor (log_memo_textbuffer, &it);


      if (mode == LM_NORMAL)
          gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, s, -1, "lm_normal", NULL);
      else
          if (mode == LM_ERROR)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, s, -1, "lm_error", NULL);
      else
          if (mode == LM_ADVICE)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, s, -1, "lm_advice", NULL);
      else
          if (mode == LM_GREET)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, s, -1, "lm_greet", NULL);
      else
          if (mode == LM_BOLD)
             gtk_text_buffer_insert_with_tags_by_name (log_memo_textbuffer, &it, s, -1, "lm_bold", NULL);


  g_free (message);
  g_free (s);
  g_free (prefix);
  tv_logmemo_set_pos (0);
}


gboolean find_space (gunichar ch, gpointer user_data)
{
  if (g_unichar_isspace (ch) || (ch == '\0'))
     return TRUE;
  else
      return FALSE;
}


gboolean find_quote (gunichar ch, gpointer user_data)
{
  if ((ch == '"') || (ch == '\''))
     return TRUE;
  else
      return FALSE;
}


void doc_set_new_text (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend, const gchar *text)
{
  if (! doc && ! text)
     return;

  gtk_text_buffer_begin_user_action (doc->text_buffer);

  gtk_text_buffer_delete (doc->text_buffer, itstart, itend);
  gtk_text_buffer_insert (doc->text_buffer, itstart, text, -1);

  gtk_text_buffer_end_user_action (doc->text_buffer);
}


void doc_rep_sel (t_note_page *page, const gchar *newstring)
{
  if (! page && ! newstring)
     return;

  GtkTextIter start;
  GtkTextIter end;
  if (gtk_text_buffer_get_selection_bounds (page->text_buffer, &start, &end))
     {
      if (newstring)
         doc_set_new_text (page, &start, &end, newstring);
      else
         {
          gtk_text_buffer_begin_user_action (page->text_buffer);
          gtk_text_buffer_delete (page->text_buffer, &start, &end);
          gtk_text_buffer_end_user_action (page->text_buffer);
         }
     }
   else
       if (newstring)
          {
           gtk_text_buffer_begin_user_action (page->text_buffer);
           gtk_text_buffer_set_text (page->text_buffer, newstring, -1);
           gtk_text_buffer_end_user_action (page->text_buffer);
          }
}


gchar* get_c_url (t_note_page *doc)
{
  if (! doc)
     return;

  gchar *dir;
  gchar *filename;
  gchar *seldata;

  if (doc_has_selection (doc))
     {
      dir = g_path_get_dirname (doc->file_name_local);
      seldata = doc_get_sel (doc);
      filename = create_full_path (seldata, dir);
      g_free (dir);
      g_free (seldata);
      return filename;
     }

  GtkTextIter ittemp;
  GtkTextIter itstart;
  GtkTextIter itend;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  itstart = ittemp;
  itend = ittemp;

  if (! gtk_text_iter_forward_find_char (&itend, find_quote, NULL, NULL))
      gtk_text_buffer_get_end_iter (doc->text_buffer, &itend);

  if (! gtk_text_iter_backward_find_char (&itstart, find_quote, NULL, NULL))
     gtk_text_buffer_get_start_iter (doc->text_buffer, &itstart);
    else
       gtk_text_iter_forward_char (&itstart);

  gchar *result = gtk_text_buffer_get_text (doc->text_buffer, &itstart, &itend, FALSE);

  if (g_utf8_get_char (result) == '#')
     {
      doc_goto_local_label (result);
      g_free (result);
      return NULL;
     }

  dir = g_path_get_dirname (doc->file_name_local);

  gchar *tl = result;

  if (g_utf8_get_char (tl) == '.')
     tl = g_utf8_find_next_char (tl, NULL);

  if (g_utf8_get_char (tl) == G_DIR_SEPARATOR)
     filename = create_full_path (g_utf8_find_next_char (tl, NULL), dir);
  else
      filename = create_full_path (tl, dir);

  g_free (dir);
  g_free (result);

  return filename;
}


gboolean doc_search_f (t_note_page *doc, const gchar *text)
{
  if (! doc && ! text)
      return FALSE;

  GtkTextIter ittemp;
  GtkTextIter end;
  GtkTextIter match_start;
  GtkTextIter match_end;
  gboolean result = FALSE;
  gtk_widget_grab_focus (doc->text_view);
  gtk_text_buffer_get_bounds (doc->text_buffer, &ittemp, &end);

  if (gtk_text_iter_forward_search (&ittemp, text, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, &end))
     {
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &match_end);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &match_start);
      gtk_text_view_scroll_to_iter (doc->text_view, &match_start, 0.0, TRUE, 0.0, 0.0 );
      g_free (doc->last_searched_text);
      doc->last_searched_text = g_strdup (text);
      result = TRUE;
      }

  return result;
}


gboolean doc_search_f_next (t_note_page *doc)
{
  if (! doc)
     return;

  gboolean result = FALSE;
  if (! doc->last_searched_text)
     return FALSE;

  GtkTextIter ittemp;
  GtkTextIter end;
  GtkTextIter match_start;
  GtkTextIter match_end;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  gtk_text_buffer_get_end_iter (doc->text_buffer, &end);

  if (gtk_text_iter_forward_search (&ittemp, doc->last_searched_text, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, &end))
     {
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &match_end);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &match_start);
      gtk_text_view_scroll_to_iter (doc->text_view, &match_start, 0.0, TRUE, 0.0, 0.0 );
      result = TRUE;
     }

  return result;
}


gint find_index_by_page (t_note_page *page)
{
  if (! page)
     return -1;

  gint i = -1;

  GList *p = g_list_first (dox);

  while (p)
        {
         i++;
         if (page == p->data)
            return i;

         p = g_list_next (p);
        }

  return i;
}


t_note_page* doc_ins_to_new (const gchar *newstring)
{
  if (! newstring)
     return NULL;

  t_note_page *page = doc_clear_new ();
  gtk_window_set_title (GTK_WINDOW (tea_main_window), page->file_name_utf8);
  gtk_widget_grab_focus (page->text_view);

  gtk_source_buffer_begin_not_undoable_action (page->text_buffer);
  gtk_text_buffer_set_text (page->text_buffer, newstring, -1);
  gtk_source_buffer_end_not_undoable_action (page->text_buffer);

  tabs_reload ();
  return page;
}


t_note_page* get_page_text (void)
{
  cur_text_doc = NULL;

  gint i = gtk_notebook_get_current_page (notebook1);
  if (i == -1)
    return NULL;
  else
      cur_text_doc = (t_note_page *) g_list_nth_data (dox, i);

  return cur_text_doc;
}


t_note_page* open_file_std (const gchar *f, const gchar *charset)
{
  cur_text_doc = doc_open_file (f, charset);
  tabs_reload ();
  set_title (cur_text_doc);
  return cur_text_doc;
}


//n.p. Albinoni - Adagio G-moll
void doc_move_to_pos_bw_quote (t_note_page* doc)
{
  if (! doc && ! doc->text_buffer && ! doc->text_view)
     return;

  GtkTextMark *mark;
  GtkTextIter it;
  mark = gtk_text_buffer_get_mark (doc->text_buffer, "insert");
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &it, mark);

  if (gtk_text_iter_backward_find_char (&it, find_quote, NULL, NULL))
      gtk_text_buffer_place_cursor (doc->text_buffer, &it);
}


void editor_set_pos (t_note_page *doc, guint pos)
{
  if (! doc)
     return;

  GtkTextIter it;
  GtkTextMark* m;

  gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &it, pos);
  gtk_text_buffer_place_cursor (doc->text_buffer, &it);
  m = gtk_text_buffer_get_mark (doc->text_buffer, "insert");
  if (m)
     gtk_text_view_scroll_to_mark (doc->text_view, m, 0.0, TRUE, 0.0, 0.0);
}


void editor_goto_selection (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextIter start;
  GtkTextIter end;

  if (gtk_text_buffer_get_selection_bounds (doc->text_buffer, &start, &end))
      gtk_text_view_scroll_to_iter (doc->text_view, &start, 0.0, TRUE, 0.0, 0.0);
}


gboolean doc_search_f_ncase (t_note_page *doc, const gchar *text)
{
  if (! doc || ! text)
     return FALSE;

  gboolean result = FALSE;

  gchar *f = g_utf8_strdown (text, -1);

  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter ittemp;
  GtkTextIter b_end;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  gtk_text_iter_forward_chars (&ittemp, g_utf8_strlen (f, -1));
  gint int_char_offset = gtk_text_iter_get_offset (&ittemp);
  gtk_text_buffer_get_end_iter (doc->text_buffer, &b_end);

  gchar *t = gtk_text_buffer_get_text (doc->text_buffer, &ittemp, &b_end, FALSE);
  gchar *x = g_utf8_strdown (t, -1);
  gchar *found = strstr (x, f);

  if (found)
     {
      glong i = g_utf8_pointer_to_offset (x, found) + int_char_offset;
      gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &start, i);
      gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &end, i + g_utf8_strlen (f, -1));

      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &start);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &end);
      g_free (doc->last_searched_text);
      doc->last_searched_text = g_strdup (text);

      result = TRUE;
     }

  g_free (t);
  g_free (x);
  g_free (f);

  return result;
}


gboolean doc_search_f_next_ncase (t_note_page *doc)
{
  if (! doc && ! doc->last_searched_text)
      return FALSE;

  GtkTextIter start;
  GtkTextIter end;

  gchar *f = g_utf8_strdown (doc->last_searched_text, -1);

  gboolean result = FALSE;
  GtkTextIter ittemp;
  GtkTextIter b_start;
  GtkTextIter b_end;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  gtk_text_iter_forward_chars (&ittemp, g_utf8_strlen (f, -1));
  gint int_char_offset = gtk_text_iter_get_offset (&ittemp);
  gtk_text_buffer_get_end_iter (doc->text_buffer, &b_end);

  gchar *t = gtk_text_buffer_get_text (doc->text_buffer, &ittemp, &b_end, FALSE);
  gchar *x = g_utf8_strdown (t, -1);
  gchar *found = strstr (x, f);

  if (found)
     {
      glong i = g_utf8_pointer_to_offset (x, found) + int_char_offset;

      gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &start, i);
      gtk_text_buffer_get_iter_at_offset (doc->text_buffer, &end, i + g_utf8_strlen (f, -1));
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &start);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &end);

      result = TRUE;
     }

  g_free (t);
  g_free (x);
  g_free (f);
  return result;
}


gchar* upcase_each_first_letter_ofw (t_note_page *doc)
{
  if (! doc)
     return NULL;

  gunichar u;

  GtkTextIter start;
  GtkTextIter end;

  if (! gtk_text_buffer_get_selection_bounds (doc->text_buffer, &start, &end))
      return NULL;

  GString* gs = g_string_sized_new (1024);

  do
    {
     if (gtk_text_iter_equal (&start, &end))
        break;

     u = gtk_text_iter_get_char (&start);

     if ( gtk_text_iter_starts_word (&start))
        u = g_unichar_toupper (u);

     gs = g_string_append_unichar (gs, u);
    }
   while ( gtk_text_iter_forward_char (&start) );

  gchar *s = gs->str;
  g_string_free (gs, FALSE);

  return s;
}


void doc_move_cursor_backw (t_note_page *doc, gint i)
{
  if (! doc)
      return;

  GtkTextIter ittemp;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  if (gtk_text_iter_backward_chars (&ittemp, i))
     {
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &ittemp);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &ittemp);
     }
}


static gboolean predicate_find_tag_end (gunichar ch, gpointer user_data)
{
  if (ch == '>')
     return TRUE;
  else
      return FALSE;
}


void doc_move_cursor_backw_middle_tags (t_note_page *doc)
{
  if (! doc)
      return;

  GtkTextIter ittemp;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  if (gtk_text_iter_backward_find_char (&ittemp, predicate_find_tag_end, NULL, NULL))
  if (gtk_text_iter_backward_find_char (&ittemp, predicate_find_tag_end, NULL, NULL))
  if (gtk_text_iter_forward_char (&ittemp))
     {
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &ittemp);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &ittemp);
     }
}


void doc_header_source_switch (t_note_page *doc)
{
  if (! doc)
      return;

  gchar *f = get_c_header_fname (doc->file_name_utf8);
  if (f)
     {
      handle_file (f, doc->encoding, 0, FALSE);

      g_free (f);
      return;
     }

  f = get_c_source_fname (doc->file_name_utf8);

  if (f)
     {
      handle_file (f, doc->encoding, 0, FALSE);
      g_free (f);
      return;
     }
}


void doc_toggle_images_visibility (t_note_page *p)
{
  if (! p)
      return;

  GtkTextIter it_start;
  GtkTextIter it_end;

  if (! p->toggle_images_visibility)
     {
      p->toggle_images_visibility = TRUE;
      gchar *f;
      gchar *t;
      GdkPixbuf *pb;

      gtk_text_buffer_get_start_iter (p->text_buffer, &it_start);
      gtk_text_buffer_get_end_iter (p->text_buffer, &it_end);

      while (gtk_text_iter_forward_search (&it_start, "src=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &it_start, &it_end, NULL))
           {
            it_start = it_end;
            if (gtk_text_iter_forward_find_char (&it_end, find_quote, NULL, NULL))
               {
                f = gtk_text_buffer_get_text (p->text_buffer, &it_start, &it_end, FALSE);
                t = get_full_fname (p->file_name_utf8, f);
                if (t)
                if (is_image (t))
                   {
                    pb = gdk_pixbuf_new_from_file_at_size (t, confile.thumb_width, confile.thumb_height, NULL);
                    gtk_text_buffer_insert_pixbuf (p->text_buffer, &it_end, pb);
                    g_object_unref (pb);
                    gtk_text_buffer_set_modified (p->text_buffer, FALSE);
                   }

                g_free (f);
                g_free (t);
               }

            it_start = it_end;
          }

      while (gtk_text_iter_forward_search (&it_start, "SRC=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &it_start, &it_end, NULL))
           {
            it_start = it_end;
            if (gtk_text_iter_forward_find_char (&it_end, find_quote, NULL, NULL))
               {
                f = gtk_text_buffer_get_text (p->text_buffer, &it_start, &it_end, FALSE);
                t = get_full_fname (p->file_name_utf8, f);
                if (t)
                if (is_image (t))
                   {
                    pb = gdk_pixbuf_new_from_file_at_size (t, confile.thumb_width, confile.thumb_height, NULL);
                    gtk_text_buffer_insert_pixbuf (p->text_buffer, &it_end, pb);
                    g_object_unref (pb);
                    gtk_text_buffer_set_modified (p->text_buffer, FALSE);
                   }

                 g_free (f);
                 g_free (t);
                }
           it_start = it_end;
          }
       }
  else
     {
      p->toggle_images_visibility = FALSE;
      gtk_text_buffer_get_start_iter (p->text_buffer, &it_start);
      do
        {
         if (gtk_text_iter_get_pixbuf (&it_start))
            {
             it_end = it_start;
             if (gtk_text_iter_forward_char (&it_end))
                gtk_text_buffer_delete (p->text_buffer, &it_start, &it_end);
             gtk_text_buffer_set_modified (p->text_buffer, FALSE);
            }
        }
      while (gtk_text_iter_forward_char (&it_start));
     }
}


void doc_close_all (void)
{
  gint c = gtk_notebook_get_n_pages (notebook1);
  if (c == -1)
     return;

  c--;
  gint i;

  t_note_page *page;

  for (i = 0; i <= c; ++i)
      {
       page = glist_find_by_index (dox, 0);
       dox = g_list_remove (dox, page);
       page_free (page);
       gtk_notebook_remove_page (notebook1, 0);
      }

  update_recent_list_menu (FALSE);
}


GList* add_session_item_composed (GList *list, t_note_page *doc)
{
  if (! doc)
     return list;

  GList *l = g_list_append (list, g_strdup_printf ("%s,%s,%d",
                            doc->file_name_utf8, doc->encoding,
                            editor_get_pos (doc)));
  return l;
}


GList* add_recent_item_composed (GList *list, t_note_page *doc)
{
  if (! doc)
     return list;

  GList *l = g_list_prepend (list, g_strdup_printf ("%s,%s,%d",
                             doc->file_name_utf8, doc->encoding,
                             editor_get_pos (doc)));

  return l;
}


void current_tag_close (t_note_page *doc, gint mode)
{
  if (! doc)
     return;

  gboolean found = FALSE;

  GtkTextIter ittemp;
  GtkTextIter itstart;
  GtkTextIter itend;
  gunichar u;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  itstart = ittemp;
  itend = ittemp;

  while (gtk_text_iter_backward_char (&itstart))
        {
         u = gtk_text_iter_get_char (&itstart);
         if (u == '/')
            {
             found = FALSE;
             break;
            }

         if (u == '<')
            {
             found = TRUE;
             break;
            }
        }

  if (! found)
     return;

  if (! gtk_text_iter_forward_char (&itstart))
     return;

  itend = itstart;
  while (gtk_text_iter_forward_char (&itend))
        {
         u = gtk_text_iter_get_char (&itend);
         if (! g_unichar_isalpha (u) && ! g_unichar_isdigit (u) && u != '_')
            break;
        }

  gchar *tag = gtk_text_buffer_get_text (doc->text_buffer, &itstart, &itend, FALSE);

  if (def_mm == MM_HTML)
  if (str_in (tag, "br", "hr", "img", NULL))
     {
      gtk_text_buffer_insert (doc->text_buffer, &ittemp, ">", -1);
      g_free (tag);
      return;
     }

  gchar *complete;
  if (mode == 0)
      complete = g_strconcat ("></", tag, ">", NULL);
  else
      complete = g_strconcat ("</", tag, ">", NULL);

  gtk_text_buffer_insert (doc->text_buffer, &ittemp, complete, -1);

  g_free (tag);
  g_free (complete);
  doc_move_cursor_backw_middle_tags (doc);
}


void doc_goto_local_label (const gchar *l)
{
  if (! l)
     return;

  gchar *t = g_utf8_find_next_char (l, NULL);
  if (! t)
     return;

  gchar *s = g_strconcat ("<a name=\"", t, NULL);

  GtkTextIter iter;
  GtkTextIter match_start;

  GtkTextMark *m = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &iter, m);

  if (gtk_text_iter_forward_search  (&iter, s, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, NULL, NULL))
     {
      gtk_text_buffer_place_cursor (cur_text_doc->text_buffer, &match_start );
      gtk_text_view_scroll_to_iter (cur_text_doc->text_view, &match_start, 0.0, TRUE, 0.0, 0.0 );
      gtk_text_view_place_cursor_onscreen (GTK_TEXT_VIEW (cur_text_doc->text_view));
     }

  g_free (s);
}


void set_last_dir (t_note_page *doc)
{
  if (! doc)
     return;

  if (g_file_test (doc->file_name_local, G_FILE_TEST_EXISTS))
     last_dir = ch_str (last_dir, g_path_get_dirname (doc->file_name_local));
}


void swap_lines (t_note_page *doc, gboolean with_upper)
{
  if (! doc)
     return;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  GtkTextIter it_line1_start;
  GtkTextIter it_line1_end;
  GtkTextIter it_line2_start;
  GtkTextIter it_line2_end;

  it_line1_start = iter;
  gtk_text_iter_set_line_offset (&it_line1_start, 0);

  it_line1_end = iter;
  if (gtk_text_iter_forward_line (&it_line1_end))
     gtk_text_iter_backward_char (&it_line1_end);

  if (with_upper)
      gtk_text_iter_backward_line (&it_line1_start);
  else
      {
       if (gtk_text_iter_forward_lines (&it_line1_end, 2))
          gtk_text_iter_backward_char (&it_line1_end);
      }

  gchar *t = gtk_text_buffer_get_slice (doc->text_buffer,
                                        &it_line1_start,
                                        &it_line1_end,
                                        TRUE);

  if (! t)
     return;

  GList *list = glist_from_string (t);
  g_free (t);

  if (g_list_length (list) < 2)
     {
      glist_strings_free (list);
      return;
     }

  GList *tl1 = g_list_first (list);
  GList *tl2 = g_list_next (list);

  t = tl2->data;
  tl2->data = tl1->data;
  tl1->data = t;

  t = string_from_glist_sep (list, "\n");
  glist_strings_free (list);

  gtk_text_buffer_delete (doc->text_buffer, &it_line1_start, &it_line1_end);
  gtk_text_buffer_insert (doc->text_buffer, &it_line1_start, t, -1);

  g_free (t);
}


void doc_save_all (void)
{
  if (! dox)
     return;

  t_note_page *page;

  GList *tl = g_list_first (dox);

  log_to_memo (_("Saving all documents... begin"), NULL, LM_BOLD);

  while (tl)
        {
         page = tl->data;
         if (page && page->b_saved)
            {
             gtk_text_view_set_editable (page->text_view, FALSE);
             text_doc_save (page, page->file_name_local);
             gtk_text_view_set_editable (page->text_view, TRUE);
             log_to_memo (_("%s is saved"), page->file_name_utf8, LM_NORMAL);
            }

         tl = g_list_next (tl);
        }

  log_to_memo (_("Saving all documents... end"), NULL, LM_BOLD);
}


void doc_search_and_paint (t_note_page *doc, const gchar *str)
{
  if (! doc)
      return;

  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gtk_text_buffer_get_bounds (doc->text_buffer, &start, &end);
  match_end = start;

  while (gtk_text_iter_forward_search (&match_end, str, GTK_TEXT_SEARCH_TEXT_ONLY,
                                       &match_start, &match_end, NULL))
         gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "paint"), &match_start, &match_end);

}


void doc_paint_lines_bigger_than (t_note_page *doc, gint val)
{
  if (! doc)
      return;

  GtkTextIter iter;
  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gchar *text;

  gtk_text_buffer_get_bounds (doc->text_buffer, &start, &end);
  iter = start;

  while (gtk_text_iter_forward_char (&iter))
        {
         if (gtk_text_iter_starts_line (&iter))
            match_start = iter;
         if (gtk_text_iter_ends_line (&iter))
            {
             match_end = iter;
             text = gtk_text_iter_get_text (&match_start, &match_end);
             if (g_utf8_strlen (text, -1) > val)
                gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "paint"), &match_start, &match_end);

             g_free (text);
            }
        }
}


void session_save_to_file (const gchar *filename)
{
  if (! filename)
     return;

  if (! dox)
     unlink (filename);

  GList *p = g_list_first (dox);
  if (! p) //a hack for the confile.lastsession - to delete last session if no files are opened
     {
      unlink (filename);
      return;
     }

  GList *t = NULL;

  while (p)
        {
         t = add_session_item_composed (t, p->data);
         p = g_list_next (p);
        }

  glist_save_to_file (t, filename);
  g_list_free (t);
}


void session_open_from_file (const gchar *filename)
{
  if (! filename)
     return;

  GList *list = load_file_to_glist (filename);

  if (! list)
      return;

  GList *t = g_list_first (list);

  while (t)
       {
        execute_recent_item (t->data);
        t = g_list_next (t);
       }

  glist_strings_free (list);
}


void doc_undo (t_note_page *doc)
{
  if (! doc)
     return;

#ifdef LEGACY_MODE
  if (undomgr_can_undo (doc->um))
      undomgr_undo (doc->um);

#else

  if (gtk_source_buffer_can_undo (doc->text_buffer))
      gtk_source_buffer_undo (doc->text_buffer);

#endif
}


void doc_redo (t_note_page *doc)
{
  if (! doc)
     return;

  if (gtk_source_buffer_can_redo (doc->text_buffer))
     gtk_source_buffer_redo (doc->text_buffer);
}


void doc_save_emergency (void)
{
  if (! get_page_text ())
     return;

  gchar *f;

  if (g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     f = g_strconcat (cur_text_doc->file_name_local, ".crash", NULL);
   else
       f = g_strdup (confile.crash_file);

  doc_save_buffer_to_file (cur_text_doc->text_buffer, f);

  g_free (f);
}


gboolean find_space_or_lf (gunichar ch, gpointer user_data)
{
  if ((ch == ' ') || (ch == '\n'))
     return TRUE;
  else
      return FALSE;
}


gchar* doc_get_cur_mispelled (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend)
{
  if (! doc)
     return NULL;

  GtkTextIter ittemp;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  if (! gtk_text_iter_has_tag (&ittemp, get_tag_by_name (doc, "spell_err")))
     return NULL;

  *itstart = ittemp;
  *itend = ittemp;

  while (gtk_text_iter_forward_char (itend))
        {
         if (! gtk_text_iter_has_tag (itend, get_tag_by_name (doc, "spell_err")))
            break;
        }

  while (gtk_text_iter_backward_char (itstart))
        {
         if (! gtk_text_iter_has_tag (itstart, get_tag_by_name (doc, "spell_err")))
            break;
        }

  return (gtk_text_buffer_get_text (doc->text_buffer, itstart, itend, FALSE));
}


/*parts of this code are taken from

 * gedit-languages-manager.c, gedit_prefs_manager.c
 * This file is part of gedit
 *
 * Copyright (C) 2003 - Paolo Maggi

*/

#ifdef CHAI_MODE
/*
static GConfClient* gconf_client = NULL;


static gchar* my_gdk_color_2_string (GdkColor color)
{
  return g_strdup_printf("#%02x%02x%02x", color.red / 256, color.green / 256, color.blue / 256);
}


static gchar* tag_style_to_string (const GtkSourceTagStyle *style)
{
  if (! style)
     return NULL;

  gchar *background = my_gdk_color_2_string (style->background);
  gchar *foreground = my_gdk_color_2_string (style->foreground);

  gchar *res = g_strdup_printf ("%d/%s/%s/%d/%d/%d/%d",
                                style->mask,
                                foreground,
                                background,
                                style->italic,
                                style->bold,
                                style->underline,
                                style->strikethrough);

  g_free (foreground);
  g_free (background);

  return res;
}


void language_set_tag_style (GtkSourceLanguage *language,
                             const gchar *tag_id,
                             const GtkSourceTagStyle *style)
{
  if (! language || ! tag_id)
     return;

  gchar *key = get_gconf_key (language, tag_id);
  if (! key)
     return;

  if (! style)
     {
      gconf_client_unset (gconf_client, key, NULL);
      gtk_source_language_set_tag_style (language, tag_id, NULL);
     }
  else
      {
       gchar *value = tag_style_to_string (style);
       gconf_client_set_string (gconf_client, key, value, NULL);
       g_free (value);
      }

  g_free (key);
}


gchar* get_gconf_key (GtkSourceLanguage *language, const gchar *tag_id)
{
  if (! language || ! tag_id)
     return NULL;

  gchar *lang_id = gtk_source_language_get_id (language);
  gchar *key = g_strconcat (CHAI_SYNTAX_HL_DIR, G_DIR_SEPARATOR_S, lang_id, G_DIR_SEPARATOR_S, tag_id, NULL);
  g_free (lang_id);
  return key;
}


void hl_init (void)
{
  g_type_init();
  gconf_client = gconf_client_get_default ();

  if (! gconf_client)
     dbm ("Cannot initialize gconf_client");
}


GList* get_supported_langs (void)
{
  GSList *l = gtk_source_languages_manager_get_available_languages (source_languages_manager);
  GSList *t = l;
  GList *r = NULL;
  while (t)
       {
        r = g_list_append (r, gtk_source_language_get_name (t->data));
        t = g_slist_next (t);
       }

  return r;
}


 //from gedit-languages-manager.c, modified by Roxton
 
 //* Copyright (C) 2003 - Paolo Maggi

void language_init_tag_styles (GtkSourceLanguage *language)
{
  if (! language)
     return;

  GSList *tags = gtk_source_language_get_tags (language);

  gchar *group_name = gtk_source_language_get_id (language);
  GError *error = NULL;

  GSList *l = tags;

  while (l)
        {
         GtkSourceTag *tag = l->data;

         gchar *id = gtk_source_tag_get_id (tag);
         gchar *key = get_gconf_key (language, id);

         gchar *value = gconf_client_get_string (gconf_client, key, NULL);

         error = NULL;

         if (value)
            {
             GtkSourceTagStyle *style = string_to_tag_style (value);
             if (style)
                {
                // g_print ("%s: %s -- %s -- %s\n", id, group_name, key, value);
                 gtk_source_language_set_tag_style (language, id, style);
                 gtk_source_tag_style_free (style);
                }
             else
                 g_print ("%s --- key %s contains an invalid value\n", group_name, id);

             g_free (value);
            }

         g_free (key);
         g_free (id);
         l = g_slist_next (l);
       }

   g_slist_foreach (tags, (GFunc)g_object_unref, NULL);
   g_slist_free (tags);
   g_free (group_name);
}


GtkSourceTagStyle* string_to_tag_style (const gchar *string)
{
  if (! string)
     return NULL;

  GtkSourceTagStyle *style = gtk_source_tag_style_new ();
  gchar **items = g_strsplit (string, G_DIR_SEPARATOR_S, -1);

  if (get_gchar_array_size (items) != 7)
      {
       g_strfreev (items);
       return style;
      }

  style->is_default = FALSE;
  style->mask = items [0][0] - '0';

  gdk_color_parse (items [1], &style->foreground);
  gdk_color_parse (items [2], &style->background);

  style->italic = items [3][0] - '0';
  style->bold = items [4][0] - '0';
  style->underline = items [5][0] - '0';
  style->strikethrough = items [6][0] - '0';

  g_strfreev (items);
  return style;
}

///////////////


void save_tag (GtkSourceLanguage *language, GtkSourceTag *tag)
{
  if (! language || ! tag)
      return;

  GtkSourceTagStyle *style = gtk_source_tag_get_style (tag);

  gchar *group_name = gtk_source_language_get_id (language);
  gchar *id = gtk_source_tag_get_id (tag);
  language_set_tag_style (language, id, style);

  g_free (group_name);
  g_free (id);
  g_free (style);
}


void langs_load (void)
{
  GSList *l = gtk_source_languages_manager_get_available_languages (source_languages_manager);

  if (! l)
     return;

  GSList *t = l;

  while (t)
        {
         language_init_tag_styles (t->data);
         t = g_slist_next (t);
        }
}


void done_hl (void)
{
  g_object_unref (gconf_client);
}


void document_apply_hl (t_note_page* doc)
{
  if (! doc)
     return;

  GnomeVFSFileInfo *info = gnome_vfs_file_info_new ();
  GnomeVFSResult r = gnome_vfs_get_file_info (doc->file_name_local, info, GNOME_VFS_FILE_INFO_GET_MIME_TYPE);

  gchar *m = gnome_vfs_file_info_get_mime_type (info);
  if (! m)
     {
      gnome_vfs_file_info_unref (info);
      return;
     }

  GtkSourceLanguage *l = NULL;

#if defined(DARWIN)
  // GnomeVFS see xml files type as application/xml
  // But GtkSourceView wants them as text/xml
  // at least with gnomevfs 1.0.5 and gtksourceview 1.4.2
  // So we change the string on the fly
  // And get syntax highlighting with xml files :-)
  gchar *m2 = g_strdup (m);

  if (strcmp (m2, "application/xml") == 0)
      m2 = ch_str (m2, "text/xml");
  if (strcmp (m2, "application/xslt+xml") == 0)
      m2 = ch_str (m2, "text/xml");

  if (m2)
     l = gtk_source_languages_manager_get_language_from_mime_type (source_languages_manager, m2);
#else
     l = gtk_source_languages_manager_get_language_from_mime_type (source_languages_manager, m);
#endif

  if (l)
     {
      gtk_source_buffer_set_language (doc->text_buffer, l);
      gtk_source_buffer_set_highlight (doc->text_buffer, TRUE);
     }

#if defined(DARWIN)
   g_free (m2);
#endif

  gnome_vfs_file_info_unref (info);
}
*/
#endif



static gchar* my_gdk_color_2_string (GdkColor color)
{
  return g_strdup_printf("#%02x%02x%02x", color.red / 256, color.green / 256, color.blue / 256);
}



//fropm test-widget.c (GtkSourceView source)

/* Note this is wrong for several reasons, e.g. g_pattern_match is broken
 * for glob matching. */
static GtkSourceLanguage *
get_language_for_filename (const gchar *filename)
{
        const gchar * const *languages;
        gchar *filename_utf8;
        GtkSourceLanguageManager *manager;

        filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
        g_return_val_if_fail (filename_utf8 != NULL, NULL);

        manager = gtk_source_language_manager_get_default ();
        languages = gtk_source_language_manager_get_language_ids (manager);

        while (*languages != NULL)
        {
                GtkSourceLanguage *lang;
                gchar **globs, **p;

                lang = gtk_source_language_manager_get_language (manager,
                                                                 *languages);
                //g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (lang), NULL);
		if (! lang)
		    return NULL;
		
		
                ++languages;

                globs = gtk_source_language_get_globs (lang);
                if (globs == NULL)
                        continue;

                for (p = globs; *p != NULL; p++)
                {
                        if (g_pattern_match_simple (*p, filename_utf8))
                        {
                                g_strfreev (globs);
                                g_free (filename_utf8);

                                return lang;
                        }
                }

                g_strfreev (globs);
        }

        g_free (filename_utf8);
        return NULL;
}


void document_apply_hl (t_note_page* doc)
{
  if (! doc)
     return;

  GtkSourceLanguage *l = NULL;

  l = get_language_for_filename (doc->file_name_local);
  if (l)
     {
      gtk_source_buffer_set_language (doc->text_buffer, l);
      gtk_source_buffer_set_highlight_syntax (doc->text_buffer, TRUE);
     }
}


void doc_set_text (t_note_page *doc, const gchar *text)
{
  if (! doc || ! text)
      return;

  gtk_text_buffer_set_text (doc->text_buffer, text, -1);
}


//n.p. The Pixies - Velouria
gboolean doc_replace_from_cursor_ones (t_note_page *doc,
                                       const gchar *what_to_find,
                                       const gchar *replace_to,
                                       gboolean undoable)
{
  if (! doc || ! what_to_find || ! replace_to)
    return FALSE;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  GtkTextIter match_start = iter;
  GtkTextIter match_end = iter;

  if (! gtk_text_iter_forward_search (&iter, what_to_find, GTK_TEXT_SEARCH_TEXT_ONLY,
                                      &match_start, &match_end, NULL))
     return FALSE;

  if (undoable)
     gtk_text_buffer_begin_user_action (doc->text_buffer);

  gtk_text_buffer_delete (doc->text_buffer, &match_start, &match_end);
  gtk_text_buffer_insert (doc->text_buffer, &match_start, replace_to, -1);

  if (undoable)
     gtk_text_buffer_end_user_action (doc->text_buffer);

  gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &match_start);
  gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &match_start);

  return TRUE;
}


//n.p. The Pixies - The Thing
void doc_replace_from_cursor_all (t_note_page *doc, const gchar *what_to_find, const gchar *replace_to)
{
  if (! doc)
     return;

  gint c = 0;

  gtk_text_buffer_begin_user_action (doc->text_buffer);

  while (doc_replace_from_cursor_ones (doc, what_to_find, replace_to, FALSE))
        c++;

  gtk_text_buffer_end_user_action (doc->text_buffer);

  gchar *s = g_strdup_printf (_("%d words were replaced"), c);
  log_to_memo (s, NULL, LM_NORMAL);
  g_free (s);
}


void doc_smarthomeend (t_note_page *doc, GdkEventKey *event)
{
  if (! doc)
    return;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);

  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  if (event->keyval == GDK_KEY_End)
  if (gtk_text_view_forward_display_line (doc->text_view, &iter))
     gtk_text_iter_backward_char (&iter);

  if (event->keyval == GDK_KEY_Home)
    gtk_text_view_backward_display_line_start (doc->text_view, &iter);

  gtk_text_buffer_place_cursor (doc->text_buffer, &iter);
  gtk_text_view_scroll_to_iter (doc->text_view, &iter, 0.0, FALSE, 0.0, 0.0);
}


//from gedit-viev.c
static G_CONST_RETURN gchar* get_widget_name (GtkWidget *w)
{
  const gchar *name;

  name = gtk_widget_get_name (w);
  if (! name)
     return NULL;

  if (strcmp (name, g_type_name ( G_OBJECT_TYPE (w))) == 0)
    {
     static guint d = 0;
     gchar *n = g_strdup_printf ("%s_%u_%u", name, d, (guint) g_random_int);
     d++;
     gtk_widget_set_name (w, n);
     g_free (n);

     name = gtk_widget_get_name (w);
    }

  return name;
}


void modify_cursor_color (GtkWidget *textview,  const gchar *color_name)
{

#ifdef LEGACY_MODE

  static const char cursor_color_rc[] = "style \"svs-cc\"\n"
                                        "{\n"
                                        "GtkTextView::cursor-color=\"#%04x%04x%04x\"\n"
                                         "}\n"
                                         "widget \"*.%s\" style : application \"svs-cc\"\n";

#else

  static const char cursor_color_rc[] = "style \"svs-cc\"\n"
                                        "{\n"
                                        "GtkSourceView::cursor-color=\"#%04x%04x%04x\"\n"
                                         "}\n"
                                        "widget \"*.%s\" style : application \"svs-cc\"\n";
#endif

  GdkColor color;
  gdk_color_parse (color_name, &color);

  const gchar *name = get_widget_name (textview);
  gchar *rc_temp = g_strdup_printf (cursor_color_rc, color.red, color.green, color.blue, name);
  gtk_rc_parse_string (rc_temp);
  gtk_widget_reset_rc_styles (textview);
  g_free (rc_temp);
}


/////////////////////////////


//n.p. Nirvana - I Hate Myself And I Want To Die
gchar* doc_get_current_word (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend)
{
  if (! doc)
     return NULL;

  GtkTextIter ittemp;
  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  gunichar c;

  *itstart = ittemp;
  *itend = ittemp;

  if (! gtk_text_iter_forward_find_char (itend, find_space, NULL, NULL))
      gtk_text_buffer_get_end_iter (doc->text_buffer, itend);

  if (! gtk_text_iter_backward_find_char (itstart, find_space, NULL, NULL))
     gtk_text_buffer_get_start_iter (doc->text_buffer, itstart);
    else
       gtk_text_iter_forward_char (itstart);

  return (gtk_text_buffer_get_text (doc->text_buffer, itstart, itend, FALSE));
}


gchar* doc_move_to_next_word (t_note_page *doc) //from the cursor's position
{
  if (! doc)
    return NULL;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  GtkTextIter start = iter;
  GtkTextIter end = iter;

  gtk_text_iter_forward_word_end (&end);

  gchar *s = gtk_text_buffer_get_text (doc->text_buffer, &start, &end, FALSE);

  while (gtk_text_iter_forward_char (&end))
        {
         if (gtk_text_iter_get_char (&end) == 0)
            break;

         if (gtk_text_iter_inside_word (&end))
            break;
         }

  gtk_text_buffer_place_cursor (doc->text_buffer, &end);
  gtk_text_view_scroll_to_mark (doc->text_view,
                                gtk_text_buffer_get_insert (doc->text_buffer),
                                0.0, FALSE, 0.0, 0.0);
  return s;
}


gboolean doc_move_to_next_word_sel_if_regexp (t_note_page *doc, const gchar *text) //from the cursor's position
{
  if (! doc)
    return FALSE;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

   if (gtk_text_iter_is_end (&iter))
      return FALSE;

  GtkTextIter start = iter;
  GtkTextIter end = iter;

  gtk_text_iter_forward_word_end (&end);

  gchar *s = gtk_text_buffer_get_text (doc->text_buffer, &start, &end, FALSE);

  if (s)
     {
      if (g_pattern_match_simple (text, s))
         gtk_text_buffer_apply_tag (doc->text_buffer, get_tag_by_name (doc, "paint"), &start, &end);

      g_free (s);
     }

  while (gtk_text_iter_forward_char (&end))
        {
         if (gtk_text_iter_get_char (&end) == 0)
            break;

         if (gtk_text_iter_inside_word (&end))
            break;
         }

  gtk_text_buffer_place_cursor (doc->text_buffer, &end);
  gtk_text_view_scroll_to_mark (doc->text_view,
                                gtk_text_buffer_get_insert (doc->text_buffer),
                                0.0, FALSE, 0.0, 0.0);
  return TRUE;
}


gboolean text_doc_save_silent (t_note_page *doc, const gchar *a_filename)
{
  if (! doc || ! a_filename)
     return FALSE;

  if (doc->readonly)
     {
      log_to_memo (_("This file is read-only"), NULL, LM_ERROR);
      return FALSE;
     }

  gboolean result = FALSE;

  if (g_file_test (a_filename, G_FILE_TEST_EXISTS))
     if (! is_writable (a_filename))
        {
         dlg_info (_("Warning!"), _("%s is not writable for you!"), a_filename);
         log_to_memo (_("%s is not writable for you!"), a_filename, LM_ERROR);
         return FALSE;
        }

  if (g_file_test (a_filename, G_FILE_TEST_IS_DIR))
     {
      log_to_memo (_("And how I can save this text file as a DIRECTORY?!"), NULL, LM_ERROR);
      return FALSE;
     }

  if (g_utf8_collate (doc->encoding, "UTF-8") == 0)
      result = doc_save_buffer_to_file (doc->text_buffer, a_filename);
  else
      result = doc_save_buffer_to_file_iconv (doc->text_buffer, a_filename, doc->encoding);

  if (result)
     log_to_memo (_("%s is saved"), a_filename, LM_NORMAL);

  return result;
}


gboolean doc_search_f_silent (t_note_page *doc, const gchar *text)
{
  if (! doc || ! text)
      return FALSE;

  GtkTextIter ittemp;
  GtkTextIter end;
  GtkTextIter match_start;
  GtkTextIter match_end;
  gboolean result = FALSE;
  gtk_widget_grab_focus (doc->text_view);
  gtk_text_buffer_get_bounds (doc->text_buffer, &ittemp, &end);

  if (gtk_text_iter_forward_search (&ittemp, text, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, &end))
     {
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "insert", &match_start);
      gtk_text_buffer_move_mark_by_name (doc->text_buffer, "selection_bound", &match_start);
      gtk_text_view_scroll_to_iter (doc->text_view, &match_start, 0.0, TRUE, 0.0, 0.0 );
      g_free (doc->last_searched_text);
      doc->last_searched_text = g_strdup (text);
      result = TRUE;
      }

  return result;
}


void doc_update_statusbar (t_note_page *doc)
{
  if (! doc)
     return;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &iter, mark);

  gchar *m;
  if (confile.show_cursor_pos)
     m = g_strdup_printf ("%s %s %d:%d", doc->encoding, doc->end_of_line, doc->row, doc->col);
  else
      m = g_strdup_printf ("%s %s", doc->encoding, doc->end_of_line, doc->row, doc->col);

  statusbar_msg (m);
  g_free (m);
}


void set_title (t_note_page *t)
{
  if (! t)
     return;

  if (! t->file_name_utf8)
     return;

  if (! g_utf8_validate (t->file_name_utf8, -1, NULL))
      return;

  gchar *nc;

  if (confile.main_wnd_show_full_path)
      nc = g_strdup (t->file_name_utf8);
  else
      nc = g_path_get_basename (t->file_name_utf8);

  gtk_window_set_title (GTK_WINDOW (tea_main_window), nc);

  doc_update_statusbar (t);

 /* gtk_tooltips_set_tip (tooltips, t->tab_label,
                        t->file_name_utf8,
                        t->file_name_utf8);
*/
 
 gtk_widget_set_tooltip_text (t->tab_label, t->file_name_utf8);
  set_last_dir (t);
  g_free (nc);
}


gboolean doc_has_selection (t_note_page *doc)
{
  return gtk_text_buffer_get_selection_bounds (doc->text_buffer, NULL, NULL);
}


/*
 *  test-widget.c
 *
 *  Copyright (C) 2001
 *  Mikael Hermansson<tyan@linux.se>
 *
 *  Copyright (C) 2003 - Gustavo Giraldez <gustavo.giraldez@gmx.net>
*/
void update_cursor_position (GtkTextBuffer *text_buffer, gpointer user_data)
{
  if (! confile.show_cursor_pos)
      return;

  t_note_page *doc = user_data;

  gint chars; //, tabwidth;
  GtkTextIter iter, start;
  doc->text_view;
  //tabwidth = gtk_source_view_get_tabs_width (doc->text_view);
  gtk_text_buffer_get_iter_at_mark (text_buffer, &iter, gtk_text_buffer_get_insert (text_buffer));
  chars = gtk_text_iter_get_offset (&iter);
  doc->row = gtk_text_iter_get_line (&iter) + 1;

  start = iter;
  gtk_text_iter_set_line_offset (&start, 0);
  doc->col = 0;

  while (! gtk_text_iter_equal (&start, &iter))
        {
         if (gtk_text_iter_get_char (&start) == '\t')
            doc->col += (confile.tab_size - (doc->col % confile.tab_size));
         else
             ++doc->col;

         gtk_text_iter_forward_char (&start);
        }

  doc_update_statusbar (doc);
}


void move_cursor_cb (GtkTextBuffer *buffer,
                     GtkTextIter *cursoriter,
                     GtkTextMark *mark,
                     gpointer user_data)
{
  if (! confile.show_cursor_pos)
      return;

  if (mark != gtk_text_buffer_get_insert (buffer))
      return;

  update_cursor_position (buffer, user_data);
}


void doc_tabs_to_spaces (t_note_page *doc, gint tabsize)
{
  if (! doc)
     return;

  if (tabsize < 1)
     tabsize = 1;

  gchar *new_text = g_strnfill (tabsize, ' ');
  gchar *text = g_strdup ("\t");

  if (doc_search_f (doc, text))
     {
      doc_rep_sel (doc, new_text);
      while (doc_search_f_next (doc))
            doc_rep_sel (doc, new_text);
     }

  g_free (text);
  g_free (new_text);
}


void doc_spaces_to_tabs (t_note_page *doc, gint tabsize)
{
  if (! doc)
     return;

  if (tabsize < 1)
     tabsize = 1;

  gchar *text = g_strnfill (tabsize, ' ');
  gchar *new_text = g_strdup ("\t");

  if (doc_search_f (doc, text))
     {
      doc_rep_sel (doc, new_text);
      while (doc_search_f_next (doc))
            doc_rep_sel (doc, new_text);
     }

  g_free (text);
  g_free (new_text);
}

//from Gedit - plugin
/*
 * docinfo.c
 * This file is part of gedit
 *
 * Copyright (C) 2002 Paolo Maggi

modified by roxton
*/

void make_stats (t_note_page *doc)
{
  if (! doc)
     return;

  if (! doc->text_buffer)
     return;

  gboolean selected = TRUE;

  gchar *text;
  PangoLogAttr *attrs;
  gint words = 0;
  gint chars = 0;
  gint white_chars = 0;
  gint lines = 0;
  gint bytes = 0;
  gint i;
  gchar *tmp_str;

  selected = doc_has_selection (doc);

  if (! selected)
      text = doc_get_buf (doc->text_buffer);
  else
      text = doc_get_sel (doc);

  if (! text)
     return;

  if (! g_utf8_validate (text, -1, NULL))
     return;

  lines = gtk_text_buffer_get_line_count (doc->text_buffer);

  chars = g_utf8_strlen (text, -1);
  attrs = g_new0 (PangoLogAttr, chars + 1);

  pango_get_log_attrs (text,
                       -1,
                       0,
                       pango_language_from_string ("C"),
                       attrs,
                       chars + 1);

  i = 0;

  while (i < chars)
        {
         if (attrs [i].is_white)
            ++white_chars;

         if (attrs [i].is_word_start)
            ++words;

         ++i;
        }

  if (chars == 0)
     lines = 0;

  bytes = strlen (text);

//  lines = count_of_char (text, "\n");

  gchar *s_bytes = g_strdup_printf(_("bytes: %d\n"), bytes);
  gchar *s_lines = g_strdup_printf(_("lines: %d\n"), lines);
  gchar *s_words = g_strdup_printf(_("words: %d\n"), words);
  gchar *s_chars = g_strdup_printf(_("chars: %d\n"), chars);
  gchar *s_charsnsp = g_strdup_printf(_("chars non-space: %d\n"), chars - white_chars);

  gchar *result;

  if (! selected)
     result = g_strconcat (_("stats for "),
                           doc->file_name_utf8,
                           ":\n",
                           s_charsnsp,
                           s_chars,
                           s_words,
                           s_lines,
                           s_bytes,
                           NULL);
   else
       result = g_strconcat (_("stats for selected:\n"),
                             s_charsnsp,
                             s_chars,
                             s_words,
                             s_bytes,
                             NULL);

  log_to_memo (result, NULL, LM_NORMAL);

  g_free (s_bytes);
  g_free (s_charsnsp);
  g_free (s_chars);
  g_free (s_words);
  g_free (s_lines);
  g_free (result);
  g_free (attrs);
  g_free (text);
}


static gboolean compare_first_element (const gchar *item1, const gchar *item2)
{
  if (! item1 || ! item2)
     return FALSE;

  gchar **a = NULL;
  gchar **b = NULL;

  gboolean result = FALSE;

  a = g_strsplit (item1, ",", -1);
  b = g_strsplit (item2, ",", -1);

  if (! a && ! b)
     return FALSE;

  if (a[0] && b[0])
     {
      if (g_utf8_collate (a[0], b[0]) == 0)
        result = TRUE;
      else
          result = FALSE;
     }

  g_strfreev (a);
  g_strfreev (b);
  return result;
}


GList* find_in_recent (t_note_page *doc)
{
  if (! doc)
     return NULL;

  GList *r = NULL;
  gchar *composed = g_strdup_printf ("%s,%s,%d", doc->file_name_utf8,
                                     doc->encoding,
                                     editor_get_pos (doc));

  GList *t = g_list_first (recent_list);
  while (t)
        {
         if (compare_first_element (composed, t->data))
            {
             r = t;
             break;
            }
         t = g_list_next (t);
        }

  g_free (composed);
  return r;
}


void add_recent_internal (t_note_page *doc)
{
  if (! doc)
     return;

  GList *p = find_in_recent (doc);

  if (p)
     {
      g_free (p->data);
      recent_list = g_list_delete_link (recent_list, p);
     }

  if (g_list_length (recent_list) > (confile.max_recent_items - 1))
    {
     GList *p = g_list_last (recent_list);
     g_free (p->data);
     recent_list = g_list_delete_link (recent_list, p);
    }

  recent_list = add_recent_item_composed (recent_list, doc);
}


void str_walk_extract_word (gpointer key,
                            gpointer value,
                            gpointer user_data)
{
  l_words = g_list_prepend (l_words, key);
}


void run_extract_words (t_note_page *page)
{
  if (! page)
     return;

  GList *list = NULL;
  GtkTextIter a;
  GtkTextIter b;

  l_words = NULL;
  GHashTable *words = g_hash_table_new (g_str_hash, g_str_equal);

  gchar *s;
  gchar *t;
  GtkTextIter start;
  GtkTextIter end;

  if (doc_has_selection (page))
     gtk_text_buffer_get_selection_bounds (page->text_buffer, &start, &end);
  else
      gtk_text_buffer_get_bounds (page->text_buffer, &start, &end);

  do
    {
     if (gtk_text_iter_starts_word (&start))
        a = start;

     if (gtk_text_iter_ends_word (&start))
        {
         s = gtk_text_buffer_get_text (page->text_buffer, &a, &start, FALSE);

         t = g_hash_table_lookup (words, s);
         if (! t)
            g_hash_table_insert (words, s, s);
         else
             g_free (s);
         }
     }
  while (gtk_text_iter_forward_char (&start));

  if (words)
     g_hash_table_foreach (words, str_walk_extract_word, NULL);

  GList *tmplist = g_list_first (g_list_reverse (l_words));
  while (tmplist)
        {
         list = g_list_prepend (list, tmplist->data);
         tmplist = g_list_next (tmplist);
        }

  gchar *sr = string_from_glist (g_list_reverse (list));
  cur_text_doc = doc_ins_to_new (sr);

  g_free (sr);
  glist_strings_free (list);
  g_list_free (l_words);
  g_hash_table_destroy (words);
}


void walk_by_words (gpointer key,
                    gpointer value,
                    gpointer user_data)
{
  l_words = g_list_prepend (l_words, value);
}


void run_unitaz (t_note_page *page, gint sort_type, gboolean case_insensetive)
{
  if (! page)
     return;

  GList *list = NULL;
  GtkTextIter a;
  GtkTextIter b;

  gint sentences = 0;
  gint w_total = 0;
  gint w_unique = 0;
  gint char_count;
//  gint line_count;

  l_words = NULL;

  GHashTable *words = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, free_word_data);

  t_struct_word *ts = NULL;
  t_struct_word *t = NULL;
  gchar *s;
  gchar *z;

  GtkTextIter start;
  GtkTextIter end;

  if (doc_has_selection (page))
     gtk_text_buffer_get_selection_bounds (page->text_buffer, &start, &end);
  else
      gtk_text_buffer_get_bounds (page->text_buffer, &start, &end);

  do
    {
     while (gtk_events_pending ())
       gtk_main_iteration ();

     if (gtk_text_iter_starts_sentence (&start))
         sentences++;

     if (gtk_text_iter_starts_word (&start))
        a = start;

     if (gtk_text_iter_ends_word (&start))
        {
         w_total++;
         if (case_insensetive)
            {
             z = gtk_text_buffer_get_text (page->text_buffer, &a, &start, FALSE);
             s = g_utf8_strdown (z, -1);
             g_free (z);
            }
         else
             s = gtk_text_buffer_get_text (page->text_buffer, &a, &start, FALSE);

         t = g_hash_table_lookup (words, s);
         if (! t)
              {
               ts = (t_struct_word *) g_malloc (sizeof (t_struct_word));
               ts->count = 1;
               ts->word = s;
               g_hash_table_insert (words, s, ts);
               w_unique++;
              }
            else
                {
                 t->count++;
                 g_free (s);
                }
           }
    }
  while ( gtk_text_iter_forward_char (&start));

  gint i;

  char_count = gtk_text_buffer_get_char_count (page->text_buffer);
  //line_count = gtk_text_buffer_get_line_count (page->text_buffer);

  gchar *st = NULL;

  if (words)
     g_hash_table_foreach (words, walk_by_words, NULL);

  GList *tl;

  list = g_list_prepend (list, g_strdup_printf (_("Text analysis results for %s:"), page->file_name_utf8));
  list = g_list_prepend (list, g_strdup_printf (_("Words total: %d"), w_total));
  list = g_list_prepend (list, g_strdup_printf (_("Words unique: %d"), w_unique));
  list = g_list_prepend (list, g_strdup_printf (_("Words total / Words unique = %.6f"), (float)w_total / (float)w_unique));
  list = g_list_prepend (list, g_strdup_printf (_("Sentences = %d"), sentences));
  list = g_list_prepend (list, g_strdup_printf (_("Average sentence length (words) = %.6f"), (float)w_total / (float)sentences));
  list = g_list_prepend (list, g_strdup_printf (_("Average word length = %.6f"), (float)char_count / (float)w_total));

  list = g_list_prepend (list, g_strdup_printf ("%-10s %s", _("Number:"), _("Word:")));

  l_words = glist_word_sort_mode (l_words, sort_type);

  GList *tmplist = g_list_first (l_words);
  while (tmplist)
        {
         t = (t_struct_word *) tmplist->data;
         st = g_strdup_printf ("%-10d %s", t->count, t->word);
         list = g_list_prepend (list, st);
         tmplist = g_list_next (tmplist);
        }

  gchar *sr = string_from_glist (g_list_reverse (list));
  cur_text_doc = doc_ins_to_new (sr);

  g_free (sr);
  glist_strings_free (list);
  g_list_free (l_words);
  g_hash_table_destroy (words);
}


void make_readability (t_note_page *page)
{
  if (! page)
     return;

  if (! page->text_buffer)
     return;

  gint words = 0;
  gint chars = 0;
  gint sents = 0;
  gint spaces = 0;

  GtkTextIter start;
  GtkTextIter end;
  gchar *result;

  guint32 ch;

  if (doc_has_selection (page))
     gtk_text_buffer_get_selection_bounds (page->text_buffer, &start, &end);
  else
      gtk_text_buffer_get_bounds (page->text_buffer, &start, &end);

  do
    {
     if (gtk_text_iter_equal (&start, &end))
        break;

     ++chars;

     ch = gtk_text_iter_get_char (&start);

     if (! g_unichar_isgraph (ch))
        ++spaces;

     if (gtk_text_iter_starts_word (&start))
        ++words;

     if (gtk_text_iter_starts_sentence (&start))
        ++sents;
    }
  while (gtk_text_iter_forward_char (&start));

  chars = chars - spaces;
  if (chars <= 0)
     return NULL;

  double ari = ((4.71 * (chars / words)) + (0.5 * (words / sents))) - 21.43;
  double coleman_liau = (5.89 * chars / words) - (30 * sents / words) - 15.8;

  gchar *s_sents = g_strdup_printf(_("sentences: %d\n"), sents);
  gchar *s_words = g_strdup_printf(_("words: %d\n"), words);
  gchar *s_chars = g_strdup_printf(_("chars: %d\n"), chars);
  gchar *s_ari = g_strdup_printf ("%.3f", ari);
  gchar *s_coleman_liau = g_strdup_printf ("%.3f", coleman_liau);

  result = g_strconcat (_("Readability:\n"),
                        _("Automated Readability Index (ARI): "), s_ari, "\n",
                        _("Coleman-Liau Index: "), s_coleman_liau, "\n",
                        _("Stats:\n"),
                          s_sents,
                          s_words,
                          s_chars,
                          NULL);

  log_to_memo (result, NULL, LM_NORMAL);

  g_free (s_ari);
  g_free (s_sents);
  g_free (s_chars);
  g_free (s_words);
  g_free (result);
  g_free (s_coleman_liau);
}


void document_do_hl (t_note_page* doc)
{
#ifdef LEGACY_MODE

  apply_hl (doc);

#else

  document_apply_hl (doc);

#endif
}


gchar* doc_get_word_at_left (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend)
{
  if (! doc)
     return NULL;

  gunichar u;
  GtkTextIter ittemp;

  GtkTextMark *mark = gtk_text_buffer_get_insert (doc->text_buffer);
  if (! mark)
     return NULL;

  gtk_text_buffer_get_iter_at_mark (doc->text_buffer, &ittemp, mark);

  *itstart = ittemp;
  *itend = ittemp;

  if (! gtk_text_iter_backward_find_char (itstart, find_space, NULL, NULL))
     gtk_text_buffer_get_start_iter (doc->text_buffer, itstart);
  else
      gtk_text_iter_forward_char (itstart);

  *itend = *itstart;
  while (gtk_text_iter_forward_char (itend))
           {
            u = gtk_text_iter_get_char (itend);
            if (! g_unichar_isalpha (u) && ! g_unichar_isdigit (u) && u != '_')
               break;
           }

  return (gtk_text_buffer_get_text (doc->text_buffer, itstart, itend, FALSE));
}
