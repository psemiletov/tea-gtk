/***************************************************************************
                          callbacks.c  -  description
                             -------------------
    begin                : Mon Dec 1 2003
    copyleft             : 2003-2013 by Peter Semiletov
    email                : tea@list.ru
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/


#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef PRINTING_SUPPORTED
#include <gtk-unix-print-2.0/gtk/gtkprintunixdialog.h>
#endif

#include <stddef.h> // Needed for NULL
#include <stdlib.h> // Needed for system, strtol
#include <limits.h> // Needed for strtol
#include <sys/types.h> // Needed for mkdir
#include <sys/stat.h> // Needed for mkdir
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>

#include <libintl.h>
#include <locale.h>


#ifdef HAVE_LIBASPELL
#include "aspell.h"
#endif

#include "tea_spell.h" // Needed for get_suggestions_list
#include "tea_text_document.h"  // Needs to be above for t_note_page struct
#include "callbacks.h"
#include "tea_funx.h"
#include "interface.h"
#include "tea_config.h"
#include "tea_options.h"
#include "tea_defs.h"
#include "tea_fr.h"
#include "tea_about.h"
#include "tea_proj.h"
#include "tea_gtk_utils.h"
#include "tea_calendar.h"
#include "image_browser.h"
#include "latex_support.h"
#include "rox_strings.h"
#include "tea_encode.h"
#include "expr.h"
#include "tea_findfiles_mult.h"
#include "tea_kwas.h" // Needed for create_kwas_window


#ifdef ZZIPLIB_SUPPORTED
#include "zzip/lib.h"
#endif


#ifdef LIBCURL_SUPPORTED
#include <curl/curl.h>
#endif


#define STRP_STR_REVERSE 1
#define STRP_STR_LINKIFY 2
#define STRP_STR_BIN2DEC 3
#define STRP_STR_STRIPHTML 4
#define STRP_STR_MARKUPESCAPE 5
#define STRP_STR_FACEQUOTES 6
#define STRP_STR_HEX2DEC 7
#define STRP_STR_KILLDUPS 8
#define STRP_STR_LISTENUM 9
#define STRP_STR_KILLFMT 10
#define STRP_STR_KILLFMT_EACHLINE 11
#define STRP_STR_WIKIINTLINK 12



void process_string (gint mode, gpointer menuitem)
{
  mni_last = menuitem;

  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  switch (mode)
        {
         case STRP_STR_REVERSE:
                              {
                               t = g_utf8_strreverse (buf, -1);
                               break;
                               }

        case STRP_STR_LINKIFY:
                              {
                               t = linkify_text (buf);
                               break;
                              }

        case STRP_STR_BIN2DEC:
                              {
                               t = g_strdup_printf ("%d", strtol (buf, NULL, 2));
                               break;
                              }

        case STRP_STR_STRIPHTML:
                               {
                                t = strip_html (buf);
                                break;
                               }

        case STRP_STR_MARKUPESCAPE:
                                  {
                                   t = g_markup_escape_text (buf, -1);
                                   break;
                                  }

        case STRP_STR_FACEQUOTES:
                                 {
                                  t = str_face_quotes (buf, "\xC2\xAB", "\xC2\xBB");
                                  break;
                                 }
        case STRP_STR_HEX2DEC:
                              {
                               gchar *xx = g_utf8_strdown (buf, -1);
                               t = g_strdup_printf ("%d", strtol (xx, NULL, 16));
                               g_free (xx);
                               break;
                              }

        case STRP_STR_KILLDUPS:
                              {
                               t = str_kill_dups (buf);
                               break;
                              }

        case STRP_STR_LISTENUM:
                              {
                               set_fam_text ("%d.)%s");
                               t = glist_enum (buf, gtk_entry_get_text (ent_search));
                               break;
                              }

        case STRP_STR_KILLFMT:
                              {
                               t = kill_formatting (buf);
                               break;
                              }

        case STRP_STR_KILLFMT_EACHLINE:
                              {
                               t = kill_formatting_on_each_line (buf);
                               break;
                              }

        case STRP_STR_WIKIINTLINK:
                                 {
                                  t = g_strconcat ("[[", buf, "]]", NULL);
                                  break;
                                 }

        }

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
}


static GtkFileFilter* tea_file_filter_from_lang (GtkSourceLanguage *language)
{
  if (! language)
     return NULL;

  gchar *name = gtk_source_language_get_name (language);
  if (! name)
     return NULL;

  GtkFileFilter *f = gtk_file_filter_new ();
  gtk_file_filter_set_name (f, name);

  gchar **l = gtk_source_language_get_mime_types (language);

  gint len = get_gchar_array_size (l) - 1;
  gint i;

  for (i = 0; i <= len; i++)
      gtk_file_filter_add_mime_type (f, l[i]);

  g_strfreev (l);
  return f;
}


void gtk_file_chooser_add_filters_from_langs (GtkFileChooser *chooser)
{
  gchar **langs;
  g_object_get (source_languages_manager, "language-ids", &langs, NULL);

  gint c = get_gchar_array_size (langs) - 1;
  gint i;

  for (i = 0; i <= c; i++)
       gtk_file_chooser_add_filter (chooser, tea_file_filter_from_lang (
                                    gtk_source_language_manager_get_language (gtk_source_language_manager_get_default (),
                                    langs[i])));
}


static void file_open_file2click (GtkFileChooser *chooser,
                                  gpointer user_data)
{
  gtk_dialog_response (GTK_DIALOG (user_data), GTK_RESPONSE_ACCEPT);
}


static gboolean on_fnentr_key_press_event (GtkWidget *widget,
                                           GdkEventKey *event,
                                           gpointer user_data)
{
  if (event->keyval == GDK_KEY_Return)
     {
      gtk_dialog_response (GTK_DIALOG (user_data), GTK_RESPONSE_ACCEPT);
      return TRUE;
     }

  return FALSE;
}


//n.p. Depeche Mode - Enjoy The Silence
void set_fam_text (const gchar *def)
{
  if (strlen (gtk_entry_get_text (ent_search)) == 0)
     {
      gtk_entry_set_text (ent_search, def);
      log_to_memo (_("Read the manual!"), NULL, LM_ERROR);
      return;
     }

 GtkEntryCompletion *c = gtk_entry_get_completion (ent_search);
 model_add_new_item (gtk_entry_completion_get_model (c), gtk_entry_get_text (ent_search));
}


void on_mni_file_open_at_cursor (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  if (! get_page_text())
     return;

  gchar *w = get_c_url (cur_text_doc);
  handle_file (w, cur_text_doc->encoding, 0, FALSE);
  g_free (w);
  mni_last = menuitem;
}


static gchar *selected_enc;

static void tea_handle_param_pair (const gchar *key, const gchar *val)
{
  if (g_utf8_collate (key, "charset") == 0)
     selected_enc = ch_str (selected_enc, val);

  if (g_utf8_collate (key, "n") == 0)
     {
      gchar *dir = g_get_current_dir ();
      gchar *fname = g_strconcat (dir, G_DIR_SEPARATOR_S, val, NULL);
      create_empty_file (fname, NULL);
      doc_open_file (fname, selected_enc);
      g_free (dir);
      g_free (fname);
     }
}


void tea_handle_param (const gchar *p)
{
  if (! p)
     return;

  gchar *t = g_utf8_offset_to_pointer (p, 2);

  if (! t)
     return;

  if (strstr (t, "="))
     {
      gchar **a = g_strsplit (t, "=", -1);

      if (a && a[0] && a[1])
         tea_handle_param_pair (a[0], a[1]);

      g_strfreev (a);
     }
  else
      if (g_utf8_collate (t, "crapbook") == 0)
         {
          on_mni_file_crapbook (NULL, NULL);
          return;
         }
}


gboolean tea_init (gpointer data)
{
  gint i;
  gchar *fn;
  t_note_page *p = NULL;

  selected_enc = g_strdup (confile.default_charset);

  if (g_argc > 1)
     for (i = 1; i < g_argc; ++i)
         {
          if (! g_argv[i])
             break;

          if (g_str_has_prefix (g_argv[i], "--"))
             tea_handle_param (g_argv[i]);
          else
              {
               fn = g_malloc (2048);
               if (realpath (g_argv[i], fn))
                   p = open_file_std (fn, selected_enc);
               g_free (fn);
              }
          }

  if (g_argc <= 1)
    {
     if (confile.start_with_blank_file)
       on_mni_new_file_activate (NULL, NULL);

     if (confile.do_last_session)
       session_open_from_file (confile.last_session);
    }

  if ((g_utf8_collate (confile.last_version, VERSION) != 0))
     {
      #ifdef FOR_DEBIAN
      gchar *news = g_strconcat (TEA_DOC_DIR, "NEWS.gz", NULL);
      gchar *Changelog = g_strconcat (TEA_DOC_DIR, "changelog.gz", NULL);
      #else
      gchar *news = get_tea_doc_compose_name ("NEWS");
      gchar *Changelog = get_tea_doc_compose_name ("ChangeLog");
      #endif

      cur_text_doc = open_file_std (Changelog, "UTF-8");
      if (cur_text_doc)
          cur_text_doc->readonly = TRUE;

      cur_text_doc = open_file_std (news, "UTF-8");
      if (cur_text_doc)
          cur_text_doc->readonly = TRUE;

     g_free (Changelog);
     g_free (news);
    }

  g_free (selected_enc);
  return TRUE;
}

//current music: Guano Apes - Quietly
void file_new (void)
{
  cur_text_doc = doc_clear_new ();
  gtk_window_set_title (GTK_WINDOW (tea_main_window), cur_text_doc->file_name_utf8);
  gtk_widget_grab_focus (cur_text_doc->text_view);
  tabs_reload();
}


void on_mni_new_file_activate (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  file_new();
  mni_last = menuitem;
}


static GtkWidget *ent_dlg_filename;


void change_teaspec_location (GtkComboBox *combobox,
                              gpointer user_data)
{
  gint i = gtk_combo_box_get_active (combobox);
  if (i == -1)
     return;

  gchar *dir = NULL;

  switch (i)
         {
          case 4:
                 dir = confile.templates_dir;
                 break;

          case 3:
                 dir = confile.tables_dir;
                 break;

          case 2:
                 dir = confile.snippets_dir;
                 break;

          case 1:
                 dir = confile.sessions_dir;
                 break;

          case 0:
                 dir = confile.scripts_dir;
                 break;
         }

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (user_data), dir);
}


//n.p. The Rolling Stones - Mother's Little Helper
void on_mni_file_open_activate (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  tea_file_open_dialog();
  mni_last = menuitem;
}


void file_save (void)
{
  if (! get_page_text())
     return;

  if (! cur_text_doc->b_saved)
     on_mni_file_save_as_activate (NULL, NULL);
  else
      if (text_doc_save (cur_text_doc, cur_text_doc->file_name_local))
         log_to_memo (_("%s saved OK"), cur_text_doc->file_name_utf8, LM_NORMAL);
}


void on_mni_file_save_activate (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  file_save();
  mni_last = menuitem;
}


void file_save_as (void)
{
  if (get_page_text())
     tea_file_save_dialog (0);
}


void on_mni_file_save_as_activate (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  file_save_as();
  mni_last = menuitem;
}


void on_mni_out_activate (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  ui_done();
  gtk_main_quit();
}


void on_mni_case_upcase_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t;

  if (doc_has_selection (cur_text_doc))
     {
      gchar *buf = doc_get_sel (cur_text_doc);
      t = g_utf8_strup (buf, -1);
      doc_rep_sel (cur_text_doc, t);
      g_free (buf);
      g_free (t);
     }
  else
      {
       GtkTextIter itstart;
       GtkTextIter itend;
       gchar *buf = doc_get_current_word (cur_text_doc, &itstart, &itend);

       if (! buf)
          return;

       t = g_utf8_strup (buf, -1);
       doc_set_new_text (cur_text_doc, &itstart, &itend, t);

       g_free (buf);
       g_free (t);
      }
}


void on_mni_case_locase_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t;

  if (doc_has_selection (cur_text_doc))
     {
      gchar *buf = doc_get_sel (cur_text_doc);
      t = g_utf8_strdown (buf, -1);
      doc_rep_sel (cur_text_doc, t);
      g_free (buf);
      g_free (t);
     }
  else
      {
       GtkTextIter itstart;
       GtkTextIter itend;
       gchar *buf = doc_get_current_word (cur_text_doc, &itstart, &itend);

       if (! buf)
          return;

       t = g_utf8_strdown (buf, -1);
       doc_set_new_text (cur_text_doc, &itstart, &itend, t);

       g_free (buf);
       g_free (t);
      }
}


void on_mni_reverse (GtkMenuItem *menuitem,
                     gpointer user_data)
{
  process_string (STRP_STR_REVERSE, menuitem);
}


void on_mni_file_save_version (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *s = get_file_ver_name (cur_text_doc->file_name_local, confile.date_time_ver);
  if (! s)
     {
      log_to_memo(_("Save the file first..."), NULL, LM_ADVICE);
      return;
     }

  text_doc_save_silent (cur_text_doc, s);
  g_free (s);
}


void on_mni_Markup_bold_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         doc_insert_at_cursor (cur_text_doc, "<b></b>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<emphasis role=\"bold\"></emphasis>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          t = g_strconcat ("<b>", buf, "</b>", NULL);
       else
           if (def_mm == MM_WIKI)
              t = g_strconcat ("\'\'\'", buf, "\'\'\'", NULL);
           else
               if (def_mm == MM_TEX)
                  t = g_strconcat ("\\textbf{", buf, "} ", NULL);
               else
                   if (def_mm == MM_DOCBOOK)
                      t = g_strconcat ("<emphasis role=\"bold\">",
                                       buf, "</emphasis>",
                                       NULL);

       doc_rep_sel (cur_text_doc, t);
      }

  g_free (buf);
  g_free (t);
}


void on_mni_Markup_link_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         doc_insert_at_cursor (cur_text_doc, "<a href=\"\"></a>");
      else
          if (def_mm == MM_DOCBOOK)
            doc_insert_at_cursor (cur_text_doc, "<ulink url=\"\"></ulink>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          t = g_strconcat ("<a href=\"\">", buf, "</a>", NULL);
       else
           if (def_mm == MM_WIKI)
              t = g_strconcat ("[", buf, "]", NULL);
           else
               if (def_mm == MM_DOCBOOK)
                  t = g_strconcat ("<ulink url=\"\">", buf, "</ulink>", NULL);

       doc_rep_sel (cur_text_doc, t);

       if (def_mm == MM_HTML || def_mm == MM_XHTML || def_mm == MM_DOCBOOK)
          doc_move_to_pos_bw_quote (cur_text_doc);
      }

  g_free (buf);
  g_free (t);
}


void on_mni_markup_wiki_internal_link (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
     return;

  process_string (STRP_STR_WIKIINTLINK, menuitem);
}


void on_mni_Markup_italic_activate (GtkMenuItem *menuitem,
                                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         {
          doc_insert_at_cursor (cur_text_doc, "<i></i>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<emphasis></emphasis>");
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          t = g_strconcat ("<i>", buf, "</i>", NULL);
       else
           if (def_mm == MM_WIKI)
              t = g_strconcat ("\'\'", buf, "\'\'", NULL);
           else
               if (def_mm == MM_TEX)
                  t = g_strconcat ("\\textit{", buf, "}", NULL);
               else
                   if (def_mm == MM_DOCBOOK)
                      t = g_strconcat ("<emphasis>", buf, "</emphasis>", NULL);

       doc_rep_sel (cur_text_doc, t);
       g_free (t);
      }

  g_free (buf);
}


void on_mni_Markup_underline_activate (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  gchar *t;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         {
          doc_insert_at_cursor (cur_text_doc, "<span style=\"text-decoration: underline;\"></span>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
      else
          if (def_mm == MM_WIKI)
             doc_insert_at_cursor (cur_text_doc, "<u></u>");
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          t = g_strconcat ("<span style=\"text-decoration: underline;\">", buf, "</span>", NULL);
       else
           if (def_mm == MM_TEX)
              t = g_strconcat ("\\underline{", buf, "} ", NULL);
           else
               if (def_mm == MM_WIKI)
                  t = g_strconcat ("<u>", buf, "</u>", NULL);

       doc_rep_sel (cur_text_doc, t);

       g_free (buf);
       g_free (t);
      }
}


void on_mni_Markup_para_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
          doc_insert_at_cursor (cur_text_doc, "<p></p>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<para></para>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          t = g_strconcat ("<p>", buf, "</p>", NULL);
       else
           if (def_mm == MM_DOCBOOK)
              t = g_strconcat ("<para>", buf, "</para>", NULL);

       doc_rep_sel (cur_text_doc, t);
       g_free (t);
      }

  g_free (buf);
}


void on_mni_markup_common (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      t = g_strconcat ("<", gtk_widget_get_name (menuitem), ">",
                       "</", gtk_widget_get_name (menuitem), ">",
                       NULL);
      doc_insert_at_cursor (cur_text_doc, t);
      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       t = g_strconcat ("<", gtk_widget_get_name (menuitem), ">",
                        buf,
                        "</", gtk_widget_get_name (menuitem), ">",
                        NULL);
       doc_rep_sel (cur_text_doc, t);
      }

  g_free (buf);
  g_free (t);
}


static gint last_page = -1;

void on_notebook1_switch_page (GtkNotebook *notebook,
                               gpointer *page,
                               guint page_num,
                               gpointer user_data)
{
  t_note_page *dc = get_page_by_index (page_num);

  if (dc)
     {
      set_title (dc);
      gtk_widget_grab_focus (dc->text_view);
      last_page = gtk_notebook_get_current_page (notebook);
      markup_change_by_ext (dc->file_name_utf8);
     }
}


void on_mni_file_close_current (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  page_del_by_index (gtk_notebook_get_current_page ((GtkNotebook *) notebook1));
}


void on_mni_view_wrap (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (gtk_text_view_get_wrap_mode (cur_text_doc->text_view) != GTK_WRAP_WORD)
      gtk_text_view_set_wrap_mode (cur_text_doc->text_view, GTK_WRAP_WORD);
  else
      gtk_text_view_set_wrap_mode (cur_text_doc->text_view, GTK_WRAP_NONE);
}


void on_mni_Markup_br (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  if (def_mm == MM_HTML)
     doc_insert_at_cursor (cur_text_doc, "<br>");
  else
      if (def_mm == MM_XHTML)
         doc_insert_at_cursor (cur_text_doc, "<br />");
      else
          if (def_mm == MM_TEX)
              doc_insert_at_cursor (cur_text_doc, "\\newline");
}


void on_mni_Functions_number_arabian2roman (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gchar *t = g_malloc (1024);

  tNumber2Roman (strtol (buf, NULL, 10), TRUE, t);

  if (t)
     doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_free (buf);
}


void on_mni_Functions_number_roman2arabian (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  long l = romanToDecimal (buf);

  gchar *t = g_strdup_printf ("%d", l);

  if (t)
     doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_free (buf);
}


void on_mni_view_show_line_numbers (GtkMenuItem *menuitem,
                                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! cur_text_doc->linenums)
      document_set_line_numbers (cur_text_doc, TRUE);
  else
      document_set_line_numbers (cur_text_doc, FALSE);
}


void on_mni_html_default_template (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  gchar *t = g_strconcat (
                          "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n",
                          "<html>\n",
                          "<head>\n",
                          "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n",
                          "<title></title>\n",
                          "</head>\n",
                          "<body>\n",
                          "</body>\n",
                          "</html>",
                          NULL);

  doc_insert_at_cursor (cur_text_doc, t);
  g_free (t);
}


void on_mni_file_save_session (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  gchar *filename;
  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Save the session as:"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), confile.sessions_dir);
  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
     filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
     session_save_to_file (filename);
     g_free (filename);
     reload_sessions ();
    }

  gtk_widget_destroy (dialog);
}


void on_mni_antispam (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gint c = g_utf8_strlen (buf, STRMAXLEN) - 1;
  GString *result = g_string_sized_new (16 * c);

  gint i;

  for (i = 0; i <= c; i++)
       g_string_append_printf (result, "&#%d;", buf[i]);

  doc_rep_sel (cur_text_doc, result->str);

  g_string_free (result, TRUE);
  g_free (buf);
}


void on_mni_Functions_number_hex2dec (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  process_string (STRP_STR_HEX2DEC, menuitem);
}


void on_mni_config_open (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.tea_rc, "UTF-8");
}


void on_mni_utils_stats (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  make_stats (cur_text_doc);
}


void on_mni_Functions_number_bin2dec (GtkMenuItem *menuitem, gpointer user_data)
{
  process_string (STRP_STR_BIN2DEC, menuitem);
}


void on_mni_Markup_nbsp (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (def_mm == MM_HTML || def_mm == MM_XHTML || def_mm == MM_WIKI || def_mm == MM_DOCBOOK)
     doc_insert_at_cursor (cur_text_doc, "&nbsp;");
  else
     if (def_mm == MM_TEX)
       doc_insert_at_cursor (cur_text_doc, "\\space");
}


void on_mni_undo (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_undo (cur_text_doc);
}


void on_mni_redo (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_redo (cur_text_doc);
}


static void update_preview_cb (GtkFileChooser *file_chooser, gpointer data)
{
  gchar *filename = gtk_file_chooser_get_preview_filename (file_chooser);
  if (! filename)
     return;

  GtkWidget *preview = GTK_WIDGET (data);
  
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (filename, confile.thumb_width, confile.thumb_height, NULL);
  gboolean have_preview = (pixbuf != NULL);
  g_free (filename);

  gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);

  if (pixbuf)
     gdk_pixbuf_unref (pixbuf);

  gtk_file_chooser_set_preview_widget_active (file_chooser, have_preview);
}


void on_mni_image_activate (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkImage *img = gtk_image_new ();

  GtkWidget *file_dialog = gtk_file_chooser_dialog_new (_("File open"),
                                                        tea_main_window,
                                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                        NULL);

  GtkFileFilter *f = tea_file_filter_with_patterns (_("Image files"),
                                                    "*.JPG", "*.jpg",
                                                    "*.png", "*.PNG",
                                                    "*.jpeg", "*.JPEG",
                                                    "*.GIF", "*.gif",
                                                    "*.bmp", "*.BMP",
                                                    "*.tiff", "*.TIFF",
                                                    "*.svg", "*.SVG",
                                                    "*.tga", "*.TGA",
                                                    "*.wbmp", "*.WBMP",
                                                    "*.eps", "*.EPS",
                                                    "*.ps", "*.PS",
                                                     NULL);

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_dialog), f);

  f = tea_file_filter_with_patterns (_("All files"), "*", NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_dialog), f);

  gtk_file_chooser_set_preview_widget (file_dialog, img);
  g_signal_connect (file_dialog, "update-preview", G_CALLBACK (update_preview_cb), img);

  if (g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     {
      gchar *dir = g_path_get_dirname (cur_text_doc->file_name_local);
      gtk_file_chooser_set_current_folder (file_dialog, dir);
      g_free (dir);
     }

  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (file_dialog), TRUE);
  gtk_window_resize (GTK_WINDOW (file_dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_dialog), TRUE);

  if (gtk_dialog_run (GTK_DIALOG (file_dialog)) == GTK_RESPONSE_ACCEPT)
     {
      gchar *filename;

      GSList *l = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (file_dialog));
      if (l)
         {
          GSList *t = l;
          while (t)
               {
                filename = t->data;
                if (! g_file_test (filename, G_FILE_TEST_IS_DIR))
                   if (is_image (filename))
                     insert_image (filename, 0, 0, TRUE);

                g_free (filename);
                t = g_slist_next (t);
               }
           g_slist_free (l);
         }
     }
  gtk_widget_destroy (file_dialog);
}


void on_mni_html_enclose_link (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  process_string (STRP_STR_LINKIFY, menuitem);
}


gboolean on_ent_search_key_press_event (GtkWidget *widget,
                                        GdkEventKey *event,
                                        gpointer user_data)
{
  if (event->keyval != GDK_KEY_Return)
     return FALSE;

  if (get_page_text())
     {
      set_fam_text ("text to find");
      doc_search_f (cur_text_doc, gtk_entry_get_text (ent_search));
     }

  return FALSE;
}


void on_mni_func_strings_removeblanks (GtkMenuItem *menuitem,
                                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = remove_blank_lines (temp);
  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_func_strings_enclose (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  set_fam_text ("<li>%s</li>");

  if (! strstr (gtk_entry_get_text (ent_search), "%s"))
     {
      g_free (buf);
      return;
     }

  GList *temp = glist_from_string (buf);

  temp = glist_repl (temp, gtk_entry_get_text (ent_search));
  gchar *t = string_from_glist (temp);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_func_strings_rev (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  temp = g_list_reverse (temp);
  gchar *t = string_from_glist (temp);
  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_search_repall (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("text to find~text to replace");
  gchar *s = gtk_entry_get_text (ent_search);

  if (! strstr (s, "~"))
     return;

  gchar **a = g_strsplit (s, "~", -1);
  if (! a)
     return;

  gchar *x;
  gchar *y;

  x = a[0];
  if (! x)
     {
      g_strfreev (a);
      return;
     }

  if (! a[1])
     {
      g_strfreev (a);
      return;
     }

  y = a[1];

  GtkTextIter iter;

  gtk_text_buffer_get_iter_at_offset (cur_text_doc->text_buffer, &iter, 0);
  gtk_text_buffer_move_mark_by_name (cur_text_doc->text_buffer, "insert", &iter);
  gtk_text_buffer_move_mark_by_name (cur_text_doc->text_buffer, "selection_bound", &iter);

  doc_replace_from_cursor_all (cur_text_doc, x, y);

  g_strfreev (a);
}


void on_mni_recent_activate (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  execute_recent_item (gtk_widget_get_name (menuitem));
}


void on_mni_about_activate (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  create_wnd_about();
}


void on_mni_file_save_as_template_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  tea_file_save_dialog (1);
}


void on_mni_file_edit_at_cursor (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *w = get_c_url (cur_text_doc);
  handle_file (w, cur_text_doc->encoding, 1, FALSE);
  g_free (w);
}


void on_mni_new_link_select (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  handle_file (gtk_widget_get_name (menuitem), user_data, 0, FALSE);
}


void add_link_item (const gchar *fname, const gchar *linkname, const gchar *charset)
{
  if (! fname || ! linkname)
     return;

  gchar *dir = g_path_get_dirname (fname);
  gchar *filename = create_full_path (linkname, dir);

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
      mni_temp = new_menu_item_with_udata (filename, mni_links_menu, on_mni_new_link_select, charset);

  g_free (dir);
  g_free (filename);
}

//n.p. Defecation - Intention Superpassed - 2/3 Pure
void scan_links (void)
{
  if (! get_page_text())
     return;

  gchar *f;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gtk_widget_destroy (mni_links_menu);
  mni_links_menu = new_menu_submenu (mni_links);
  mni_temp = new_menu_item_with_udata (cur_text_doc->file_name_utf8, mni_links_menu, on_mni_new_link_select, cur_text_doc->encoding);
  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "ref=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            add_link_item (cur_text_doc->file_name_utf8, f, cur_text_doc->encoding);
            g_free (f);
           }
        match_start = match_end;
       }

  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "REF=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            add_link_item (cur_text_doc->file_name_utf8, f, cur_text_doc->encoding);
            g_free (f);
           }
        match_start = match_end;
       }
}


void on_mni_get_links (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  scan_links();
  log_to_memo (_("Now look into the Links menu ;)"), NULL, LM_ADVICE);
}


//n.p. Massive Attack - Karmacoma [album version]
void on_mni_co_select (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  if (! doc_reload_text (cur_text_doc, cur_text_doc->file_name_local, gtk_widget_get_name (menuitem)))
     log_to_memo (_("No, I can't!"), NULL, LM_ERROR);
  else
      cur_text_doc->encoding = ch_str (cur_text_doc->encoding, gtk_widget_get_name (menuitem));
}


void on_mni_html_strip_tags (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  process_string (STRP_STR_STRIPHTML, menuitem);
}


void on_mni_markup_align (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         {
          if (strcmp (gtk_widget_get_name (menuitem), "Center") == 0)
             doc_insert_at_cursor (cur_text_doc, "<p style=\"text-align:center;\"></p>");
          else
          if (strcmp (gtk_widget_get_name (menuitem), "Left") == 0)
             doc_insert_at_cursor (cur_text_doc, "<p style=\"text-align:left;\"></p>");
          else
          if (strcmp (gtk_widget_get_name (menuitem), "Right") == 0)
             doc_insert_at_cursor (cur_text_doc, "<p style=\"text-align:right;\"></p>");
          else
          if (strcmp (gtk_widget_get_name (menuitem), "Justify") == 0)
             doc_insert_at_cursor (cur_text_doc, "<p style=\"text-align:justify;\"></p>");

            doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
       else
       if (def_mm == MM_TEX)
          {
           if (strcmp (gtk_widget_get_name (menuitem), "Center") == 0)
              doc_insert_at_cursor (cur_text_doc, "\\begin{center}\n\\end{center}");
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Left") == 0)
              doc_insert_at_cursor (cur_text_doc, "\\begin{left}\n\\end{left}");
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Right") == 0)
              doc_insert_at_cursor (cur_text_doc, "\\begin{right}\n\\end{right}");
          }

      g_free (buf);
      return;
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           if (strcmp (gtk_widget_get_name (menuitem), "Center") == 0)
              t = g_strconcat ("<p style=\"text-align:center;\">", buf, "</p>", NULL);
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Left") == 0)
               t = g_strconcat ("<p style=\"text-align:left;\">", buf, "</p>", NULL);
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Right") == 0)
              t = g_strconcat ("<p style=\"text-align:right;\">", buf, "</p>", NULL);
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Justify") == 0)
              t =  g_strconcat ("<p style=\"text-align:justify;\">", buf, "</p>", NULL);
          }
       else
       if (def_mm == MM_TEX)
          {
           if (strcmp (gtk_widget_get_name (menuitem), "Center") == 0)
              t = g_strconcat ("\\begin{center}\n", buf, "\\end{center}", NULL);
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Left") == 0)
               t = g_strconcat ("\\begin{left}\n", buf, "\\end{left}", NULL);
           else
           if (strcmp (gtk_widget_get_name (menuitem), "Right") == 0)
              t = g_strconcat ("\\begin{right}\n", buf, "\\end{right}", NULL);
         }
      }

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
}

//the mega-spagetti code is here:
void on_mni_html_make_table (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("2~2");

  gchar *table = gtk_entry_get_text (ent_search);
  gchar **a = g_strsplit (table, "~", 3);

  if (! a)
     {
      g_free (table);
      return;
     }

  gint cols = 1;
  gint rows = 1;
  gint c;
  gint r;

  GList *l = NULL;

  if (a[0])
     rows = strtol (a[0], NULL, 10);
  if (a[1])
     cols = strtol (a[1], NULL, 10);

  if (def_mm == MM_HTML || def_mm == MM_XHTML)
     l = g_list_prepend (l, strdup ("<table>"));
  else
      if (def_mm == MM_WIKI)
         l = g_list_prepend (l, strdup ("{|"));
      else
      if (def_mm ==  MM_DOCBOOK)
         l = g_list_prepend (l, g_strdup_printf ("<informaltable frame=\"none\">\n<tgroup cols=\"%d\">\n<tbody>", cols));
      else
          if (def_mm == MM_TEX)
             l = g_list_prepend (l, strdup ("\\begin{table}\n\\end{table}"));

  for (r = 1; r <= rows; r++)
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          l = g_list_prepend (l, strdup ("<tr>"));
       else
       if (def_mm == MM_DOCBOOK)
          l = g_list_prepend (l, strdup ("<row>"));

       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           for (c = 1; c <= cols; c++)
               l = g_list_prepend (l, strdup ("<td></td>"));
          }
       else
           if (def_mm == MM_WIKI)
              {
               for (c = 1; c <= cols; c++)
                  l = g_list_prepend (l, strdup ("|"));
              }
       else
       if (def_mm == MM_DOCBOOK)
          {
           for (c = 1; c <= cols; c++)
               l = g_list_prepend (l, strdup ("<entry></entry>"));
          }

       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          l = g_list_prepend (l, strdup ("</tr>"));
       else
       if (def_mm == MM_DOCBOOK)
          l = g_list_prepend (l, strdup ("</row>"));
       else
           if (def_mm == MM_WIKI)
              if (r != rows)
                 l = g_list_prepend (l, strdup ("|-"));
      }

  if (def_mm == MM_HTML || def_mm == MM_XHTML)
     l = g_list_prepend (l, strdup ("</table>"));
  else
      if (def_mm == MM_WIKI)
         l = g_list_prepend (l, strdup ("|}"));
      else
          if (def_mm ==  MM_DOCBOOK)
             l = g_list_prepend (l, strdup ("</tbody>\n</tgroup>\n</informaltable>"));

  gchar *buf = string_from_glist (g_list_reverse (l));

  doc_insert_at_cursor (cur_text_doc, buf);

  glist_strings_free (l);
  g_strfreev (a);
  g_free (buf);
}


void on_mni_file_revert_to_saved (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  if (! doc_reload_text (cur_text_doc, cur_text_doc->file_name_local, cur_text_doc->encoding))
    log_to_memo (_("I can't reload %s :("), cur_text_doc->file_name_utf8, LM_ERROR);
  else
      log_to_memo (_("%s is reloaded :)"), cur_text_doc->file_name_utf8, LM_NORMAL);
}


void on_mni_funx_ins_time (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = get_time (confile.date_time);
  doc_insert_at_cursor (cur_text_doc, buf);
  g_free (buf);
}


void on_mni_goto_line (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;
  
  set_fam_text ("0");
  
  if (check_is_number (gtk_entry_get_text (ent_search)))
     doc_select_line (cur_text_doc, strtol (gtk_entry_get_text (ent_search), NULL, 10));
}


void on_mni_convert_ascii_2_html (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  process_string (STRP_STR_MARKUPESCAPE, menuitem);
}


void on_mni_func_filter_exclude_from_list (GtkMenuItem *menuitem,
                                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("Chirurgus mente prius et oculis agat, quam armata manu");
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = filter_list (temp, gtk_entry_get_text (ent_search), FALSE);

  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_func_filter_antiexclude_from_list (GtkMenuItem *menuitem,
                                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("Tantum possumus, quantum scimus");
  gchar *s = gtk_entry_get_text (ent_search);
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = filter_list (temp, gtk_entry_get_text (ent_search), TRUE);

  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_html_calc_weight (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  GList *common_temp_list = NULL;

  gchar *f;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gtk_widget_destroy (mni_links_menu);
  mni_links_menu = new_menu_submenu (mni_links);
  mni_temp = new_menu_item (cur_text_doc->file_name_utf8, mni_links_menu, on_mni_new_link_select);
  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "src=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            gchar *x = get_full_fname (cur_text_doc->file_name_utf8, f);
            if (g_file_test (x, G_FILE_TEST_EXISTS))
                common_temp_list = g_list_prepend (common_temp_list, x);
            g_free (f);
           }
        match_start = match_end;
       }

  while (gtk_text_iter_forward_search (&match_start, "SRC=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            gchar *x = get_full_fname (cur_text_doc->file_name_utf8, f);
            if (g_file_test (x, G_FILE_TEST_EXISTS))
                common_temp_list = g_list_prepend (common_temp_list, x);
            g_free (f);
           }
        match_start = match_end;
       }

  gint bytes_total = 0;
  gchar *t = NULL;
  gint x;

  GList *p = g_list_first (common_temp_list);

  while (p)
        {
         x = get_file_size (p->data);
         bytes_total = bytes_total + x;
         t = g_strdup_printf (_("%s = %d Kb"), p->data, (gint) x / 1024);
         log_to_memo (t, NULL, LM_NORMAL);
         g_free (t);
         p = g_list_next (p);
        }

  x = get_file_size (cur_text_doc->file_name_local);
  bytes_total = bytes_total + x;
  t = g_strdup_printf (_("%s = %d Kb"), cur_text_doc->file_name_utf8, (gint) x / 1024);
  log_to_memo (t, NULL, LM_NORMAL);
  g_free (t);

  glist_strings_free (common_temp_list);

  t = g_strdup_printf (_("Total weight for %s is %d Kb"), cur_text_doc->file_name_utf8, bytes_total / 1024);
  log_to_memo (t, NULL, LM_NORMAL);
  g_free (t);
}


void on_mni_get_src (GtkMenuItem *menuitem,
                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  gchar *f;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gtk_widget_destroy(mni_links_menu);
  mni_links_menu = new_menu_submenu (mni_links);
  mni_temp = new_menu_item (cur_text_doc->file_name_utf8, mni_links_menu, on_mni_new_link_select);
  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "src=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            add_link_item (cur_text_doc->file_name_utf8, f, cur_text_doc->encoding);
            g_free (f);
           }
        match_start = match_end;
       }

  while (gtk_text_iter_forward_search (&match_start, "SRC=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            add_link_item (cur_text_doc->file_name_utf8, f, cur_text_doc->encoding);
            g_free (f);
           }
        match_start = match_end;
       }

  log_to_memo (_("Now look into Links menu ;)"), NULL, LM_ADVICE);
}


void on_mni_markup_select_color_activate (GtkMenuItem *menuitem,
                                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  GtkWidget *dialog =  gtk_color_chooser_dialog_new (_("Select a color"), 0);
      
  int r = gtk_dialog_run (GTK_DIALOG(dialog));
  if (r == GTK_RESPONSE_OK)
     {
      GdkColor color;
       
      gtk_color_chooser_get_rgba (dialog, &color);
       
      gchar *s = g_strdup_printf ("#%02x%02x%02x", color.red / 256, color.green / 256, color.blue / 256);

      gchar *cm;
      gchar *t;
      gchar *buf = NULL;

      if (doc_has_selection (cur_text_doc))
          buf = doc_get_sel (cur_text_doc);

     if (! buf)
         doc_insert_at_cursor (cur_text_doc, s);
     else
         {
          gchar *cm = g_strdup (confile.fmt_color_function);

     if (strstr (cm, "@color"))
        {
         t = g_str_replace (cm, "@color", s);
         g_free (cm);
         cm = t;
        }

     if (strstr (cm, "@text"))
       {
        t = g_str_replace (cm, "@text", buf);
        g_free (cm);
        cm = t;
       }

     doc_rep_sel (cur_text_doc, cm);
     g_free (cm);
     g_free (buf);
    }

   g_free (s);
  }
    
 gtk_widget_destroy (dialog);
}


void on_mni_file_backup (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  gchar *f = g_strconcat (cur_text_doc->file_name_local, ".bak", NULL);
  text_doc_save_silent (cur_text_doc, f);
  g_free (f);
}


void on_mni_file_add_bookmark (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gboolean flag = FALSE;

  GList *t = g_list_first (bookmarks);
  while (t)
        {
         if (g_str_has_prefix (t->data, cur_text_doc->file_name_utf8))
            {
             g_free (t->data);
             t->data = g_strdup_printf ("%s,%s,%d",
                                        cur_text_doc->file_name_utf8, cur_text_doc->encoding,
                                        editor_get_pos (cur_text_doc));
             flag = TRUE;
            }

         t = g_list_next (t);
        }

  if (! flag)
     bookmarks = add_recent_item_composed (bookmarks, cur_text_doc);

  glist_save_to_file (bookmarks, confile.bmx_file);
  bmx_reload();
}


void on_mni_file_open_bookmark (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.bmx_file, "UTF-8");
}


void on_mni_real_recent_activate (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  execute_recent_item (gtk_widget_get_name (menuitem));

  GList *p = str_in_glist (recent_list, gtk_widget_get_name (menuitem));
  if (p)
     {
      g_free (p->data);
      recent_list = g_list_delete_link (recent_list, p);
      update_recent_list_menu (FALSE);
     }
}

/*
void on_mni_refresh_hl (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  apply_hl (cur_text_doc);
}
*/

void on_mni_make_numbering (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("1~5~10");
  gchar *s = gtk_entry_get_text (ent_search);

  gchar *buf = doc_get_sel (cur_text_doc);
  if (doc_has_selection (cur_text_doc)) //exit if selected
     {
      log_to_memo (_("Please unselect the text, then try again"), NULL, LM_NORMAL);
      g_free (buf);
      return;
     }

  gchar **a = g_strsplit (s, "~", 4);

  if (! a)
     return;

  gint c;
  gint x = 0;
  gint y = 0;
  gint step = 1;

  if (a[0])
     {
      x = strtol (a[0], NULL, 10);
      if (a[1])
         {
          y = strtol (a[1], NULL, 10);
          if (a[2])
              step = strtol (a[2], NULL, 10);
         }
     }

  gchar *counter = NULL;
  GList *list = NULL;

  for (c = x; c <= y; c++)
      {
       counter = g_strdup_printf ("%d", (c * step));
       list = g_list_append (list, counter);
      }

  gchar *t = string_from_glist (list);
  doc_insert_at_cursor (cur_text_doc, t);

  g_free (t);
  glist_strings_free (list);
}


void on_mni_nav_block_start (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter iter;
  gint r = 0;
  gint l = 0;

  GtkTextMark *m = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &iter, m);

  while (gtk_text_iter_backward_char (&iter))
        {
         if (gtk_text_iter_get_char (&iter) == '}')
            r++;
         else
             if (gtk_text_iter_get_char (&iter) == '{')
                {
                 l++;
                 if (l > r)
                 if (gtk_text_iter_forward_char (&iter))
                    {
                     gtk_text_buffer_place_cursor (cur_text_doc->text_buffer, &iter );
                     gtk_text_view_scroll_to_iter (cur_text_doc->text_view, &iter, 0.0, FALSE, 0.0, 0.0 );
                     break;
                    }
                }
        }
}


void on_mni_nav_block_end (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter iter;
  gint r = 0;
  gint l = 0;

  GtkTextMark *m = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &iter, m);

  while (gtk_text_iter_forward_char (&iter))
        {
         if (gtk_text_iter_get_char (&iter) == '{')
            l++;
         else
             if (gtk_text_iter_get_char (&iter) == '}')
                {
                 r++;
                 if (l < r)
                    {
                     gtk_text_buffer_place_cursor (cur_text_doc->text_buffer, &iter );
                     gtk_text_view_scroll_to_iter (cur_text_doc->text_view, &iter, 0.0, FALSE, 0.0, 0.0 );
                     break;
                    }
                }
        }
}


void on_mni_open_hotkeys (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.tea_hotkeys, "UTF-8");
}


enum
{
  TARGET_URILIST
};


static const GtkTargetEntry drag_types[] = {
                                            {"text/uri-list", 0, TARGET_URILIST}
                                           };

static gint inner_dnd_switch = 0;

void open_files_list (const gchar *s)
{
  GList *l = glist_from_string (s);
  if (! l)
     return;

  gchar *f;
  gchar *z;

  GList *t = g_list_first (l);
  while (t && t->data)
        {
         if (strlen (t->data) < 1)
            break;

         f = g_filename_from_uri (t->data, NULL, NULL);

         if (f)
            {
             g_strchomp (f);
             if (strstr (f, "%20"))
                {
                 z = str_replace_all (f, "%20", " ");
                 g_free (f);
                 f = z;
                }

             if (is_image (f))
                {
                 if (get_page_text ())
                    insert_image (f, 0, 0, TRUE);
                }
             else
                 open_file_std (f, confile.default_charset);

             g_free (f);
            }

         t = g_list_next (t);
        }

   glist_strings_free (l);
}


void tab_on_drag_data_cb (GtkWidget *widget,
                              GdkDragContext *context,
                              gint x, gint y, GtkSelectionData *data,
                              guint info, guint time)
{
  if (inner_dnd_switch == 1)
     {
      inner_dnd_switch = 0;
      return;
     }

  gchar *filename;
  inner_dnd_switch++;

 if ((gtk_selection_data_get_length (data) == 0) || 
       (gtk_selection_data_get_format (data ) != 8) || (info != TARGET_URILIST))
     {
      gtk_drag_finish (context, FALSE, TRUE, time);
      return;
     }
     
  filename = g_strndup (gtk_selection_data_get_data (data), gtk_selection_data_get_length( data ));
  //if (info == TARGET_URILIST)
  open_files_list (filename);


  gtk_drag_finish (context, TRUE, TRUE, time);
  g_free (filename);
}


void notebook_on_drag_data_cb (GtkWidget *widget,
                              GdkDragContext *context,
                              gint x, gint y, GtkSelectionData *data,
                              guint info, guint time)
{
  gchar *filename;
  inner_dnd_switch++;

  if ((gtk_selection_data_get_length ( data ) == 0) || (gtk_selection_data_get_format ( data ) != 8) || (info != TARGET_URILIST))
     {
      gtk_drag_finish (context, FALSE, TRUE, time);
      return;
     }

  filename = g_strndup (gtk_selection_data_get_data( data ), gtk_selection_data_get_length( data ));
  //if (info == TARGET_URILIST)
  open_files_list (filename);

  gtk_drag_finish (context, TRUE, TRUE, time);
  g_free (filename);
}


void set_dnd_accept_notebook (GtkWidget *widget)
{
  gtk_drag_dest_set (widget, (GTK_DEST_DEFAULT_ALL),
                     drag_types, 1,
                    (GDK_ACTION_DEFAULT | GDK_ACTION_COPY | GDK_ACTION_MOVE));

  g_signal_connect (G_OBJECT (widget), "drag_data_received", G_CALLBACK (notebook_on_drag_data_cb), widget);
}


void set_dnd_accept_tab (GtkWidget *widget)
{
  gtk_drag_dest_set (widget, (GTK_DEST_DEFAULT_ALL),
                     drag_types, 1,
                    (GDK_ACTION_DEFAULT | GDK_ACTION_COPY | GDK_ACTION_MOVE));

  g_signal_connect (G_OBJECT (widget), "drag_data_received", G_CALLBACK (tab_on_drag_data_cb), widget);
}


gboolean on_tea_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  ui_done();
  return FALSE;
}


void on_mni_snippet_click (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *f = g_strconcat (confile.snippets_dir, gtk_widget_get_name (menuitem), NULL);

  gsize length;
  GError *error;
  gchar *buf;
  gchar *t;

  if (! g_file_get_contents (f, &buf, &length, &error))
     {
      g_free (f);
      return;
     }
  else
      if (! g_utf8_validate (buf, -1, NULL))
         {
          g_free (buf);
          g_free (f);
          return;
         }

  if (doc_has_selection (cur_text_doc))
     {
      if (strstr (buf, "%s"))
        {
         gchar *sel = doc_get_sel (cur_text_doc);
         t = rep_all_s (buf, sel);
         doc_rep_sel (cur_text_doc, t);
         g_free (t);
         g_free (sel);
        }
     }
     else
         doc_insert_at_cursor (cur_text_doc, buf);

  g_free (f);
  g_free (buf);
}


void on_mni_file_save_as_snippet_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  tea_file_save_dialog (2);
}


static GList *gl_menu_item_names;

void lookup_widget_cb_2 (GtkWidget *widget, gpointer data)
{
  if (! widget)
     return;

  gchar *s = gtk_widget_get_name (widget);
  if (strcmp (MENUITEMUTIL, s) != 0)
     {
      gl_menu_item_names = g_list_append (gl_menu_item_names, s);

      if (GTK_IS_MENU_ITEM (widget))
         lookup_widget2 (gtk_menu_item_get_submenu (widget), data);
     }
}


GtkWidget* lookup_widget2 (GtkContainer *widget, const gchar *widget_name)
{
  if (widget)
     gtk_container_foreach (widget, lookup_widget_cb_2, NULL);
  return NULL;
}


GList *get_menu_items (void)
{
  gl_menu_item_names = NULL;

  lookup_widget2 (GTK_CONTAINER (menubar1), NULL);

  return glist_copy_with_data (gl_menu_item_names, -1);
}


void on_mni_dump_menu (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  gl_menu_item_names = NULL;

  lookup_widget2 (GTK_CONTAINER (menubar1), NULL);

  gchar *t = string_from_glist (gl_menu_item_names);

  doc_ins_to_new (t);
  g_free (t);
  g_list_free (gl_menu_item_names);
}


void on_mni_nav_focus_to_famous (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  gtk_widget_grab_focus (ent_search);
}


void on_mni_nav_focus_to_text (GtkMenuItem* menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  gtk_widget_grab_focus (cur_text_doc->text_view);
}


void on_mni_tools_unitaz_sort (GtkMenuItem* menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  mpb_start();
  run_unitaz (cur_text_doc, 0, FALSE);
  mpb_stop();
}


void on_mni_tools_unitaz_sort_by_count (GtkMenuItem *menuitem,
                                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  mpb_start();
  run_unitaz (cur_text_doc, 1, FALSE);
  mpb_stop();
}


void on_mni_tools_unitaz_plain (GtkMenuItem* menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  mpb_start();
  run_unitaz (cur_text_doc, -1, TRUE);
  mpb_stop();
}


void on_mni_str_kill_dups (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  process_string (STRP_STR_KILLDUPS, menuitem);
}


void on_mni_func_strings_sort (GtkMenuItem* menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  temp = glist_strings_sort (temp);
  gchar *t = string_from_glist (temp);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_extract_words (GtkMenuItem* menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  run_extract_words (cur_text_doc);
}


void on_mni_edit_copy_all (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  clipboard_put_text (buf);
  g_free (buf);
}


void on_mni_edit_replace_with_clipboard (GtkMenuItem *menuitem,
                                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gchar *t = get_clipboard_text ();
  doc_rep_sel (cur_text_doc, t);
  clipboard_put_text (buf);

  g_free (buf);
  g_free (t);
}


void on_mni_count_string_list (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  process_string (STRP_STR_LISTENUM, menuitem);
}


void on_mni_paste_to_new (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  cur_text_doc = doc_clear_new();
  gtk_window_set_title (GTK_WINDOW (tea_main_window), cur_text_doc->file_name_utf8);
  gtk_widget_grab_focus (cur_text_doc->text_view);

  gchar *t = get_clipboard_text();
  if (! t)
     return;

  doc_insert_at_cursor (cur_text_doc, t);
  g_free (t);
}


void on_mni_copy_to_new (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = doc_get_sel (cur_text_doc);
  if (! t)
     return;

  cur_text_doc = doc_clear_new ();
  gtk_window_set_title (GTK_WINDOW (tea_main_window), cur_text_doc->file_name_utf8);
  gtk_widget_grab_focus (cur_text_doc->text_view);

  doc_insert_at_cursor (cur_text_doc, t);
  g_free (t);
}


void on_mni_cut_to_new (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
     return;

  gchar *t = doc_get_sel (cur_text_doc);
  if (! t)
     return;

  doc_rep_sel (cur_text_doc, "");
  doc_ins_to_new (t);
  g_free (t);
}


void on_mni_shuffle_strings (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  temp = glist_shuffle (temp);
  gchar *t = string_from_glist (temp);
  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_tools_unitaz_sort_caseinsens (GtkMenuItem *menuitem,
                                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  run_unitaz (cur_text_doc, 0, TRUE);
}


void on_mni_tools_unitaz_sort_by_count_caseinsens (GtkMenuItem *menuitem,
                                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  run_unitaz (cur_text_doc, 1, TRUE);
}


void on_mni_tabs_to_spaces (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text ("1");
  if (check_is_number (gtk_entry_get_text (ent_search)))
     doc_tabs_to_spaces (cur_text_doc, strtol (gtk_entry_get_text (ent_search), NULL, 10));
}


void on_mni_spaces_to_tabs (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text ("2");
  if (check_is_number (gtk_entry_get_text (ent_search)))
     doc_spaces_to_tabs (cur_text_doc, strtol (gtk_entry_get_text (ent_search), NULL, 10));
}


void on_mni_edit_delete_current_line (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter ittemp;
  GtkTextIter itend;

  GtkTextMark *mark = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);

  gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &ittemp, mark);

  itend = ittemp;

  if (gtk_text_iter_forward_to_line_end (&itend))
     while (! gtk_text_iter_starts_line (&ittemp))
           gtk_text_iter_backward_char (&ittemp);

  if (gtk_text_iter_forward_char (&itend))
     {
      gtk_text_buffer_begin_user_action (cur_text_doc->text_buffer);
      gtk_text_buffer_delete (cur_text_doc->text_buffer, &ittemp, &itend);
      gtk_text_buffer_end_user_action (cur_text_doc->text_buffer);
     }
}


void on_mni_Markup_comment (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *temp = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);
  GList *x;

  if (is_markup (cur_text_doc->file_name_local))
    {
     if (! doc_has_selection (cur_text_doc))
        doc_insert_at_cursor (cur_text_doc, "<!-- -->");
     else
         {
          temp = g_strconcat ("<!-- ", buf, " -->", NULL);
          doc_rep_sel (cur_text_doc, temp);
          g_free (temp);
         }

     g_free (buf);
     return;
    }

  if (is_c (cur_text_doc->file_name_local) || is_css (cur_text_doc->file_name_local))
     {
     if (! doc_has_selection (cur_text_doc))
        doc_insert_at_cursor (cur_text_doc, "/* */");
      else
          {
           temp = g_strconcat ("/*", buf, "*/", NULL);
           doc_rep_sel (cur_text_doc, temp);
           g_free (temp);
          }

      g_free (buf);
      return;
     }

  if (is_pascal (cur_text_doc->file_name_local))
     {
     if (! doc_has_selection (cur_text_doc))
         doc_insert_at_cursor (cur_text_doc, "{ }");
      else
          {
           temp = g_strconcat ("{", buf, "}", NULL);
           doc_rep_sel (cur_text_doc, temp);
           g_free (temp);
          }

      g_free (buf);
      return;
     }

  if (is_tex (cur_text_doc->file_name_local))
     {
     if (buf)
         {
          x = glist_from_string (buf);
          x = glist_repl (x, "% %s");
          gchar *t = string_from_glist (x);
          doc_rep_sel (cur_text_doc, t);
          glist_strings_free (x);
          g_free (t);
         }

      g_free (buf);
      return;
     }
}


void edit_paste (void)
{
  if (! get_page_text())
     return;
  
  gchar *t = get_clipboard_text();
  if (! t)
     return;

  doc_insert_at_cursor (cur_text_doc, t);
  g_free (t);
}

//current music is Free Kitten - Sentimental Education - Teenie Weenie Boppie
void on_mni_paste (GtkMenuItem *menuitem,
                   gpointer user_data)
{
  mni_last = menuitem;
  edit_paste();
}


void edit_copy (void)
{
  if (! get_page_text())
     return;
  
  gchar *buf = doc_get_sel (cur_text_doc);
  clipboard_put_text (buf);
  g_free (buf);
}


void on_mni_edit_copy (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  edit_copy();
}


void edit_cut (void)
{
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (buf)
     {
      clipboard_put_text (buf);
      doc_rep_sel (cur_text_doc, NULL);
     }

  g_free (buf);
}


void on_mni_edit_cut (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  edit_cut();
}


void on_mni_edit_delete (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_rep_sel (cur_text_doc, "");
}


void on_mni_edit_select_all (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter start_iter;
  GtkTextIter end_iter;

  gtk_text_buffer_get_bounds (cur_text_doc->text_buffer, &start_iter, &end_iter);

  gtk_text_buffer_move_mark_by_name (cur_text_doc->text_buffer,
                                     "insert",
                                     &start_iter);

  gtk_text_buffer_move_mark_by_name (cur_text_doc->text_buffer,
                                     "selection_bound",
                                     &end_iter);
}


void on_mni_sort_case_insensetive (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  temp = sort_list_case_insensetive (temp);
  gchar *t = string_from_glist (temp);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_kill_formatting (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  process_string (STRP_STR_KILLFMT, menuitem);
}


void on_mni_wrap_raw (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("72");

  if (! check_is_number (gtk_entry_get_text (ent_search)))
      return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (buf)
     {
      gchar *temp = wrap_raw (buf, strtol (gtk_entry_get_text (ent_search), NULL, 10));
      doc_rep_sel (cur_text_doc, temp);
      g_free (buf);
      g_free (temp);
     }
}


void on_mni_wrap_on_spaces (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("72");

  if (! check_is_number (gtk_entry_get_text (ent_search)))
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (buf)
     {
      wrap_on_spaces (buf, strtol (gtk_entry_get_text (ent_search), NULL, 10));
      doc_rep_sel (cur_text_doc, buf);
      g_free (buf);
     }
}


void on_doit_button (GtkButton *button,
                     gpointer user_data)
{
  if (! get_page_text())
     return;

  set_fam_text ("text to find");
  doc_search_f (cur_text_doc, gtk_entry_get_text (ent_search));
}


void on_mni_kill_formatting_on_each_line (GtkMenuItem *menuitem,
                                          gpointer user_data)
{
  process_string (STRP_STR_KILLFMT_EACHLINE, menuitem);
}


void on_mni_file_crapbook (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! g_file_test (confile.crapbook_file, G_FILE_TEST_EXISTS))
     create_empty_file (confile.crapbook_file, _("you can put some crap here..."));

  handle_file_enc (confile.crapbook_file, "UTF-8");
}


void on_mni_spellcheck (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  confile.def_spell_lang = ch_str (confile.def_spell_lang, gtk_widget_get_name (menuitem));
  do_hl_spell_check (cur_text_doc, gtk_widget_get_name (menuitem));
}


void on_mni_filter_kill_lesser (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("2");
  if (! check_is_number (gtk_entry_get_text (ent_search)))
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (buf)
     {
      gchar *t = kill_str_by_size (buf, strtol (gtk_entry_get_text (ent_search), NULL, 10), TRUE);
      doc_rep_sel (cur_text_doc, t);
      g_free (buf);
      g_free (t);
     }
}


void on_mni_filter_kill_more_than (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("2");
  if (! check_is_number (gtk_entry_get_text (ent_search)))
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (buf)
     {
      gchar *t = kill_str_by_size (buf, strtol (gtk_entry_get_text (ent_search), NULL, 10), FALSE);
      doc_rep_sel (cur_text_doc, t);
      g_free (buf);
      g_free (t);
     }
}


void on_mni_func_strings_sort_by_q (GtkMenuItem* menuitem,
                                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  temp = glist_word_sort_mode (temp, 2);
  gchar *t = string_from_glist (temp);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
}


void on_mni_tabs_menuitem_click (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  t_note_page *doc = user_data;
  if (doc)
     handle_file (doc->file_name_utf8, doc->encoding, 0, FALSE);
}


void on_mni_show_images_in_text (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     return;

  doc_toggle_images_visibility (cur_text_doc);
}


void on_mni_nav_goto_recent_tab (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  gint t = gtk_notebook_get_current_page (notebook1);

  if (last_page != -1)
      gtk_notebook_set_current_page (notebook1, last_page);

  last_page = t;
}


void on_mni_nav_goto_selection (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gtk_text_view_scroll_to_mark (cur_text_doc->text_view, gtk_text_buffer_get_insert
                               (cur_text_doc->text_buffer), 0.0, TRUE, 0.0, 0.5);
}


void on_mni_nav_goto_prev_tab (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  last_page = gtk_notebook_get_current_page (notebook1);
  gtk_notebook_prev_page (notebook1);
}


void on_mni_nav_goto_next_tab (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  last_page = gtk_notebook_get_current_page (notebook1);
  gtk_notebook_next_page (notebook1);
}


void on_mni_view_hide_highlighting (GtkMenuItem *menuitem,
                                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  remove_tags (cur_text_doc);
}


void on_mni_nav_goto_first_tab (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  last_page = gtk_notebook_get_current_page (notebook1);
  gtk_notebook_set_current_page (notebook1, 0);
}


void on_mni_nav_goto_last_tab (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  last_page = gtk_notebook_get_current_page (notebook1);
  gtk_notebook_set_current_page (notebook1, -1);
}


void on_mni_quest_find (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text (_("text to find"));
  doc_search_f (cur_text_doc, gtk_entry_get_text (ent_search));
}


void on_mni_quest_find_next (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_search_f_next (cur_text_doc);
}


void on_mni_quest_find_ncase (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text (_("text to find"));
  doc_search_f_ncase (cur_text_doc, gtk_entry_get_text (ent_search));
}


void on_mni_quest_find_next_ncase (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_search_f_next_ncase (cur_text_doc);
}


//n.p. Cypress Hill - Insane In The Brain
void on_mni_find_and_replace_wnd (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  create_fr();
}


void on_mni_rtfm (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  mni_last = menuitem;
  run_doc_in_browser();
}


void on_mni_crackerize (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = doc_get_sel (cur_text_doc);

  if (! t)
     return;

  gchar *x = str_crackerize (t);
  doc_rep_sel (cur_text_doc, x);

  g_free (t);
  g_free (x);
}


void show_options (void)
{
  wnd_options_create();

 // glist_strings_free (gl_tags_list);
 // gl_tags_list = NULL;
}


void on_mni_show_options (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  wnd_options_create();
}


void on_mni_new_kwas (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  create_kwas_window();
}

//n.p. Orbital - The Box
void on_mni_open_kwas_bookmarks_file (GtkMenuItem *menuitem,
                                      gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.tea_kwas_bookmarks, "UTF-8");
}


void on_mni_eol_to_crlf (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  if (! buf)
     return;

  gchar *z = str_eol_to_crlf (buf);
  if (z)
     {
      gtk_text_buffer_set_text (cur_text_doc->text_buffer, z, -1);
      cur_text_doc->end_of_line = ch_str (cur_text_doc->end_of_line, "CR/LF");
      doc_update_statusbar (cur_text_doc);
      g_free (z);
     }

  g_free (buf);
}


void on_mni_eol_to_lf (GtkMenuItem* menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  if (! buf)
     return;

  gchar *z = str_eol_to_lf (buf);

  if (z)
     {
      gtk_text_buffer_set_text (cur_text_doc->text_buffer, z, -1);
      cur_text_doc->end_of_line = ch_str (cur_text_doc->end_of_line, "LF");
      doc_update_statusbar (cur_text_doc);
      g_free (z);
     }

  g_free (buf);
}


void on_mni_upcase_each_first_letter_ofw (GtkMenuItem *menuitem,
                                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
     return;

  gchar *z = upcase_each_first_letter_ofw (cur_text_doc);
  if (z)
     doc_rep_sel (cur_text_doc, z);

  g_free (z);
}


void on_mni_nav_mplayer_jump (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! confile.cm_mplayer)
     return;

  if (! get_page_text())
     return;

  if (! confile.current_movie)
     {
      log_to_memo (_("Before using that, go to File > Open different > Open movie."), NULL, LM_ERROR);
      return;
     }

  GtkTextIter iter;
  GtkTextMark *m = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);
  gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &iter, m);

  GtkTextIter end;
  GtkTextIter start;

  if (! gtk_text_iter_forward_search (&iter, "\n\n", GTK_TEXT_SEARCH_TEXT_ONLY,
                                      &end, NULL, NULL))
     return;

  if (! gtk_text_iter_backward_search (&end, "-->", GTK_TEXT_SEARCH_TEXT_ONLY,
                                       &iter, NULL, NULL))
     return;


  if (! gtk_text_iter_backward_search (&iter, "\n", GTK_TEXT_SEARCH_TEXT_ONLY,
                                       &start, NULL, NULL))
     return;

  gchar *t = gtk_text_iter_get_text (&start, &iter);
  if (! t)
     return;

  t = g_strstrip (t);

  gchar *s;
  gchar *m1;
  gchar *m2;

  gchar *cm = g_strdup (confile.cm_mplayer);

  if (strstr (cm, "@encoding"))
     {
      s = g_str_replace (cm, "@encoding", cur_text_doc->encoding);
      g_free (cm);
      cm = s;
     }

  if (strstr (cm, "@time"))
     {
      s = g_str_replace (cm, "@time", t);
      g_free (cm);
      cm = s;
     }

  if (strstr (cm, "@sub_file"))
     {
      s = g_str_replace (cm, "@sub_file", cur_text_doc->file_name_utf8);
      g_free (cm);
      cm = s;
     }

  if (strstr (cm, "@movie_file"))
     {
      s = g_str_replace (cm, "@movie_file", confile.current_movie);
      g_free (cm);
      cm = s;
     }

  system (cm);

  g_free (t);
  g_free (cm);
}


void on_mni_open_open_movie (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open a movie:"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  gchar *t;
  gchar *dir;

  if (get_page_text())
  if (g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
     {
      t = g_path_get_dirname (cur_text_doc->file_name_local);
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), t);
      g_free (t);
     }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
     gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
     confile.current_movie = ch_str (confile.current_movie, filename);
     g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

#ifdef FOR_DEBIAN
void on_mni_help_item (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;

  gchar *n = gtk_widget_get_name (menuitem);
  gchar *f;

  if (g_utf8_collate (n, "ChangeLog") == 0)
     f = g_strconcat (TEA_DOC_DIR, "changelog.gz", NULL);
  else
  if (g_utf8_collate (n, "COPYING") == 0)
     f = g_strdup ("/usr/share/common-licenses/GPL-2");
  else
  if (g_utf8_collate (n, "NEWS") == 0)
     f = g_strconcat (TEA_DOC_DIR, "NEWS.gz", NULL);
  else
      f = get_tea_doc_compose_name (gtk_widget_get_name (menuitem));

  cur_text_doc = open_file_std (f, "UTF-8");
  
  if (cur_text_doc)
     cur_text_doc->readonly = TRUE;
  
  g_free (f);
}
#else

void on_mni_help_item (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  gchar *f = get_tea_doc_compose_name (gtk_widget_get_name (menuitem));
  cur_text_doc = open_file_std (f, "UTF-8");
  if (cur_text_doc)
     cur_text_doc->readonly = TRUE;
  g_free (f);
}

#endif


void on_mni_insert_doctype (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
      return;

  gchar *n = gtk_widget_get_name (menuitem);
  gchar *t = NULL;

  if (g_utf8_collate (n, "HTML 4.1 Transitional") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">");
  else
  if (g_utf8_collate (n, "HTML 4.1 Strict") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">");
  else
  if (g_utf8_collate (n, "HTML 4.1 Frameset") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/frameset.dtd\">");
  else
  if (g_utf8_collate (n, "XHTML 1.0 Strict") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");
  else
  if (g_utf8_collate (n, "XHTML 1.0 Transitional") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
  else
  if (g_utf8_collate (n, "XHTML 1.0 Frameset") == 0)
     t = g_strdup ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Frameset//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd\">");
  else
  if (g_utf8_collate (n, "WML 1.1") == 0)
     t = g_strdup ("<xml version='1.0'> \n<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">");

  doc_insert_at_cursor (cur_text_doc, t);

  g_free (t);
}


void on_mni_entity (GtkMenuItem *menuitem,
                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *n = gtk_widget_get_name (menuitem);
  gchar *x = g_hash_table_lookup (ht_entities, n);
  if (x)
     t = g_strdup_printf ("&#%s;", x);

  doc_insert_at_cursor (cur_text_doc, t);

  g_free (t);
}


void on_mni_sessions_click (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  gchar *f = g_strconcat (confile.sessions_dir, gtk_widget_get_name (menuitem), NULL);
  session_open_from_file (f);
  g_free (f);
}


void on_mni_templates_click (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  gchar *f = g_strconcat (confile.templates_dir, gtk_widget_get_name (menuitem), NULL);
  gchar *t = str_file_read (f);
  doc_ins_to_new (t);
  g_free (f);
  g_free (t);
}


void on_mni_morse_encode (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = doc_get_sel (cur_text_doc);

  if (! t)
     return;

  gchar *x = morse_encode (t);
  doc_rep_sel (cur_text_doc, x);
  g_free (t);
  g_free (x);
}


void on_mni_morse_decode (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = doc_get_sel (cur_text_doc);

  if (! t)
     return;

  gchar *x = morse_decode (t);

  doc_rep_sel (cur_text_doc, x);
  g_free (t);
  g_free (x);
}


void on_mni_set_hl_mode (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
      return;
  
  cur_text_doc->hl_mode = ch_str (cur_text_doc->hl_mode, gtk_widget_get_name (menuitem));
  apply_hl (cur_text_doc);
}


void on_mni_save_pos (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  cur_text_doc->position = editor_get_pos (cur_text_doc);
}


void on_mni_jump_to_saved_pos (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
      return;

  editor_set_pos (cur_text_doc, cur_text_doc->position);
}


void on_mni_unindent (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  doc_indent_selection (cur_text_doc, TRUE);
}


void on_mni_indent (GtkMenuItem *menuitem,
                    gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  doc_indent_selection (cur_text_doc, FALSE);
}


void on_mni_autorep_file_open (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.tea_autoreplace, "UTF-8");
}


void on_mni_strings_remove_leading_whitespaces (GtkMenuItem *menuitem,
                                                gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = each_line_remove_lt_spaces (temp, 0);
  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_strings_remove_trailing_whitespaces (GtkMenuItem *menuitem,
                                                 gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = each_line_remove_lt_spaces (temp, 1);
  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_edit_append_to_crapbook (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  gchar *t = g_strconcat (buf, "\n", NULL);

  t_note_page *p;
  GtkTextIter iter;
  
  gint i = get_n_page_by_filename (confile.crapbook_file);

  if (i != -1)
     {
      p = get_page_by_index (i);
      gtk_text_buffer_get_iter_at_offset (p->text_buffer, &iter, 0);
      gtk_text_buffer_insert (p->text_buffer, &iter, t, -1);
      g_free (buf);
      g_free (t);
      return;
     }

  GList *l = load_file_to_glist (confile.crapbook_file);

  l = g_list_prepend (l, t);

  glist_save_to_file (l, confile.crapbook_file);
  glist_strings_free (l);
  g_free (buf);
}


void on_mni_load_last_file (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! recent_list)
    return;

  execute_recent_item (recent_list->data);
}


void on_source_header_switch (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_header_source_switch (cur_text_doc);
}


gboolean cb_logmemo_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if (lm_dclick_mode == DCM_NONE)
      return FALSE;

  gchar *r = NULL;
  GtkTextView *text_view = user_data;
  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (text_view);

  if (event->button == 1)
     if (event->type == GDK_2BUTTON_PRESS)
        {
         gchar *r = NULL;

         GtkTextIter target_iter;
         GtkTextIter end_line_iter;
         gint line_top;

         GtkTextMark *mark = gtk_text_buffer_get_insert (text_buffer);
         gtk_text_buffer_get_iter_at_mark (text_buffer, &target_iter, mark);
         gtk_text_iter_set_line (&target_iter, gtk_text_iter_get_line (&target_iter)) ;
         end_line_iter = target_iter;

         if (gtk_text_iter_forward_to_line_end (&end_line_iter))
             r = gtk_text_buffer_get_text (text_buffer, &target_iter, &end_line_iter, FALSE);

         if (! r)
            return FALSE;

         if (lm_dclick_mode == DCM_MAKE || lm_dclick_mode == DCM_GREP)
            {
             gchar *filename = NULL;
             gint lineno;

             if (lm_dclick_mode == DCM_MAKE)
                if (parse_error_line (r, &filename, &lineno))
                   handle_file_ide (filename, (lineno - 1));

             if (lm_dclick_mode == DCM_GREP)
                if (parse_grep_line (r, &filename, &lineno))
                   {
                    handle_file (filename, "UTF-8", 0, FALSE);
                    if (get_page_text ())
                       doc_select_line (cur_text_doc, lineno);
                   }

             g_free (filename);
             g_free (r);

             return TRUE;
            }
        }

  return FALSE;
}


void on_mni_show_project_props (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  if (cur_tea_project)
     create_proj_props_window (cur_tea_project);
}


void on_mni_project_new (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  cur_tea_project = tea_proj_clear_new (cur_tea_project);
  if (cur_tea_project)
     create_proj_props_window (cur_tea_project);
}


void on_mni_project_save_as (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  mni_last = menuitem;
  if (! cur_tea_project)
     return;

  gchar *filename;

  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Save a project"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
     {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      tea_proj_save_as (cur_tea_project, filename);
      g_free (filename);
     }

  gtk_widget_destroy (dialog);
}


void on_mni_show_project_save (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! cur_tea_project)
     return;

  if (! g_file_test (cur_tea_project->project_file_name, G_FILE_TEST_EXISTS))
     {
      on_mni_project_save_as (NULL, NULL);
      return;
     }

  tea_proj_save (cur_tea_project);
}


void on_mni_project_open (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  gchar *filename;

  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open a project"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
     {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      tea_proj_free (cur_tea_project);
      cur_tea_project = tea_proj_open (filename);
      g_free (filename);
     }

  gtk_widget_destroy (dialog);
}


void on_mni_project_make (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;

  if (! cur_tea_project)
      return;

  if (! cur_tea_project->dir_makefile)
      return;

  if (! g_file_test (cur_tea_project->dir_makefile, G_FILE_TEST_EXISTS))
     return;

  gchar *dir = g_strconcat (cur_tea_project->dir_makefile, G_DIR_SEPARATOR_S, NULL);
  chdir (dir);
  g_free (dir);

  gchar *standard_output = NULL;
  gchar *standard_error = NULL;

  gint exit_status;
  GError *err = NULL;

  gchar *makefile = file_combine_path (cur_tea_project->dir_makefile, "Makefile");
  gchar *cmd = g_strconcat ("make -f ", makefile, NULL);
  g_free (makefile);

  gsize bytes_read;
  gsize bytes_written;
  gchar *x = NULL;

  lm_dclick_mode = DCM_MAKE;

  if (! g_spawn_command_line_sync  (cmd, &standard_output, &standard_error, NULL, &err))
     {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      return;
     }
  else
      {
       x = g_locale_to_utf8 (standard_output, -1, &bytes_read, &bytes_written, NULL);
       log_to_memo (x, NULL, LM_NORMAL);
       g_free (x);

       x = g_locale_to_utf8 (standard_error, -1, &bytes_read, &bytes_written, NULL);
       log_to_memo (x, NULL, LM_NORMAL);
       g_free (x);
       do_errors_hl (tv_logmemo);
      }

  g_free (cmd);
  g_free (standard_output);
  g_free (standard_error);
}


//n.p. Scorn - Evanescence - Silver Rain Fell
void on_mni_show_project_run (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  tea_proj_run (cur_tea_project);
}


void on_mni_add_to_autosave_list (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;

  if (! get_page_text())
     return;

  if (! cur_text_doc->b_saved)
     return;

  gl_autosave = g_list_prepend (gl_autosave, g_strdup (cur_text_doc->file_name_utf8));
  glist_save_to_file (gl_autosave, confile.autosave_file);
  reload_autosave();
}


void on_mni_autosave_list_open (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.autosave_file, "UTF-8");
}


void on_mni_string_unquote (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
      return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  gchar *t = g_shell_unquote (buf, NULL);
  doc_rep_sel (cur_text_doc, t);
  g_free (buf);
  g_free (t);
}


void on_mni_copy_filename (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! cur_text_doc->b_saved)
     return;

  clipboard_put_text (cur_text_doc->file_name_utf8);
}


void on_mni_quote_string (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gchar *t = g_shell_quote (buf);
  doc_rep_sel (cur_text_doc, t);
  g_free (buf);
  g_free (t);
}


void on_mni_file_close_all (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  doc_close_all();
}


void on_mni_text_to_html_fmt (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gboolean xhtml_mode = FALSE;
  if (def_mm == MM_XHTML)
     xhtml_mode = TRUE;

  gchar *t = str_to_html (buf, xhtml_mode);
  doc_ins_to_new (t);
  g_free (buf);
  g_free (t);
}


void on_mni_mm_change (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  confile.default_markup_mode = ch_str (confile.default_markup_mode, gtk_widget_get_name (menuitem));
  def_mm = get_markup_mode ();
}


void on_mni_imageplane (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! cur_text_doc->b_saved)
     {
      log_to_memo (_("Open the file first"), NULL, LM_ERROR);
      return;
     }

  gchar *dir = g_path_get_dirname (cur_text_doc->file_name_local);
  t_image_plane *plane = create_image_plane (dir);
  g_free (dir);
}


void on_mni_quotes_to_tex (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  gchar *t = quotes_to_tex (buf);
  doc_rep_sel (cur_text_doc, t);
  g_free (buf);
  g_free (t);
}


void on_mni_doc_close_all (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  doc_close_all();
}


void on_mni_latex_process (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  latex_process (cur_text_doc->file_name_local);
}


void on_mni_dvi_to_ps (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  latex_dvi_to_ps (cur_text_doc->file_name_local);
}


void on_mni_dvi_to_pdf (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  latex_dvi_to_pdf (cur_text_doc->file_name_local);
}


void on_mni_view_dvi (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  view_dvi (cur_text_doc->file_name_local);
}


void on_mni_view_pdf (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  view_pdf  (cur_text_doc->file_name_local);
}


void on_mni_view_ps (GtkMenuItem *menuitem,
                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  view_ps (cur_text_doc->file_name_local);
}


void on_mni_split_to_tex_paragraphs (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  gchar *t = tex_paras (buf);
  doc_rep_sel (cur_text_doc, t);
  g_free (buf);
  g_free (t);
}


void on_mni_markup_list_item (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           doc_insert_at_cursor (cur_text_doc, "<li></li>");
           doc_move_cursor_backw_middle_tags (cur_text_doc);
           }
       else
           if (def_mm == MM_TEX)
              doc_insert_at_cursor (cur_text_doc, "\\item");
       else
           if (def_mm == MM_DOCBOOK)
              doc_insert_at_cursor (cur_text_doc, "<listitem><para></para></listitem>");
      }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<li>", buf, "</li>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
          if (def_mm == MM_TEX)
            {
             t = g_strconcat ("\\item", buf, NULL);
             doc_rep_sel (cur_text_doc, t);
             g_free (t);
            }
       else
           if (def_mm == MM_DOCBOOK)
            {
             t = g_strconcat ("<listitem><para>", buf, "</para></listitem>", NULL);
             doc_rep_sel (cur_text_doc, t);
             g_free (t);
            }
      }

  g_free (buf);
}


void on_mni_markup_tex_begend (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      doc_insert_at_cursor (cur_text_doc, "\\begin{}\n\\end{}");
      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      if (def_mm == MM_TEX)
        {
         t = g_strconcat ("\\begin{}\n", buf, "\\end{}", NULL);
         doc_rep_sel (cur_text_doc, t);
         g_free (t);
        }

  g_free (buf);
}


void on_mni_markup_col (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
          doc_insert_at_cursor (cur_text_doc, "<td></td>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<entry></entry>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<td>", buf, "</td>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
           if (def_mm == MM_DOCBOOK)
            {
             t = g_strconcat ("<entry>", buf, "</entry>", NULL);
             doc_rep_sel (cur_text_doc, t);
             g_free (t);
            }
      }

  g_free (buf);
}


void on_mni_markup_row (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
          doc_insert_at_cursor (cur_text_doc, "<tr></tr>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<row></row>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<tr>", buf, "</tr>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
          if (def_mm == MM_WIKI)
            {
             t = g_strconcat (buf, "\\par", NULL);
             doc_rep_sel (cur_text_doc, t);
             g_free (t);
            }
       else
           if (def_mm == MM_DOCBOOK)
             {
              t = g_strconcat ("<row>", buf, "</row>", NULL);
              doc_rep_sel (cur_text_doc, t);
              g_free (t);
             }
      }

  g_free (buf);
}


void on_mni_markup_table (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
          doc_insert_at_cursor (cur_text_doc, "<table></table>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc,
                                   "<informaltable frame=\"none\">\n<tgroup cols=\"7\">\n<tbody>\n</tbody>\n</tgroup>\n</informaltable>");

      doc_move_cursor_backw_middle_tags (cur_text_doc);
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<table>\n", buf, "</table>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
          if (def_mm == MM_WIKI)
            {
             t = g_strconcat (buf, "{|\n|}", NULL);
             doc_rep_sel (cur_text_doc, t);
             g_free (t);
            }
       else
           if (def_mm == MM_DOCBOOK)
             {
              t = g_strconcat ("<informaltable frame=\"none\">\n<tgroup cols=\"777\">\n<tbody>",
                               buf, "</tbody>\n</tgroup>\n</informaltable>", NULL);
              doc_rep_sel (cur_text_doc, t);
              g_free (t);
             }
      }
      
  g_free (buf);
}


void on_mni_markup_itemizedlist (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         {
          doc_insert_at_cursor (cur_text_doc, "<ul></ul>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
       else
           if (def_mm == MM_TEX)
               doc_insert_at_cursor (cur_text_doc, "\\begin{itemize}\n\\end{itemize}");
           else
               if (def_mm == MM_DOCBOOK)
                  {
                   doc_insert_at_cursor (cur_text_doc, "<itemizedlist></itemizedlist>");
                   doc_move_cursor_backw_middle_tags (cur_text_doc);
                  }
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<ul>", buf, "</ul>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
           if (def_mm == MM_TEX)
              {
               t = g_strconcat ("\\begin{itemize}\n", buf, "\\end{itemize}", NULL);
               doc_rep_sel (cur_text_doc, t);
               g_free (t);
              }
           else
               if (def_mm == MM_DOCBOOK)
                  {
                   t = g_strconcat ("<itemizedlist>", buf, "</itemizedlist>", NULL);
                   doc_rep_sel (cur_text_doc, t);
                   g_free (t);
                  }
      }

  g_free (buf);
}


void on_mni_markup_enumeratedlist (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
         {
          doc_insert_at_cursor (cur_text_doc, "<ol></ol>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
         else
             if (def_mm == MM_TEX)
                doc_insert_at_cursor (cur_text_doc, "\\begin{enumerate}\n\\end{enumerate}");
             else
                 if (def_mm == MM_DOCBOOK)
                    {
                     doc_insert_at_cursor (cur_text_doc, "<orderedlist></orderedlist>");
                     doc_move_cursor_backw_middle_tags (cur_text_doc);
                    }
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<ol>", buf, "</ol>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
           }
       else
           if (def_mm == MM_TEX)
              {
               t = g_strconcat ("\\begin{enumerate}\n", buf, "\\end{enumerate}", NULL);
               doc_rep_sel (cur_text_doc, t);
               g_free (t);
              }
           else
               if (def_mm == MM_DOCBOOK)
                  {
                   t = g_strconcat ("<orderedlist>", buf, "</orderedlist>", NULL);
                   doc_rep_sel (cur_text_doc, t);
                   g_free (t);
                  }
       }

  g_free (buf);
}


void on_mni_markup_em (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_HTML || def_mm == MM_XHTML)
          doc_insert_at_cursor (cur_text_doc, "<em></em>");
      else
          if (def_mm == MM_DOCBOOK)
             doc_insert_at_cursor (cur_text_doc, "<emphasis></emphasis>");
      else
          if (def_mm == MM_TEX)
             doc_insert_at_cursor (cur_text_doc, "\\emph{}");

      //doc_move_cursor_backw_middle_tags (cur_text_doc);
      //CHECKME: but how doc_move_cursor_backw_middle_tags works if MM_TEX?
     }
  else
      {
       if (def_mm == MM_HTML || def_mm == MM_XHTML)
          {
           t = g_strconcat ("<em>", buf, "</em", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
          }
       else
           if (def_mm == MM_DOCBOOK)
             {
              t = g_strconcat ("<emphasis>", buf, "</emphasis>", NULL);
              doc_rep_sel (cur_text_doc, t);
              g_free (t);
             }
        else
           if (def_mm == MM_TEX)
             {
              t = g_strconcat ("\\emph{", buf, "}", NULL);
              doc_rep_sel (cur_text_doc, t);
              g_free (t);
             }
      }

  g_free (buf);
}


void on_mni_calendar (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! wnd_calendar)
      wnd_calendar = make_calendar_window ();
  else
     {
      gtk_widget_destroy (wnd_calendar);
      wnd_calendar = make_calendar_window ();
     }
}


void on_mni_markup_section (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_DOCBOOK)
         {
          doc_insert_at_cursor (cur_text_doc, "<section>\n<title></title>\n</section>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
      else
          if (def_mm == MM_TEX)
            doc_insert_at_cursor (cur_text_doc, "\\section{}");
     }
  else
      {
       if (def_mm == MM_DOCBOOK)
          {
           t = g_strconcat ("<section>", buf, "</section>", NULL);
           doc_rep_sel (cur_text_doc, t);
           g_free (t);
           }
      }

  g_free (buf);
}


void on_mni_markup_chap (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! doc_has_selection (cur_text_doc))
     {
      if (def_mm == MM_DOCBOOK)
         {
          doc_insert_at_cursor (cur_text_doc, "<chapter>\n<title></title>\n</chapter>");
          doc_move_cursor_backw_middle_tags (cur_text_doc);
         }
      else
      if (def_mm == MM_TEX)
         doc_insert_at_cursor (cur_text_doc, "\\chapter[]{}");
     }
  else
      if (def_mm == MM_DOCBOOK)
         {
          t = g_strconcat ("<chapter>", buf, "</chapter>", NULL);
          doc_rep_sel (cur_text_doc, t);
          g_free (t);
         }

  g_free (buf);
}


void on_mni_markup_tex_common (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = NULL;

  t = g_strconcat ("\\", gtk_widget_get_name (menuitem), NULL);
  doc_insert_at_cursor (cur_text_doc, t);
  g_free (t);
}


void on_mni_pdflatex_process (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  pdflatex_process (cur_text_doc->file_name_local);
}


void on_mni_close_current_tag (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;
  
  current_tag_close (cur_text_doc, 1);
}


void on_mni_csv_to_latex_table (GtkMenuItem* menuitem,
                                gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  set_fam_text (",");
  
  gchar *t = separated_table_to_latex_tabular (buf, gtk_entry_get_text (ent_search));
  doc_ins_to_new (t);
  g_free (buf);
}


void on_mni_fullscreen (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  gtk_window_fullscreen (tea_main_window);
}


void on_mni_unfullscreen (GtkMenuItem *menuitem,
                        gpointer user_data)
{
  mni_last = menuitem;
  gtk_window_unfullscreen (tea_main_window);
}


void on_mni_ins_ast (GtkMenuItem *menuitem,
                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  doc_insert_at_cursor (cur_text_doc, "\\begin{center}\n***\\\\\n\\end{center}\n");
}


static void fam_open (void)
{
  set_fam_text ("noname");

  gchar *z = gtk_entry_get_text (ent_search);

  if (g_utf8_get_char (z) != G_DIR_SEPARATOR)
     z = g_utf8_find_next_char (z, NULL);

  gchar *lf = get_l_filename (z);
  if (! lf)
      return;

  g_strstrip (lf);

  gchar *x = filename_from_xuri (z);

  open_file_std (x, confile.default_charset);

  g_free (x);
  g_free (lf);
}


void on_bt_open (GtkButton *button,
                 gpointer user_data)
{
  fam_open();
}


void on_mni_fam_open (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  fam_open();
}


void on_mni_copied_html_to_ftext (GtkMenuItem* menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("   %s");
  if (! strstr (gtk_entry_get_text (ent_search), "%s"))
      return;

  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  gchar *t = copied_from_browser_to_ftext (buf, gtk_entry_get_text (ent_search));
  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
}


void on_mni_move_line_up (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  swap_lines (cur_text_doc, TRUE);
}


void on_mni_move_line_down (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  swap_lines (cur_text_doc, FALSE);
}


void on_mni_save_all (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  doc_save_all();
}


gboolean save_all_cb (gpointer data)
{
  if (b_exit_flag)
     return FALSE;

  if (confile.do_autosave)
     doc_save_all();

  if (confile.do_autosave_session)
     {
      session_save_to_file (confile.autosave_session);
      log_to_memo (_("%s session is saved"), confile.autosave_session, LM_NORMAL);
     }

  return TRUE;
}


static gboolean on_ent_fname_key_press_event (GtkWidget *widget,
                                       GdkEventKey *event,
                                       gpointer user_data)
{
  if (event->keyval != GDK_KEY_Return)
     return FALSE;

  gtk_dialog_response (GTK_DIALOG (user_data), GTK_RESPONSE_ACCEPT);

  return FALSE;
}


void tea_file_open_dialog (void)
{
  gboolean result = FALSE;

  GtkWidget *file_dialog = gtk_dialog_new_with_buttons (_("File open"),
                                                        NULL,
                                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_STOCK_CANCEL,
                                                        GTK_RESPONSE_REJECT,
                                                        GTK_STOCK_OK,
                                                        GTK_RESPONSE_ACCEPT,
                                                        NULL);

  if (confile.exclusive_enter_at_dialogs)
     gtk_dialog_set_default_response (file_dialog, GTK_RESPONSE_ACCEPT);

  g_signal_connect (G_OBJECT (file_dialog), "key_press_event", G_CALLBACK (dlg_key_handler), file_dialog);
  
  GtkFileChooserWidget *fcw = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_OPEN);
  
  GtkFileFilter *f = tea_file_filter_with_patterns (_("All files"), "*", NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fcw), f);
  f = tea_file_filter_with_patterns (_("Plain text files"), "*.txt", "*.TXT", NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fcw), f);

  f = tea_file_filter_with_patterns (_("Office text format files"),
                                       "*.odt", "*.kwd", "*.sxw",
                                       "*.abw", "*.rtf", "*.doc",
                                       NULL);

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fcw), f);

  gtk_file_chooser_add_filters_from_langs (GTK_FILE_CHOOSER (fcw));


  GSList *lfilters = gtk_file_chooser_list_filters (GTK_FILE_CHOOSER (fcw));
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fcw),  g_slist_nth_data (lfilters, confile.filter_index));

  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (fcw), TRUE);
  g_signal_connect ((gpointer) fcw, "file-activated",
                    G_CALLBACK (file_open_file2click),
                    file_dialog);

  gtk_widget_show (fcw);
  gtk_box_pack_start (gtk_dialog_get_content_area (file_dialog), fcw, TRUE, TRUE, UI_PACKFACTOR);
  
  GtkWidget *vb = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vb);

  gint i = find_str_index (confile.iconv_encs, confile.default_charset);
  if (i == -1)
      i = 0;

  GtkComboBox *cb_tea_places = tea_combo_combo (vb, gl_tea_special_places, _("TEA special locations:"), -1, change_teaspec_location, GTK_FILE_CHOOSER (fcw));
  GtkWidget *cb_charset = tea_combo_combo (vb, confile.iconv_encs, _("Encoding: "), i, NULL, NULL);

  gtk_file_chooser_set_extra_widget (fcw, vb);

  gtk_window_resize (GTK_WINDOW (file_dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  gchar *t;
  gchar *filename;

  if (confile.use_def_open_dir)
     gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), confile.def_open_dir);
  else
      {
       if (last_dir)
          gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), last_dir);
       else
           gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), g_get_home_dir ());
      }

  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (fcw), TRUE);

  GSList *lt = NULL;

  if (gtk_dialog_run (GTK_DIALOG (file_dialog)) == GTK_RESPONSE_ACCEPT)
    {
     GSList *l = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (fcw));
     if (! l)
        {
         g_slist_free (lfilters);
         gtk_widget_destroy (file_dialog);

         return;
        }

     lt = l;

     while (lt)
           {
            filename = lt->data;
            open_file_std (filename, combo_get_value (cb_charset, confile.iconv_encs));
            g_free (filename);
            lt = g_slist_next (lt);
           }

     g_slist_free (l);
    }

//////////
  lfilters = gtk_file_chooser_list_filters (GTK_FILE_CHOOSER (fcw));
  f = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (fcw));
  GSList *lx = g_slist_find (lfilters, f);
  if (lx)
     confile.filter_index = g_slist_position (lfilters, lx);
  g_slist_free (lfilters);
///////////

  gtk_widget_destroy (file_dialog);
}


void tea_file_save_dialog (int mode)
{
  if (! get_page_text())
     return;

  gboolean result = FALSE;

  GtkWidget *file_dialog = gtk_dialog_new_with_buttons (_("Save as..."),
                                                        NULL,
                                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_STOCK_CANCEL,
                                                        GTK_RESPONSE_REJECT,
                                                        GTK_STOCK_OK,
                                                        GTK_RESPONSE_ACCEPT,
                                                        NULL);

  if (confile.exclusive_enter_at_dialogs)
     gtk_dialog_set_default_response (file_dialog, GTK_RESPONSE_ACCEPT);

  g_signal_connect (G_OBJECT (file_dialog), "key_press_event", G_CALLBACK (dlg_key_handler), file_dialog);

  GtkFileChooserWidget *fcw = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_SAVE);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (fcw), TRUE);

  gtk_widget_show (fcw);
  //gtk_box_pack_start (GTK_DIALOG (file_dialog)->vbox, fcw, TRUE, TRUE, UI_PACKFACTOR);
  gtk_box_pack_start (GTK_DIALOG (gtk_dialog_get_content_area (file_dialog)), fcw, TRUE, TRUE, UI_PACKFACTOR);

  
  GtkWidget *label;
  GtkWidget *vb = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vb);

  gint i = find_str_index (confile.gl_save_charsets, cur_text_doc->encoding);

  if (i == -1)
     i = 0;

  ent_dlg_filename = gtk_entry_new ();
  gtk_widget_show (ent_dlg_filename);

  g_signal_connect ((gpointer) ent_dlg_filename, "key_press_event",
                    G_CALLBACK (on_fnentr_key_press_event),
                    file_dialog);

  GtkComboBox *cb_charset = tea_combo_combo (vb, confile.gl_save_charsets, _("Encoding: "), i, NULL/*select_charset_filesave*/, NULL);

  gtk_file_chooser_set_extra_widget (fcw, vb);

  g_signal_connect ((gpointer) fcw, "file-activated",
                    G_CALLBACK (file_open_file2click),
                    file_dialog);

  gtk_window_resize (GTK_WINDOW (file_dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  if (mode == 0)
     {
      if (cur_text_doc)
         {
          if (confile.use_def_save_dir)
             gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), confile.def_save_dir);
          else

               if (g_file_test (cur_text_doc->file_name_local, G_FILE_TEST_EXISTS))
                  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fcw), cur_text_doc->file_name_local);
               else
                   if (last_dir)
                      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), last_dir);
                   else
                       gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), g_get_home_dir ());
         }
     }

  if (mode == 1)
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), confile.templates_dir);

  if (mode == 2)
     gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), confile.snippets_dir);

  if (mode == 3)
     gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fcw), confile.tables_dir);

  if (gtk_dialog_run (GTK_DIALOG (file_dialog)) == GTK_RESPONSE_ACCEPT)
     {
      gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fcw));

      if (confile.add_def_file_ext)
         if (confile.def_file_ext)
            if (! has_ext (filename))
               {
                gchar *new_filename = g_strconcat (filename, ".", confile.def_file_ext, NULL);
                filename = ch_str (filename, new_filename);
                g_free (new_filename);
               }

      gchar *utf8n = get_8_filename (filename);

      if (confile.prompt_on_file_exists && g_file_test (filename, G_FILE_TEST_EXISTS))
      if (! dlg_question (_("Question"), _("%s is exists. Do you want to overwrite it?"), utf8n))
         {
          g_free (utf8n);
          gtk_widget_destroy (file_dialog);
          return;
         }

      g_free (cur_text_doc->file_name_local);
      g_free (cur_text_doc->file_name_utf8);

      cur_text_doc->file_name_utf8 = utf8n;
      cur_text_doc->file_name_local = g_strdup (filename);

      cur_text_doc->encoding = ch_str (cur_text_doc->encoding, combo_get_value (cb_charset, confile.gl_save_charsets));

      if (! text_doc_save (cur_text_doc, filename))
         {
          log_to_memo (_("error by saving %s!!!"), cur_text_doc->file_name_utf8, LM_ERROR);
          gtk_widget_destroy (file_dialog);
          return;
         }

      document_do_hl (cur_text_doc);
      cur_text_doc->b_saved = TRUE;
      gchar *bsname = g_path_get_basename (cur_text_doc->file_name_utf8);
      gtk_label_set_label (cur_text_doc->tab_label, bsname);
      g_free (bsname);
      set_title (cur_text_doc);

      if (strstr (cur_text_doc->file_name_local, confile.snippets_dir))
        reload_snippets ();

     if (strstr (cur_text_doc->file_name_local, confile.templates_dir))
        reload_templates ();

     if (strstr (cur_text_doc->file_name_local, confile.tables_dir))
        reload_reptables ();

     tabs_reload ();
     g_free (filename);
    }

   gtk_widget_destroy (file_dialog);
}


void on_mni_paint (GtkMenuItem *menuitem,
                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter iter;

  if (gtk_text_buffer_get_selection_bounds (cur_text_doc->text_buffer, &start, &end))
     {
      gtk_text_buffer_apply_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &start, &end);
      start = end;
      gtk_text_buffer_select_range (cur_text_doc->text_buffer, &start, &end);
     }
  else
      {
       GtkTextMark *mark = gtk_text_buffer_get_insert (cur_text_doc->text_buffer);
       gtk_text_buffer_get_iter_at_mark (cur_text_doc->text_buffer, &iter, mark);
       if (gtk_text_iter_has_tag (&iter, get_tag_by_name (cur_text_doc, "paint")))
          {
           start = iter;
           end = iter;

           while (! gtk_text_iter_begins_tag (&start, get_tag_by_name (cur_text_doc, "paint")) && gtk_text_iter_backward_char (&start))
                 ;

           while (! gtk_text_iter_ends_tag (&end, get_tag_by_name (cur_text_doc, "paint")) && gtk_text_iter_forward_char (&end))
                 ;

           gtk_text_buffer_remove_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &start, &end);
          }
      }
}


void on_mni_copy_painted (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter iter;

  GList *l = NULL;

  gtk_text_buffer_get_bounds (cur_text_doc->text_buffer, &start, &end);

  iter = start;

  while (gtk_text_iter_forward_char (&iter))
        {
         if (gtk_text_iter_begins_tag (&iter, get_tag_by_name (cur_text_doc, "paint")))
            start = iter;

         if (gtk_text_iter_ends_tag (&iter, get_tag_by_name (cur_text_doc, "paint")))
            {
             end = iter;
             l = g_list_prepend (l, gtk_text_buffer_get_text (cur_text_doc->text_buffer, &start, &end, FALSE));
            }
        }

   gchar *t = string_from_glist (g_list_reverse (l));
   clipboard_put_text (t);
   g_free (t);

  glist_strings_free (l);
}


void on_mni_unpaint_all (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  GtkTextIter start;
  GtkTextIter end;

  gtk_text_buffer_get_bounds (cur_text_doc->text_buffer, &start, &end);
  gtk_text_buffer_remove_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &start, &end);
}


void on_mni_search_and_paint (GtkMenuItem* menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("find this text");
  doc_search_and_paint (cur_text_doc, gtk_entry_get_text (ent_search));
}


void on_mni_paint_lines_bigger_than (GtkMenuItem* menuitem,
                                     gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text ("72");

  if (! check_is_number (gtk_entry_get_text (ent_search)))
     return;

  gint size = strtol (gtk_entry_get_text (ent_search), NULL, 10);
  doc_paint_lines_bigger_than (cur_text_doc, size);
}


void on_mni_ext_programs_menu_item (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  gchar *t = g_hash_table_lookup (ht_ext_programs, gtk_widget_get_name (menuitem));
  if (! t)
     return;

  gchar *s = g_strdup (t);
  gchar *cmd = NULL;

  if (get_page_text())
     {
      cmd = str_replace_all (s, "%s", cur_text_doc->file_name_utf8);
      system (cmd);
      g_free (cmd);
     }

  g_free (s);
}


void on_mni_ext_programs_file_open (GtkMenuItem *menuitem,
                                    gpointer user_data)
{
  mni_last = menuitem;
  if (! g_file_test (confile.ext_programs, G_FILE_TEST_EXISTS))
     create_empty_file (confile.ext_programs, "Konqueror=konqueror %s &");

  open_file_std (confile.ext_programs, "UTF-8");
}


void on_mni_mark_each_n_line (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text ("2");

  if (! check_is_number (gtk_entry_get_text (ent_search)))
     return;

  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  if (! buf)
     return;

  gint n = strtol (gtk_entry_get_text (ent_search), NULL, 10);

  GList *l = glist_from_string (buf);

  gint c;
  gint len = g_list_length (l);

  for (c = -1; c <= len; c = c + n)
      hl_line_n (cur_text_doc, c);

  glist_strings_free (l);
  g_free (buf);
}


void on_mni_crashfile_open (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  open_file_std (confile.crash_file, "UTF-8");
}


void on_mni_mark_all_lines_with_string (GtkMenuItem *menuitem,
                                        gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("text to find");

  gchar *f = gtk_entry_get_text (ent_search);
  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  GList *l = glist_from_string (buf);
  GList *t = g_list_first (l);
  gint c = -1;

  while (t)
        {
         ++c;
         if (t->data)
            if (strstr (t->data, f))
               hl_line_n (cur_text_doc, c);

         t = g_list_next (t);
        }

  glist_strings_free (l);
  g_free (buf);
}


static void on_mni_suggestion_select (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *t = gtk_widget_get_name (menuitem);

  gtk_text_buffer_begin_user_action (temp_text_part->page->text_buffer);

  gtk_text_buffer_delete (temp_text_part->page->text_buffer,
                          &temp_text_part->itstart,
                          &temp_text_part->itend);

  gtk_text_buffer_insert (temp_text_part->page->text_buffer,
                          &temp_text_part->itstart,
                          t,
                          -1);

  gtk_text_buffer_end_user_action (temp_text_part->page->text_buffer);
}


//code from gtktextview.c Copyright (C) 2000 Red Hat, Inc
//modified by roxton
static void set_autocomp_menu_pos (GtkMenu *menu,
                                   gint *x,
                                   gint *y,
                                   gboolean *push_in,
                                   gpointer user_data)
{
  t_text_part *p = user_data;

  GtkWidget *widget;
  GdkRectangle cursor_rect;
  GdkRectangle onscreen_rect;
  gint root_x, root_y;
  GtkTextIter iter;
  GtkRequisition req;
  GdkScreen *screen;

  widget = GTK_WIDGET (p->page->text_view);
  screen = gtk_widget_get_screen (widget);

  gdk_window_get_origin (gtk_widget_get_window (widget), &root_x, &root_y);
  
  gtk_text_view_get_iter_location (p->page->text_view,
                                   &p->itstart,
                                   &cursor_rect);

  gtk_text_view_get_visible_rect (p->page->text_view, &onscreen_rect);

  gtk_widget_size_request (menu, &req);

  /* can't use rectangle_intersect since cursor rect can have 0 width */
  if (cursor_rect.x >= onscreen_rect.x &&
      cursor_rect.x < onscreen_rect.x + onscreen_rect.width &&
      cursor_rect.y >= onscreen_rect.y &&
      cursor_rect.y < onscreen_rect.y + onscreen_rect.height)
    {
      gtk_text_view_buffer_to_window_coords (p->page->text_view,
                                             GTK_TEXT_WINDOW_WIDGET,
                                             cursor_rect.x, cursor_rect.y,
                                             &cursor_rect.x, &cursor_rect.y);

      *x = root_x + cursor_rect.x + cursor_rect.width;
      *y = root_y + cursor_rect.y + cursor_rect.height;
    }
  else
    {
      /* Just center the menu, since cursor is offscreen. */
      *x = root_x + (gtk_widget_get_allocated_width (widget) / 2 - req.width / 2);
      *y = root_y + (gtk_widget_get_allocated_height (widget) / 2 - req.height / 2);
    }

  /* Ensure sanity */
  *x = CLAMP (*x, root_x, (root_x + gtk_widget_get_allocated_width (widget)));
  *y = CLAMP (*y, root_y, (root_y + gtk_widget_get_allocated_height (widget)));

  *x = CLAMP (*x, 0, MAX (0, gdk_screen_get_width (screen) - req.width));
  *y = CLAMP (*y, 0, MAX (0, gdk_screen_get_height (screen) - req.height));
}
//ending of code from gtktextview.c


static void make_suggestions_menu (t_text_part *p)
{

  if (mni_suggestions)
     gtk_widget_destroy (mni_suggestions);

  mni_suggestions = gtk_menu_new ();

#if defined (HAVE_LIBASPELL) || defined (ENCHANT_SUPPORTED)
  GList *l = get_suggestions_list (p->prefix);

  if (l)
     {
      build_menu_wudata_from_glist (l, mni_suggestions, on_mni_suggestion_select, temp_text_part);
      gtk_menu_popup (mni_suggestions, NULL, NULL, set_autocomp_menu_pos, p, 0, 0);
      gtk_widget_grab_focus (mni_temp);
      gtk_menu_item_select (mni_temp);
     }

  glist_strings_free (l);

#endif
}


void on_mni_suggest (GtkMenuItem *menuitem,
                     gpointer user_data)
{
  mni_last = menuitem;

  if (! get_page_text())
     return;

  GtkTextIter itstart;
  GtkTextIter itend;

  gchar *t = doc_get_cur_mispelled (cur_text_doc, &itstart, &itend);

  if (! t)
     return;

  if (temp_text_part)
     {
      g_free (temp_text_part->prefix);
      g_free (temp_text_part);
     }

  temp_text_part = g_malloc (sizeof (t_text_part));
  temp_text_part->page = cur_text_doc;
  gtk_text_iter_forward_char (&itstart);
  temp_text_part->itstart = itstart;
  temp_text_part->itend = itend;
  temp_text_part->prefix = g_strdup (t);
  make_suggestions_menu (temp_text_part);
  g_free (t);
}


void on_bt_close_current_doc (GtkButton *button,
                              gpointer user_data)
{
  on_mni_file_close_current (NULL, NULL);
}


void on_mni_hide_error_marks (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  hide_error_marks (cur_text_doc);
}


void on_mni_spellcheck_def (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  do_hl_spell_check (cur_text_doc, confile.def_spell_lang);
}


void on_mni_split_by (GtkMenuItem *menuitem,
                      gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  set_fam_text (";");
  
  gchar *t = string_split_by (buf, gtk_entry_get_text (ent_search));

  doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_free (buf);
}


void on_mni_split_after_delimiter (GtkMenuItem *menuitem,
                                   gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  set_fam_text (";");

  gchar *y = g_strconcat (gtk_entry_get_text (ent_search), "\n", NULL);
  doc_replace_from_cursor_all (cur_text_doc, gtk_entry_get_text (ent_search), y);
  g_free (y);
}


void on_mni_replace_in_place (GtkMenuItem *menuitem,
                              gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! doc_has_selection (cur_text_doc))
     return;

  set_fam_text ("zzzzz");

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     return;

  doc_rep_sel (cur_text_doc, gtk_entry_get_text (ent_search));

  g_free (buf);
}


void on_mni_repeat_the_last (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  if (mni_last)
     gtk_menu_item_activate (mni_last);
}


void on_mni_mark_on_regexp (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  if (! get_page_text())
     return;

  mpb_start();
  
  while (doc_move_to_next_word_sel_if_regexp (cur_text_doc, gtk_entry_get_text (ent_search)))
       gtk_progress_bar_pulse (pb_status);

  mpb_stop ();
}

//n.p. The Pixies - Vamos
gpointer find_in_files_thread_f (gpointer p)
{
  create_findfiles_mult ();
  return NULL;
}


void on_mni_find_in_files (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  create_findfiles_mult();
}


void on_mni_mark_lines_with_fuzzy_nontrans (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  if (! buf)
     return;

  GList *l = glist_from_string (buf);
  GList *t = g_list_first (l);
  gint c = -1;
  gint empty = 0;
  gint fuzzy = 0;
  gint total = 0;

  while (t)
        {
         ++c;
         if (t->data)
            {

             if (strstr (t->data, "msgstr"))
                total++;

             if (strstr (t->data, "msgstr \"\""))
               {
                hl_line_n (cur_text_doc, c);
                empty++;
               }
             else
                 if (strstr(t->data, "#, fuzzy"))
                    {
                     hl_line_n (cur_text_doc, c);
                     fuzzy++;
                    }
            }

         t = g_list_next (t);
        }

  gchar *msg = g_strdup_printf (_("for %s:\nfuzzy = %d\nempty = %d\nfuzzy + empty = %d\nnot translated: %.3f%%"),
                                cur_text_doc->file_name_utf8, fuzzy, empty, fuzzy + empty, get_percent (total, (fuzzy + empty)));



  log_to_memo (msg, NULL, LM_NORMAL);

  glist_strings_free (l);
  g_free (buf);
  g_free (msg);
}


void on_mni_lorem_ipsum (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  if (! get_page_text ())
     return;

  gchar *lorem = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis tellus. \
Donec ante dolor, iaculis nec, gravida ac, cursus in, eros. Mauris vestibulum, felis et \
egestas ullamcorper, purus nibh vehicula sem, eu egestas ante nisl non justo. Fusce tincidunt, \
lorem nec dapibus consectetuer, leo orci mollis ipsum, eget suscipit eros purus in ante.\n\
At ipsum vitae est lacinia tincidunt. Maecenas elit orci, gravida ut, molestie non, venenatis vel,\
 lorem. Sed lacinia. Suspendisse potenti. Sed ultricies cursus lectus. In id magna sit amet nibh \
suscipit euismod. Integer enim. Donec sapien ante, accumsan ut, sodales commodo, auctor quis, \
lacus. Maecenas a elit lacinia urna posuere sodales. Curabitur pede pede, molestie id, blandit \
vitae, varius ac, purus. Mauris at ipsum vitae est lacinia tincidunt. Maecenas elit orci, gravida \
ut, molestie non, venenatis vel, lorem. Sed lacinia. Suspendisse potenti. Sed ultricies cursus \
lectus. In id magna sit amet nibh suscipit euismod. Integer enim. Donec sapien ante, accumsan ut\
, sodales commodo, auctor quis, lacus. Maecenas a elit lacinia urna posuere sodales. Curabitur pede \
pede, molestie id, blandit vitae, varius ac, purus.\n\
Morbi dictum. Vestibulum adipiscing pulvinar quam. In aliquam rhoncus sem. In mi erat, sodales eget,\
 pretium interdum, malesuada ac, augue. Aliquam sollicitudin, massa ut vestibulum posuere, massa \
arcu elementum purus, eget vehicula lorem metus vel libero. Sed in dui id lectus commodo elementum. \
Etiam rhoncus tortor. Proin a lorem. Ut nec velit. Quisque varius. Proin nonummy justo dictum sapien \
tincidunt iaculis. Duis lobortis pellentesque risus. Aenean ut tortor imperdiet dolor scelerisque \
bibendum. Fusce metus nibh, adipiscing id, ullamcorper at, consequat a, nulla.\n\
Phasellus orci. Etiam tempor elit auctor magna. Nullam nibh velit, vestibulum ut, eleifend non, \
pulvinar eget, enim. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos \
hymenaeos. Integer velit mauris, convallis a, congue sed, placerat id, odio. Etiam venenatis tortor \
sed lectus. Nulla non orci. In egestas porttitor quam. Duis nec diam eget nibh mattis tempus. \
Curabitur accumsan pede id odio. Nunc vitae libero. Aenean condimentum diam et turpis. Vestibulum non risus. \
Ut consectetuer gravida elit. Aenean est nunc, varius sed, aliquam eu, feugiat sit amet, metus. Sed \
venenatis odio id eros.\n\
Phasellus placerat purus vel mi. In hac habitasse platea dictumst. Donec aliquam porta odio. \
Ut facilisis. Donec ornare ipsum ut massa. In tellus tellus, imperdiet ac, accumsan at, \
aliquam vitae, velit.";

  mni_last = menuitem;
  doc_insert_at_cursor (cur_text_doc, lorem);
}


void on_mni_rtfm_en (GtkMenuItem *menuitem,
                    gpointer user_data)
{
  mni_last = menuitem;
  gchar *f = g_strconcat (TEA_DOC_DIR, G_DIR_SEPARATOR_S, "en", G_DIR_SEPARATOR_S, "index.html", NULL);
  
  gchar *t = compose_browser_cmd (f);
  if (t)
     system (t);

  g_free (f);
  g_free (t);
}


void on_mni_extract_at_each_line (GtkMenuItem *menuitem,
                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("\"~\"");
  gchar *s = gtk_entry_get_text (ent_search);

  if (! strstr (s, "~"))
     return;

  gchar **a = g_strsplit (s, "~", -1);

  if (get_gchar_array_size (a) != 2)
     {
      if (a)
         g_strfreev (a);
      return;
     }
     
  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
     {
      g_strfreev (a);
      return;
     }

  gchar *t = extract_each_line_bewteen (buf, a[0], a[1]);

  doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_strfreev (a);
}


void on_mni_refresh_ctags (GtkMenuItem *menuitem,
                           gpointer user_data)
{
  if (! get_page_text())
     return;

  mni_last = menuitem;
  gchar *working_directory = g_path_get_dirname (cur_text_doc->file_name_local);
  chdir (working_directory);
  system ("ctags *");
  g_free (working_directory);
}


void on_mni_browse_symbol_decl (GtkMenuItem *menuitem,
                                gpointer user_data)
{
  if (! get_page_text())
     return;

  GtkTextIter itstart;
  GtkTextIter itend;
  gchar *sym = doc_get_current_word (cur_text_doc, &itstart, &itend);

  gchar *tags_file = NULL;

  mni_last = menuitem;
  gchar *dir_name = g_path_get_dirname (cur_text_doc->file_name_local);

  tags_file = g_strconcat (dir_name, G_DIR_SEPARATOR_S, "tags", NULL);
  if (! file_exists (tags_file))
     on_mni_refresh_ctags (NULL, NULL);

  t_str_quad *t = NULL;
  GList *tags = load_file_to_glist (tags_file);
  GList *l = g_list_first (tags);

  while (l)
        {
         t = parse_ctags_line (l->data);

         if (t && (strcmp (t->a, sym) == 0))
            if (strcmp (t->d, "f") == 0)
               break;

         str_quad_free (t);
         t = NULL;
         l = g_list_next (l);
        }

  gchar *filename = NULL;

  if (t)
     {
      filename = g_strconcat (dir_name, G_DIR_SEPARATOR_S, t->b, NULL);
      handle_file (filename, cur_text_doc->encoding, 0, FALSE);
      if (! get_page_text()) return;
      editor_set_pos (cur_text_doc, 0);
      doc_search_f_silent (cur_text_doc, t->c);
      gtk_text_view_scroll_to_mark (cur_text_doc->text_view, gtk_text_buffer_get_insert
                                   (cur_text_doc->text_buffer), 0.0, TRUE, 0.0, 0.5);
     }

  str_quad_free (t);
  g_free (filename);
  g_free (dir_name);
  g_free (tags_file);
}


void on_mni_mass_encode (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  GtkWidget *w = create_wnd_encode();
}


void on_mni_self_check (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  gchar *s = g_find_program_in_path ("antiword");

  if (s)
      log_to_memo (_("antiword - ok //it helps TEA to open MS Word documents"), NULL, LM_NORMAL);
   else
       log_to_memo (_("antiword - not found //TEA cannot open MS Word documents without it"), NULL, LM_ERROR);

   g_free (s);

   s = g_find_program_in_path ("gunzip");
   if (s)
     log_to_memo (_("gunzip - ok //it lets TEA to read ZIP and GZ files"), NULL, LM_NORMAL);
   else
       log_to_memo (_("gunzip - not found //TEA cannot open read ZIP and GZ files without it"), NULL, LM_ERROR);

   g_free (s);
}


void on_mni_extract_at_each_line_aft_sep (GtkMenuItem *menuitem,
                                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
      return;

  set_fam_text (",");

  gchar *t = str_each_line_after_sep (buf, gtk_entry_get_text (ent_search));
  doc_rep_sel (cur_text_doc, t);

  g_free (t);
}


void on_mni_extract_at_each_line_bef_sep (GtkMenuItem *menuitem,
                                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text ())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf)
      return;

  set_fam_text (",");
  
  gchar *t = str_each_line_before_sep (buf, gtk_entry_get_text (ent_search));
  doc_rep_sel (cur_text_doc, t);

  g_free (t);
}


void on_mni_quotes_to_facing_quotes (GtkMenuItem *menuitem,
                                     gpointer user_data)
{
  process_string (STRP_STR_FACEQUOTES, menuitem);
}


void on_mni_script_click (GtkMenuItem *menuitem,
                          gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *intrp = NULL;

  if (check_ext (gtk_widget_get_name (menuitem), ".rb"))
     intrp = g_strdup ("ruby ");
  else
  if (check_ext (gtk_widget_get_name (menuitem), ".py"))
     intrp = g_strdup ("python ");
  else
  if (check_ext (gtk_widget_get_name (menuitem), ".pl"))
     intrp = g_strdup ("perl ");
  else
  if (check_ext (gtk_widget_get_name (menuitem), ".sh"))
     intrp = g_strdup ("sh ");

  if (! intrp)
      return;

  gchar *sel = doc_get_sel (cur_text_doc);
  if (! sel)
      return;

  put_tmp_file_string (sel);
  gchar *file_param = g_strconcat (g_get_tmp_dir (), G_DIR_SEPARATOR_S, "tea_params_file", NULL);
  save_string_to_file (file_param, gtk_entry_get_text (ent_search));


  gchar *f = g_strconcat (intrp, confile.scripts_dir, gtk_widget_get_name (menuitem), " ", confile.tmp_file, " ", file_param, NULL);
  system (f);
  gchar *x = get_tmp_file_string();

  doc_rep_sel (cur_text_doc, x);

  if (x)
     g_free (x);

  unlink (file_param);
  g_free (file_param);

  g_free (intrp);
  g_free (f);
  g_free (sel);
}


void on_mni_calc (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *sel = doc_get_sel (cur_text_doc);
  if (! sel)
      return;

  gfloat f = math_string_eval (sel, 0);
  gchar *s =  g_strdup_printf ("%f", f);
  log_to_memo (s, NULL, LM_NORMAL);
  g_free (s);
  g_free (sel);
}


void on_mni_load_autosaved_session (GtkMenuItem *menuitem,
                                    gpointer user_data)
{
  session_open_from_file (confile.autosave_session);
}


static gboolean is_link_exists (const gchar *fname, const gchar *linkname)
{
  if (! fname || ! linkname)
     return FALSE;

  gboolean result = FALSE;
  gchar *dir = g_path_get_dirname (fname);
  gchar *filename = create_full_path (linkname, dir);

  if (! g_file_test (filename, G_FILE_TEST_EXISTS))
     result = TRUE;

  g_free (dir);
  g_free (filename);
  
  return result;
}


void scan_missing_local_links (void)
{
  if (! get_page_text())
     return;

  on_mni_unpaint_all (NULL, NULL);

  gchar *f, *t;
  GtkTextIter match_start;
  GtkTextIter match_end;

  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "ref=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            if (! is_prefixed (f, "http://", "ftp://", "mailto:", "#", NULL))
                if (is_link_exists (cur_text_doc->file_name_utf8, f))
                   gtk_text_buffer_apply_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &match_start, &match_end);

            g_free (f);
           }
        match_start = match_end;
       }

  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "REF=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            if (! is_prefixed (f, "http://", "ftp://", "mailto:", "#", NULL))
               if (is_link_exists (cur_text_doc->file_name_utf8, f))
                  gtk_text_buffer_apply_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &match_start, &match_end);

            g_free (f);
           }
        match_start = match_end;
       }


  gtk_text_buffer_get_start_iter (cur_text_doc->text_buffer, &match_start);
  gtk_text_buffer_get_end_iter (cur_text_doc->text_buffer, &match_end);

  while (gtk_text_iter_forward_search (&match_start, "src=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            if (! is_prefixed (f, "http://", "ftp://", "mailto:", "#", NULL))
               if (is_link_exists (cur_text_doc->file_name_utf8, f))
                  gtk_text_buffer_apply_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &match_start, &match_end);

            g_free (f);
           }
        match_start = match_end;
       }


  while (gtk_text_iter_forward_search (&match_start, "SRC=\"", GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
       {
        match_start = match_end;
        if (gtk_text_iter_forward_find_char (&match_end, find_quote, NULL, NULL))
           {
            f = gtk_text_buffer_get_text (cur_text_doc->text_buffer, &match_start, &match_end, FALSE);
            if (! is_prefixed (f, "http://", "ftp://", "mailto:", "smb://", "sftp://", "#", NULL))
               if (is_link_exists (cur_text_doc->file_name_utf8, f))
                  gtk_text_buffer_apply_tag (cur_text_doc->text_buffer, get_tag_by_name (cur_text_doc, "paint"), &match_start, &match_end);

            g_free (f);
           }
        match_start = match_end;
       }
}


void on_mni_reptables_click (GtkMenuItem *menuitem,
                             gpointer user_data)
{
  if (! get_page_text())
     return;

  gchar *sel = doc_get_sel (cur_text_doc);
  if (! sel)
      return;

  mni_last = menuitem;
  gchar *f = g_strconcat (confile.tables_dir, gtk_widget_get_name (menuitem), NULL);

  gchar *t = apply_repl_table (sel, f);
  doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_free (f);
  g_free (sel);
}


void file_save_as_reptable (void)
{
  if (get_page_text())
     tea_file_save_dialog (3);
}

void on_mni_test2 (GtkMenuItem *menuitem,
                  gpointer user_data)
{
//  if (! get_page_text()) return;
  //doc_unlock_file (cur_text_doc);

}


void on_mni_dec2hex (GtkMenuItem *menuitem, gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_sel (cur_text_doc);
  if (! buf) 
     return;

  gint i = strtol (buf, NULL, 10);
  gchar *t = g_strdup_printf ("%x", i);

  if (t)
     doc_rep_sel (cur_text_doc, t);

  g_free (t);
  g_free (buf);
}


#ifdef PRINTING_SUPPORTED

void on_mni_print (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  mni_last = menuitem;

  if (! get_page_text ())
     return;

  GtkWidget *w = gtk_print_unix_dialog_new (_("Print"),  tea_main_window);
  gtk_print_unix_dialog_set_manual_capabilities (w, GTK_PRINT_CAPABILITY_PAGE_SET |
                                                 GTK_PRINT_CAPABILITY_COPIES |
                                                 GTK_PRINT_CAPABILITY_COLLATE |
                                                 GTK_PRINT_CAPABILITY_REVERSE |
                                                 GTK_PRINT_CAPABILITY_SCALE |
                                                 GTK_PRINT_CAPABILITY_GENERATE_PDF |
                                                 GTK_PRINT_CAPABILITY_GENERATE_PS
                                                 );

  if (gtk_dialog_run (GTK_DIALOG (w)) == GTK_RESPONSE_OK)
     {
      GtkPrintJob *job = gtk_print_job_new (cur_text_doc->file_name_utf8, gtk_print_unix_dialog_get_selected_printer (w),
                                            gtk_print_unix_dialog_get_settings (w),
                                            gtk_print_unix_dialog_get_page_setup (w));

      gtk_print_job_set_source_file   (job, cur_text_doc->file_name_local, NULL);
      gtk_print_job_send (job,
                          NULL,//           GtkPrintJobCompleteFunc callback,
                          NULL,            //gpointer user_data,
                          NULL);     //       GDestroyNotify dnotify);
     }

  gtk_widget_destroy (w);
}

#endif


void load_profile (const gchar *filename)
{
  if (! filename)
     return;

  GList *ui = NULL;
  ui = load_file_to_glist (filename);
  if (! ui)
     return;

  gtk_paned_set_position (vpaned1, conf_get_int_value (ui, "vpaned1_y", 300));

  gtk_window_move (GTK_WINDOW (tea_main_window),
                   conf_get_int_value (ui, "main_wnd_x", 2),
                   conf_get_int_value (ui, "main_wnd_y", 2));

  gtk_window_resize (GTK_WINDOW (tea_main_window),
                     conf_get_int_value (ui, "main_wnd_w", 630),
                     conf_get_int_value (ui, "main_wnd_h", 450));

  glist_strings_free (ui);
}


void save_profile (const gchar *filename)
{
  if (! filename)
     return;

  GList *list = NULL;

  if (vpaned1)
     list = add_to_glist_combined_int (list, "vpaned1_y", gtk_paned_get_position (vpaned1));

  gint x = 1;
  gint y = 1;

  if (tea_main_window)
     gtk_window_get_position (GTK_WINDOW (tea_main_window), &x, &y);

  list = add_to_glist_combined_int (list, "main_wnd_x", x);
  list = add_to_glist_combined_int (list, "main_wnd_y", y);

  if (tea_main_window)
     gtk_window_get_size (GTK_WINDOW (tea_main_window), &x, &y);

  list = add_to_glist_combined_int (list, "main_wnd_w", x);
  list = add_to_glist_combined_int (list, "main_wnd_h", y);

  glist_save_to_file (list, filename);
  glist_strings_free (list);
}


void on_mni_profiles_click (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  mni_last = menuitem;
  gchar *f = g_strconcat (confile.profiles_dir, gtk_widget_get_name (menuitem), NULL);
  load_profile (f);
  g_free (f);
}


void on_mni_file_save_profile (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  gchar *filename;
  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Save the profile as:"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), confile.profiles_dir);
  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
     filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
     save_profile (filename);
     g_free (filename);
     reload_profiles ();
    }

  gtk_widget_destroy (dialog);
}


void on_mni_utils_readability (GtkMenuItem *menuitem,
                               gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;
  
  make_readability (cur_text_doc);
}


void on_mni_func_filter_regexp_exclude_from_list (GtkMenuItem *menuitem,
                                                  gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("*.txt");
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = filter_list_pattern (temp, gtk_entry_get_text (ent_search), FALSE);

  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_func_filter_regexp_incl_to_list (GtkMenuItem *menuitem,
                                             gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  set_fam_text ("*.txt");
  gchar *buf = doc_get_sel (cur_text_doc);

  if (! buf)
     return;

  GList *temp = glist_from_string (buf);
  GList *temp2 = filter_list_pattern (temp, gtk_entry_get_text (ent_search), TRUE);

  gchar *t = string_from_glist (temp2);

  doc_rep_sel (cur_text_doc, t);

  g_free (buf);
  g_free (t);
  glist_strings_free (temp);
  glist_strings_free (temp2);
}


void on_mni_eol_to_cr (GtkMenuItem *menuitem,
                       gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  gchar *buf = doc_get_buf (cur_text_doc->text_buffer);
  if (! buf)
     return;

  gchar *z = str_eol_to_cr (buf);

  if (z)
     {
      gtk_text_buffer_set_text (cur_text_doc->text_buffer, z, -1);
      cur_text_doc->end_of_line = ch_str (cur_text_doc->end_of_line, "CR");
      doc_update_statusbar (cur_text_doc);
      g_free (z);
     }

  g_free (buf);
}


void on_mni_grep (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  gchar *dir = NULL;

  if (get_page_text())
     {
      dir = g_path_get_dirname (cur_text_doc->file_name_local);
      if (dir)
          chdir (dir);
     }
  else
      dir = g_get_current_dir();

  lm_dclick_mode = DCM_GREP;
  set_fam_text ("hello");

  put_tmp_file_string ("grep:\n");
  gchar *cmline = g_strconcat ("grep -n -H -r ", gtk_entry_get_text (ent_search), " ", dir, G_DIR_SEPARATOR_S, "* >> ", confile.tmp_file, NULL);
  system (cmline);

  gchar *s = get_tmp_file_string ();
  if (s)
     {
      gchar *x = locale_to_utf8 (g_strstrip (s));
      log_to_memo ("%s", x, LM_NORMAL);
      g_free (s);
      g_free (x);
     }

  g_free (dir);
  g_free (cmline);
}


void on_mni_save_packed (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;
  if (! get_page_text())
     return;

  if (! file_exists (cur_text_doc->file_name_local))
     {
      log_to_memo (_("Save your document as a normal text file first, then save it packed"), NULL, LM_ERROR);
      return;
     }


  gchar *cmd = str_replace_all (confile.cm_pack, "%s", cur_text_doc->file_name_utf8);
  system (cmd);

  log_to_memo (_("A file is packed OK"), NULL, LM_NORMAL);
}


void on_mni_toggle_topmost (GtkMenuItem *menuitem,
                            gpointer user_data)
{
  b_topmost = ! b_topmost;
  gtk_window_set_keep_above (tea_main_window, b_topmost);
}


void on_mni_add_to_dict (GtkMenuItem *menuitem,
                         gpointer user_data)
{
  mni_last = menuitem;

  if (! get_page_text())
     return;

  GtkTextIter itstart;
  GtkTextIter itend;

#ifdef HAVE_LIBASPELL

  gchar *t = doc_get_cur_mispelled (cur_text_doc, &itstart, &itend);

  if (! t)
     return;

  dict_add_to_user_dict (g_strstrip (t));

  g_free (t);

#endif

#ifdef ENCHANT_SUPPORTED

  gchar *t = doc_get_cur_mispelled (cur_text_doc, &itstart, &itend);

  if (! t)
     return;

  dict_add_to_user_dict (g_strstrip (t));

  g_free (t);


#endif

}


void on_mni_test (GtkMenuItem *menuitem,
                  gpointer user_data)
{
  dbm (get_lang_name());


}



