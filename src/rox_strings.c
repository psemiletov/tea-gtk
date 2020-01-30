/***************************************************************************
                          rox_strings.c  -  description
                             -------------------
    begin                : Fri Oct 17 12:08:36 EEST 2003
    copyleft            :  2003-2006 by Peter Semiletov
    email                : tea@list.ru
***************************************************************************/

/*
Copyright (C) 1999 Antti-Juhani Kaijanaho
Copyright (C) 1998-2003 A.J. van Os
Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
2003, Nathan Walp <faceprint@faceprint.com>
(C) 2002 Olivier Sessink
Copyright (C) 1997-2003 Stuart Parmenter //Balsa::misc.c, quote_color.c
Chris Phelps <chicane@reninet.com>
Alejandro Dubrovsky <s328940@student.uq.edu.au>
Copyright (C) 1998 Adam Rogoyski

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


#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include <errno.h>
#include <regex.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h> // needed for gprintf
#include <gtk/gtk.h>
#include <glib/gi18n.h>


#ifdef ZZIPLIB_SUPPORTED
#include "zzip/lib.h"
#endif

#include "tea_defs.h"

#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "rox_strings.h"
#include "tea_config.h"
#include "tea_funx.h"
#include "tea_enc.h"
#include "interface.h"


//*  Glimmer - misc.c
/*
Chris Phelps <chicane@reninet.com>
Alejandro Dubrovsky <s328940@student.uq.edu.au>
modified by roxton
*/

gchar* g_str_replace (const gchar *input, const gchar *replace, const gchar *with)
{
  if (! input)
     return NULL;

  if (! replace || ! with)
     return g_strdup (input);

  gchar *text = g_strdup (input);
  gchar *start = strstr (text, replace);

  if (! start)
      return text;

  gchar *end = start + strlen (replace);
  *start = '\0';
  gchar *value = g_strconcat (text, with, end, NULL);
  g_free (text);
  return value;
}

//from gaim:: html.c
// *
// * Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
// *               2003, Nathan Walp <faceprint@faceprint.com>

gchar* strip_html (const gchar *text)
{
  if (! text)
     return NULL;

  gint i, j, k;
  gint visible = 1;
  gchar *text2 = g_strdup (text);

  for (i = 0, j = 0; text2[i]; i++)
      {
       if (text2[i] == '<')
          {
           k = i + 1;
           if (g_ascii_isspace (text2[k]))
               visible = 1;
           else
               {
                while (text2[k])
                      {
                       if (text2[k] == '<')
                          {
                           visible = 1;
                           break;
                          }
                       if (text2[k] == '>')
                          {
                           visible = 0;
                           break;
                          }
                       k++;
                      }
                }
           }
       else
           if (text2[i] == '>' && ! visible)
              {
               visible = 1;
               continue;
              }

       if (text2[i] == '&' && strncasecmp (text2 + i, "&quot;", 6) == 0)
          {
           text2[j++] = '\"';
           i = i + 5;
           continue;
          }

       if (visible)
          text2[j++] = text2[i];
      }

  text2[j] = '\0';
  return text2;
}
//end of gaim code


/*
 * tNumber2Roman - convert a number to Roman Numerals
 *
 * returns the number of characters written
 */

//taken from misc.c of Antiword
//Copyright (C) 1998-2003 A.J. van Os; Released under GNU GPL
//mod. by me, roxton
size_t tNumber2Roman (guint uiNumber, gint bUpperCase, gchar *szOutput)
{
  gchar *outp, *p, *q;
  guint uiNextVal, uiValue;

  uiNumber %= 4000; /* Very high numbers can't be represented */
  if (uiNumber == 0)
     {
      szOutput[0] = '\0';
      return 0;
     }

  outp = szOutput;
  p = bUpperCase ? "M\2D\5C\2L\5X\2V\5I" : "m\2d\5c\2l\5x\2v\5i";
  uiValue = 1000;
  for (;;)
      {
       while (uiNumber >= uiValue)
             {
              *outp++ = *p;
              uiNumber -= uiValue;
             }

       if (uiNumber == 0)
          {
           *outp = '\0';
           return (size_t)(outp - szOutput);
          }

       q = p + 1;
       uiNextVal = uiValue / (uint)(gchar)*q;

       if ((int)*q == 2)
           /* magic */
           uiNextVal /= (guint)(gchar)*(q += 2);

       if (uiNumber + uiNextVal >= uiValue)
          {
           *outp++ = *++q;
           uiNumber += uiNextVal;
          }
       else
           {
            p++;
            uiValue /= (guint)(char)(*p++);
           }
      }
} /* end of tNumber2Roman */


gchar* get_file_ver_name (const gchar *name, const gchar *format)
{
  if (! name)
     return NULL;

  if (! g_file_test (name, G_FILE_TEST_EXISTS))
     return NULL;

  gchar *dname = g_path_get_dirname (name);
  gchar *filename = extract_filename (name);
  gchar *ext = extract_ext (name);

  gchar *st = get_time (format);

  st = g_strdelimit (st, ": ", '-');
  st = g_strstrip (st);

  gchar *result = g_strconcat (dname, G_DIR_SEPARATOR_S, filename, "-", st, ext, NULL);

  g_free (st);
  g_free (dname);
  g_free (filename);
  g_free (ext);

  return result;
}


GList* add_to_glist_combined_int (GList *list, const gchar *key, gint value)
{
  if (! key)
     return list;

  return g_list_prepend (list, g_strdup_printf ("%s=%d", key, value));
}


GList* add_to_glist_combined_str (GList *list, const gchar *key, const gchar *value)
{
  if (! key || ! value)
     return list;

  return g_list_prepend (list, g_strdup_printf ("%s=%s", key, value));
}


GList* glist_from_string_sep (const gchar *string, const gchar *sep)
{
  if (! string || ! sep)
     return NULL;

  gchar **a = NULL;
  GList *result = NULL;

  a = g_strsplit (string, sep, -1);
  gint c = -1;
  if (a)
     while (a[++c])
           result = g_list_prepend (result, g_strdup (a[c]));

  g_strfreev (a);
  return g_list_reverse (result);
}


GList* glist_from_string_remove_blanks (const gchar *string)
{
  if (! string)
      return NULL;

  gchar **a = NULL;
  GList *result = NULL;

  a = g_strsplit (string, "\n", -1);
  gint c = -1;
  if (a)
     while (a[++c] && (strlen (a[c]) > 1))
           result = g_list_prepend (result, g_strdup (a[c]));

  g_strfreev (a);
  return g_list_reverse (result);
}


gchar* string_from_glist_sep (GList *list, const gchar *sep)
{
  if (! list)
     return NULL;

  if (! sep)
    return list;

  gchar *result = NULL;
  GList *p = g_list_first (list);
  gchar *t = NULL;
  GList *l = g_list_last (list);

  GString *s = g_string_sized_new (1024);

  while (p)
        {
         if ((p != l) && (p->data))
            t = g_strconcat (p->data, sep, NULL);
         else
             t = g_strdup (p->data);

         if (t)
            s = g_string_append (s, t);
         p = g_list_next (p);
         g_free (t);
        }

  result = s->str;
  g_string_free (s, FALSE);

  return result;
}


void glist_print (GList *list)
{
  GList *p = g_list_first (list);

  while (p)
        {
         g_printf ("%s\n", p->data);
         p = g_list_next (p);
        }
}


GList* load_file_to_glist (const gchar *filename)
{
  if (! filename)
      return NULL;

  if (! g_file_test (filename, G_FILE_TEST_EXISTS))
     return NULL;

  gsize length;
  gchar *buf = NULL;

  if (! g_file_get_contents (filename, &buf, &length, NULL))
     return NULL;

  GList *t = glist_from_string (buf);
  g_free (buf);
  return t;
}


void glist_save_to_file (GList *list, const gchar *filename)
{
  if (! filename)
     return;

  if ((! list) || (g_list_length (list) == 0))
     {
      unlink (filename);
      return;
     }

  FILE *out = fopen (filename, "w+");
  if (! out)
      return;

  GList *p = g_list_first (list);
  GList *last = g_list_last (list);

  while (p)
       {
        if (p->data)
           {
            if (p != last)
               fprintf (out, "%s\n", p->data);
             else
                 fprintf (out, "%s", p->data); //we do not add a LF to the end
           }

        p = g_list_next (p);
       }

  fclose (out);
}


GList* filter_list (GList *list, const gchar *phrase, gboolean incld)
{
  if (! phrase || ! list)
     return list;

  GList *t = NULL;
  GList *p = g_list_first (list);

  while (p)
        {
         if (incld)
            {
             if (strstr (phrase, (gchar *) p->data))
               t = g_list_prepend (t, g_strdup (p->data));
            }
         else
             if (! strstr (phrase, (gchar *) p->data))
               t = g_list_prepend (t, g_strdup (p->data));

         p = g_list_next (p);
        }

  return g_list_reverse (t);
}


GList* str_in_glist (GList *list, const gchar *st)
{
  if (! st || ! list)
      return list;

  GList *result = NULL;
  GList *p = g_list_first (list);

  while (p)
        {
         if (g_utf8_collate (st, p->data) == 0)
            {
             result = p;
             break;
            }
         p = g_list_next (p);
        }

  return result;
}


gchar* linkify_text (const gchar *text)
{
  if (! text)
     return NULL;

  gboolean need_for_protocol = TRUE;
  gboolean is_email = FALSE;
  gchar *protocol = NULL;
  gchar *result = NULL;

  if (strstr (text, "http://"))
      need_for_protocol = FALSE;

  if (strstr (text, "ftp://"))
      need_for_protocol = FALSE;

  if (strstr (text, "mailto:"))
      need_for_protocol = FALSE;

  if (strstr (text, "ftp."))
     protocol = g_strdup ("ftp://");
  else
      if (strstr (text, "@"))
         {
          protocol = g_strdup ("mailto:");
          is_email = TRUE;
         }
      else
          protocol = g_strdup ("http://");

  if (need_for_protocol)
     {
      if (! is_email)
         result = g_strdup_printf ("<a href=\"%s%s\">%s%s</a>", protocol, text, protocol, text);
      else
          result = g_strdup_printf ("<a href=\"%s%s\">%s</a>", protocol, text, text);
     }
  else
      result = g_strdup_printf ("<a href=\"%s\">%s</a>", text, text);

  g_free (protocol);
  return result;
}


GList* remove_blank_lines (GList *lines)
{
  if (! lines)
      return NULL;

  GList *l = NULL;
  GList *t = g_list_first (lines);
  while (t)
       {
        if (t->data)
        if (strlen (t->data) > 0)
           l = g_list_prepend (l, g_strdup (t->data));

        t = g_list_next (t);
       }

  return g_list_reverse (l);
}


void glist_strings_free (GList *list)
{
  if (! list)
     return;

  GList *t = list;
  while (t)
        {
         g_free (t->data);
         t = g_list_next (t);
        }

  g_list_free (t);
}


gchar* enc_guess (const gchar *s)
{
  if (! s)
     return NULL;

  if (g_utf8_validate (s, -1, NULL))
       return g_strdup ("UTF-8");

  if (confile.enc_det_cp1251)
     if (enc_is_cp1251 (s))
         return g_strdup ("CP1251");

  if (confile.enc_det_koi8u)
     if (enc_is_koi8u (s))
        return g_strdup ("KOI8-U");

  if (confile.enc_det_koi8r)
     if (enc_is_koi8r (s))
        return g_strdup ("KOI8-R");

  if (confile.enc_det_866)
     if (enc_is_dos866 (s))
        return g_strdup ("866");

  if (confile.enc_det_finnish)
     if (enc_is_finnish (s))
         return g_strdup ("ISO-8859-15");

  if (confile.enc_det_portuguese)
     if (enc_is_portuguese (s))
         return g_strdup ("ISO-8859-1");

  if (confile.enc_det_slovak)
     if (enc_is_slovak (s))
        return g_strdup ("ISO-8859-2");

  if (confile.enc_det_czech)
     if (enc_is_czech (s))
         return g_strdup ("ISO-8859-2");

  if (confile.enc_det_slovenian)
     if (enc_is_slovenian (s))
         return g_strdup ("ISO-8859-2");

  if (confile.enc_det_turkish)
     if (enc_is_turkish (s))
         return g_strdup ("ISO-8859-9");

  if (confile.enc_det_spanish)
     if (enc_is_spanish (s))
         return g_strdup ("ISO-8859-1");

  if (confile.enc_det_polish)
     if (enc_is_polish (s))
         return g_strdup ("ISO-8859-2");

  if (confile.enc_det_latvian)
     if (enc_is_latvian (s))
        return g_strdup ("ISO-8859-13");

  if (confile.enc_det_german)
     if (enc_is_german (s))
         return g_strdup ("ISO-8859-15");

  if (confile.enc_det_serbian)
     if (enc_is_serbian (s))
         return g_strdup ("ISO-8859-2");

  return NULL;
}


gchar* str_file_read (const gchar *filename)
{
  if (! filename)
     return NULL;

  if (! g_file_test (filename, G_FILE_TEST_EXISTS))
     return NULL;

  gsize length;
  gchar *buf;

  if (! g_file_get_contents (filename, &buf, &length, NULL))
     return NULL;
  else
      return buf;
}


static gint sort_node (const gchar *a, const gchar *b)
{
  if (! a && ! b)
     return 0;
  else if (! a)
      return -1;
  else if (! b)
      return 1;

  gchar *t1 = g_utf8_collate_key (a, -1);
  gchar *t2 = g_utf8_collate_key (b, -1);
  gint result = strcmp (t1, t2);

  g_free (t1);
  g_free (t2);

  return result;
}


gint sort_node_m1 (t_struct_word *a, t_struct_word *b)
{
  gchar *t1 = g_utf8_collate_key (a->word, -1);
  gchar *t2 = g_utf8_collate_key (b->word, -1);
  gint result = strcmp (t1, t2);
  g_free (t1);
  g_free (t2);

  return result;
}


gint sort_node_m2 (t_struct_word *a, t_struct_word *b)
{
  if (a->count == b->count)
     return 0;

  if (a->count < b->count)
     return -1;

  if (a->count > b->count)
     return 1;
}


gint sort_node_m3 (const gchar *sa, const gchar *sb)
{
  if (! sa && ! sb)
     return 0;
  else if (! sa)
      return -1;
  else if (! sb)
      return 1;

  gchar *a = g_utf8_casefold (sa, -1);
  gchar *b = g_utf8_casefold (sb, -1);

  gchar *t1 = g_utf8_collate_key (a, -1);
  gchar *t2 = g_utf8_collate_key (b, -1);

  gint result = strcmp (t1, t2);

  g_free (t1);
  g_free (t2);
  g_free (a);
  g_free (b);

  return result;
}


gint sort_node_m4 (const gchar *a, const gchar *b)
{
  glong x = g_utf8_strlen (a, - 1);
  glong z = g_utf8_strlen (b, - 1);

  if (x == z)
     return 0;

  if (x < z)
     return -1;

  if (x > z)
     return 1;
}


GList* glist_strings_sort (GList *list)
{
  if (! list)
     return NULL;

  return g_list_sort (list, (GCompareFunc) sort_node);
}


GList* sort_list_case_insensetive (GList *list)
{
  if (! list)
     return NULL;

  return g_list_sort (list, (GCompareFunc) sort_node_m3);
}


GList* glist_word_sort_mode (GList *list, gint mode)
{
  if (mode == 0) return g_list_sort (list, (GCompareFunc) sort_node_m1);
  if (mode == 1) return g_list_reverse (g_list_sort (list, (GCompareFunc) sort_node_m2));
  if (mode == 2) return g_list_reverse (g_list_sort (list, (GCompareFunc) sort_node_m4));
  if (mode == -1) return list;
}


void free_word_data (gpointer data)
{
  t_struct_word *ts = data;
  g_free (ts->word);
  g_free (ts);
}


GList* glist_shuffle (GList *list)
{
  if (! list)
     return NULL;

  GList *i1 = NULL;
  gchar *t;
  GRand *r;
  gint32 i;

  GList *p = g_list_first (list);
  gint c = (g_list_length (p) - 1);

  while (p)
        {
         r = g_rand_new ();
         i = g_rand_int_range (r, 0, c);
         i1 = g_list_nth (list, i);
         g_rand_free (r);

         if (i1 != p)
            {
             t = g_strdup (p->data);
             g_free (p->data);
             p->data = g_strdup (i1->data);
             g_free (i1->data);
             i1->data = t;
            }

         p = g_list_next (p);
        }

  return list;
}


gchar* parse_rtf_hex_str (const gchar *s)
{
  if (! s)
     return NULL;

  GString *gs = g_string_sized_new (1048576);

  gchar *st = s;
  gint i;
  gint c;
  gint code;
  gchar *hex;

  for (i = 0; st[i]; i++)
      {
       if ((st[i] == '\\') && (st[i+1]) == '\'')
          //hex char
          {
           hex = g_malloc (3);
           hex[0] = st [i+2];
           hex[1] = st[i+3];
           hex[3] = '\0';
           i = i+3;
           code = strtol (hex, NULL, 16);
           gs = g_string_append_c (gs, (gchar) code);
           g_free (hex);
          }
       else //non-hex
           if ((st[i] == '\\') && (st[i+1] == 'p') &&  (st[i+2] == 'a'))
              {
               i = i + 4;
               gs = g_string_append_c (gs, '\n');
              }
           else
              if ((st[i] == '\\') && (st[i+1] == 't') &&  (st[i+2] == 'a'))
              {
               i = i + 4;
               gs = g_string_append_c (gs, '\t');
              }
           else
               if ((st[i] == '{') && (st[i+1] == '\\'))
                   while (st[++i] != ' ')
                   ;
               else
                   if (st[i] > 31)
                      gs = g_string_append_c (gs, st [i]);
      }

  gchar *result = g_convert (gs->str, strlen (gs->str), "UTF-8", confile.rtf_enc, NULL, NULL, NULL);

  g_string_free (gs, TRUE);
  return result;
}


gchar* text_load_rtf (const gchar *filename)
{
  if (! filename)
    return NULL;

  gchar *t = str_file_read (filename);
  if (! t)
     return NULL;
  gchar *s = parse_rtf_hex_str (t);
  g_free (t);
  return s;
}


GList* arr_to_glist (const gchar **a)
{
  if (! a)
     return NULL;

  GList *l = NULL;
  gint i = 0;
  while (a[i])
        l = g_list_prepend (l, g_strdup (a[i++]));

  return g_list_reverse (l);
}


gchar* kill_formatting (const gchar *s)
{
  if (! s)
     return NULL;

  gchar **a = g_strsplit (s, " ", -1);

  if (! a)
     return g_strdup (s);

  GList *l = arr_to_glist (a);

  GList *t = g_list_first (l);
  while (t)
        {
         g_strstrip (t->data);
         if (g_utf8_strlen (t->data, -1) <= 0)
            t->data = NULL;
         t = g_list_next(t);
        }

  gchar *r = string_from_glist_sep (l, " ");
  glist_strings_free (l);
  g_strfreev (a);
  return r;
}


gchar* wrap_raw (const gchar *s, gint str_len)
{
  if (! s)
     return NULL;

  GString* gs = g_string_sized_new (strlen (s));

  gchar *p = s;
  gchar *t;
  gunichar unichar = g_utf8_get_char (p);
  gint c = -1;

  while ( unichar )
        {
         if (unichar == '\n')
            {
             p = g_utf8_next_char (p);
             unichar = g_utf8_get_char (p);
             continue;
            }

         if (++c == str_len)
            {
             gs = g_string_append_unichar (gs, '\n');
             gs = g_string_append_unichar (gs, unichar);
             c = -1;
            }
         else
             gs = g_string_append_unichar (gs, unichar);

         p = g_utf8_next_char (p);
         unichar = g_utf8_get_char (p);
        }

  t = gs->str;
  g_string_free (gs, FALSE);
  return t;
}


void wrap_on_spaces (gchar *str, gint width)
/*
Copyright (C) 1997-2003 Stuart Parmenter //Balsa::misc.c

libbalsa_wrap_string
   wraps given string replacing spaces with '\n'.  do changes in place.
   lnbeg - line beginning position, sppos - space position,
   te - tab's extra space.
modified by roxton
*/
{
  const gint minl = width / 2;

  if (! str)
     return;

  gchar *space_pos;
  gchar *ptr;
  gint te = 0;

  gint ptr_offset;
  gint line_begin_offset;
  gint space_pos_offset;

  line_begin_offset = ptr_offset = space_pos_offset = 0;
  space_pos = ptr = str;

  while (*ptr)
        {
         switch (*ptr)
                {
                 case '\t':
                           te += 7;
                           break;
                 case '\n':
                           line_begin_offset = ptr_offset + 1;
                           te = 0;
                           break;
                 case ' ':
                          space_pos = ptr;
                          space_pos_offset = ptr_offset;
                          break;
                 }

         if (ptr_offset - line_begin_offset >= width - te
             && space_pos_offset >= line_begin_offset + minl)
            {
             *space_pos = '\n';
             line_begin_offset = space_pos_offset + 1;
             te = 0;
            }
        ptr = g_utf8_next_char (ptr);
        ptr_offset++;
       }
}


gchar* kill_formatting_on_each_line (const gchar *s)
{
  if (! s)
     return NULL;

  GList *l = glist_from_string (s);
  GList *tl = g_list_first (l);
  gchar *t = NULL;

  while (tl)
        {
         t = kill_formatting (tl->data);
         g_free (tl->data);
         tl->data = t;
         tl = g_list_next (tl);
        }

   t = string_from_glist (l);
   glist_strings_free (l);
   return t;
}


gchar* kill_str_by_size (const gchar *s, gint val, gboolean killlesser)
{
  if (! s)
     return NULL;

  GList *l = glist_from_string (s);
  GList *tl = g_list_first (l);
  gchar *t = NULL;
  GList *n = NULL;

  while (tl)
        {
         if (tl->data)
         {
         if (killlesser)
            {
             if (g_utf8_strlen (tl->data, -1) > val )
                n = g_list_prepend (n, g_strdup (tl->data));
            }
         else
             if (g_utf8_strlen (tl->data, -1) <= val )
                n = g_list_prepend (n, g_strdup (tl->data));
         }

         tl = g_list_next (tl);
        }

   t = string_from_glist (g_list_reverse (n));
   glist_strings_free (l);
   glist_strings_free (n);
   return t;
}


gboolean save_string_to_file (const gchar *filename, const gchar *s)
{
  if (! s || ! filename)
     return FALSE;

  FILE *f = fopen (filename, "w");
  if (! f)
     return FALSE;

  fputs (s, f);
  fclose (f);
  return TRUE;
}


gchar* rep_all_s (const gchar *s, const gchar *r)
{
  if (! s || ! r)
     return NULL;

  if (! strstr (s, "%s"))
     return g_strdup (s);

  return (str_replace_all (s, "%s", r));
}


//CHANGEME: change %s macro to @s?
GList* glist_repl (GList* list, const gchar *format)
{
  if (! format || ! list)
     return list;

  GList *p = g_list_first (list);
  gchar *t = NULL;

  while (p)
        {
         if (! strstr (p->data, "%s"))
            {
             t = str_replace_all (format, "%s", p->data);
             g_free (p->data);
             p->data = t;
            }
         p = g_list_next (p);
        }

  return list;
}


gchar* glist_enum (const gchar *buf, const gchar *format_str)
{
  if (! buf || ! format_str)
    return NULL;

  GList *list = glist_from_string (buf);

  gchar **a = g_strsplit (format_str, "~", -1);

  if (! a)
      return NULL;

  gchar *s_fmt = a[0];

  gchar *t = NULL;
  gchar *t2 = NULL;
  gchar *t3 = NULL;
  gint step = 1;
  gint counter = 0;

  GList *temp = NULL;

  if (a[1])
     {
      step = strtol (a[1], NULL, 10);
      if (a[2])
         counter = strtol (a[2], NULL, 10);
     }

  GList *p = g_list_first (list);

  while (p)
        {
         if (p->data)
           {
            t = str_replace_all (s_fmt, "%s", (gchar *) p->data);
            t3 = g_strdup_printf ("%d",(++counter * step));
            t2 = str_replace_all (t, "%d", t3);
            temp = g_list_prepend (temp, t2);
            g_free (t);
            g_free (t3);
           }
         p = g_list_next (p);
        }

  t = string_from_glist (g_list_reverse (temp));

  glist_strings_free (list);
  glist_strings_free (temp);
  g_strfreev (a);

  return t;
}


GHashTable* load_file_to_hashtable (const gchar *filename)
{
  if (! filename)
     return NULL;

  GList *l = load_file_to_glist (filename);

  if (! l)
     return NULL;

  GHashTable *ht = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  GList *tl;
  gchar *key = NULL;
  gchar **pair = NULL;

  tl = g_list_first (l);
  while (tl)
        {
         if (! g_str_has_prefix (tl->data, "#"))
            {
             pair = g_strsplit (tl->data, "=", 2);
             if (pair)
                if (pair[0])
                   {
                    key = pair [0];
                    if (pair [1])
                       g_hash_table_insert (ht, g_strdup (key), g_strdup (pair[1]));
                   }

             g_strfreev (pair);
            }

         tl = g_list_next (tl);
        }

  glist_strings_free (l);
  return ht;
}


static void cb_print_ht (gpointer key,
                         gpointer value,
                         gpointer user_data)
{
  g_print ("%s=%s\n", key, value);
}


void print_ht (GHashTable *ht)
{
  g_hash_table_foreach (ht, cb_print_ht, NULL);
}


gchar* str_crackerize (const gchar *buf)
{
  if (! buf)
     return NULL;

  gint32 i;
  GString* gs = g_string_sized_new (strlen (buf));

  gchar *p = buf;
  gunichar u = g_utf8_get_char (p);

  while ( u )
        {
         GRand *r = g_rand_new ();

         i = g_rand_int_range (r, 0, 3);

         switch (i)
                {
                 case 0: u = g_unichar_toupper (u);
                         break;
                 case 1: u = g_unichar_tolower (u);
                         break;
                 case 2: u = g_unichar_toupper (u);
                         break;
                 case 3: u = g_unichar_tolower (u);
                         break;
                }

         gs = g_string_append_unichar (gs, u);

         g_rand_free (r);

         p = g_utf8_next_char (p);
         u = g_utf8_get_char (p);
        }

  p = gs->str;
  g_string_free (gs, FALSE);
  return p;
}


gint get_eol_mode (const gchar *buf)
{
  if (strstr (buf, "\r"))
     {
      if (strstr (buf, "\n"))
         return eol_CRLF;
      else
          return eol_CR;
     }
  else
      return eol_LF;
}


gchar* str_eol_to_crlf (const gchar *buf)
{
  if (! buf)
     return NULL;

  gint mode = get_eol_mode (buf);

  if (mode == eol_CRLF)
     return NULL;

  GString *gs = g_string_sized_new (strlen (buf));

  gchar *p = buf;
  gunichar unichar = g_utf8_get_char (p);
  gchar *s;

  while ( unichar )
        {
         if (mode == eol_LF)
            {
             if (unichar == '\n')
                 gs = g_string_append (gs, "\r\n");
             else
                 gs = g_string_append_unichar (gs, unichar);
            }
         else // (mode == eol_LF)
             {
              if (unichar == '\r')
                  gs = g_string_append (gs, "\r\n");
              else
                  gs = g_string_append_unichar (gs, unichar);
             }

          p = g_utf8_next_char (p);
          unichar = g_utf8_get_char (p);
         }

  s = gs->str;
  g_string_free (gs, FALSE);

  return s;
}

//remove when use Gtk 2.6 (see g_strv_length)
gint get_gchar_array_size (const gchar **a)
{
  if (! a)
     return 0;
  gint c = 0;
  while (a[++c])
        ;
  return c;
}



gchar* morse_encode (const gchar *s)
{
  if (! s)
     return NULL;

  GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

  g_hash_table_insert (ht, "A", ".-");
  g_hash_table_insert (ht, "B", "-...");
  g_hash_table_insert (ht, "C", "-.-.");
  g_hash_table_insert (ht, "D", "-..");
  g_hash_table_insert (ht, "E", ".");
  g_hash_table_insert (ht, "F", "..-.");
  g_hash_table_insert (ht, "G", "--.");
  g_hash_table_insert (ht, "H", "....");
  g_hash_table_insert (ht, "I", "..");
  g_hash_table_insert (ht, "J", ".---");
  g_hash_table_insert (ht, "K", "-.-");
  g_hash_table_insert (ht, "L", ".-..");
  g_hash_table_insert (ht, "M", "--");
  g_hash_table_insert (ht, "N", "-.");
  g_hash_table_insert (ht, "O", "---");
  g_hash_table_insert (ht, "P", ".--.");
  g_hash_table_insert (ht, "Q", "--.-");
  g_hash_table_insert (ht, "R", ".-.");
  g_hash_table_insert (ht, "S", "...");
  g_hash_table_insert (ht, "T", "-");
  g_hash_table_insert (ht, "U", "..-");
  g_hash_table_insert (ht, "V", "...-");
  g_hash_table_insert (ht, "W", ".--");
  g_hash_table_insert (ht, "X", "-..-");
  g_hash_table_insert (ht, "Y", "-.--");
  g_hash_table_insert (ht, "Z", "--..");
  g_hash_table_insert (ht, "0", "-----");
  g_hash_table_insert (ht, "1", ".----");
  g_hash_table_insert (ht, "2", "..---");
  g_hash_table_insert (ht, "3", "...--");
  g_hash_table_insert (ht, "4", "....-");
  g_hash_table_insert (ht, "5", ".....");
  g_hash_table_insert (ht, "6", "-....");
  g_hash_table_insert (ht, "7", "--...");
  g_hash_table_insert (ht, "8", "---..");
  g_hash_table_insert (ht, "9", "----.");
  g_hash_table_insert (ht, ",", "--..--");
  g_hash_table_insert (ht, ".", ".-.-.-"); //fullstop
  g_hash_table_insert (ht, "?", "..--.."); //query
  g_hash_table_insert (ht, ";", "-.-.-");
  g_hash_table_insert (ht, ":", "---...");
  g_hash_table_insert (ht, "/", "-..-.");
  g_hash_table_insert (ht, "-", "-....-");
  g_hash_table_insert (ht, "\'", ".----.");
  g_hash_table_insert (ht, "(", "-.--.-");
  g_hash_table_insert (ht, "_", "..--.-");
  g_hash_table_insert (ht,  ")", "-.--.-");
  g_hash_table_insert (ht, "/", "-..-.");
  g_hash_table_insert (ht, "+", ".-.-.");
  g_hash_table_insert (ht, "=", "-...-");
  g_hash_table_insert (ht, "\"", ".-..-.");
  g_hash_table_insert (ht, "$", "...-..-");

  gchar *t;
  gint size;
  GString* gs = g_string_sized_new (strlen (s));

  gchar *p = s;
  gunichar u = g_utf8_get_char (p);

  gchar *x;
  gchar *x2;

  while (u)
        {
         u = g_unichar_toupper (u);
         x = g_malloc (6);
         g_unichar_to_utf8 (u, x);
         x2 = g_strndup (x, 1);
         t = g_hash_table_lookup (ht, x2);

         if (t)
             gs = g_string_append (gs, t);

         gs = g_string_append (gs, " ");

         g_free (x);
         g_free (x2);

         p = g_utf8_next_char (p);
         u = g_utf8_get_char (p);
        }

  p = gs->str;
  g_string_free (gs, FALSE);
  g_hash_table_destroy (ht);

  return p;
}


gchar* morse_decode (const gchar *s)
{
  if (! s)
     return NULL;

  gchar** a = g_strsplit (s, " ", -1);
  if (! a)
      return NULL;

  GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

  g_hash_table_insert (ht, ".-", "A");
  g_hash_table_insert (ht, "-...", "B");
  g_hash_table_insert (ht, "-.-.", "C");
  g_hash_table_insert (ht, "-..", "D");
  g_hash_table_insert (ht, ".", "E");
  g_hash_table_insert (ht, "..-.", "F");
  g_hash_table_insert (ht, "--.", "G");
  g_hash_table_insert (ht, "....", "H");
  g_hash_table_insert (ht, "..", "I");
  g_hash_table_insert (ht, ".---", "J");
  g_hash_table_insert (ht, "-.-", "K");
  g_hash_table_insert (ht, ".-..", "L");
  g_hash_table_insert (ht, "--", "M");
  g_hash_table_insert (ht, "-.", "N");
  g_hash_table_insert (ht, "---", "O");
  g_hash_table_insert (ht, ".--.", "P");
  g_hash_table_insert (ht, "--.-", "Q");
  g_hash_table_insert (ht, ".-.", "R");
  g_hash_table_insert (ht, "...", "S");
  g_hash_table_insert (ht, "-", "T");
  g_hash_table_insert (ht, "..-", "U");
  g_hash_table_insert (ht, "...-", "V");
  g_hash_table_insert (ht, ".--", "W");
  g_hash_table_insert (ht, "-..-", "X");
  g_hash_table_insert (ht, "-.--", "Y");
  g_hash_table_insert (ht, "--..", "Z");
  g_hash_table_insert (ht, "-----", "0");
  g_hash_table_insert (ht, ".----", "1");
  g_hash_table_insert (ht, "..---", "2");
  g_hash_table_insert (ht, "...--", "3");
  g_hash_table_insert (ht, "....-", "4");
  g_hash_table_insert (ht, ".....", "5");
  g_hash_table_insert (ht, "-....", "6");
  g_hash_table_insert (ht, "--...", "7");
  g_hash_table_insert (ht, "---..", "8");
  g_hash_table_insert (ht, "----.", "9");
  g_hash_table_insert (ht, "--..--", ",");
  g_hash_table_insert (ht, ".-.-.-", "."); //fullstop
  g_hash_table_insert (ht, "..--..", "?"); //query
  g_hash_table_insert (ht, "-.-.-", ";");
  g_hash_table_insert (ht, "---...", ":");
  g_hash_table_insert (ht, "-..-.", "/");
  g_hash_table_insert (ht, "-....-", "-");
  g_hash_table_insert (ht, ".----.", "'");
  g_hash_table_insert (ht, "..--.-", "_");
  g_hash_table_insert (ht, "-..-.", "/");
  g_hash_table_insert (ht, ".-.-.", "+");
  g_hash_table_insert (ht, "-...-", "=");
  g_hash_table_insert (ht, ".-..-.", "\"");
  g_hash_table_insert (ht, "...-..-", "$");

  GString* gs = g_string_sized_new (strlen (s));

  gint c = -1;
  gchar *t;
  gchar *p;

  while (a[++c])
        {
         t = g_hash_table_lookup (ht, a[c]);

         if (t)
             gs = g_string_append (gs, t);
         else
             gs = g_string_append (gs, " ");
        }

  p = gs->str;
  g_string_free (gs, FALSE);
  g_hash_table_destroy (ht);
  g_strfreev (a);
  //.-. .- -.. .. --- .- -.-. - .. ...- .. - -.--
  return p;
}


static GList *temp_glist;

static void ht_to_glist_cb (gpointer key,
                            gpointer value,
                            gpointer user_data)
{
  temp_glist = g_list_prepend (temp_glist, g_strconcat (key, "=", value, NULL));
}


static void ht_to_glist_keys_cb (gpointer key,
                                 gpointer value,
                                 gpointer user_data)
{
  temp_glist = g_list_prepend (temp_glist, g_strconcat (key, NULL));
}


GList* ht_to_glist (GHashTable *hash_table, gint option)
{
  if (! hash_table)
     return NULL;

  glist_strings_free (temp_glist);

  if ((option == opt_ht_to_glist_full) && hash_table)
     g_hash_table_foreach (hash_table, ht_to_glist_cb, NULL);
  else
      if ((option == opt_ht_to_glist_keys) && hash_table)
         g_hash_table_foreach (hash_table, ht_to_glist_keys_cb, NULL);

  return glist_copy_with_data (temp_glist, -1);
}


void save_ht_to_file (GHashTable *hash_table, const gchar *filename)
{
  temp_glist = NULL;

  GList *l = ht_to_glist (hash_table, opt_ht_to_glist_full);
  if (! l)
     return;

  glist_save_to_file (l, filename);
  glist_strings_free (l);
}


gchar* ch_str (gchar *s, const gchar *new_val)
{
   if (! s)
      return g_strdup (new_val);

   if (new_val)
      {
       g_free (s);
       return g_strdup (new_val);
      }
   else
       return s;
}


GList* each_line_remove_lt_spaces (GList *lines, gint mode)
{
  if (! lines)
      return NULL;

  GList *l = NULL;
  GList *t = g_list_first (lines);
  while (t)
       {
        if (t->data)
          {
           if (strlen ( t->data) <= 1)
              l = g_list_prepend (l, g_strdup ( t->data));
           else
              {
               if (mode == 1)
                  l = g_list_prepend (l, g_strchomp (g_strdup (t->data)));
               else
                   if (mode == 0)
                      l = g_list_prepend (l, g_strchug (g_strdup (t->data)));
              }
          }
        t = g_list_next (t);
       }

  return g_list_reverse (l);
}


gchar* get_c_source_fname (const gchar *f)
{
  if (! f)
     return NULL;

  gchar *t = NULL;

  if (g_str_has_suffix (f, ".hh"))
     {
      t = g_str_replace (f, ".hh", ".cc");
      if (g_file_test (t, G_FILE_TEST_EXISTS))
          return t;
     }
  else
  if (g_str_has_suffix (f, ".h"))
     {
      t = g_str_replace (f, ".h", ".cpp");
      if (g_file_test (t, G_FILE_TEST_EXISTS))
          return t;

      g_free (t);
      t = g_str_replace (f, ".h", ".c");
      if (g_file_test (t, G_FILE_TEST_EXISTS))
          return t;
     }

  g_free (t);
  return NULL;
}


gchar* get_c_header_fname (const gchar *f)
{
  if (! f)
     return NULL;

  gchar *t = NULL;

  if (g_str_has_suffix (f, ".cpp"))
     t = g_str_replace (f, ".cpp", ".h");
  else
      if (g_str_has_suffix (f, ".c"))
         t = g_str_replace (f, ".c", ".h");
  else
      if (g_str_has_suffix (f, ".cc"))
         t = g_str_replace (f, ".cc", ".hh");

  if (g_file_test (t, G_FILE_TEST_EXISTS))
     return t;

  g_free (t);
  return NULL;
}


gchar* ht_get_gchar_value (GHashTable *hash_table, const gchar *key, const gchar *def)
{
  if (! hash_table)
      return g_strdup (def);

  gchar *v = g_hash_table_lookup (hash_table, key);
  if (v)
     return g_strdup (v);

  return strdup (def);
}


gchar* str_kill_dups (const gchar *s)
{
  if (! s)
     return NULL;

  GList *l = glist_from_string (s);
  GList *t = g_list_first (l);
  GList *x = NULL;

  while (t)
        {
         if (! str_in_glist (x, t->data))
            x = g_list_prepend (x, g_strdup (t->data));

         t = g_list_next (t);
        }

  gchar *result = string_from_glist (g_list_reverse (x));

  glist_strings_free (x);
  glist_strings_free (l);
  return result;
}


GList* str_remove_from_list (GList *list, const gchar *s)
{
  if (! s || ! list)
     return list;

  GList *t = NULL;
  GList *p = g_list_first (list);

  while (p)
        {
         if (g_utf8_collate (s, p->data) != 0)
             t = g_list_prepend (t, g_strdup (p->data));
         p = g_list_next (p);
        }

  return g_list_reverse (t);
}


gchar* text_load_gzip (const gchar *filename)
{
  if (! filename)
     return NULL;

#ifdef ZZIPLIB_SUPPORTED

  if (is_ext (filename, ".zip", NULL))
      return read_single_file_from_zip (filename);

#endif

  gchar *cm = NULL;

  if (strstr (filename, ".tar."))
     {
      if (is_ext (filename, ".zip", ".gz", NULL))
         cm = g_strconcat ("tar -xOz -f ", filename, NULL);
      else
         if (is_ext (filename, ".bz2", NULL))
            cm = g_strconcat ("tar -xOj -f ", filename, NULL);
     }
  else
      {
       if (is_ext (filename, ".zip", ".gz", NULL))
          cm = g_strconcat ("gunzip -cd ", filename, NULL);
       else
          if (is_ext (filename, ".bz2", NULL))
             cm = g_strconcat ("bzip2 -dck ", filename, NULL);
      }

  gchar *standard_output = NULL;
  gchar *standard_error = NULL;
  gint exit_status;
  gchar *x = NULL;

  if (cm)
     if (g_spawn_command_line_sync (cm, &standard_output,
                                   &standard_error,
                                   &exit_status,
                                   NULL))
       {
        g_free (standard_error);
        x = locale_to_utf8 (standard_output);
        g_free (standard_output);
        g_free (cm);
       }

  return x;
}


gchar* locale_to_utf8 (const gchar *s)
{
  if (! s)
     return NULL;
  gsize bytes_written;
  return g_locale_to_utf8 (s, strlen (s), NULL, &bytes_written, NULL);
}


gchar* quotes_to_tex (const gchar *buf)
{
  if (! buf)
     return NULL;

  GString* gs = g_string_sized_new (strlen (buf));

  gchar *p = buf;
  gunichar unichar = g_utf8_get_char (p);
  gchar *s;
  gboolean x = FALSE;

  while ( unichar )
        {
         if (unichar == '\"')
            {
             if (x)
                gs = g_string_append (gs, "``");
             else
                 gs = g_string_append (gs, "\'\'");
              x = ! x;
             }
         else
             gs = g_string_append_unichar (gs, unichar);

         p = g_utf8_next_char (p);
         unichar = g_utf8_get_char (p);
        }

  s = gs->str;
  g_string_free (gs, FALSE);

  return s;
}


gchar* tex_paras (const gchar *buf)
{
  if (! buf)
     return NULL;

  GString* gs = g_string_sized_new (strlen (buf));

  gchar *p = buf;
  gunichar unichar = g_utf8_get_char (p);
  gchar *s;

  while ( unichar )
        {
         if (unichar == '\n')
            gs = g_string_append (gs, "\n\n");
         else
             gs = g_string_append_unichar (gs, unichar);

         p = g_utf8_next_char (p);
         unichar = g_utf8_get_char (p);
        }

  s = gs->str;
  g_string_free (gs, FALSE);

  return s;
}


gchar* change_file_ext (const gchar *filename, const gchar *new_ext)
{
  if (! filename || ! new_ext)
     return NULL;

  if (! strstr (filename, "."))
     return g_strconcat (filename, ".", new_ext, NULL);

  GList *l = glist_from_string_sep (filename, ".");
  GList *t = g_list_last (l);

  if (t && t->data)
     t->data = ch_str (t->data, new_ext);

  gchar *f = string_from_glist_sep (l, ".");
  glist_strings_free (l);

  return f;
}


gboolean str_in (const gchar *s, gchar *ext1, ...)
{
  if (! s)
     return FALSE;

  va_list args;
  gchar *t;
  gchar *x;
  gboolean r = FALSE;

  va_start (args, ext1);
  t = ext1;

  while (t)
        {
         x = g_utf8_strdown (t, -1);
         if (g_utf8_collate (s, x) == 0)
            {
             r = TRUE;
             g_free (x);
             break;
            }
         g_free (x);
         t = va_arg (args, gchar*);
        }
  va_end (args);

  return r;
}


static GList *gl_buf_import;
static gboolean tag_text_sw;


void par_start_element (GMarkupParseContext *context,
                        const gchar *element_name,
                        const gchar **attribute_names,
                        const gchar **attribute_values,
                        gpointer user_data,
                        GError **error)
{
  if (g_utf8_collate (element_name, user_data) == 0)
     tag_text_sw = TRUE;
}


static void par_end_element (GMarkupParseContext *context,
                             const gchar *element_name,
                             gpointer user_data,
                             GError **error)

{
  if (g_utf8_collate (element_name, user_data) == 0)
     tag_text_sw = FALSE;
}


static void par_text (GMarkupParseContext *context,
                      const gchar *text,
                      gsize text_len,
                      gpointer user_data,
                      GError **error)

{
  if (! tag_text_sw)
     return;

  gl_buf_import = g_list_prepend (gl_buf_import, g_strdup (text));
}


gchar* xml_to_plain_text (const gchar *xml_text, const gchar *para)
{
  if (! xml_text || ! para)
      return NULL;

  gl_buf_import = NULL;
  tag_text_sw = FALSE;
  GError *error = NULL;

  GMarkupParser parser;

  parser.text = par_text;
  parser.start_element = par_start_element;
  parser.end_element = par_end_element;
  parser.passthrough = NULL;
  parser.error = NULL;

  GMarkupParseContext *context = g_markup_parse_context_new (&parser, 0, para, NULL);
  g_markup_parse_context_parse (context, xml_text, strlen (xml_text), &error);
  g_markup_parse_context_end_parse (context, NULL);

  gchar *s = string_from_glist (g_list_reverse (gl_buf_import));
  glist_strings_free (gl_buf_import);
  g_markup_parse_context_free (context);

  return s;
}


gchar* read_xml_text (const gchar *filename, const gchar *xml_file, const gchar *para)
{
  if (! filename || ! xml_file || ! para)
     return NULL;

#ifdef ZZIPLIB_SUPPORTED

   gchar *xml = read_file_from_zip (filename, xml_file);
   if (! xml)
      return NULL;

  gchar *r = xml_to_plain_text (xml, para);
  g_free (xml);
  return r;

#else

  if (g_file_test (confile.tmp_dir, G_FILE_TEST_EXISTS))
     delete_dir (confile.tmp_dir);

  if (mkdir (confile.tmp_dir, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
     return NULL;

  gchar *cm = g_strconcat ("unzip -o \"", filename, "\" -d ", confile.tmp_dir, NULL);

  system (cm);

  gchar *f = g_strconcat (confile.tmp_dir, G_DIR_SEPARATOR_S, xml_file, NULL);
  gchar *t = str_file_read (f);
  gchar *r = xml_to_plain_text (t, para);

  delete_dir (confile.tmp_dir);
  g_free (cm);
  g_free (f);
  g_free (t);
  return r;

#endif
}


gchar* read_abw_text (const gchar *filename, const gchar *para)
{
  if (! filename || ! para)
     return NULL;

  gchar *t = str_file_read (filename);
  gchar *r = xml_to_plain_text (t, para);

  g_free (t);
  return r;
}


gchar* separated_table_to_latex_tabular (const gchar *st, const gchar *sep)
{
  if (! st || ! sep)
     return NULL;

  GList *l = glist_from_string (st);
  if (! l)
     return NULL;

  gchar **a;
  GList *table = NULL;
  gchar *x;
  GList *z;
  GList *y;
  GList *hl = NULL;
  guint c = 0;

  GList *last = NULL;
  gchar *n;
  gchar *m;

  table = g_list_prepend (table, g_strdup ("\\begin{tabular}"));

  GList *t = g_list_first (l);

  y = glist_from_string_sep (t->data, sep);

  guint cols = g_list_length (y);

  hl = g_list_append (hl, g_strconcat ("{|", NULL));

  for (c = 0; c < cols; c++)
       hl = g_list_append (hl, g_strconcat ("l|", NULL));

  hl = g_list_append (hl, g_strconcat ("}", NULL));
  table = g_list_prepend (table, string_from_glist_sep (hl, " "));

  glist_strings_free (y);
  glist_strings_free (hl);

  while (t && t->data)
        {
         if (strstr (t->data, sep))
            {
             y = glist_from_string_sep (t->data, sep);

             z = g_list_first (y);
             last = g_list_last (y);
             while (z && z->data)
                  {
                   if (strlen (z->data) > 1)
                     {
                      n = g_shell_unquote (z->data, NULL);
                      if (n)
                        {
                         if (z == y)
                            x = g_strdup (n);
                         else
                             {
                              m = g_strconcat (x, " & ", n, NULL);
                              g_free (x);
                              x = m;
                             }
                         g_free (n);
                        }
                     }
                   z = g_list_next (z);
                  }

              m = g_strconcat (x, "\\\\", NULL);
              g_free (x);
              x = m;

              table = g_list_prepend (table, g_strdup (x));
              g_free (x);
              x = NULL;
              glist_strings_free (y);
             }

         t = g_list_next (t);
        }

  table = g_list_prepend (table, g_strdup ("\\end{tabular}"));
  GList *rev = g_list_reverse (table);

  gchar *r = string_from_glist (rev);
  glist_strings_free (l);
  glist_strings_free (table);
  return r;
}


gint find_str_index (GList *l, const gchar *s)
{
  if (! l || ! s)
      return -1;

  gint r = -1;
  GList *t = g_list_first (l);
  while (t && t->data)
        {
         ++r;
         if (g_utf8_collate (t->data, s) == 0)
            break;
         t = g_list_next (t);
        }

  return r;
}


gchar* str_to_html (const gchar *buf, gboolean xhtml)
{
  if (! buf)
     return NULL;

  GList *l = glist_from_string (buf);
  gchar *dtd;

  if (! xhtml)
     dtd = g_strdup ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">\n");
  else
      dtd = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");

  l = glist_repl (l, "<p class=\"p1\">%s</p>");

  l = g_list_prepend (l,
                      g_strconcat (
                          dtd ,
                          "<html>\n",
                          "<head>\n",
                          "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n",
                          "<style type=\"text/css\">\n",
                          ".p1\n",
                          "{\n",
                          "margin: 0px 0px 0px 0px;\n"
                          "padding: 0px 0px 0px 0px;\n"
                          "text-indent: 1.5em;\n"
                          "text-align: justify;\n"
                          "}\n"
                          "</style>\n",
                          "<title></title>\n",
                          "</head>\n",
                          "<body>\n",
                           NULL)
                          );

  l = g_list_append (l, g_strdup ("</body>\n</html>"));

  gchar *r = string_from_glist (l);
  glist_strings_free (l);
  gchar *x;
  if (xhtml)
     x = str_replace_all (r, "<p class=\"p1\"></p>", "<br />");
  else
      x = str_replace_all (r, "<p class=\"p1\"></p>", "<br>");

  g_free (dtd);
  g_free (r);
  return x;
}


gchar* copied_from_browser_to_ftext (const gchar *s, const gchar *pattern)
{
  if (! s || ! pattern)
     return NULL;

  GList *temp = glist_from_string (s);
  if (! temp)
     return NULL;

  GList *temp2 = remove_blank_lines (temp);
  temp2 = glist_repl (temp2, pattern);

  gchar *t = string_from_glist (temp2);

  glist_strings_free (temp);
  glist_strings_free (temp2);
  return t;
}


gchar* get_charset_from_meta (const gchar *text)
{
  if (! text)
     return;

  gchar *s1 = strstr (text, "<meta");
  if (! s1)
    return NULL;

  s1 = strstr (text, "charset=");
  if (! s1)
    return NULL;

  s1 = strstr (s1, "=");
  if (! s1)
     return NULL;

  s1++;

  GString *s = g_string_sized_new (64);
  gint i = -1;

  while ( s1[++i] != '\"' )
        s = g_string_append_c (s, s1[i]);

  s1 = g_string_free (s, FALSE);

  return g_strstrip (s1);
}


void gslist_strings_free (GSList *list)
{
  if (! list)
     return;

  GSList *t = list;
  while (t)
        {
         if (t->data)
            g_free (t->data);
         t = g_slist_next (t);
        }

  g_slist_free (t);
}


GList* parse_string_to_list_sep (const gchar *s, const gchar *sep)
{
  if (! s || ! sep)
     return NULL;

  gchar **a = NULL;
  GList *result = NULL;

  a = g_strsplit (s, sep, -1);
  gint c = -1;

  if (a)
     while (a[++c])
           result = g_list_prepend (result, g_strdup (a[c]));

  return g_list_reverse (result);
}


gchar* string_split_by (const gchar *source, const gchar *delimeter)
{
  if (! source || ! delimeter)
     return NULL;

  if (! strstr (source, delimeter))
     return NULL;

  GList *l = parse_string_to_list_sep (source, delimeter);
  gchar *r = string_from_glist (l);

  gslist_strings_free (l);
  return r;
}


gchar* recent_item_compose (const gchar *file_name_utf8, const gchar *charset, gint pos)
{
  return g_strdup_printf ("%s,%s,%d", file_name_utf8, charset, pos);
}


gchar* tea_convert_charset (const gchar *text, const gchar *from, const gchar *to)
{
  if (! text || ! from || ! to)
     return NULL;

  gsize bytes_read;
  gsize bytes_written;
  GError *error = NULL;

  if (g_utf8_collate (to, from) == 0)
     return g_strdup (text);

  return (g_convert (text, strlen (text), to, from, &bytes_read, &bytes_written, &error));
}


gboolean check_is_number (const gchar *value)
{
  if (! value)
     return FALSE;

  gchar *p = value;
  gunichar u = g_utf8_get_char (p);

  while ( u )
        {
         if (! g_unichar_isdigit (u))
              return FALSE;

         p = g_utf8_next_char (p);
         u = g_utf8_get_char (p);
        }

   return TRUE;
}


gchar* text_load_msdoc (const gchar *filename)
{
  if (! filename)
     return NULL;

  gchar *cm = g_strconcat ("antiword -m UTF-8.txt \"", filename, "\" > ", confile.tmp_file, NULL);
  system (cm);
  g_free (cm);
  return get_tmp_file_string ();
}


t_str_quad* str_quad_new (const gchar *a, const gchar *b, const gchar *c, const gchar *d)
{
   t_str_quad *t = g_malloc (sizeof (t_str_quad));
   t->a = NULL;
   t->b = NULL;
   t->c = NULL;
   t->d = NULL;

  if (a)
     t->a = g_strdup (a);
  if (b)
     t->b = g_strdup (b);
  if (c)
     t->c = g_strdup (c);
  if (d)
     t->d = g_strdup (d);

   return t;
}


void str_quad_free (t_str_quad *quad)
{
   if (! quad)
     return;

   if (quad->a)
     g_free (quad->a);

   if (quad->b)
     g_free (quad->b);

   if (quad->c)
     g_free (quad->c);

   if (quad->d)
     g_free (quad->d);

   g_free (quad);
}


gchar* str_extract_between (const gchar *source, const gchar *a, const gchar *b)
{
  if (! source || ! a || !b)
     return NULL;

  gchar *a_pos = strstr (source, a);
  if (! a_pos)
     return NULL;

  a_pos = a_pos + strlen (a);

  gchar **arr = g_strsplit (a_pos, b, -1);
  if (get_gchar_array_size (arr) == 0)
     return NULL;

  gchar *result = g_strdup (arr[0]);

  if (arr)
     g_strfreev (arr);

  return result;
}


t_str_quad* parse_ctags_line (const gchar *line)
{
  if (! line)
     return NULL;

  if (strlen (line) < 1)
     return NULL;

  gchar **a = g_strsplit (line, "\t",  -1);

  gint c = get_gchar_array_size (a);

  if (c < 4)
     return NULL;

  t_str_quad *t = str_quad_new (NULL, NULL, NULL, NULL);

  t->a = g_strdup (a[0]); //symbol
  t->b = g_strdup (a[1]); //filename
  t->c = str_extract_between (a[2], "/^", ")"); //declaration
  t->d = g_strdup (a[3]); //type

  g_strfreev (a);

  return t;
}


void str_quad_print (t_str_quad *t)
{
  if (! t)
     return;
  dbm ("-----------");
  dbm (t->a);
  dbm (t->b);
  dbm (t->c);
  dbm (t->d);
  dbm ("-----------");
}


gchar* extract_each_line_bewteen (const gchar *text, const gchar *a, const gchar *b)
{
  if (! text)
     return NULL;

  GList *l = glist_from_string (text);
  GList *t = g_list_first (l);
  gchar *x = NULL;

  while (t)
        {
         x = str_extract_between (t->data, a, b);
         if (x)
            {
             g_free (t->data);
             t->data = x;
            }

         t = g_list_next (t);
        }

  gchar *result = string_from_glist (l);
  glist_strings_free (l);
  return result;
}


gchar* str_before_sep (const gchar *str, const gchar *sep)
{
  if (str == NULL || sep == NULL)
     return NULL;

  gchar **a = g_strsplit (str, sep,  -1);
  gint c = get_gchar_array_size (a);
  if (c < 1)
     return NULL;

  gchar *result = g_strdup (a[0]);
  g_strfreev (a);
  return result;
}


gchar* str_after_sep (const gchar *str, const gchar *sep)
{
   if ((str == NULL) || (sep == NULL))
      return;

   gchar *t = strstr (str, sep);
   if (t)
      t = t + g_utf8_strlen (sep, -1);
   if (! t)
      return NULL;

   return g_strdup (t);
}


gchar* str_each_line_after_sep (const gchar *text, const gchar *sep)
{
  if (! text || ! sep)
     return NULL;

  GList *l = glist_from_string (text);
  GList *t = g_list_first (l);
  gchar *x = NULL;

  while (t)
        {
         x = str_after_sep (t->data, sep);
         if (x)
            {
             g_free (t->data);
             t->data = x;
            }

         t = g_list_next (t);
        }

  gchar *result = string_from_glist (l);
  glist_strings_free (l);
  return result;
}


gchar* str_each_line_before_sep (const gchar *text, const gchar *sep)
{
  if (! text || ! sep)
     return NULL;

  GList *l = glist_from_string (text);
  GList *t = g_list_first (l);
  gchar *x = NULL;

  while (t)
        {
         x = str_before_sep (t->data, sep);
         if (x)
            {
             g_free (t->data);
             t->data = x;
            }

         t = g_list_next (t);
        }

  gchar *result = string_from_glist (l);
  glist_strings_free (l);
  return result;
}


gchar* str_face_quotes (const gchar *buf, const gchar *q1, const gchar *q2)
{
  if (! buf || ! q1 || ! q2)
     return NULL;

  GString* gs = g_string_sized_new (strlen (buf));

  gchar *p = buf;
  gunichar unichar = g_utf8_get_char (p);
  gchar *s;
  gchar *q = q1;
  gboolean start = TRUE;

  while ( unichar )
        {
         if (unichar == '"')
           {
            if (start)
               q = q1;
            else
                q = q2;

            gs = g_string_append (gs, q);
            start = !start;

            }
          else
             gs = g_string_append_unichar (gs, unichar);

          p = g_utf8_next_char (p);
          unichar = g_utf8_get_char (p);
        }


  s = gs->str;
  g_string_free (gs, FALSE);

  return s;
}


gchar* strinfile (const gchar *filename, const gchar *text_to_find)
{
  gchar *buf = str_file_read (filename);
  if (! buf)
     return NULL;

  gchar *converted_text;
  gchar *pos = NULL;

  GList *l = g_list_first (confile.iconv_encs);
  while (l)
        {
         converted_text = tea_convert_charset (text_to_find, "UTF-8", l->data);

        if (converted_text)
           {
            pos = strstr (buf, converted_text);
            if (pos)
              {
               pos = recent_item_compose (filename /* UTF-8!*/, l->data, 0);
               g_free (converted_text);
               break;
              }
           }

        g_free (converted_text);
        l = g_list_next (l);
        }

  g_free (buf);
  return pos;
}


gchar* strinfile_office (const gchar *filename, const gchar *text_to_find)
{
  gchar *buf = NULL;
  if (check_ext (filename, ".sxw") || check_ext (filename, ".odt"))
     buf = read_xml_text (filename, "content.xml", "text:p");
  else
      if (check_ext (filename, ".abw"))
         buf = read_abw_text (filename, "p");
      else
          if (check_ext (filename, ".kwd"))
             buf = read_xml_text (filename, "maindoc.xml", "TEXT");

  if (! buf)
     return NULL;

  gchar *pos = NULL;

  pos = strstr (buf, text_to_find);
  if (pos)
      pos = recent_item_compose (filename, "UTF-8", 0);

  g_free (buf);

  return pos;
}


GList* glist_copy_with_data (GList *source, gint count)
{
  if (! source)
      return NULL;

  if (count == -1)
     count = g_list_length (source);
  else
       count = count - 1;

  GList *result = NULL;
  GList *t = g_list_first (source);
  gint c = 0;

  while (t && (c++ < count))
        {
         if (t->data)
            result = g_list_prepend (result, g_strdup (t->data));
         t = g_list_next (t);
        }

  return g_list_reverse (result);
}


GList* glist_copy_with_filter (GList *source, FilterStrFunc func)
{
  if (! source)
      return NULL;

  GList *result = NULL;
  GList *t = g_list_first (source);
  gint c = 0;

  while (t)
        {
         if (t->data && func (t->data))
            result = g_list_prepend (result, g_strdup (t->data));
         t = g_list_next (t);
        }

  return g_list_reverse (result);
}


gchar *apply_repl_table (const gchar *text, const gchar *table_filename)
{
  if (! text || ! table_filename)
     return NULL;

  gchar *st = g_strdup (text);
  GList *l = load_file_to_glist (table_filename);
  GList *t = g_list_first (l);
  while (t)
        {
         gchar *key = NULL;
         gchar *val = NULL;
         gchar **a = NULL;
         a = g_strsplit (t->data, "=", 2);

         if (a)
            {
             if (get_gchar_array_size (a) > 1)
                {
                 key = a[0];
                 val = a[1];
                 gchar *x = str_replace_all (st, key, val);
                 if (x)
                   {
                    g_free (st);
                    st = x;
                   }
                }

             g_strfreev (a);
            }

         t = g_list_next (t);
        }

  gslist_strings_free (l);
  return st;
}


gchar* str_replace_all (const gchar *input, const gchar *replace, const gchar *with)
{
  if (! input)
     return NULL;

  if (! replace)
     return g_strdup (input);

  if (! with)
     return g_strdup (input);

  gint len = strlen (replace);

  gchar *text = g_strdup (input);
  gchar *start = strstr (text, replace);

  if (! start)
     return g_strdup (input);

  gchar *end = start + len;
  *start = '\0';

  gchar *value = g_strconcat (text, with, end, NULL);
  g_free (text);

  while (strstr (end, replace))
        {
         text = value;
         start = strstr (text, replace);
         if (! start)
            break;

        end = start + len;
        *start = '\0';
        gchar *x = g_strconcat (text, with, end, NULL);
        g_free (value);
        value = x;
        }

  return (value);
}


gchar* strinfile_charset (const gchar *filename, const gchar *text_to_find, const gchar *charset)
{
  gchar *buf = str_file_read (filename);
  if (! buf)
     return NULL;

  gchar *converted_text;
  gchar *pos = NULL;
  gchar *result = NULL;

  converted_text = tea_convert_charset (text_to_find, "UTF-8", charset);

  if (converted_text)
      {
       pos = strstr (buf, converted_text);
       if (pos)
           result = recent_item_compose (filename /* UTF-8!*/, charset, 0);
        g_free (converted_text);
       }

  g_free (buf);
  return result;
}


GList* filter_list_pattern (GList *list, const gchar *pattern, gboolean incld)
{
  if (! pattern || ! list)
     return list;

  GPatternSpec *pat = g_pattern_spec_new (pattern);

  GList *t = NULL;
  GList *p = g_list_first (list);

  while (p)
        {
         if (incld)
            {
             if (g_pattern_match_string (pat, p->data))
               t = g_list_prepend (t, g_strdup (p->data));
            }
         else
             if (! g_pattern_match_string (pat, p->data))
               t = g_list_prepend (t, g_strdup (p->data));

         p = g_list_next (p);
        }

  g_pattern_spec_free (pat);

  return g_list_reverse (t);
}


gchar* read_file_from_zip (const gchar *archive_name, const gchar *file_name)
{
  gchar *buf = NULL;

#ifdef ZZIPLIB_SUPPORTED

  ZZIP_DIR* dir = zzip_dir_open (archive_name, 0);
  if (dir)
     {
      ZZIP_FILE* fp = zzip_file_open (dir,file_name, 0);
      if (fp)
         {
          ZZIP_STAT zs;
          zzip_fstat (fp, &zs);
          buf = g_malloc (zs.st_size);
          zzip_file_read (fp, buf, zs.st_size);
          free (zs.d_name);
          zzip_file_close(fp);
         }
     zzip_dir_close(dir);
    }

#endif

  return buf;
}


gchar* read_single_file_from_zip (const gchar *archive_name)
{
  gchar *buf = NULL;

#ifdef ZZIPLIB_SUPPORTED

  ZZIP_DIR* dir = zzip_dir_open (archive_name, 0);
  if (dir)
     {
      ZZIP_DIRENT dirent;
      if (zzip_dir_read (dir, &dirent))
         {
          ZZIP_FILE* fp = zzip_file_open (dir, dirent.d_name, 0);
          if (fp)
             {
              //dbm (dirent.d_name);
              ZZIP_STAT zs;
              zzip_fstat (fp, &zs);
              buf = g_malloc (zs.st_size);
              zzip_ssize_t len = zzip_file_read (fp, buf, zs.st_size);
              free (zs.d_name);
              zzip_file_close(fp);
             }
          }
     zzip_dir_close(dir);
    }


#endif

  return buf;
}


gchar* str_eol_to_lf (const gchar *buf)
{
  if (! buf)
     return NULL;

  gint mode = get_eol_mode (buf);

  if (mode == eol_LF)
     return NULL;

  if (mode == eol_CRLF)
     return (str_replace_all (buf, "\r\n", "\n"));

  if (mode == eol_CR)
     return (str_replace_all (buf, "\r", "\n"));

   return NULL;
}


gchar* str_eol_to_cr (const gchar *buf)
{
  if (! buf)
     return NULL;

  gint mode = get_eol_mode (buf);

  if (mode == eol_CR)
     return NULL;

  if (mode == eol_CRLF)
     return (str_replace_all (buf, "\r\n", "\r"));

  if (mode == eol_LF)
     return (str_replace_all (buf, "\n", "\r"));

   return NULL;
}

/*
from:

/* roman.c by Adam Rogoyski (apoc@laker.net) Temperanc on EFNet irc
 * Copyright (C) 1998 Adam Rogoyski
 * Converts Decimal numbers to Roman Numerals and Roman Numberals to
 * Decimals on the command line or in Interactive mode.
 * Uses an expanded Roman Numeral set to handle numbers up to 999999999


*/
#define I 1
#define V 5
#define X 10
#define L 50
#define C 100
#define D 500
#define M 1000
#define P 5000
#define Q 10000
#define R 50000
#define S 100000
#define T 500000
#define U 1000000
#define B 5000000
#define W 10000000
#define N 50000000
#define Y 100000000
#define Z 500000000

static long
value (char c)
{
   switch (c)
   {
      case 'I':
         return I;
      case 'V':
         return V;
      case 'X':
         return X;
      case 'L':
         return L;
      case 'C':
         return C;
      case 'D':
         return D;
      case 'M':
         return M;
      case 'P':
         return P;
      case 'Q':
         return Q;
      case 'R':
         return R;
      case 'S':
         return S;
      case 'T':
         return T;
      case 'U':
         return U;
      case 'B':
         return B;
      case 'W':
         return W;
      case 'N':
         return N;
      case 'Y':
         return Y;
      case 'Z':
         return Z;
      default:
         return 0;
   }
}

long romanToDecimal (const gchar *roman)
{
   long decimal = 0;
   for (; *roman; roman++)
   {
      /* Check for four of a letter in a fow */
      if ((*(roman + 1) && *(roman + 2) && *(roman + 3))
         && (*roman == *(roman + 1))
         && (*roman == *(roman + 2))
         && (*roman == *(roman + 3)))
         return 0;
      /* Check for two five type numbers */
      if (  ((*roman == 'V') && (*(roman + 1) == 'V'))
         || ((*roman == 'L') && (*(roman + 1) == 'L'))
         || ((*roman == 'D') && (*(roman + 1) == 'D'))
         || ((*roman == 'P') && (*(roman + 1) == 'P'))
         || ((*roman == 'R') && (*(roman + 1) == 'R'))
         || ((*roman == 'T') && (*(roman + 1) == 'T'))
         || ((*roman == 'B') && (*(roman + 1) == 'B'))
         || ((*roman == 'N') && (*(roman + 1) == 'N'))
         || ((*roman == 'Z') && (*(roman + 1) == 'Z')))
         return 0;
      /* Check for two lower characters before a larger one */
      if ((value(*roman) == value(*(roman + 1))) && (*(roman + 2))
         && (value(*(roman + 1)) < value(*(roman + 2))))
         return 0;
      /* Check for the same character on either side of a larger one */
      if ((*(roman + 1) && *(roman + 2))
         && (value(*roman) == value(*(roman + 2)))
         && (value(*roman) < value(*(roman + 1))))
         return 0;
      /* Check for illegal nine type numbers */
      if (!strncmp(roman, "LXL", 3) || !strncmp(roman, "DCD", 3)
       || !strncmp(roman, "PMP", 3) || !strncmp(roman, "RQR", 3)
       || !strncmp(roman, "TST", 3) || !strncmp(roman, "BUB", 3)
       || !strncmp(roman, "NWN", 3) || !strncmp(roman, "VIV", 3))
         return 0;
      if (value(*roman) < value(*(roman + 1)))
      {
         /* check that subtracted value is at least 10% larger,
            i.e. 1990 is not MXM, but MCMXC */
         if ((10 * value(*roman)) < value(*(roman + 1)))
            return 0;
         /* check for double subtraction, i.e. IVX */
         if (value(*(roman + 1)) <= value(*(roman + 2)))
            return 0;
         /* check for subtracting by a number starting with a 5
            ie.  VX, LD LM */
         if (*roman == 'V' || *roman == 'L' || *roman == 'D'
          || *roman == 'P' || *roman == 'R' || *roman == 'T'
          || *roman == 'B' || *roman == 'N')
            return 0;
         decimal += value (*(roman + 1)) - value (*roman);
         roman++;
      }
      else
      {
         decimal += value (*roman);
      }
   }
   return decimal;
}


gchar* extract_filename (const gchar *filename)
{
  if (! has_ext (filename))
     return g_path_get_basename (filename);

  gchar *fname = g_path_get_basename (filename);
  gchar *x = g_strrstr (fname, ".");
  *x = '\0';
  gchar *result = g_strndup (fname, strlen (fname));
  g_free (fname);
  return result;
}


gchar* extract_ext (const gchar *filename)
{
  if (! has_ext (filename))
     return NULL;

  return g_strdup (strstr (filename, "."));
}

////////////
/*
gint count_of_char (const gchar *s, gchar c)
{
  if (! s)
     return -1;

  gint i = 0;
  gint count = 0;

  while (s[i++] != '\0')
        {
         if (s[i] == c)
            ++count;
        }

  g_print ("%d", i);
  return count;
}
*/