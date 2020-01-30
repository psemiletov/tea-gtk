/***************************************************************************
                          tea_config.h  -  description
                             -------------------
    begin                : Fri Dec 19 2003
    copyright            : 2003-2013 by Peter Semiletov
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

#ifndef TEA_CONFIG_H
#define TEA_CONFIG_H

typedef gboolean (ExitFunc) (GtkWidget *widget, GdkEventAny *event, gpointer data);


typedef struct
{

  gint jpeg_q;
  gint screenshot_delay;

  gchar *def_spell_lang;
  gchar *def_color_id;

  gchar *cm_pdflatex;
  gchar *cm_dvi_to_pdf;
  gchar *cm_dvi_to_ps;
  gchar *cm_dvi_view;
  gchar *cm_pdf_view;
  gchar *cm_ps_view;
  gchar *cm_run_latex;
  gchar *last_version;

  gchar *dir_thumbs;
  gchar *date_time;
  gchar *date_time_ver;

  gint margin_width;
  gboolean margin_visible;

  gboolean add_def_file_ext;
  gchar *def_file_ext;

  gboolean exclusive_enter_at_dialogs;


  //confirmations
  gboolean prompt_on_not_saved;
  gboolean prompt_on_file_exists;
  //paths

  gboolean common_closing_button_visible;
  gboolean show_line_markers;
  gboolean automatic_markup_mode;

  gchar *cm_pack;


  gchar *ext_pic_editor;
  gchar *ext_pic_viewer;

  gchar *tea_main_dir;
  gchar *templates_dir;
  gchar *snippets_dir;
  gchar *scripts_dir;
  gchar *tables_dir;
  gchar *profiles_dir;

  gchar *crapbook_file;
  gchar *tmp_dir;
  gchar *tmp_file;
  gchar *ext_programs;

  gboolean homeend;

  gchar *current_movie;
  gchar *cm_mplayer;

  gchar *bmx_file;
  gchar *sessions_dir;
  gchar *tea_rc;
  gchar *tea_ui;

  gchar *tea_kwas_bookmarks;
  gchar *tea_autoreplace;
  gchar *tea_choosed_charsets;

  gchar *now_playing;
  gchar *default_markup_mode;
  gboolean implane_ins_image;
  gboolean autoclose_tags;

  gboolean hl_current_line;
  gboolean allow_multiple_instances;

  gboolean do_last_session;
  gboolean do_autosave_session;

  gchar *last_session;
  gchar *autosave_session;


  gchar *tea_recent;
  gchar *tea_hotkeys;
  //gchar *tea_hotkeys_old;

  gchar *def_save_dir;
  gchar *def_open_dir;

  gchar *fmt_color_function;

  //bro paths
  gchar *cmd_def_doc_browser;

  gint logmemo_lines_max;
  //gint fam_autocomp_lines_max;
//  gint fte_min_prefix_len;
//  gboolean fte_autocomp;

  gint saveopen_w;
  gint saveopen_h;

  gchar *text_foreground;
  gchar *text_background;
  gchar *text_sel_foreground;
  gchar *text_sel_background;

//colors

  gchar *tag_current_line_fg;
  gchar *tag_current_line_bg;

  gchar *tag_color_lm_error_fg;
  gchar *tag_color_lm_error_bg;

  gchar *tag_color_paint_fg;
  gchar *tag_color_paint_bg;

  gchar *color_cursor;

  gchar *tag_comment;
  gchar *tag_identifier;
  gchar *tag_digit;
  gchar *tag_string;
  gchar *tag_html_tag;
  gchar *tag_preprocessor;
  gchar *tag_type;
  gchar *tag_spellcheck;
  gchar *tag_comment_font;
  gchar *tag_identifier_font;
  gchar *tag_digit_font;
  gchar *tag_string_font;
  gchar *tag_html_tag_font;
  gchar *tag_preprocessor_font;
  gchar *tag_control_font;
  gchar *tag_type_font;

  gint tab_size;
  gint thumb_width;
  gint thumb_height;

  gchar *default_charset;
  gchar *def_filesave_charset;
  gchar *autosave_file;
  gchar *crash_file;
  //gchar *fam_autocomp_file;

  gboolean do_crash_file;

  gint last_prefs_tab;
  gboolean ins_spaces_on_tab_press;
  gboolean use_auto_indent;
  gboolean show_hidden_files;

  gboolean fullscreen;

  gboolean do_save_all_on_termination;
  gboolean show_cursor_pos;


  gint autosave_interval; //minutes
  gint screen_h;
  gint screen_w;

  gint max_undo_levels;

  gboolean start_with_blank_file;

  gboolean use_def_save_dir;
  gboolean use_def_open_dir;

  gboolean use_cursor_blink;

  gboolean det_charset_by_meta;


  //gboolean use_snippets;
  gboolean use_ext_image_viewer;

  gboolean do_autosave;
  //gboolean do_hl_on_fileopen;
  gboolean do_backup;
  gboolean do_autorep;
  gboolean do_show_main_toolbar;

  gboolean do_det_scripts_by_content;

 //pos and size of main window

  gint top;
  gint left;
  gint height;
  gint width;
 //--------------------------

  gint notebook_tab_position;

  gint max_recent_items;

  gchar *editor_font;
  gboolean main_wnd_show_full_path;
  gboolean show_line_nums;
  gboolean word_wrap;
 //

  gboolean scan_for_links_on_doc_open;
  gboolean use_def_doc_browser;
 //---------------

  gboolean enc_det_cp1251;
  gboolean enc_det_koi8r;
  gboolean enc_det_koi8u;
  gboolean enc_det_866;
  gboolean enc_det_finnish;
  gboolean enc_det_german;
  gboolean enc_det_serbian;
  gboolean enc_det_latvian;
  gboolean enc_det_polish;
  gboolean enc_det_portuguese;
  gboolean enc_det_slovak;
  gboolean enc_det_slovenian;
  gboolean enc_det_spanish;
  gboolean enc_det_turkish;
  gboolean enc_det_czech;

  gboolean enc_use_fallback;
  gboolean fallback_charset;


  gchar *rtf_enc;

  GList *gl_save_charsets;
  GList *iconv_encs;
  gint msg_counter;
  gint filter_index;

} tea_confile;


typedef struct
{
  gpointer a;
  gpointer b;
  gboolean c;
} t_ppair;


GHashTable *ht_tea_hotkeys;


//gchar* enc_guess_value;
tea_confile confile; //global config
//t_cur_settings cur_settings;

GList *gl_color_schemes;

GList *gl_tea_special_places;

GList *gl_enc_available;
GList *gl_enc_choosed;
GList *gl_markup_modes;
GList *gl_image_formats;
GList *gl_supported_hlangs;
GList *l_words;

GList *gl_notetab_positions;
GList *bookmarks;
GList *recent_list;
GList *gl_scripts;
GList *gl_tea_kwas_bookmarks;

GList *gl_autosave;

GHashTable *ht_ext_programs;
GHashTable *ht_entities;
GHashTable *ht_config;
GHashTable *ht_autoreplace;

//GtkTooltip* tooltips;
GList *gl_found_files;
gchar *temp_url;

gboolean b_topmost;

gboolean b_exit_flag;
gboolean ui_done_flag;


gint lm_dclick_mode;
gint def_mm;
gint g_argc;
char **g_argv;

void confile_reload (void);
void confile_free (void);

gchar* conf_get_char_value (GList *confdata, const gchar *key, const gchar *def);
gint conf_get_int_value (GList *confdata, const gchar *key, gint def);
gchar* conf_get_char_value_at_current (GList *confdata, const gchar *def);
gchar* conf_get_char_key_at_current (GList *confdata, const gchar *def);



GtkWidget* lookup_widget (GtkContainer *widget, const gchar *widget_name);

void ui_init (void);
void ui_done (void);

void get_iconv_sup (void);

void execute_recent_item (const gchar *item);

void update_enc_menu (void);
void reload_snippets (void);

void  hash_remove_cb (gpointer data);

void reload_usermenu (void);
void reload_sessions (void);
void reload_autoreplace (void);
void reload_autosave (void);
void reload_ext_programs (void);
void reload_scripts (void);
void put_tmp_file_string (const gchar *s);
gchar* get_tmp_file_string (void);
gchar* ht_get_char_value (const gchar *key, const gchar *def);
gint ht_get_int_value (const gchar *key, gint def);
void tea_start (void);
void bmx_reload (void);
void reload_dicts (void);
void reload_templates (void);
void reload_hotkeys (void);
void update_recent_list_menu (gboolean);
void tea_done (void);
void fill_entities (void);
void tabs_reload (void);
void markup_change_by_ext (const gchar *filename);
void reload_reptables (void);
void reload_profiles (void);

//GtkTreeModel* reload_completion_model (void);


gchar* get_default_shortcuts (void);
void hotkeys_initial_check (void);

#if ((GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >=10))

void printer_init (void);
void printer_done (void);

#endif

GList* get_color_scheme_ids (void);


#endif