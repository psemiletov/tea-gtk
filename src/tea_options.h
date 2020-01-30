#ifndef TEA_OPTIONS
#define TEA_OPTIONS

GtkWidget *wnd_options;

GtkWidget *page_fonts;

GSList *gsl_tags_list;
GList *gl_tags_list;

GtkCheckButton *cb_margin_visible;
GtkWidget *sb_margin_width;

//GtkSpinButton *sb_fte_min_prefix_len;

GtkWidget *sb_jpeg_q;

GtkColorButton *bt_tag_background;
GtkColorButton *bt_tag_foreground;

GtkCheckButton *cb_homeend;
GtkCheckButton *cb_automatic_markup_mode;

GtkCheckButton *cb_hl_italic;
GtkCheckButton *cb_hl_bold;
GtkCheckButton *cb_hl_underline;
GtkCheckButton *cb_hl_strikethrough;

GtkTreeView *tv_available_hlangs;
GtkTreeView *tv_hlang_elements;
GtkTreeView *tv_menuitems;


GtkWidget *cbm_color_cheme;


GtkEntry *ent_shortcut;


GtkEntry *ent_def_save_dir;
GtkEntry *ent_def_open_dir;


GtkEntry *ent_fmt_color_function;

GtkEntry *ent_cm_dvi_to_ps;
GtkEntry *ent_cm_dvi_view;
GtkEntry *ent_cm_pdf_view;
GtkEntry *ent_cm_ps_view;
GtkEntry *ent_cm_run_latex;
GtkEntry *ent_cm_dvi_to_pdf;

GtkEntry *ent_cm_pdflatex;

GtkEntry *ent_cm_manual_browser;

GtkEntry *ent_cm_mplayer;

GtkEntry *ent_cm_pack;


GtkEntry *ent_tag_color_lm_error_fg;
GtkEntry *ent_tag_color_lm_error_bg;


GtkEntry *ent_tag_comment;
GtkEntry *ent_tag_identifier;
GtkEntry *ent_tag_digit;
GtkEntry *ent_tag_string;
GtkEntry *ent_tag_html_tag;
GtkEntry *ent_tag_preprocessor;
GtkEntry *ent_tag_type;
GtkEntry *ent_tag_spellcheck;

GtkEntry *ent_text_foreground;
GtkEntry *ent_text_background;
GtkEntry *ent_text_sel_foreground;
GtkEntry *ent_text_sel_background;

GtkCheckButton *cb_add_def_file_ext;
GtkEntry *ent_def_file_ext;


GtkCheckButton *cb_exclusive_enter_at_dialogs;
GtkCheckButton *cb_autorestore_last_session;
GtkCheckButton *cb_det_charset_by_meta;
GtkCheckButton *cb_do_save_all_on_termination;
GtkCheckButton *cb_do_crash_file;
GtkCheckButton *cb_common_closing_button_visible;
GtkCheckButton *cb_hl_current_line;
GtkCheckButton *cb_use_autocomp;
GtkCheckButton *cb_autoclose_tags;
GtkCheckButton *cb_implane_ins_image;
GtkCheckButton * cb_use_cursor_blink;
GtkCheckButton *cb_use_snippets;
GtkCheckButton *cb_use_ext_image_viewer;
//GtkCheckButton *cb_do_hl_on_fileopen;
GtkCheckButton *cb_do_backup;
GtkCheckButton *cb_ins_spaces_on_tab_press;
GtkCheckButton *cb_do_autorep;
GtkCheckButton *cb_do_show_main_toolbar;
GtkCheckButton *cb_prompt_on_not_saved;
GtkCheckButton *cb_prompt_on_file_exists;
GtkCheckButton *cb_use_def_doc_browser;
GtkCheckButton *cb_do_det_scripts_by_content;
GtkCheckButton *cb_show_hidden_files;
GtkCheckButton *cb_show_line_nums;
GtkCheckButton *cb_word_wrap;
GtkCheckButton *cb_use_autocomp;
GtkCheckButton *cb_use_def_save_dir;
GtkCheckButton *cb_use_def_open_dir;
GtkCheckButton *cb_use_auto_indent;
GtkCheckButton *cb_fullscreen;
GtkCheckButton *cb_do_autosave;
GtkCheckButton *cb_do_autosave_session;
GtkCheckButton *cb_show_cursor_pos;
GtkCheckButton *cb_scan_for_links_on_doc_open;
GtkCheckButton *cb_main_wnd_show_full_path;
GtkCheckButton *cb_start_with_blank_file;

GtkWidget *sb_logmemo_lines_max;
GtkWidget *sb_famous_history_max;
GtkWidget *sb_max_recent_items;
GtkWidget *sb_autosave_interval;

//GtkWidget *sb_fam_autocomp_lines_max;
//GtkWidget *sb_fam_min_prefix_len;



GtkWidget *sb_max_undo_levels;
GtkWidget *sb_autocomp_wordlen_min;
GtkWidget *sb_autocomp_list_items_max;

GtkWidget *sb_thumb_width;
GtkWidget *sb_thumb_height;


GtkWidget *sb_saveopen_w;
GtkWidget *sb_saveopen_h;

GtkWidget *sb_tab_size;

GtkEntry *ent_date_time;
GtkEntry *ent_date_time_ver;

GtkEntry *ent_now_playing;

GtkEntry *ent_color_cursor;

GtkEntry *ent_tag_current_line_fg;
GtkEntry *ent_tag_current_line_bg;


GtkEntry *ent_tag_color_paint_fg;
GtkEntry *ent_tag_color_paint_bg;


GtkFontButton *fb_tag_comment_font;
GtkFontButton *fb_tag_identifier_font;
GtkFontButton *fb_tag_digit_font;
GtkFontButton *fb_tag_string_font;
GtkFontButton *fb_tag_html_tag_font;
GtkFontButton *fb_tag_preprocessor_font;
GtkFontButton *fb_tag_type_font;
GtkFontButton *fb_editor_font;


GtkEntry *ent_ext_pic_editor;
GtkEntry *ent_ext_pic_viewer;

/*
GtkCheckButton *cb_enc_det_shift_jis;
GtkCheckButton *cb_enc_det_euc_jp;
GtkCheckButton *cb_enc_det_gb18030;
GtkCheckButton *cb_enc_det_big5;
GtkCheckButton *cb_enc_det_iso_2022_jp;
*/

GtkCheckButton *cb_enc_czech;
GtkCheckButton *cb_enc_det_cp1251;
GtkCheckButton *cb_enc_det_koi8r;
GtkCheckButton *cb_enc_det_koi8u;
GtkCheckButton *cb_enc_det_866;
GtkCheckButton *cb_enc_det_finnish;
GtkCheckButton *cb_enc_det_german;
GtkCheckButton *cb_enc_det_serbian;
GtkCheckButton *cb_enc_det_latvian;
GtkCheckButton *cb_enc_det_polish;
GtkCheckButton *cb_enc_det_portuguese;
GtkCheckButton *cb_enc_det_slovak;
GtkCheckButton *cb_enc_det_slovenian;
GtkCheckButton *cb_enc_det_spanish;
GtkCheckButton *cb_enc_det_turkish;

GtkCheckButton *cb_enc_use_fallback;

GtkWidget *bt_rtf_enc;
GtkWidget *bt_fallback_charset;

GtkWidget *bt_default_charset;
GtkWidget *bt_def_filesave_charset;


GtkEntry *ent_def_markup_mode;

GtkWidget* create_wnd_about (void);

GtkTreeView *tv_available_charsets;
GtkTreeView *tv_choosed_charsets;
GtkTreeView *tv_notetab_positions;

GtkWidget* wnd_options_create (void);
GtkWidget* lookup_widget3 (GtkContainer *widget, const gchar *widget_name);

#endif