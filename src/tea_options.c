/***************************************************************************
                          tea_options.c  -  description
                             -------------------
    begin                : aug. 2004
    copyright            : (C) 2004-2007 by Peter 'Roxton' Semiletov,
                          Michele Garoche
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


#include "tea_defs.h"
#include "tea_options.h"
#include "tea_gtk_utils.h"
#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "tea_config.h"
#include "rox_strings.h" // Needed for glist_save_to_file
#include "free_desktop_support.h" // Needed for free_desktop_delete_shortcut
#include "tea_tree_view.h" // Needed for tv_fill_with_glist
#include "interface.h" // Needed for ui_update
#include "tea_funx.h" // Needed for copy_file
#include "callbacks.h"




static void cb_scheme_change (GtkComboBox *combobox,
                              gpointer user_data)
{
  if (! gl_color_schemes)
     return;

  gint index = gtk_combo_box_get_active (combobox);
  if (index == -1)
     return;

  gchar *x = combo_get_value (cbm_color_cheme, gl_color_schemes);

  GtkSourceStyleScheme *shm = gtk_source_style_scheme_manager_get_scheme (gtk_source_style_scheme_manager_get_default (), x);

  gchar *s = g_strconcat (
                          gtk_source_style_scheme_get_name (shm), "\n",
                          gtk_source_style_scheme_get_description (shm), "\n",
                          gtk_source_style_scheme_get_filename (shm),
                          NULL);

  gtk_text_buffer_set_text (gtk_text_view_get_buffer (user_data), s, -1);
  g_free (s);
}


static GtkWidget* wnd_options_hl_schemes (void)
{
//  GtkWidget *page = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
 
  gtk_widget_show (page);

  //GtkWidget *vb = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *vb = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  
  gtk_widget_show (vb);
  gtk_box_pack_start (page, vb, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *text_view = gtk_text_view_new();


  cbm_color_cheme = tea_combo_combo (vb, gl_color_schemes, _("Highlighting color scheme"),
                                     find_str_index (gl_color_schemes, confile.def_color_id), cb_scheme_change, text_view);

  gtk_widget_show (text_view);

  gtk_box_pack_start (vb, text_view, FALSE, TRUE, UI_PACKFACTOR);


  return page;
}


static void cb_put_launcher (GtkButton *button,
                             gpointer user_data)
{
  free_desktop_create_shortcut ();
}


static void cb_del_launcher (GtkButton *button,
                             gpointer user_data)
{
  free_desktop_delete_shortcut ();
}


static void cb_on_del_enc (GtkButton *button,
                           gpointer user_data)
{
  gchar *s = tv_get_selected_single (tv_choosed_charsets);
  if (! s)
     return;

  GList *l = str_remove_from_list (gl_enc_choosed, s);
  glist_save_to_file (l, confile.tea_choosed_charsets);
  get_iconv_sup ();
  tv_fill_with_glist (tv_choosed_charsets, gl_enc_choosed);
  g_free (s);
}


static void on_kwas_notebook_switch_page (GtkNotebook *notebook,
                                          GtkWidget *page,
                                          guint page_num,
                                          gpointer user_data)
{
  confile.last_prefs_tab = page_num;
}


static void cb_on_bt_apply (GtkWidget *w)
{
   GList *l = NULL;
   
   l = add_to_glist_combined_str (l, "def_color_id", combo_get_value (cbm_color_cheme, gl_color_schemes));

   l = add_to_glist_combined_str (l, "fmt_color_function", gtk_entry_get_text (GTK_ENTRY (ent_fmt_color_function)));

   l = add_to_glist_combined_str (l, "def_save_dir", gtk_entry_get_text (GTK_ENTRY (ent_def_save_dir)));
   l = add_to_glist_combined_str (l, "def_open_dir", gtk_entry_get_text (GTK_ENTRY (ent_def_open_dir)));

   l = add_to_glist_combined_str (l, "cm_pdflatex", gtk_entry_get_text (GTK_ENTRY (ent_cm_pdflatex)));
   l = add_to_glist_combined_str (l, "cm_dvi_to_ps", gtk_entry_get_text (GTK_ENTRY (ent_cm_dvi_to_ps)));
   l = add_to_glist_combined_str (l, "cm_dvi_view", gtk_entry_get_text (GTK_ENTRY (ent_cm_dvi_view)));
   l = add_to_glist_combined_str (l, "cm_pdf_view", gtk_entry_get_text (GTK_ENTRY (ent_cm_pdf_view)));
   l = add_to_glist_combined_str (l, "cm_ps_view", gtk_entry_get_text (GTK_ENTRY (ent_cm_ps_view)));
   l = add_to_glist_combined_str (l, "cm_run_latex", gtk_entry_get_text (GTK_ENTRY (ent_cm_run_latex)));
   l = add_to_glist_combined_str (l, "cm_dvi_to_pdf", gtk_entry_get_text (GTK_ENTRY (ent_cm_dvi_to_pdf)));

   l = add_to_glist_combined_str (l, "mplayer", gtk_entry_get_text (GTK_ENTRY (ent_cm_mplayer)));
   l = add_to_glist_combined_str (l, "cm_pack", gtk_entry_get_text (GTK_ENTRY (ent_cm_pack)));

   l = add_to_glist_combined_str (l, "def_doc_browser", gtk_entry_get_text (GTK_ENTRY (ent_cm_manual_browser)));

   l = add_to_glist_combined_str (l, "date_time", gtk_entry_get_text (GTK_ENTRY (ent_date_time)));
   l = add_to_glist_combined_str (l, "date_time_ver", gtk_entry_get_text (GTK_ENTRY (ent_date_time_ver)));

   l = add_to_glist_combined_str (l, "now_playing", gtk_entry_get_text (GTK_ENTRY (ent_now_playing)));

   l = add_to_glist_combined_str (l, "tag_color_lm_error_fg", gtk_entry_get_text (GTK_ENTRY (ent_tag_color_lm_error_fg)));
   l = add_to_glist_combined_str (l, "tag_color_lm_error_bg", gtk_entry_get_text (GTK_ENTRY (ent_tag_color_lm_error_bg)));

   l = add_to_glist_combined_str (l, "tag_current_line_fg", gtk_entry_get_text (GTK_ENTRY (ent_tag_current_line_fg)));
   l = add_to_glist_combined_str (l, "tag_current_line_bg", gtk_entry_get_text (GTK_ENTRY (ent_tag_current_line_bg)));


   //l = add_to_glist_combined_int (l, "fte_min_prefix_len", gtk_spin_button_get_value_as_int (sb_fte_min_prefix_len));

   l = add_to_glist_combined_int (l, "margin_visible", gtk_toggle_button_get_active (&cb_margin_visible->toggle_button));
   l = add_to_glist_combined_int (l, "margin_width", (int) (gtk_range_get_value (GTK_RANGE (sb_margin_width))));
   

   l = add_to_glist_combined_str (l, "tag_color_paint_fg", gtk_entry_get_text (GTK_ENTRY (ent_tag_color_paint_fg)));
   l = add_to_glist_combined_str (l, "tag_color_paint_bg", gtk_entry_get_text (GTK_ENTRY (ent_tag_color_paint_bg)));

   l = add_to_glist_combined_str (l, "tag_comment", gtk_entry_get_text (GTK_ENTRY (ent_tag_comment)));
   l = add_to_glist_combined_str (l, "tag_identifier", gtk_entry_get_text (GTK_ENTRY (ent_tag_identifier)));
   l = add_to_glist_combined_str (l, "tag_digit", gtk_entry_get_text (GTK_ENTRY (ent_tag_digit)));
   l = add_to_glist_combined_str (l, "tag_string", gtk_entry_get_text (GTK_ENTRY (ent_tag_string)));
   l = add_to_glist_combined_str (l, "tag_html_tag", gtk_entry_get_text (GTK_ENTRY (ent_tag_html_tag)));

   l = add_to_glist_combined_str (l, "tag_preprocessor", gtk_entry_get_text (GTK_ENTRY (ent_tag_preprocessor)));
   l = add_to_glist_combined_str (l, "tag_type", gtk_entry_get_text (GTK_ENTRY (ent_tag_type)));
   l = add_to_glist_combined_str (l, "tag_spellcheck", gtk_entry_get_text (GTK_ENTRY (ent_tag_spellcheck)));

   l = add_to_glist_combined_str (l, "text_foreground", gtk_entry_get_text (GTK_ENTRY (ent_text_foreground)));
   l = add_to_glist_combined_str (l, "text_background", gtk_entry_get_text (GTK_ENTRY (ent_text_background)));

   l = add_to_glist_combined_str (l, "text_sel_foreground", gtk_entry_get_text (GTK_ENTRY (ent_text_sel_foreground)));
   l = add_to_glist_combined_str (l, "text_sel_background", gtk_entry_get_text (GTK_ENTRY (ent_text_sel_background)));

   l = add_to_glist_combined_str (l, "rtf_enc", gtk_button_get_label (bt_rtf_enc));
   l = add_to_glist_combined_str (l, "fallback_charset", gtk_button_get_label (bt_fallback_charset));
   l = add_to_glist_combined_str (l, "default_charset", gtk_button_get_label (bt_default_charset));
   l = add_to_glist_combined_str (l, "def_filesave_charset", gtk_button_get_label (bt_def_filesave_charset));

   l = add_to_glist_combined_str (l, "tag_comment_font", gtk_font_button_get_font_name (fb_tag_comment_font));
   l = add_to_glist_combined_str (l, "tag_identifier_font", gtk_font_button_get_font_name (fb_tag_identifier_font));
   l = add_to_glist_combined_str (l, "tag_digit_font", gtk_font_button_get_font_name (fb_tag_digit_font));
   l = add_to_glist_combined_str (l, "tag_string_font", gtk_font_button_get_font_name (fb_tag_string_font));
   l = add_to_glist_combined_str (l, "tag_html_tag_font", gtk_font_button_get_font_name (fb_tag_html_tag_font));
   l = add_to_glist_combined_str (l, "tag_preprocessor_font", gtk_font_button_get_font_name (fb_tag_preprocessor_font));
   l = add_to_glist_combined_str (l, "tag_type_font", gtk_font_button_get_font_name (fb_tag_type_font));
   l = add_to_glist_combined_str (l, "editor_font", gtk_font_button_get_font_name (fb_editor_font));

   l = add_to_glist_combined_str (l, "ext_pic_editor", gtk_entry_get_text (GTK_ENTRY (ent_ext_pic_editor)));
   l = add_to_glist_combined_str (l, "ext_pic_viewer", gtk_entry_get_text (GTK_ENTRY (ent_ext_pic_viewer)));

   l = add_to_glist_combined_str (l, "def_file_ext", gtk_entry_get_text (GTK_ENTRY (ent_def_file_ext)));

   l = add_to_glist_combined_int (l, "exclusive_enter_at_dialogs", gtk_toggle_button_get_active (&cb_exclusive_enter_at_dialogs->toggle_button));

   l = add_to_glist_combined_int (l, "common_closing_button_visible", gtk_toggle_button_get_active (&cb_common_closing_button_visible->toggle_button));
   l = add_to_glist_combined_int (l, "do_crash_file", gtk_toggle_button_get_active (&cb_do_crash_file->toggle_button));
   l = add_to_glist_combined_int (l, "do_save_all_on_termination", gtk_toggle_button_get_active (&cb_do_save_all_on_termination->toggle_button));
   l = add_to_glist_combined_int (l, "det_charset_by_meta", gtk_toggle_button_get_active (&cb_det_charset_by_meta->toggle_button));
   l = add_to_glist_combined_int (l, "do_last_session", gtk_toggle_button_get_active (&cb_autorestore_last_session->toggle_button));
   l = add_to_glist_combined_int (l, "hl_current_line", gtk_toggle_button_get_active (&cb_hl_current_line->toggle_button));
   l = add_to_glist_combined_int (l, "do_autosave", gtk_toggle_button_get_active (&cb_do_autosave->toggle_button));
   l = add_to_glist_combined_int (l, "do_autosave_session", gtk_toggle_button_get_active (&cb_do_autosave_session->toggle_button));

   l = add_to_glist_combined_int (l, "autoclose_tags", gtk_toggle_button_get_active (&cb_autoclose_tags->toggle_button));
   l = add_to_glist_combined_int (l, "fullscreen", gtk_toggle_button_get_active (&cb_fullscreen->toggle_button));
   l = add_to_glist_combined_int (l, "notebook_tab_position", gtk_combo_box_get_active (tv_notetab_positions));
   l = add_to_glist_combined_int (l, "prompt_on_not_saved", gtk_toggle_button_get_active (&cb_prompt_on_not_saved->toggle_button));
   l = add_to_glist_combined_int (l, "prompt_on_file_exists", gtk_toggle_button_get_active (&cb_prompt_on_file_exists->toggle_button));

   l = add_to_glist_combined_int (l, "do_det_scripts_by_content", gtk_toggle_button_get_active (&cb_do_det_scripts_by_content->toggle_button));
   l = add_to_glist_combined_int (l, "do_show_main_toolbar", gtk_toggle_button_get_active (&cb_do_show_main_toolbar->toggle_button));
   l = add_to_glist_combined_int (l, "implane_ins_image", gtk_toggle_button_get_active (&cb_implane_ins_image->toggle_button));
   l = add_to_glist_combined_int (l, "use_cursor_blink", gtk_toggle_button_get_active (&cb_use_cursor_blink->toggle_button));

   l = add_to_glist_combined_int (l, "use_ext_image_viewer", gtk_toggle_button_get_active (&cb_use_ext_image_viewer->toggle_button));
   l = add_to_glist_combined_int (l, "automatic_markup_mode", gtk_toggle_button_get_active (&cb_automatic_markup_mode->toggle_button));

   l = add_to_glist_combined_int (l, "add_def_file_ext", gtk_toggle_button_get_active (&cb_add_def_file_ext->toggle_button));
   l = add_to_glist_combined_int (l, "homeend", gtk_toggle_button_get_active (&cb_homeend->toggle_button));
   l = add_to_glist_combined_int (l, "show_cursor_pos", gtk_toggle_button_get_active (&cb_show_cursor_pos->toggle_button));

   l = add_to_glist_combined_int (l, "do_backup", gtk_toggle_button_get_active (&cb_do_backup->toggle_button));

   l = add_to_glist_combined_int (l, "start_with_blank_file", gtk_toggle_button_get_active (&cb_start_with_blank_file->toggle_button));

   l = add_to_glist_combined_int (l, "use_def_doc_browser", gtk_toggle_button_get_active (&cb_use_def_doc_browser->toggle_button));

   l = add_to_glist_combined_int (l, "show_line_nums", gtk_toggle_button_get_active (&cb_show_line_nums->toggle_button));
   l = add_to_glist_combined_int (l, "word_wrap", gtk_toggle_button_get_active (&cb_word_wrap->toggle_button));
   l = add_to_glist_combined_int (l, "use_def_save_dir", gtk_toggle_button_get_active (&cb_use_def_save_dir->toggle_button));
   l = add_to_glist_combined_int (l, "use_def_open_dir", gtk_toggle_button_get_active (&cb_use_def_open_dir->toggle_button));
   l = add_to_glist_combined_int (l, "use_auto_indent", gtk_toggle_button_get_active (&cb_use_auto_indent->toggle_button));

   l = add_to_glist_combined_int (l, "ins_spaces_on_tab_press", gtk_toggle_button_get_active (&cb_ins_spaces_on_tab_press->toggle_button));
   l = add_to_glist_combined_int (l, "scan_for_links_on_doc_open", gtk_toggle_button_get_active (&cb_scan_for_links_on_doc_open->toggle_button));
   l = add_to_glist_combined_int (l, "main_wnd_show_full_path", gtk_toggle_button_get_active (&cb_main_wnd_show_full_path->toggle_button));

   l = add_to_glist_combined_int (l, "enc_use_fallback", gtk_toggle_button_get_active (&cb_enc_use_fallback->toggle_button));

   l = add_to_glist_combined_int (l, "enc_det_cp1251", gtk_toggle_button_get_active (&cb_enc_det_cp1251->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_koi8r", gtk_toggle_button_get_active (&cb_enc_det_koi8r->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_koi8u", gtk_toggle_button_get_active (&cb_enc_det_koi8u->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_866", gtk_toggle_button_get_active (&cb_enc_det_866->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_czech", gtk_toggle_button_get_active (&cb_enc_czech->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_finnish", gtk_toggle_button_get_active (&cb_enc_det_finnish->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_german", gtk_toggle_button_get_active (&cb_enc_det_german->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_serbian", gtk_toggle_button_get_active (&cb_enc_det_serbian->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_latvian", gtk_toggle_button_get_active (&cb_enc_det_latvian->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_polish", gtk_toggle_button_get_active (&cb_enc_det_polish->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_portuguese", gtk_toggle_button_get_active (&cb_enc_det_portuguese->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_slovak", gtk_toggle_button_get_active (&cb_enc_det_slovak->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_slovenian", gtk_toggle_button_get_active (&cb_enc_det_slovenian->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_spanish", gtk_toggle_button_get_active (&cb_enc_det_spanish->toggle_button));
   l = add_to_glist_combined_int (l, "enc_det_turkish", gtk_toggle_button_get_active (&cb_enc_det_turkish->toggle_button));
   l = add_to_glist_combined_int (l, "show_hidden_files", gtk_toggle_button_get_active (&cb_show_hidden_files->toggle_button));
   l = add_to_glist_combined_int (l, "do_autorep", gtk_toggle_button_get_active (&cb_do_autorep->toggle_button));

   l = add_to_glist_combined_int (l, "autosave_interval", (int) gtk_range_get_value (GTK_RANGE (sb_autosave_interval)));
   l = add_to_glist_combined_int (l, "logmemo_lines_max", (int) gtk_range_get_value (GTK_RANGE (sb_logmemo_lines_max)));
   l = add_to_glist_combined_int (l, "max_recent_items", (int) gtk_range_get_value (GTK_RANGE (sb_max_recent_items)));

   l = add_to_glist_combined_int (l, "thumb_width", (int) gtk_range_get_value (GTK_RANGE (sb_thumb_width)));
   l = add_to_glist_combined_int (l, "thumb_height", (int) gtk_range_get_value (GTK_RANGE (sb_thumb_height)));

   l = add_to_glist_combined_int (l, "saveopen_w", (int) gtk_range_get_value (GTK_RANGE (sb_saveopen_w)));
   l = add_to_glist_combined_int (l, "saveopen_h", (int) gtk_range_get_value (GTK_RANGE (sb_saveopen_h)));
   l = add_to_glist_combined_int (l, "tab_size", (int) gtk_range_get_value (GTK_RANGE (sb_tab_size)));

   l = add_to_glist_combined_int (l, "jpeg_q", (int) gtk_range_get_value (GTK_RANGE (sb_jpeg_q)));
   l = add_to_glist_combined_int (l, "max_undo_levels", (int) gtk_range_get_value (GTK_RANGE (sb_max_undo_levels)));

   
   glist_save_to_file (l, confile.tea_rc);
   glist_strings_free (l);

   confile_reload ();

   ui_update ();
   doc_update_all ();

   update_enc_menu();

   if (get_page_text ())
      markup_change_by_ext (cur_text_doc->file_name_utf8);
}


static void cb_on_add_enc (GtkButton *button,
                           gpointer user_data)
{
  gchar *s = request_item_from_tvlist (gl_enc_available);
  if (! s)
     return;

  gl_enc_choosed = g_list_prepend (gl_enc_choosed, s);
  tv_fill_with_glist (tv_choosed_charsets, gl_enc_choosed);

  glist_save_to_file (gl_enc_choosed, confile.tea_choosed_charsets);
  get_iconv_sup ();
}


static GtkWidget* wnd_options_encoding (void)
{
  GtkWidget *enc_tabs = gtk_notebook_new ();
  gtk_widget_show (enc_tabs);

  GtkWidget *wnd = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (wnd);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  //GtkWidget *page = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
    
  gtk_widget_show (page);
  gtk_scrolled_window_add_with_viewport (wnd, page);

  GtkWidget *fr1 = gtk_frame_new (_("Defaults"));
  gtk_widget_show (fr1);
  gtk_box_pack_start (page, fr1, FALSE, FALSE, UI_PACKFACTOR);

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (box);
  gtk_container_add (fr1, box);

  bt_default_charset = tea_button_list_items (box, confile.iconv_encs, _("Default charset for File open"), confile.default_charset);
  bt_def_filesave_charset = tea_button_list_items (box, confile.gl_save_charsets, _("Default charset for File save as"), confile.def_filesave_charset);
  bt_rtf_enc = tea_button_list_items (box, gl_enc_available, _("Default charset for RTF"), confile.rtf_enc);

  fr1 = gtk_frame_new (_("Misc"));
  gtk_widget_show (fr1);
  gtk_box_pack_start (box, fr1, FALSE, FALSE, UI_PACKFACTOR);

  box = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (box);
  gtk_container_add (fr1, box);

  cb_det_charset_by_meta = tea_checkbox (box, _("Detect a charset by the [X]HTML meta header"), confile.det_charset_by_meta);
  cb_enc_use_fallback = tea_checkbox (box, _("On autodetect failed, fallback to this charset:"), confile.enc_use_fallback);
  bt_fallback_charset = tea_button_list_items (box, gl_enc_available, _("Fallback charset"), confile.fallback_charset);

  GtkWidget *fr2 = gtk_frame_new (_("Autodetect"));
  gtk_widget_show (fr2);

  gtk_box_pack_start (page, fr2, FALSE, FALSE, UI_PACKFACTOR);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (box);
  gtk_container_add (fr2, box);

  cb_enc_det_cp1251 = tea_checkbox (box, _("CP 1251"), confile.enc_det_cp1251);
  cb_enc_det_koi8r = tea_checkbox (box, _("KOI8-R"), confile.enc_det_koi8r);
  cb_enc_det_koi8u = tea_checkbox (box, _("KOI8-U"), confile.enc_det_koi8u);
  cb_enc_det_866 = tea_checkbox (box, _("DOS 866"), confile.enc_det_866);
  cb_enc_czech = tea_checkbox (box, _("Czech"), confile.enc_det_czech);
  cb_enc_det_finnish = tea_checkbox (box, _("Finnish"), confile.enc_det_finnish);
  cb_enc_det_german = tea_checkbox (box, _("German"), confile.enc_det_german);
  cb_enc_det_serbian = tea_checkbox (box, _("Serbian"), confile.enc_det_serbian);
  cb_enc_det_latvian = tea_checkbox (box, _("Latvian"), confile.enc_det_latvian);
  cb_enc_det_polish = tea_checkbox (box, _("Polish"), confile.enc_det_polish);
  cb_enc_det_portuguese = tea_checkbox (box, _("Portuguese"), confile.enc_det_portuguese);
  cb_enc_det_slovak = tea_checkbox (box, _("Slovak"), confile.enc_det_slovak);
  cb_enc_det_slovenian = tea_checkbox (box, _("Slovenian"), confile.enc_det_slovenian);
  cb_enc_det_spanish = tea_checkbox (box, _("Spanish"), confile.enc_det_spanish);
  cb_enc_det_turkish = tea_checkbox (box, _("Turkish"), confile.enc_det_turkish);

  gtk_notebook_append_page (enc_tabs, wnd, gtk_label_new (_("Management")));

  GtkWidget *hb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 

  gtk_widget_show (hb);

  GtkWidget *vbuttons = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_box_pack_start (hb, vbuttons, FALSE, FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbuttons);

  GtkWidget *b1 = gtk_button_new_from_stock (GTK_STOCK_ADD);
  GtkWidget *b2 = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_widget_show (b1);
  gtk_widget_show (b2);

  gtk_box_pack_start (vbuttons, b1, FALSE, FALSE, UI_PACKFACTOR);
  gtk_box_pack_start (vbuttons, b2, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect (b1, "clicked", G_CALLBACK (cb_on_add_enc), NULL);
  g_signal_connect (b2, "clicked", G_CALLBACK (cb_on_del_enc), NULL);

  tv_choosed_charsets = tv_create_framed (hb, _("Selected"), GTK_SELECTION_SINGLE);
  tv_fill_with_glist (tv_choosed_charsets, gl_enc_choosed);

  gtk_notebook_append_page (enc_tabs, hb, gtk_label_new (_("Configure charsets")));

  return enc_tabs;
}


static GtkWidget* wnd_options_commands (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  GtkWidget *fr_misc = gtk_frame_new (_("Misc"));
  gtk_widget_show (fr_misc);
  gtk_container_add (page, fr_misc);

  GtkWidget *b = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_container_add (fr_misc, b);

  ent_ext_pic_editor = tea_text_entry (b, _("External image editor"), confile.ext_pic_editor);
  ent_ext_pic_viewer = tea_text_entry (b, _("External image viewer"), confile.ext_pic_viewer);
  ent_cm_mplayer = tea_text_entry (b, _("Run Mplayer with subtitles"), confile.cm_mplayer);
  ent_cm_pack = tea_text_entry (b, _("A command for Save packed"), confile.cm_pack);

  ent_cm_manual_browser = tea_text_entry (b, _("Browser for the manual"), confile.cmd_def_doc_browser);
  cb_use_def_doc_browser = tea_checkbox (b, _("Use this browser"), confile.use_def_doc_browser);

  GtkWidget *fr_latex = gtk_frame_new (_("LaTeX"));
  gtk_widget_show (fr_latex);
  gtk_container_add (page, fr_latex);

  b = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (b);
  gtk_container_add (fr_latex, b);

  ent_cm_run_latex = tea_text_entry (b, _("Process with LaTeX"), confile.cm_run_latex);
  ent_cm_pdflatex = tea_text_entry (b, _("Process with pdflatex"), confile.cm_pdflatex);
  ent_cm_dvi_to_pdf = tea_text_entry (b, _("dvi to PDF"), confile.cm_dvi_to_pdf);
  ent_cm_dvi_to_ps = tea_text_entry (b, _("dvi to PS"), confile.cm_dvi_to_ps);
  ent_cm_dvi_view = tea_text_entry (b, _("View dvi"), confile.cm_dvi_view);
  ent_cm_pdf_view = tea_text_entry (b, _("View PDF"), confile.cm_pdf_view);
  ent_cm_ps_view = tea_text_entry (b, _("View PS"), confile.cm_ps_view);

  return page;
}


static GtkWidget* wnd_options_functions (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  
  ent_now_playing = tea_text_entry (page, _("Now Playing string format"), confile.now_playing);
  ent_date_time = tea_text_entry (page, _("Date and time format"), confile.date_time);
  ent_date_time_ver = tea_text_entry (page, _("Date format for Save version"), confile.date_time_ver);

  ent_fmt_color_function = tea_text_entry (page, _("The format of the Color function"), confile.fmt_color_function);
  sb_jpeg_q = tea_scale_widget (page, _("JPEG compression (1..100)"), 1.0d, 100.0d, confile.jpeg_q);
//  sb_screenshot_delay = tea_spinbutton (page, _("Screenshot delay (in seconds)"), confile.screenshot_delay);

  return page;
}


static GtkWidget* wnd_options_interface (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  sb_saveopen_w = tea_scale_widget (page, _("Save/Open dialogs width per cents"), 1.0d, 100.0d, confile.saveopen_w);
  sb_saveopen_h = tea_scale_widget (page, _("Save/Open dialogs height per cents"), 1.0d, 100.0d, confile.saveopen_h);
  sb_thumb_width = tea_scale_widget (page, _("Image thumbnail width"), 16.0d, 320.0d, confile.thumb_width);
  sb_thumb_height = tea_scale_widget (page, _("Image thumbnail height"), 16.0d, 256.0d, confile.thumb_height);

  cb_exclusive_enter_at_dialogs = tea_checkbox (page, _("Exclusive \"Enter\" mode at save/open dialogs"), confile.exclusive_enter_at_dialogs);
  cb_main_wnd_show_full_path = tea_checkbox (page, _("Show the full path in the window caption"), confile.main_wnd_show_full_path);
  cb_do_show_main_toolbar = tea_checkbox (page, _("Show the main toolbar"), confile.do_show_main_toolbar);
  cb_common_closing_button_visible = tea_checkbox (page, _("Show the common close button"), confile.common_closing_button_visible);
  cb_show_cursor_pos = tea_checkbox (page, _("Show the cursor position at statusbar"), confile.show_cursor_pos);

  cb_fullscreen = tea_checkbox (page, _("Start in the full-screen mode"), confile.fullscreen);

  tv_notetab_positions = tea_combo_combo (page, gl_notetab_positions, _("Tabs position"), confile.notebook_tab_position, NULL, NULL);

  sb_max_recent_items = tea_scale_widget (page, _("Max. recent files"), 1.0d, 100.0d, confile.max_recent_items);
  sb_logmemo_lines_max = tea_scale_widget (page, _("Logmemo lines limit"), 1.0d, 4096.0d, confile.logmemo_lines_max);

  //sb_fte_min_prefix_len = tea_spinbutton (page, _("Famous text entry autocompletion min. prefix, in characters"), confile.fte_min_prefix_len);

  return page;
}


static GtkWidget* wnd_options_switches (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  cb_automatic_markup_mode = tea_checkbox (page, _("Automatic markup mode"), confile.automatic_markup_mode);

  cb_do_crash_file = tea_checkbox (page, _("Save the crashfile on terminate"), confile.do_crash_file);
  cb_do_save_all_on_termination = tea_checkbox (page, _("Save all on the abnormal termination (SIGTERM, etc)"), confile.do_save_all_on_termination);

  cb_autorestore_last_session = tea_checkbox (page, _("Restore the last session on start"), confile.do_last_session);
  cb_start_with_blank_file = tea_checkbox (page, _("Start TEA with a blank file"), confile.start_with_blank_file);

  cb_do_det_scripts_by_content = tea_checkbox (page, _("Determine a scripts highlighting by the content"), confile.do_det_scripts_by_content);

  gtk_widget_hide (cb_do_det_scripts_by_content);

  cb_show_hidden_files = tea_checkbox (page, _("Show hidden files in Kwas"), confile.show_hidden_files);
  cb_use_ext_image_viewer = tea_checkbox (page, _("Use external image viewer"), confile.use_ext_image_viewer);
  cb_do_backup = tea_checkbox (page, _("Do backup"), confile.do_backup);
  cb_show_line_nums = tea_checkbox (page, _("Show line numbers"), confile.show_line_nums);
  cb_word_wrap = tea_checkbox (page, _("Word wrap"), confile.word_wrap);
  cb_scan_for_links_on_doc_open = tea_checkbox (page, _("Scan for links on file open"), confile.scan_for_links_on_doc_open);
  cb_implane_ins_image = tea_checkbox (page, _("Imageplane insert tags by default"), confile.implane_ins_image);

  return page;
}


static GtkWidget* wnd_options_confirmations (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  cb_prompt_on_not_saved = tea_checkbox (page, _("Warn before closing a modified file"), confile.prompt_on_not_saved);
  cb_prompt_on_file_exists = tea_checkbox (page, _("Overwrite the existing file"), confile.prompt_on_file_exists);

  return page;
}


static void cb_backup_config (GtkButton *button,
                              gpointer user_data)
{
  gchar *f = g_strconcat (confile.tea_rc, ".bak", NULL);
  copy_file (confile.tea_rc, f);
  g_free (f);
}


static void cb_change_all_fonts (GtkButton *button,
                                 gpointer user_data)
{
  GtkFontSelectionDialog *w = gtk_font_selection_dialog_new (_("Please select the font"));
  gchar *font_name;

  if (gtk_dialog_run (GTK_DIALOG (w)) == GTK_RESPONSE_OK)
    {
     font_name = gtk_font_selection_dialog_get_font_name (w);
     if (font_name)
       {
        gtk_font_button_set_font_name (fb_tag_comment_font, font_name);
        gtk_font_button_set_font_name (fb_tag_identifier_font, font_name);
        gtk_font_button_set_font_name (fb_tag_digit_font, font_name);
        gtk_font_button_set_font_name (fb_tag_string_font, font_name);
        gtk_font_button_set_font_name (fb_tag_html_tag_font, font_name);
        gtk_font_button_set_font_name (fb_tag_preprocessor_font, font_name);
        gtk_font_button_set_font_name (fb_tag_type_font, font_name);
        gtk_font_button_set_font_name (fb_editor_font, font_name);
        g_free (font_name);
       }
     }

  gtk_widget_destroy (w);
}


static void cb_restore_config (GtkButton *button,
                               gpointer user_data)
{
  gchar *f = g_strconcat (confile.tea_rc, ".bak", NULL);
  copy_file (f, confile.tea_rc);
  g_free (f);
  confile_reload ();
  doc_update_all();
  update_enc_menu();
  dlg_info (wnd_options, _("Configuration restored, please close and reopen the Preferences window."), NULL);
}


static void cb_config_reset (GtkButton *button,
                             gpointer user_data)
{
   if (unlink (confile.tea_rc) == 0 )
      {
       confile_reload ();
       doc_update_all();
       update_enc_menu();
       dlg_info (wnd_options, _("Configuration is reseted to default, please close and reopen the Preferences window."), NULL);
      }
}


static void cb_template_remove (GtkButton *button,
                                gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = file_combine_path (confile.templates_dir, t);
  if (! dlg_question (_("Confirmation!"), _("Are you sure to delete %s?"), t))
     {
      g_free (t);
      g_free (filename);
      return;
     }

  unlink (filename);
  g_free (t);
  g_free (filename);
  reload_templates ();
  GList *l = read_dir_files (confile.templates_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_table_rename (GtkButton *button,
                             gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = dlg_question_char (_("Rename as:"), t);
  if (! filename)
     {
      g_free (t);
      return;
     }

  gchar *dest = file_combine_path (confile.tables_dir, filename);
  gchar *source = file_combine_path (confile.tables_dir, t);

  rename (source, dest);

  g_free (t);
  g_free (filename);
  g_free (dest);
  g_free (source);
  reload_reptables ();
  GList *l = read_dir_files (confile.tables_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_table_remove (GtkButton *button,
                             gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = file_combine_path (confile.tables_dir, t);
  if (! dlg_question (_("Confirmation!"), _("Are you sure to delete %s?"), t))
     {
      g_free (t);
      g_free (filename);
      return;
     }

  unlink (filename);
  g_free (t);
  g_free (filename);
  reload_reptables ();
  GList *l = read_dir_files (confile.tables_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_template_rename (GtkButton *button,
                                gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = dlg_question_char (_("Rename as:"), t);
  if (! filename)
     {
      g_free (t);
      return;
     }

  gchar *dest = file_combine_path (confile.templates_dir, filename);
  gchar *source = file_combine_path (confile.templates_dir, t);

  rename (source, dest);

  g_free (t);
  g_free (filename);
  g_free (dest);
  g_free (source);
  reload_templates ();
  GList *l = read_dir_files (confile.templates_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_snippet_remove (GtkButton *button,
                               gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = file_combine_path (confile.snippets_dir, t);
  if (! dlg_question (_("Confirmation!"), _("Are you sure to delete %s?"), t))
     {
      g_free (t);
      g_free (filename);
      return;
     }

  unlink (filename);
  g_free (t);
  g_free (filename);
  reload_snippets ();
  GList *l = read_dir_files (confile.snippets_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_snippet_rename (GtkButton *button,
                               gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = dlg_question_char (_("Rename as:"), t);
  if (! filename)
     {
      g_free (t);
      return;
      }

  gchar *dest = file_combine_path (confile.snippets_dir, filename);
  gchar *source = file_combine_path (confile.snippets_dir, t);

  rename (source, dest);

  g_free (t);
  g_free (filename);
  g_free (dest);
  g_free (source);
  reload_snippets ();
  GList *l = read_dir_files (confile.snippets_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_script_remove (GtkButton *button,
                              gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = file_combine_path (confile.scripts_dir, t);
  if (! dlg_question (_("Confirmation!"), _("Are you sure to delete %s?"), t))
     {
      g_free (t);
      g_free (filename);
      return;
     }

  unlink (filename);
  g_free (t);
  g_free (filename);
  reload_scripts ();
  GList *l = read_dir_files (confile.scripts_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_script_rename (GtkButton *button,
                               gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = dlg_question_char (_("Rename as:"), t);
  if (! filename)
     {
      g_free (t);
      return;
      }

  gchar *dest = file_combine_path (confile.scripts_dir, filename);
  gchar *source = file_combine_path (confile.scripts_dir, t);

  rename (source, dest);

  g_free (t);
  g_free (filename);
  g_free (dest);
  g_free (source);
  reload_scripts ();
  GList *l = read_dir_files (confile.scripts_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static void cb_script_add (GtkButton *button,
                           gpointer user_data)
{
  GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Select a file:"),
                                                   tea_main_window,
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_window_resize (GTK_WINDOW (dialog), get_value (confile.screen_w, confile.saveopen_w), get_value (confile.screen_h, confile.saveopen_h));
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT)
     {
      gtk_widget_destroy (dialog);
      return;
     }

  gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  gtk_widget_destroy (dialog);
  if (! filename)
     return;

  gchar *dest = file_replace_path (filename, confile.scripts_dir);
  copy_file (filename, dest);

  g_free (filename);
  g_free (dest);
  reload_scripts ();
  GList *l = read_dir_files (confile.scripts_dir);
  tv_fill_with_glist (user_data, l);
  g_free (filename);
  glist_strings_free (l);
}


static void cb_session_remove (GtkButton *button,
                               gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = file_combine_path (confile.sessions_dir, t);
  if (! dlg_question (_("Confirmation!"), _("Are you sure to delete %s?"), t))
     {
      g_free (t);
      g_free (filename);
      return;
     }

  unlink (filename);
  g_free (t);
  g_free (filename);
  reload_sessions ();
  GList *l = read_dir_files (confile.sessions_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


/*
static void cb_del_fteautocomp (GtkButton *button,
                               gpointer user_data)

{
  unlink (confile.fam_autocomp_file);
}
*/

static void cb_session_rename (GtkButton *button,
                              gpointer user_data)
{
  gchar *t = tv_get_selected_single (user_data);
  if (! t)
     return;

  gchar *filename = dlg_question_char (_("Rename as:"), t);
  if (! filename)
     {
      g_free (t);
      return;
     }

  gchar *dest = file_combine_path (confile.sessions_dir, filename);
  gchar *source = file_combine_path (confile.sessions_dir, t);

  rename (source, dest);

  g_free (t);
  g_free (filename);
  g_free (dest);
  g_free (source);
  reload_sessions ();
  GList *l = read_dir_files (confile.sessions_dir);
  tv_fill_with_glist (user_data, l);
  glist_strings_free (l);
}


static GtkWidget* wnd_options_maintenance (void)
{
  GtkWidget *notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);

  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Functions")));

  GtkWidget *b = tea_button_at_box (page, _("Backup the current config" ), cb_backup_config, NULL);
  b = tea_button_at_box (page, _("Config restore" ), cb_restore_config, NULL);
  b = tea_button_at_box (page, _("Reset config to default" ), cb_config_reset, NULL);
  b = tea_button_at_box (page, _("Put the launcher to the desktop" ), cb_put_launcher, NULL);
  b = tea_button_at_box (page, _("Remove the launcher from the desktop" ), cb_del_launcher, NULL);
  //b = tea_button_at_box (page, _("Clear the Famous text entry history" ), cb_del_fteautocomp, NULL);

  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Templates")));

  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbox);
  gtk_box_pack_start (page, vbox, TRUE, TRUE, UI_PACKFACTOR);

  GtkTreeView *tv_templates = tv_create_framed (vbox, _("Templates management"), GTK_SELECTION_SINGLE);
  GList *l = read_dir_files (confile.templates_dir);
  tv_fill_with_glist (tv_templates, l);
  glist_strings_free (l);

  b = tea_button_at_box (vbox, _("Remove"), cb_template_remove, tv_templates);
  b = tea_button_at_box (vbox, _("Rename"), cb_template_rename, tv_templates);
///////
  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Snippets")));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbox);
  gtk_box_pack_start (page, vbox, TRUE, TRUE, UI_PACKFACTOR);

  GtkTreeView *tv_snippets = tv_create_framed (vbox, _("Snippets management"), GTK_SELECTION_SINGLE);
  l = read_dir_files (confile.snippets_dir);
  tv_fill_with_glist (tv_snippets, l);
  glist_strings_free (l);

  b = tea_button_at_box (vbox, _("Remove"), cb_snippet_remove, tv_snippets);
  b = tea_button_at_box (vbox, _("Rename"), cb_snippet_rename, tv_snippets);

  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Sessions")));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbox);
  gtk_box_pack_start (page, vbox, TRUE, TRUE, UI_PACKFACTOR);

  GtkTreeView *tv_sessions = tv_create_framed (vbox, _("Sessions management"), GTK_SELECTION_SINGLE);
  l = read_dir_files (confile.sessions_dir);
  tv_fill_with_glist (tv_sessions, l);
  glist_strings_free (l);

  b = tea_button_at_box (vbox, _("Remove"), cb_session_remove, tv_sessions);
  b = tea_button_at_box (vbox, _("Rename"), cb_session_rename, tv_sessions);

  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Scripts")));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbox);
  gtk_box_pack_start (page, vbox, TRUE, TRUE, UI_PACKFACTOR);

  GtkTreeView *tv_scripts = tv_create_framed (vbox, _("Scripts management"), GTK_SELECTION_SINGLE);
  l = read_dir_files (confile.scripts_dir);
  tv_fill_with_glist (tv_scripts, l);
  glist_strings_free (l);

  b = tea_button_at_box (vbox, _("Add"), cb_script_add, tv_scripts);
  b = tea_button_at_box (vbox, _("Remove"), cb_script_remove, tv_scripts);
  b = tea_button_at_box (vbox, _("Rename"), cb_script_rename, tv_scripts);

  page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Tables")));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbox);
  gtk_box_pack_start (page, vbox, TRUE, TRUE, UI_PACKFACTOR);

  GtkTreeView *tv_tables = tv_create_framed (vbox, _("Replacement tables management"), GTK_SELECTION_SINGLE);
  l = read_dir_files (confile.tables_dir);
  tv_fill_with_glist (tv_tables, l);
  glist_strings_free (l);

  b = tea_button_at_box (vbox, _("Remove"), cb_table_remove, tv_tables);
  b = tea_button_at_box (vbox, _("Rename"), cb_table_rename, tv_tables);

  return notebook ;
}


static GtkWidget* wnd_options_colors (void)
{
  GtkWidget *notebook = gtk_notebook_new();
  gtk_widget_show (notebook);

  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Editor")));

  ent_text_foreground = tea_color_selector (page, _("Text foreground"), confile.text_foreground);
  ent_text_background = tea_color_selector (page, _("Text background"), confile.text_background);
  ent_text_sel_foreground = tea_color_selector (page, _("Selected foreground"), confile.text_sel_foreground);
  ent_text_sel_background = tea_color_selector (page, _("Selected background"), confile.text_sel_background);

  ent_tag_color_paint_fg = tea_color_selector (page, _("Marking function foreground"), confile.tag_color_paint_fg);
  ent_tag_color_paint_bg = tea_color_selector (page, _("Marking function background"), confile.tag_color_paint_bg);

  ent_tag_current_line_fg = tea_color_selector (page, _("Current line foreground"), confile.tag_current_line_fg);
  ent_tag_current_line_bg = tea_color_selector (page, _("Current line background"), confile.tag_current_line_bg);

  ent_tag_spellcheck = tea_color_selector (page, _("Spellchecker marks"), confile.tag_spellcheck);

  //REMOVE THAT
/*
  page = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("Highlighting")));

  ent_tag_comment = tea_color_selector (page, _("Comments"), confile.tag_comment);
  ent_tag_identifier = tea_color_selector (page, _("Identifiers"), confile.tag_identifier);
  ent_tag_digit = tea_color_selector (page, _("Digits"), confile.tag_digit);
  ent_tag_string = tea_color_selector (page, _("Strings"), confile.tag_string);
  ent_tag_html_tag = tea_color_selector (page, _("HTML tags"), confile.tag_html_tag);
  ent_tag_preprocessor = tea_color_selector (page, _("Preprocessor"), confile.tag_preprocessor);
  ent_tag_type = tea_color_selector (page, _("Types"), confile.tag_type);

  gtk_widget_hide (page);
*/
//
  page = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (page);
  gtk_notebook_append_page (notebook, page, gtk_label_new (_("IDE")));

  ent_tag_color_lm_error_fg = tea_color_selector (page, _("Errors foreground (logmemo)"), confile.tag_color_lm_error_fg);
  ent_tag_color_lm_error_bg = tea_color_selector (page, _("Errors background (logmemo)"), confile.tag_color_lm_error_bg);

  return notebook ;
}

/*
static GtkWidget* wnd_options_fonts (void)
{
  GtkWidget *page = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (page);

  fb_tag_comment_font = tea_font_button (page, _("Comments font"), confile.tag_comment_font);
  fb_tag_identifier_font = tea_font_button (page, _("Identifiers font"), confile.tag_identifier_font);
  fb_tag_digit_font = tea_font_button (page, _("Digits font"), confile.tag_digit_font);
  fb_tag_string_font = tea_font_button (page, _("Strings font"), confile.tag_string_font);
  fb_tag_html_tag_font = tea_font_button (page, _("HTML tags font"), confile.tag_html_tag_font);
  fb_tag_preprocessor_font = tea_font_button (page, _("Preprocessor font"), confile.tag_preprocessor_font);
  fb_tag_type_font = tea_font_button (page, _("Types font"), confile.tag_type_font);

  GtkButton *bt_change_all_fonts = tea_button_at_box (page, _("Change all fonts"), cb_change_all_fonts, NULL);

  return page;
}
*/

static GtkWidget* wnd_options_editor (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  fb_editor_font = tea_font_button (page, _("Font"), confile.editor_font);


  sb_max_undo_levels = tea_scale_widget (page, _("Max undo levels"), 1.0d, 64.0d, confile.max_undo_levels);

  sb_tab_size = tea_scale_widget (page, _("Tab size in spaces"), 1.0d, 32.0d, confile.tab_size);
  cb_homeend = tea_checkbox (page, _("Smart Home/End"), confile.homeend);

  cb_ins_spaces_on_tab_press = tea_checkbox (page, _("Insert spaces on TAB key"), confile.ins_spaces_on_tab_press);
  cb_use_auto_indent = tea_checkbox (page, _("Automatic indent"), confile.use_auto_indent);
  cb_do_autorep = tea_checkbox (page, _("Autoreplace"), confile.do_autorep);
  cb_autoclose_tags = tea_checkbox (page, _("Autoclose tags"), confile.autoclose_tags);
  cb_use_cursor_blink = tea_checkbox (page, _("Cursor blink"), confile.use_cursor_blink);
  sb_autosave_interval = tea_scale_widget (page, _("Autosaving interval, minutes"), 1.0d, 60.0d, confile.autosave_interval);
  cb_do_autosave = tea_checkbox (page, _("Enable the autosaving"), confile.do_autosave);
  cb_do_autosave_session = tea_checkbox (page, _("Autosave a session on each interval"), confile.do_autosave_session);

  cb_hl_current_line = tea_checkbox (page, _("Highlight the current line"), confile.hl_current_line);

  cb_margin_visible = tea_checkbox (page, _("Show the margin"), confile.margin_visible);
  sb_margin_width = tea_scale_widget (page, _("Position of the margin (GtkSourceView-only)"), 24.0d, 96.0d, confile.margin_width);


  return page;
}


static GtkWidget* wnd_options_paths (void)
{
  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);
  ent_def_save_dir = tea_dir_selector (page, _("Default file saving directory"), confile.def_save_dir);
  cb_use_def_save_dir = tea_checkbox (page, _("Use default saving directory"), confile.use_def_save_dir);
  ent_def_open_dir = tea_dir_selector (page, _("Default file opening directory"), confile.def_open_dir);
  cb_use_def_open_dir = tea_checkbox (page, _("Use default file opening directory"), confile.use_def_open_dir);

  cb_add_def_file_ext = tea_checkbox (page, _("Add this extension to the file which is *Saving as*"), confile.add_def_file_ext);
  ent_def_file_ext = tea_text_entry (page, _("(<i>for example, txt, html, etc</i>)"), confile.def_file_ext);

  return page;
}


static void cb_menuitem_ch_cursor (GtkTreeView *tree_view, gpointer user_data)
{
  gchar *sel = tv_get_selected_single (tree_view);

  if (! sel)
     return;

  gchar *value = g_hash_table_lookup (ht_tea_hotkeys, sel);
  if (value)
    gtk_entry_set_text (ent_shortcut, value);
  else
      gtk_entry_set_text (ent_shortcut, "");

 g_free (sel);
}


static void shortcut_remove_by_menuitem_name (const gchar *name, gboolean flag)
{
  if (! name)
     return;

  gchar *value = g_hash_table_lookup (ht_tea_hotkeys, name);

  if (value)
     {
      GtkWidget *w = find_menuitem (GTK_CONTAINER (menubar1), name);
      if (! w)
         return;

      gchar *key;
      gchar *t = g_strrstr (value, " ");
      if (! t)
         key = g_strdup (value);
      else
         key = g_strchug (g_strdup (t));

      gtk_widget_remove_accelerator (w, accel_group,
                                     gdk_keyval_from_name (key),
                                     string_to_mod (value));

      g_free (key);

      if (flag)
         gtk_entry_set_text (ent_shortcut, "");

      g_hash_table_remove (ht_tea_hotkeys, name);

      save_ht_to_file (ht_tea_hotkeys, confile.tea_hotkeys);
     }
}


static void cb_shortcut_remove (GtkButton *button,
                                gpointer user_data)
{
  gchar *x = tv_get_selected_single (tv_menuitems);
  if (! x)
     return;
  shortcut_remove_by_menuitem_name (x, TRUE);
  g_free (x);
}


static gchar *found_key;

static gboolean cb_find_by_val (gpointer key, gpointer value, gpointer user_data)
{
  if ((g_utf8_collate (value, user_data) == 0))
     {
      found_key = key;
      return TRUE;
     }
  return FALSE;
}


static void cb_shortcut_assign (GtkButton *button,
                                gpointer user_data)
{
  gchar *et = gtk_entry_get_text (ent_shortcut);

  if (! et || strlen (et) < 1)
     return;

  gchar *keyname = tv_get_selected_single (tv_menuitems);
  if (! keyname)
      return;

  shortcut_remove_by_menuitem_name (keyname, FALSE);

  found_key = NULL;
  gchar *shortcut = g_utf8_strup (et, -1);

  g_hash_table_foreach (ht_tea_hotkeys, cb_find_by_val, shortcut);

  if (found_key)
     shortcut_remove_by_menuitem_name (found_key, FALSE);

  GtkWidget *w = find_menuitem (GTK_CONTAINER (menubar1), keyname);
  if (! w)
     {
      g_free (keyname);
      return;
     }
  gchar *key;
  gchar *t = g_strrstr (shortcut, " ");
  if (! t)
     key = g_strdup (shortcut);
  else
      key = g_strchug (g_strdup (t));

  gtk_widget_add_accelerator (w, "activate", accel_group,
                              gdk_keyval_from_name (key), string_to_mod (shortcut),
                              GTK_ACCEL_VISIBLE);

  g_hash_table_insert (ht_tea_hotkeys, keyname, shortcut);

  save_ht_to_file (ht_tea_hotkeys, confile.tea_hotkeys);
  g_free (keyname);

}


static GtkWidget* wnd_options_keyboard (void)
{

  GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (page);

  GtkWidget *hb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, UI_PACKFACTOR); 
  gtk_widget_show (hb);

 // gtk_container_add (page, hb);
   gtk_box_pack_start (page, hb, TRUE, TRUE, UI_PACKFACTOR);

 // gtk_box_set_child_packing           (page, hb,
   //                                                      TRUE,
     //                                                    TRUE,
       //                                                  UI_PACKFACTOR,
         //                                                GTK_PACK_END);
  
  GtkWidget *vbleft = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbleft);

    
  gtk_container_add (hb, vbleft);

  GtkWidget *ent_qsearch = tea_text_entry (vbleft, _("Quick search"), NULL);

  tv_menuitems = tv_create_non_framed (vbleft, GTK_SELECTION_SINGLE);

 
  
  
  gtk_tree_view_set_search_entry (tv_menuitems, ent_qsearch);

  GList *l = get_menu_items ();

  tv_fill_with_glist (tv_menuitems, l);

  glist_strings_free (l);

  
  
  
  GtkWidget *vbright = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR); 
  gtk_widget_show (vbright);
  gtk_container_add (hb, vbright);

  ent_shortcut = tea_text_entry_hotkey (vbright, _("Shortcut"), NULL);

  GtkWidget *bt_assign = tea_button_at_box (vbright, _("Assign"), cb_shortcut_assign, NULL);
  GtkWidget *bt_remove = tea_button_at_box (vbright, _("Remove"), cb_shortcut_remove, ent_shortcut);

  g_signal_connect (G_OBJECT (tv_menuitems), "cursor-changed", G_CALLBACK (cb_menuitem_ch_cursor), ent_shortcut);

  return page;
}


GtkWidget* wnd_options_create (void)
{
  wnd_options = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (wnd_options), _("Options, I presume..."));

  //GtkWidget *vbox1 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, UI_PACKFACTOR);

  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (wnd_options), vbox1);

  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos (notebook, GTK_POS_LEFT);

  gtk_widget_show (notebook);

  
  gtk_box_pack_start (vbox1, notebook, TRUE, TRUE, UI_PACKFACTOR);
 
  gtk_notebook_append_page (notebook, wnd_options_switches(), gtk_label_new (_("Switches")));
  gtk_notebook_append_page (notebook, wnd_options_encoding(), gtk_label_new (_("Encoding")));
  
    
  
  gtk_notebook_append_page (notebook, wnd_options_functions(), gtk_label_new (_("Functions")));
  gtk_notebook_append_page (notebook, wnd_options_interface (), gtk_label_new (_("Interface")));
  gtk_notebook_append_page (notebook, wnd_options_editor(), gtk_label_new (_("Editor")));

  
  
  gtk_notebook_append_page (notebook, wnd_options_hl_schemes (), gtk_label_new (_("Highlighting")));

  gtk_notebook_append_page (notebook, wnd_options_colors (), gtk_label_new (_("Colors")));
  
  gtk_notebook_append_page (notebook, wnd_options_commands (), gtk_label_new (_("Commands")));

  gtk_notebook_append_page (notebook, wnd_options_paths (), gtk_label_new (_("Paths")));

  gtk_notebook_append_page (notebook, wnd_options_confirmations (), gtk_label_new (_("Confirmations")));

  gtk_notebook_append_page (notebook, wnd_options_maintenance (), gtk_label_new (_("Maintenance")));
  gtk_notebook_append_page (notebook, wnd_options_keyboard (), gtk_label_new (_("Keyboard")));

  
  gtk_notebook_set_current_page (notebook, confile.last_prefs_tab);

  g_signal_connect ((gpointer) notebook, "switch_page",
                    G_CALLBACK (on_kwas_notebook_switch_page),
                    NULL);

  
  GtkWidget *hbox1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL); 
//gtk_button_box_set_layout (hbox1, GTK_BUTTONBOX_SPREAD);
  
  
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (vbox1), hbox1);

  GtkWidget *bt_close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_widget_show (bt_close);
  gtk_container_add (GTK_CONTAINER (hbox1), bt_close);

//  g_signal_connect (bt_close, "clicked", G_CALLBACK (cb_on_bt_close), wnd_options);
  g_signal_connect_swapped (bt_close, "clicked", G_CALLBACK (cb_on_bt_close), wnd_options);

  
  GtkWidget *im_save = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_BUTTON);
  GtkWidget *im_apply = gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON);
  GtkWidget *hb_apply = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  GtkWidget *l_apply = gtk_label_new (_("Save and apply"));

  gtk_container_add (GTK_CONTAINER (hb_apply), im_save);
  gtk_container_add (GTK_CONTAINER (hb_apply), im_apply);
  gtk_container_add (GTK_CONTAINER (hb_apply), l_apply);
  gtk_widget_show_all (hb_apply);

  GtkWidget *bt_apply = gtk_button_new();
  gtk_container_add (GTK_CONTAINER (bt_apply), hb_apply);

  gtk_widget_show (bt_apply);
  gtk_container_add (GTK_CONTAINER (hbox1), bt_apply);

  g_signal_connect (bt_apply, "clicked", G_CALLBACK (cb_on_bt_apply), wnd_options);

  gtk_window_set_position (wnd_options, GTK_WIN_POS_CENTER);
  //gtk_window_set_modal (wnd_options, TRUE);

  g_signal_connect (G_OBJECT (wnd_options), "key_press_event", G_CALLBACK (win_key_handler), wnd_options);

  
  gtk_widget_show (wnd_options);
  
  
  gtk_window_resize (wnd_options, 800, 600);
  
  return wnd_options;
}
