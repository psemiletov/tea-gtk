/***************************************************************************
                          tea_config.c  -  description
                             -------------------
    begin                : Fir Dec 19 2003
    copyright            : (C) 2003-2007 by Peter 'Roxton' Semiletov
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <stdlib.h> // needed for strtol
#include <limits.h> // needed for strtol
#include <errno.h>
#include <sys/stat.h>
#include <string.h> // needed for strdup

#include <glib.h>
#include <glib/gi18n.h>


#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "callbacks.h"
#include "interface.h"
#include "tea_defs.h"
#include "tea_config.h"
#include "tea_spell.h"
#include "tea_proj.h"
#include "tea_gtk_utils.h"
#include "tea_calendar.h"
#include "tea_funx.h" // Needed for read_dir_files
#include "rox_strings.h"  // Needed for glist_strings_free
#include "tea_enc.h" // Needed for create_enc_list



#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>


static GHashTable *tea_options = NULL;

GList* create_supported_markup_modes (void)
{
  GList *l = NULL;
  l = g_list_prepend (l, g_strdup ("Wikipedia"));
  l = g_list_prepend (l, g_strdup ("Docbook"));
  l = g_list_prepend (l, g_strdup ("LaTeX"));
  l = g_list_prepend (l, g_strdup ("XHTML"));
  l = g_list_prepend (l, g_strdup ("HTML"));

  return l;
}


GList* create_tea_special_places (void)
{
  GList *l = NULL;

  l = g_list_prepend (l, g_strdup (_("Templates")));
  l = g_list_prepend (l, g_strdup (_("Tables")));
  l = g_list_prepend (l, g_strdup (_("Snippets")));
  l = g_list_prepend (l, g_strdup (_("Sessions")));
  l = g_list_prepend (l, g_strdup (_("Scripts")));

  return l;
}


void ui_init (void)
{
  b_topmost = FALSE;
  gl_tea_special_places = create_tea_special_places ();
  gl_markup_modes = create_supported_markup_modes ();
  gl_image_formats = image_formats_fill ();
  gl_color_schemes = NULL;

  gl_color_schemes = get_color_scheme_ids ();


  lm_dclick_mode = DCM_NONE;

  cur_tea_project = NULL;
  GList *ui = NULL;
  gl_tea_kwas_bookmarks = NULL;
  mni_suggestions = NULL;
  temp_text_part = NULL;

  ui = load_file_to_glist (confile.tea_ui);
  gl_tea_kwas_bookmarks = load_file_to_glist (confile.tea_kwas_bookmarks);


  gchar *localeid = get_lang_name ();
  confile.def_spell_lang = conf_get_char_value (ui, "def_spell_lang", localeid);
  g_free (localeid);

  confile.last_version = conf_get_char_value (ui, "last_version", "xxxxxx");
  confile.last_prefs_tab = conf_get_int_value (ui, "last_prefs_tab", 0);
  confile.filter_index = conf_get_int_value (ui, "filter_index", 0);

  gint main_wnd_w = conf_get_int_value (ui, "main_wnd_w", 630);
  gint main_wnd_h = conf_get_int_value (ui, "main_wnd_h", 450);

  gtk_paned_set_position (vpaned1, conf_get_int_value (ui, "vpaned1_y", get_value (main_wnd_h, 70)));

  gtk_window_move (GTK_WINDOW (tea_main_window),
                   conf_get_int_value (ui, "main_wnd_x", 2),
                   conf_get_int_value (ui, "main_wnd_y", 2));

  gtk_window_resize (GTK_WINDOW (tea_main_window), main_wnd_w, main_wnd_h);

  gtk_widget_set_size_request (pb_status, get_value (main_wnd_w, 60), -1);

  confile.default_markup_mode = conf_get_char_value (ui, "default_markup_mode", "HTML");
  def_mm = get_markup_mode ();

  gtk_notebook_set_tab_pos (notebook1, confile.notebook_tab_position);

  if (confile.fullscreen)
     gtk_window_fullscreen (tea_main_window);

  g_timeout_add ((60 * 1000) * confile.autosave_interval, save_all_cb, NULL);
  glist_strings_free (ui);



  source_languages_manager = gtk_source_language_manager_get_default ();
  gl_supported_hlangs = NULL;


//  GtkEntryCompletion *cmpl = gtk_entry_get_completion (ent_search);
//  model_fill_from_file (gtk_entry_completion_get_model (cmpl), confile.fam_autocomp_file);

  ui_update ();
}

//current music: Scorn - Silver Rain Fell (Meat Beat Manifesto Mix)
void ui_done (void)
{
  if (ui_done_flag)
     return;

  dbm ("ui_done");

  ui_done_flag = TRUE;

  if (confile.do_last_session)
     session_save_to_file (confile.last_session);

  GList *list = NULL;

  if (vpaned1)
     list = add_to_glist_combined_int (list, "vpaned1_y", gtk_paned_get_position (vpaned1));

  gint x = 1;
  gint y = 1;

  if (tea_main_window)
     gtk_window_get_position (GTK_WINDOW (tea_main_window), &x, &y);

  list = add_to_glist_combined_int (list, "filter_index", confile.filter_index);
  list = add_to_glist_combined_int (list, "last_prefs_tab", confile.last_prefs_tab);
  list = add_to_glist_combined_int (list, "main_wnd_x", x);
  list = add_to_glist_combined_int (list, "main_wnd_y", y);
  list = add_to_glist_combined_str (list, "def_spell_lang", confile.def_spell_lang);
  list = add_to_glist_combined_str (list, "default_markup_mode", confile.default_markup_mode);
  list = add_to_glist_combined_str (list, "last_version", VERSION);

  g_free (confile.default_markup_mode);

  if (tea_main_window)
     gtk_window_get_size (GTK_WINDOW (tea_main_window), &x, &y);

  list = add_to_glist_combined_int (list, "main_wnd_w", x);
  list = add_to_glist_combined_int (list, "main_wnd_h", y);

  glist_save_to_file (list, confile.tea_ui);
  glist_strings_free (gl_autosave);

//  GtkEntryCompletion *cmpl = gtk_entry_get_completion (ent_search);
 // model_save_to_file (gtk_entry_completion_get_model (cmpl), confile.fam_autocomp_file);

  glist_strings_free (gl_tea_special_places);

  g_free (confile.def_spell_lang);

  g_list_free (gl_enc_available);
  g_list_free (confile.gl_save_charsets);
  glist_strings_free (list);
  glist_strings_free (gl_enc_choosed);
  glist_strings_free (gl_tea_kwas_bookmarks);
  glist_strings_free (gl_markup_modes);
  glist_strings_free (gl_color_schemes);

  if (temp_text_part)
     {
      g_free (temp_text_part->prefix);
      g_free (temp_text_part);
     }

  tea_proj_free (cur_tea_project);
}


gchar* conf_get_char_value (GList *confdata, const gchar *key, const gchar *def)
{
  if (! def)
     return NULL;

  if (! confdata)
     return g_strdup (def);

  gchar *result = NULL;
  gchar **a = NULL;

  GList *p = g_list_first (confdata);

  while (p)
        {
         a = g_strsplit (p->data, "=", 2);

         if (a)
           {
            if (a[0])
               if ((g_utf8_collate (a[0], key) == 0))
                  {
                   if (a[1])
                      {
                       if (g_utf8_collate (a[1], "") == 0)
                          result = g_strdup (def);
                       else
                           result = g_strdup (a[1]);
                      }
                   g_strfreev (a);
                   return result;
                  }

            g_strfreev (a);
           }

         p = g_list_next (p);
        }

  return g_strdup (def);
}


gint conf_get_int_value (GList *confdata, const gchar *key, gint def)
{
  if (! confdata)
     return def;

  gint result = def;

  gchar *s = g_strdup_printf ("%d", def);
  gchar *res = conf_get_char_value (confdata, key, s);

  if (! res)
     {
      g_free (s);
      return def;
     }

  result = strtol (res, NULL, 10);

  g_free (res);
  g_free (s);

  return result;
}

//current music: Guano Apes - You Can't Stop Me
void confile_free (void)
{
  g_free (confile.fallback_charset);
  g_free (confile.current_movie);
  g_free (confile.ext_pic_viewer);
  g_free (confile.ext_pic_editor);
  g_free (confile.dir_thumbs);
  g_free (confile.def_file_ext);
  g_free (confile.cm_pdflatex);
  g_free (confile.cm_dvi_to_ps);
  g_free (confile.cm_dvi_view);
  g_free (confile.cm_pdf_view);
  g_free (confile.cm_ps_view);
  g_free (confile.cm_run_latex);
  g_free (confile.cm_dvi_to_pdf);
  g_free (confile.tea_choosed_charsets);
  g_free (confile.def_filesave_charset);
  g_free (confile.tea_kwas_bookmarks);
  g_free (confile.ext_programs);
  g_free (confile.crapbook_file);
  g_free (confile.snippets_dir);
  g_free (confile.profiles_dir);
  g_free (confile.tables_dir);
  g_free (confile.bmx_file);
  g_free (confile.date_time);
  g_free (confile.date_time_ver);

  g_free (confile.tea_main_dir);
  g_free (confile.templates_dir);
  g_free (confile.tmp_dir);
  g_free (confile.tmp_file);
  g_free (confile.now_playing);
  g_free (confile.sessions_dir);
  g_free (confile.tea_rc);
  g_free (confile.tea_ui);
  g_free (confile.autosave_file);
  g_free (confile.crash_file);
  g_free (confile.scripts_dir);
  g_free (confile.tea_recent);
  g_free (confile.tea_hotkeys);
//  g_free (confile.tea_hotkeys_old);

//  g_free (confile.fam_autocomp_file);
  g_free (confile.def_save_dir);
  g_free (confile.def_open_dir);
  g_free (confile.cmd_def_doc_browser);
  g_free (confile.color_cursor);
  g_free (confile.fmt_color_function);
  g_free (confile.last_session);
  g_free (confile.autosave_session);
  g_free (confile.cm_pack);

  g_free (confile.text_foreground);
  g_free (confile.text_background);
  g_free (confile.text_sel_foreground);
  g_free (confile.text_sel_background);
  g_free (confile.tag_color_lm_error_fg);
  g_free (confile.tag_color_lm_error_bg);
  g_free (confile.tag_current_line_fg);
  g_free (confile.tag_current_line_bg);
  g_free (confile.tag_comment);
  g_free (confile.tag_identifier);
  g_free (confile.tag_digit);
  g_free (confile.tag_string);
  g_free (confile.tag_html_tag);
  g_free (confile.tag_preprocessor);
  g_free (confile.tag_type);

  g_free (confile.tag_spellcheck);
  g_free (confile.tag_comment_font);
  g_free (confile.tag_identifier_font);
  g_free (confile.tag_digit_font);
  g_free (confile.tag_string_font);
  g_free (confile.tag_html_tag_font);
  g_free (confile.tag_preprocessor_font);
  g_free (confile.tag_type_font);
  g_free (confile.tag_color_paint_fg);
  g_free (confile.tag_color_paint_bg);
  g_free (confile.editor_font);
  g_free (confile.rtf_enc);
  g_free (confile.cm_mplayer);
  g_free (confile.default_charset);
  g_free (confile.tea_autoreplace);
}


void get_iconv_sup (void)
{
  if (confile.iconv_encs)
     g_list_free (confile.iconv_encs);

  if (confile.gl_save_charsets)
     g_list_free (confile.gl_save_charsets);

  confile.gl_save_charsets = NULL;

  glist_strings_free (gl_enc_choosed);
  gl_enc_choosed = load_file_to_glist (confile.tea_choosed_charsets);

  confile.iconv_encs = g_list_copy (gl_enc_choosed);

  gchar *charset;
  g_get_charset (&charset);
  confile.iconv_encs = g_list_prepend (confile.iconv_encs, charset);

  if (g_utf8_collate (charset, "UTF-8") != 0)
     confile.iconv_encs = g_list_prepend (confile.iconv_encs, "UTF-8");

  confile.gl_save_charsets = g_list_copy (confile.iconv_encs);

  confile.iconv_encs = g_list_prepend (confile.iconv_encs, CHARSET_JAPANESE);
  confile.iconv_encs = g_list_prepend (confile.iconv_encs, "autodetect");
}


void execute_recent_item (const gchar *item)
{
  if (! item)
     return;

  if (g_utf8_strlen (item, -1) <= 2)
     return;

  gchar **a = NULL;
  gchar *f = NULL;

  a = g_strsplit (item, ",", -1);

  if (get_gchar_array_size (a) != 3)
     return;

  guint pos = strtol (a[2], NULL, 10);

  f = get_l_filename (a[0]);
  if (! f)
     {
      g_strfreev (a);
      return;
     }

  if (! g_file_test ( f, G_FILE_TEST_EXISTS))
     {
      log_to_memo (_("%s is not exists"), f, LM_ERROR);
      g_strfreev (a);
      g_free (f);
      return;
     }

  cur_text_doc = doc_open_file (f, a[1]);

  if (cur_text_doc)
     {
      set_title (cur_text_doc);
      editor_set_pos (cur_text_doc, pos);
     }

  g_strfreev (a);
  g_free (f);
}


void update_enc_menu (void)
{
  gtk_widget_destroy (mni_co_menu);
  mni_co_menu = new_menu_submenu (mni_co);
  build_menu_from_glist (confile.iconv_encs, mni_co_menu, on_mni_co_select);
}


void bmx_reload (void)
{
  glist_strings_free (bookmarks);
  bookmarks = load_file_to_glist (confile.bmx_file);
  gtk_widget_destroy (mni_bookmarks_menu);
  mni_bookmarks_menu = new_menu_submenu (mni_bookmarks_root);
  build_menu_from_glist (bookmarks, mni_bookmarks_menu, on_mni_recent_activate);
}


void update_recent_list_menu (gboolean load_from_file)
{
  if (load_from_file)
     {
      glist_strings_free (recent_list);
      recent_list = load_file_to_glist (confile.tea_recent);
     }

  gtk_widget_destroy (mni_recent_menu);
  mni_recent_menu = new_menu_submenu (mni_recent_list);
  build_menu_from_glist (recent_list, mni_recent_menu, on_mni_real_recent_activate);
}


void tea_start (void)
{

  ht_tea_hotkeys = NULL;
  gl_scripts = NULL;
  ui_done_flag = FALSE;
  b_exit_flag = FALSE;
  last_dir = NULL;
//  enc_guess_value = NULL;
  sr_wnd = NULL;
  ht_ext_programs = NULL;
  ht_autoreplace = NULL;
  gl_autosave = NULL;
  gl_enc_choosed = NULL;
  ht_entities = NULL;
  mni_autocomp = NULL;
  dox = NULL;
  gl_notetab_positions = NULL;
  bookmarks = NULL;
  confile.msg_counter = 0;
  confile.iconv_encs = NULL;
  confile.current_movie = NULL;

  gl_notetab_positions = g_list_append (gl_notetab_positions, g_strdup (_("Left")));
  gl_notetab_positions = g_list_append (gl_notetab_positions, g_strdup (_("Right")));
  gl_notetab_positions = g_list_append (gl_notetab_positions, g_strdup (_("Top")));
  gl_notetab_positions = g_list_append (gl_notetab_positions, g_strdup (_("Bottom")));

  gl_found_files = NULL;
  //tooltips = gtk_tooltips_new ();
  //gtk_tooltips_enable (tooltips);
  gl_enc_available = create_enc_list ();

  calendar_init ();
}


//n.p. Angelo Badalamenti - Red Bats With Teeth
void tea_done (void)
{
  dbm ("tea_done");

  mni_tabs = NULL;

  b_exit_flag = TRUE;
  GList *p;

  if (dox)
    {
     p = g_list_first (dox);
     while (p)
          {
           page_free (p->data);
           p->data = NULL;
           p = g_list_next (p);
          }

     g_list_free (dox);
    }

  glist_save_to_file (recent_list, confile.tea_recent);

  g_list_free (confile.iconv_encs);

  glist_strings_free (recent_list);
  glist_strings_free (bookmarks);
  glist_strings_free (gl_notetab_positions);
  glist_strings_free (gl_image_formats);
  glist_strings_free (gl_found_files);
  glist_strings_free (gl_scripts);

  if (ht_tea_hotkeys)
     g_hash_table_destroy (ht_tea_hotkeys);

  if (ht_entities)
     g_hash_table_destroy (ht_entities);

  if (ht_autoreplace)
     g_hash_table_destroy (ht_autoreplace);

  //g_hash_table_destroy (po_t_keywords);
  //g_hash_table_destroy (bash_t_keywords);
  //g_hash_table_destroy (php_t_keywords);
  //g_hash_table_destroy (c_t_keywords);
  //g_hash_table_destroy (c_t_types);
  //g_hash_table_destroy (pas_t_keywords);
  //g_hash_table_destroy (pas_t_types);
  //g_hash_table_destroy (py_t_keywords);

  unlink (confile.tmp_file);
  g_free (confile.last_version);
  calendar_done ();
  confile_free ();
  spell_checker_done ();
  g_free (last_dir);


#ifdef SOUNDS_MODE
  sounds_done ();
#endif
}


GdkModifierType string_to_mod (const gchar *s)
{
  if (! s)
     return 0;

  GdkModifierType result = 0;

  if (strstr (s, "SHIFT"))
     result = result | GDK_SHIFT_MASK;

  if (strstr (s, "CTRL"))
     result = result | GDK_CONTROL_MASK;

  if (strstr (s, "ALT"))
     result = result | GDK_MOD1_MASK;

  return result;
}


void lookup_widget_cb (GtkWidget *widget, gpointer data)
{
  if ((! widget) || (! ht_tea_hotkeys))
     return;

  gchar *value = NULL;
  gchar *key;
  gchar *t;
  gchar *s = gtk_widget_get_name (widget);

  if (! s)
     return;

  if (strcmp (MENUITEMUTIL, s) != 0)
     {
      value = g_hash_table_lookup (ht_tea_hotkeys, s);

      if (value)
         {
          t = g_strrstr (value, " ");
          if (! t)
             key = g_strdup (value);
          else
              key = g_strchug (g_strdup (t));

          gtk_widget_add_accelerator (widget, "activate", accel_group,
                                      gdk_keyval_from_name (key), string_to_mod (value),
                                      GTK_ACCEL_VISIBLE);

          g_free (key);
         }

      if (GTK_IS_MENU_ITEM (widget))
         lookup_widget (gtk_menu_item_get_submenu (widget), data);
     }
}


GtkWidget* lookup_widget (GtkContainer *widget, const gchar *widget_name)
{
  if (widget)
     gtk_container_foreach (widget, lookup_widget_cb, widget_name);

  return NULL;
}


void reload_hotkeys (void)
{
  if (ht_tea_hotkeys)
     g_hash_table_destroy (ht_tea_hotkeys);

  gchar *shortcut = NULL;
  gchar *menuitem = NULL;

  ht_tea_hotkeys = load_file_to_hashtable (confile.tea_hotkeys);

  lookup_widget (GTK_CONTAINER (menubar1), "test");
}


void reload_snippets (void)
{
  if (! g_file_test (confile.snippets_dir, G_FILE_TEST_EXISTS))
     if (mkdir (confile.snippets_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  if (mni_snippets_menu)
     gtk_widget_destroy (mni_snippets_menu);

  mni_snippets_menu = new_menu_submenu (mni_snippets);
  GtkWidget *temp = new_menu_tof (mni_snippets_menu);

  GList *l = read_dir_files (confile.snippets_dir);
  build_menu_from_glist (l, mni_snippets_menu, on_mni_snippet_click);
  glist_strings_free (l);
}


void tabs_reload (void)
{
  if (! mni_tabs)
     return;

  gtk_widget_destroy (mni_tabs_menu);
  mni_tabs_menu = new_menu_submenu (mni_tabs);

  GList *p = g_list_first (dox);
  t_note_page *doc = NULL;

  while (p)
        {
         doc = p->data;
         if (doc)
            mni_temp = new_menu_item_with_udata (doc->file_name_utf8, mni_tabs_menu, on_mni_tabs_menuitem_click, doc);

         p = g_list_next (p);
        }
}

//n.p.Trio Bayanistov - Barynja (1937)
void reload_dicts (void)
{
#ifdef HAVE_LIBASPELL

  gtk_widget_destroy (mni_spellcheck_menu);
  mni_spellcheck_menu = new_menu_submenu (mni_spellcheck);
  GtkWidget *temp = new_menu_tof (mni_spellcheck_menu);
  GList *l = get_speller_modules_list ();
  build_menu_from_glist (l, mni_spellcheck_menu, on_mni_spellcheck);
  glist_strings_free (l);

#endif
}


gchar* get_tmp_file_string (void)
{
  return str_file_read (confile.tmp_file);
}


void put_tmp_file_string (const gchar *s)
{
   save_string_to_file (confile.tmp_file, s);
}


void fill_entities (void)
{
  ht_entities = g_hash_table_new (g_str_hash, g_str_equal);

  g_hash_table_insert (ht_entities, _("non-breaking space"), "160");
  g_hash_table_insert (ht_entities, _("cent sign"), "162");
  g_hash_table_insert (ht_entities, _("pound sign"), "163");
  g_hash_table_insert (ht_entities, _("currency sign"), "164");
  g_hash_table_insert (ht_entities, _("yen sign"), "165");
  g_hash_table_insert (ht_entities, _("section sign"), "167");
  g_hash_table_insert (ht_entities, _("copyright sign"), "169");
  g_hash_table_insert (ht_entities, _("trade mark sign"), "8482");
  g_hash_table_insert (ht_entities, _("feminine ordinal indicator"), "170");
  g_hash_table_insert (ht_entities, _("left-pointing double angle quotation mark"), "171");
  g_hash_table_insert (ht_entities, _("soft hyphen"), "173");
  g_hash_table_insert (ht_entities, _("registered sign"), "174");
  g_hash_table_insert (ht_entities, _("degree sign"), "176");
  g_hash_table_insert (ht_entities, _("plus-minus sign"), "177");
  g_hash_table_insert (ht_entities, _("right-pointing double angle quotation mark"), "187");
  g_hash_table_insert (ht_entities, _("left single quotation mark"), "8216");
  g_hash_table_insert (ht_entities, _("right single quotation mark"), "8217");
  g_hash_table_insert (ht_entities, _("euro sign"), "8364");
  g_hash_table_insert (ht_entities, _("greek pi symbol"), "982");
  g_hash_table_insert (ht_entities, _("leftwards arrow"), "8592");
  g_hash_table_insert (ht_entities, _("upwards arrow"), "8593");
  g_hash_table_insert (ht_entities, _("rightwards arrow"), "8594");
  g_hash_table_insert (ht_entities, _("downwards arrow"), "8595");
  g_hash_table_insert (ht_entities, _("square root"), "8730");
  g_hash_table_insert (ht_entities, _("spade suit"), "9824");
  g_hash_table_insert (ht_entities, _("club suit"), "9827");
  g_hash_table_insert (ht_entities, _("heart suit"), "9829");
  g_hash_table_insert (ht_entities, _("diamond suit"), "9830");
  g_hash_table_insert (ht_entities, _("masculine ordinal indicator"), "186");
  g_hash_table_insert (ht_entities, _("quotation mark"), "34");
  g_hash_table_insert (ht_entities, _("ampersand"), "38");
  g_hash_table_insert (ht_entities, _("less-than sign"), "60");
  g_hash_table_insert (ht_entities, _("greater-than sign"), "62");
  g_hash_table_insert (ht_entities, _("apostrophe"), "39");

  g_hash_table_insert (ht_entities, "\342\234\202", "9986");
  g_hash_table_insert (ht_entities, "\342\235\200", "10048");
  g_hash_table_insert (ht_entities, "\342\234\216", "9998");
  g_hash_table_insert (ht_entities, "\342\230\240", "9760");
  g_hash_table_insert (ht_entities, "\342\230\256", "9774");

  g_hash_table_insert (ht_entities, "\342\230\255", "9773");
  g_hash_table_insert (ht_entities, "\342\230\242", "9762");
  g_hash_table_insert (ht_entities, "\342\231\253", "9835");

  g_hash_table_insert (ht_entities, "\342\234\255", "10029");
  g_hash_table_insert (ht_entities, "\342\230\235", "9757");
  g_hash_table_insert (ht_entities, "\342\230\237", "9759");
}


void reload_sessions (void)
{
  if (! g_file_test (confile.sessions_dir, G_FILE_TEST_EXISTS))
     if (mkdir (confile.sessions_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  gtk_widget_destroy (mni_sessions_menu);
  mni_sessions_menu = new_menu_submenu (mni_sessions);
  GtkWidget *t = new_menu_tof (mni_sessions_menu);

  GList *l = read_dir_files (confile.sessions_dir);
  build_menu_from_glist (l, mni_sessions_menu, on_mni_sessions_click);
  glist_strings_free (l);
}


void reload_profiles (void)
{
  if (! g_file_test (confile.profiles_dir, G_FILE_TEST_EXISTS))
     if (mkdir (confile.profiles_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  gtk_widget_destroy (mni_profiles_menu);
  mni_profiles_menu = new_menu_submenu (mni_profiles);
  GtkWidget *t = new_menu_tof (mni_profiles_menu);

  GList *l = read_dir_files (confile.profiles_dir);
  build_menu_from_glist (l, mni_profiles_menu, on_mni_profiles_click);
  glist_strings_free (l);
}


void reload_reptables (void)
{
  if (! g_file_test (confile.tables_dir, G_FILE_TEST_EXISTS))
     if (mkdir (confile.tables_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  gtk_widget_destroy (mni_reptables_menu);
  mni_reptables_menu = new_menu_submenu (mni_reptables);
  GtkWidget *t = new_menu_tof (mni_reptables_menu);

  GList *l = read_dir_files (confile.tables_dir);
  build_menu_from_glist (l, mni_reptables_menu, on_mni_reptables_click);
  glist_strings_free (l);
}


void reload_templates (void)
{
  if (! g_file_test (confile.templates_dir, G_FILE_TEST_IS_DIR))
     if (mkdir (confile.templates_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  gtk_widget_destroy (mni_templates_menu);
  mni_templates_menu = new_menu_submenu (mni_templates);
  GtkWidget *t = new_menu_tof (mni_templates_menu);

  GList *l = read_dir_files (confile.templates_dir);
  build_menu_from_glist (l, mni_templates_menu, on_mni_templates_click);
  glist_strings_free (l);
}


gchar* ht_get_char_value (const gchar *key, const gchar *def)
{
  if ((! def) || (! key) )
     return NULL;

  if (! ht_config)
      return g_strdup (def);

  gchar *v = g_hash_table_lookup (ht_config, key);
  if (v)
     return g_strdup (v);

  return strdup (def);
}


gint ht_get_int_value (const gchar *key, gint def)
{
  if (! ht_config)
      return def;

  gint result;

  gchar *s = g_strdup_printf ("%d", def);
  gchar *res = ht_get_char_value (key, s);

  if (! res)
     {
      g_free (s);
      return def;
     }

  result = strtol (res, NULL, 10);

  g_free (res);
  g_free (s);

  return result;
}


void confile_reload (void)
{
  confile_free ();

  confile.screen_w = gdk_screen_width ();
  confile.screen_h = gdk_screen_height ();

  gchar *conf_dir = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S, ".config", NULL);

  confile.tea_main_dir = g_strconcat (conf_dir, G_DIR_SEPARATOR_S, "tea", NULL);

  confile.tmp_dir = g_strconcat (g_get_tmp_dir (), G_DIR_SEPARATOR_S, "teatmpdir", NULL);
  confile.tmp_file = g_strconcat (g_get_tmp_dir (), G_DIR_SEPARATOR_S, "tea_temp.html", NULL);

  confile.ext_programs = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "ext_programs", NULL);
  confile.tea_autoreplace = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tea_autoreplace", NULL);
  confile.bmx_file = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tea_bmx", NULL);
  confile.crapbook_file = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "crapbook.txt", NULL);
  confile.sessions_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "sessions", G_DIR_SEPARATOR_S, NULL);
  confile.snippets_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "snippets", G_DIR_SEPARATOR_S, NULL);
  confile.templates_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "templates", G_DIR_SEPARATOR_S, NULL);
  confile.scripts_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "scripts", G_DIR_SEPARATOR_S, NULL);
  confile.tables_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tables", G_DIR_SEPARATOR_S, NULL);
  confile.profiles_dir = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "profiles", G_DIR_SEPARATOR_S, NULL);

  confile.dir_thumbs = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S, ".thumbnails", G_DIR_SEPARATOR_S, "normal", NULL);
  confile.last_session = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "last_session", NULL);
  confile.autosave_session = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "autosave_session", NULL);

  //confile.fam_autocomp_file = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "fte_autocomp_file", NULL);
  confile.crash_file = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "crash_file", NULL);
  confile.autosave_file = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tea_autosave_list", NULL);
  confile.tea_rc = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "teagtk_rc", NULL);
  confile.tea_ui = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "teagtk_ui", NULL);
  confile.tea_kwas_bookmarks = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S, ".gtk-bookmarks", NULL);

  confile.tea_recent = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tea_recent", NULL);
  confile.tea_hotkeys = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "tea_keys", NULL);

  confile.tea_choosed_charsets = g_strconcat (confile.tea_main_dir, G_DIR_SEPARATOR_S, "charsets", NULL);

  if (! g_file_test (conf_dir, G_FILE_TEST_EXISTS))
     if (mkdir (conf_dir, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
        dbm ("I cannot to create $HOME/.config");

  if (! g_file_test (confile.tea_main_dir, G_FILE_TEST_EXISTS))
      if (mkdir (confile.tea_main_dir, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
         dbm ("I cannot to create $HOME/.config/tea");

//  if (! g_file_test (confile.fam_autocomp_file, G_FILE_TEST_EXISTS))
  //    create_empty_file (confile.fam_autocomp_file, "alakazar alakazun");

  if (! g_file_test (confile.tea_rc, G_FILE_TEST_EXISTS))
      create_empty_file (confile.tea_rc, _("#TEA editor configuration file. Please read the manual"));

  if (! g_file_test (confile.tea_autoreplace, G_FILE_TEST_EXISTS))
      create_empty_file (confile.tea_autoreplace, "Linux=GNU/Linux");

  ht_config = load_file_to_hashtable (confile.tea_rc);

  gchar *x = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S, NULL);
  confile.def_save_dir = ht_get_char_value ("def_save_dir", x);
  confile.def_open_dir = ht_get_char_value ("def_open_dir", x);

  gchar *t = add_slash_if_need (confile.def_save_dir);
  confile.def_save_dir = ch_str (confile.def_save_dir, t);
  g_free (t);
  t = add_slash_if_need (confile.def_open_dir);
  confile.def_open_dir = ch_str (confile.def_open_dir, t);
  g_free (t);

  g_free (x);

  confile.cm_pack = ht_get_char_value ("cm_pack", "zip -j -q -9 %s.zip %s");
  confile.editor_font = ht_get_char_value ("editor_font", "Monospace 12");

  confile.ext_pic_editor = ht_get_char_value ("ext_pic_editor", "gimp-remote-2.0 -n %s&");
  confile.ext_pic_viewer = ht_get_char_value ("ext_pic_viewer", "display %s&");
  confile.date_time = ht_get_char_value ("date_time", "%d/%m/%Y %T");
  confile.date_time_ver = ht_get_char_value ("date_time_ver", "%Y-%m-%d-%T");

  confile.max_recent_items = ht_get_int_value ("max_recent_items", 12);

  confile.max_undo_levels =  ht_get_int_value ("max_undo_levels", 32);

  confile.fullscreen = ht_get_int_value ("fullscreen", FALSE);
  confile.autosave_interval = ht_get_int_value ("autosave_interval", 3);
  confile.do_save_all_on_termination = ht_get_int_value ("do_save_all_on_termination", 0);
  confile.do_crash_file = ht_get_int_value ("do_crash_file", 0);

  confile.hl_current_line = ht_get_int_value ("hl_current_line", 0);
  confile.det_charset_by_meta = ht_get_int_value ("det_charset_by_meta", 0);

  confile.now_playing = ht_get_char_value ("now_playing", "Now playing %s");

  confile.show_cursor_pos = ht_get_int_value ("show_cursor_pos", 0);
  confile.enc_use_fallback = ht_get_int_value ("enc_use_fallback", 1);

  confile.saveopen_h = ht_get_int_value ("saveopen_h", 75);
  confile.saveopen_w = ht_get_int_value ("saveopen_w", 75);

  confile.margin_width = ht_get_int_value ("margin_width", 72);
  confile.margin_visible = ht_get_int_value ("margin_visible", 0);

  confile.show_line_markers = ht_get_int_value ("show_line_markers", 0);

  confile.notebook_tab_position = ht_get_int_value ("notebook_tab_position", GTK_POS_TOP);

  confile.prompt_on_not_saved = ht_get_int_value ("prompt_on_not_saved", 1);
  confile.prompt_on_file_exists = ht_get_int_value ("prompt_on_file_exists", 1);
  confile.common_closing_button_visible = ht_get_int_value ("common_closing_button_visible", 1);

  confile.homeend = ht_get_int_value ("homeend", 0);

  confile.start_with_blank_file = ht_get_int_value ("start_with_blank_file", 0);
//  confile.fte_autocomp = ht_get_int_value ("fte_autocomp", 1);

  confile.do_last_session = ht_get_int_value ("do_last_session", 0);
  confile.do_autosave_session = ht_get_int_value ("do_autosave_session", 0);

  confile.show_line_nums = ht_get_int_value ("show_line_nums", 0);
  confile.word_wrap = ht_get_int_value ("word_wrap", 1);
  confile.tab_size = ht_get_int_value ("tab_size", 3);

  confile.fmt_color_function = ht_get_char_value ("fmt_color_function", "<span style=\"color:@color;\">@text</span>");
  confile.fallback_charset = ht_get_char_value ("fallback_charset", "ISO-8859-1");

  confile.def_color_id = ht_get_char_value ("def_color_id", "kate");
  confile.def_file_ext = ht_get_char_value ("def_file_ext", "txt");
  confile.add_def_file_ext = ht_get_int_value ("add_def_file_ext", 0);

  confile.automatic_markup_mode = ht_get_int_value ("automatic_markup_mode", 0);

  confile.text_foreground = ht_get_char_value ("text_foreground", "black");
  confile.text_background = ht_get_char_value ("text_background", "white");
  confile.text_sel_foreground = ht_get_char_value ("text_sel_foreground", "white");
  confile.text_sel_background = ht_get_char_value ("text_sel_background", "black");

  confile.tag_color_paint_fg = ht_get_char_value ("tag_color_paint_fg", "white");
  confile.tag_color_paint_bg = ht_get_char_value ("tag_color_paint_bg", "red");

  confile.tag_current_line_fg = ht_get_char_value ("tag_current_line_fg", "black");
  confile.tag_current_line_bg = ht_get_char_value ("tag_current_line_bg", "#ffdcc3");

  confile.color_cursor = ht_get_char_value ("color_cursor", "black");
  confile.tag_color_lm_error_fg = ht_get_char_value ("tag_color_lm_error_fg", "red");
  confile.tag_color_lm_error_bg = ht_get_char_value ("tag_color_lm_error_bg", confile.text_background);

  confile.tag_comment = ht_get_char_value ("tag_comment", "#605454");
  confile.tag_identifier = ht_get_char_value ("tag_identifier", "black");
  confile.tag_digit = ht_get_char_value ("tag_digit", "navy");
  confile.tag_string = ht_get_char_value ("tag_string", "navy");
  confile.tag_html_tag = ht_get_char_value ("tag_html_tag", "black");
  confile.tag_preprocessor = ht_get_char_value ("tag_preprocessor", "#b522a6");
  confile.tag_type = ht_get_char_value ("tag_type", "#3650bd");
  confile.tag_spellcheck = ht_get_char_value ("tag_spellcheck", "red"); //#e70f1e

  confile.tag_comment_font = ht_get_char_value ("tag_comment_font", "Monospace Italic 12");
  confile.tag_identifier_font = ht_get_char_value ("tag_identifier_font", "Monospace Bold 12");
  confile.tag_digit_font = ht_get_char_value ("tag_digit_font", confile.editor_font);
  confile.tag_string_font = ht_get_char_value ("tag_string_font", confile.editor_font);
  confile.tag_html_tag_font = ht_get_char_value ("tag_html_tag_font", "Monospace Bold 12");
  confile.tag_preprocessor_font = ht_get_char_value ("tag_preprocessor_font", confile.editor_font);
  confile.tag_type_font = ht_get_char_value ("tag_type_font", confile.editor_font);

  confile.cm_mplayer = ht_get_char_value ("cm_mplayer", "mplayer \"@movie_file\" -subcp @encoding -ss @time -sub \"@sub_file\" &");

  confile.cm_pdflatex = ht_get_char_value ("cm_pdflatex", "pdflatex %s");
  confile.cm_dvi_to_pdf = ht_get_char_value ("cm_dvi_to_pdf", "dvipdf %s");
  confile.cm_dvi_to_ps = ht_get_char_value ("cm_dvi_to_ps", "dvips -Pcmz %s -o %s.ps");
  confile.cm_dvi_view = ht_get_char_value ("cm_dvi_view", "xdvi %s");
  //dviview
  confile.cm_pdf_view = ht_get_char_value ("cm_pdf_view", "kpdf %s");
  //xpdf, gsview
  confile.cm_ps_view = ht_get_char_value ("cm_ps_view", "kghostview %s");
  confile.cm_run_latex = ht_get_char_value ("cm_run_latex", "latex -file-line-error-style %s");

  //n.p. Scorn - Orgy Of Holyness

  confile.cmd_def_doc_browser = ht_get_char_value ("def_doc_browser", "firefox %s&");
  confile.use_def_doc_browser = ht_get_int_value ("use_def_doc_browser", 0);

  confile.allow_multiple_instances = ht_get_int_value ("allow_multiple_instances", 0);

  confile.exclusive_enter_at_dialogs = ht_get_int_value ("exclusive_enter_at_dialogs", 1);

  confile.use_cursor_blink = ht_get_int_value ("use_cursor_blink", 1);
  set_cursor_blink (confile.use_cursor_blink);

  confile.thumb_width = ht_get_int_value ("thumb_width", 64);
  confile.thumb_height = ht_get_int_value ("thumb_height", 64);

  confile.use_auto_indent = ht_get_int_value ("use_auto_indent", 0);
  confile.do_autorep = ht_get_int_value ("do_autorep", 0);
  confile.do_show_main_toolbar = ht_get_int_value ("do_show_main_toolbar", 1);

  confile.autoclose_tags = ht_get_int_value ("autoclose_tags", 0);

  confile.rtf_enc = ht_get_char_value ("rtf_enc", "CP1251");
  confile.def_filesave_charset = ht_get_char_value ("def_filesave_charset", "UTF-8");

  confile.default_charset = ht_get_char_value ("default_charset", "UTF-8");

  confile.main_wnd_show_full_path = ht_get_int_value ("main_wnd_show_full_path", 1);
  confile.do_backup = ht_get_int_value ("do_backup", 0);
  confile.show_hidden_files = ht_get_int_value ("show_hidden_files", 1);
  confile.implane_ins_image = ht_get_int_value ("implane_ins_image", 0);

  confile.ins_spaces_on_tab_press = ht_get_int_value ("ins_spaces_on_tab_press", 0);
  confile.do_autosave = ht_get_int_value ("do_autosave", 0);

  confile.jpeg_q = ht_get_int_value ("jpeg_q", 80);
  if (confile.jpeg_q < 1 || confile.jpeg_q > 100)
     confile.jpeg_q = 80;

  confile.screenshot_delay = ht_get_int_value ("screenshot_delay", 2);

  confile.use_def_save_dir = ht_get_int_value ("use_def_save_dir", 0);
  confile.use_def_open_dir = ht_get_int_value ("use_def_open_dir", 0);

  confile.use_ext_image_viewer = ht_get_int_value ("use_ext_image_viewer", 0);
  confile.do_det_scripts_by_content = ht_get_int_value ("do_det_scripts_by_content", 0);

  confile.enc_det_czech = ht_get_int_value ("enc_det_czech", 0);
  confile.enc_det_cp1251 = ht_get_int_value ("enc_det_cp1251", 0);
  confile.enc_det_koi8r = ht_get_int_value ("enc_det_koi8r", 0);
  confile.enc_det_koi8u = ht_get_int_value ("enc_det_koi8u", 0);
  confile.enc_det_866 = ht_get_int_value ("enc_det_866", 0);
  confile.enc_det_finnish = ht_get_int_value ("enc_det_finnish", 0);
  confile.enc_det_german = ht_get_int_value ("enc_det_german", 0);
  confile.enc_det_serbian = ht_get_int_value ("enc_det_serbian", 0);
  confile.enc_det_latvian = ht_get_int_value ("enc_det_latvian", 0);
  confile.enc_det_polish = ht_get_int_value ("enc_det_polish", 0);
  confile.enc_det_portuguese = ht_get_int_value ("enc_det_portuguese", 0);
  confile.enc_det_slovak = ht_get_int_value ("enc_det_slovak", 0);
  confile.enc_det_slovenian = ht_get_int_value ("enc_det_slovenian", 0);
  confile.enc_det_spanish = ht_get_int_value ("enc_det_spanish", 0);
  confile.enc_det_turkish = ht_get_int_value ("enc_det_turkish", 0);

  confile.scan_for_links_on_doc_open = ht_get_int_value ("scan_for_links_on_doc_open", 0);
  confile.logmemo_lines_max = ht_get_int_value ("logmemo_lines_max", 777);
//  confile.fte_min_prefix_len = ht_get_int_value ("fte_min_prefix_len", 3);

  g_hash_table_destroy (ht_config);

  widget_apply_colors (ent_search);
  widget_apply_colors (tv_logmemo);

  if (notebook1)
     gtk_notebook_set_tab_pos (notebook1, confile.notebook_tab_position);

  g_free (conf_dir);

  get_iconv_sup ();
}


void reload_autoreplace (void)
{
  ht_autoreplace = load_file_to_hashtable (confile.tea_autoreplace);
}


void reload_autosave (void)
{
  glist_strings_free (gl_autosave);
  gl_autosave = load_file_to_glist (confile.autosave_file);
}


void reload_ext_programs (void)
{
  if (! confile.ext_programs)
     return;

  if (! g_file_test (confile.ext_programs, G_FILE_TEST_EXISTS))
      {
       GList *l = NULL;

       l = g_list_prepend (l, "Konqueror=konqueror %s&");
       l = g_list_prepend (l, "Firefox=firefox %s&");
       l = g_list_prepend (l, "Opera=opera -remote 'openURL(%s,new-window)' || opera %s&");
       l = g_list_prepend (l, "Mozilla=mozilla -remote 'openURL(%s, new-window)' || mozilla %s&");
       l = g_list_prepend (l, "Galeon=galeon -x %s&");
       l = g_list_prepend (l, "Dillo=dillo %s&");
       l = g_list_prepend (l, "Links=konsole -e links %s&");
       l = g_list_prepend (l, "Elinks=konsole -e elinks %s&");
       l = g_list_prepend (l, "Lynx=konsole -e lynx %s&");
       l = g_list_prepend (l, "Epiphany=epiphany %s&");
#ifdef DARWIN
       l = g_list_prepend (l, "Mac OS X=open %s&");
#endif

       glist_save_to_file (l, confile.ext_programs);
       g_list_free (l);
      }

  if (ht_ext_programs)
     g_hash_table_destroy (ht_ext_programs);

  ht_ext_programs = load_file_to_hashtable (confile.ext_programs);

  gtk_widget_destroy (mni_ext_programs_menu);
  mni_ext_programs_menu = new_menu_submenu (mni_ext_programs);
  mni_temp = new_menu_tof (mni_ext_programs_menu);

  build_menu_from_ht (ht_ext_programs, mni_ext_programs_menu, on_mni_ext_programs_menu_item);
}


void reload_scripts (void)
{
  if (! g_file_test (confile.scripts_dir, G_FILE_TEST_IS_DIR))
     if (mkdir (confile.scripts_dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        return;

  if (mni_scripts_menu)
     gtk_widget_destroy (mni_scripts_menu);

  mni_scripts_menu = new_menu_submenu (mni_scripts);
  GtkWidget *temp = new_menu_tof (mni_scripts_menu);

  GList *l = read_dir_files (confile.scripts_dir);
  build_menu_from_glist (l, mni_scripts_menu, on_mni_script_click);
  glist_strings_free (l);
}


void markup_change_by_ext (const gchar *filename)
{
  if (! confile.automatic_markup_mode)
     return;

  gchar *mm = NULL;

  if (is_ext (filename, ".html", ".htm", NULL))
     mm = g_strdup ("HTML");

  if (is_ext (filename, ".xhtml", ".xhtm", NULL))
     mm = g_strdup ("XHTML");

  if (is_ext (filename, ".xml", NULL))
     mm = g_strdup ("Docbook");

  if (is_ext (filename, ".tex", "latex", NULL))
     mm = g_strdup ("LaTeX");

  if (! mm)
     return;

  confile.default_markup_mode = mm;
  def_mm = get_markup_mode ();
}


gchar* get_default_shortcuts (void)
{
  GList *l = NULL;

  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_new), "CTRL N");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_kwas), "ALT N");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_crapbook), "ALT M");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_open), "CTRL O");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_save), "CTRL S");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_save_ver), "ALT S");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_save_backup), "CTRL B");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_close_current), "CTRL W");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_file_quit), "CTRL Q");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_edit_delete_cur_line), "CTRL Y");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_edit_undo), "CTRL Z");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_edit_redo), "CTRL SHIFT Z");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_find_now), "CTRL F");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_find_now_caseinsens), "CTRL SHIFT F");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_find_next), "F3");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_find_next_caseinsens), "SHIFT F3");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_goto_line), "ALT G");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_bold), "ALT B");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_italic), "ALT I");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_para), "ALT P");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_link), "ALT L");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_color), "ALT C");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_align_center), "CTRL SHIFT C");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_stats), "F11");
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_apply_tpl_to_each_ln), "ALT E");

  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_open_at_cursor), "F2");
#ifndef CHAI_MODE
  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_refresh_hl), "F5");
#endif

  l = add_to_glist_combined_str (l, gtk_widget_get_name (mni_word_wrap), "CTRL W");

  gchar *x = string_from_glist (l);
  glist_strings_free (l);
  return x;
}


void hotkeys_initial_check (void)
{
  gchar *def_keys;
  gchar *old_keys;
  gchar *new_keys;

  if (! g_file_test (confile.tea_hotkeys, G_FILE_TEST_EXISTS))
     {
      def_keys = get_default_shortcuts ();
      save_string_to_file (confile.tea_hotkeys, def_keys);
      g_free (def_keys);
     }
}


GList* get_color_scheme_ids (void)
{
  GtkSourceStyleSchemeManager *manager = gtk_source_style_scheme_manager_get_default ();

  gchar **scheme_ids;
  g_object_get (manager, "scheme-ids", &scheme_ids, NULL);

  GList *ids = NULL;
  gint c = get_gchar_array_size (scheme_ids) - 1;
  gint i;

  for (i = 0; i <= c; i++)
     ids = g_list_prepend (ids, scheme_ids[i]);


  return ids;
}