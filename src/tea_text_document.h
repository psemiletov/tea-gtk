/***************************************************************************
                          tea_text_document.h  -  description
                             -------------------
    begin                : Mon Dec 1 2003
    copyleft            : 2003-2007 by Peter Semiletov
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

#ifndef TEA_TEXT_DOCUMENT_H
#define TEA_TEXT_DOCUMENT_H


#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

GtkSourceLanguageManager *source_languages_manager;


GHashTable *c_t_keywords;
GHashTable *c_t_types;
GHashTable *pas_t_keywords;
GHashTable *pas_t_types;
GHashTable *py_t_keywords;
GHashTable *php_t_keywords;
GHashTable *bash_t_keywords;
GHashTable *po_t_keywords;


typedef struct
{
  gpointer um;

  gboolean autosave;
  gboolean toggle_images_visibility;

  gint col, row;

  gint position;
  gboolean *readonly;
  gchar *hl_mode;

  gint dclicked_y;
  GtkTextTag *basic_tag;
  gboolean b_saved;
  gchar *file_name_utf8;
  gchar *file_name_local;
  gchar *encoding;
  gboolean linenums;
  gchar *end_of_line;
  //gint end_of_line;


  gpointer *text_buffer;

  GtkWidget *tab_label;
  GtkWidget *but;

  GtkScrolledWindow *scrolledwindow;
  GtkSourceView *text_view;

  gchar *last_searched_text;

} t_note_page;


typedef struct
             {
              gchar *prefix;
              GtkTextIter itstart;
              GtkTextIter itend;
              t_note_page *page;

             } t_text_part;


t_text_part *temp_text_part;
GList *dox;
gchar *last_dir;


/////////////

void do_errors_hl (GtkTextView *text_view);
void set_lm_colors (void);
void hl_line_under_cursor (t_note_page *doc);
void hl_line_n (t_note_page *doc, gint line);



GtkSourceStyle* string_to_tag_style (const gchar *string);
void langs_load (void);
void done_hl (void);
void hl_init (void);
//void save_tag (GtkSourceLanguage *language, GtkSourceTag *tag);


////////////

void do_backup (t_note_page *doc, gboolean do_check);

t_note_page* doc_clear_new (void);
t_note_page* page_create_new (void);
void page_free (t_note_page *page);
void page_del_by_index (int i);
gpointer glist_find_by_index (GList *list, int i);
t_note_page* get_page_by_index (int i);

gboolean text_doc_save (t_note_page *doc, const gchar *a_filename);
gboolean text_doc_save_silent (t_note_page *doc, const gchar *a_filename);

t_note_page* doc_open_file (const gchar *a_filename, const gchar *a_charset);

//cur_text_doc points to current text document (in any case)
t_note_page *cur_text_doc;

gchar* doc_get_buf (gpointer text_buffer);
gboolean doc_save_buffer_to_file (gpointer text_buffer, const gchar *filename);

void doc_select_line (t_note_page *doc, gint line);
void doc_rep_sel (t_note_page *page, const gchar *newstring);
t_note_page* get_page (void);
void doc_apply_settings (t_note_page *doc);
void doc_insert_at_cursor (t_note_page *doc, const gchar *text);
void document_set_line_numbers (t_note_page *doc, gboolean value);
void doc_update_all (void);
gint editor_get_pos (t_note_page *doc);
void editor_set_pos (t_note_page *doc, guint pos);
void log_to_memo (const gchar *m1, const gchar* m2, gint mode);
void widget_apply_colors (GtkWidget *w);
gboolean doc_reload_text (t_note_page *doc, const gchar *filename, const gchar *enc);
gint get_n_page_by_filename (const gchar *filename);
gboolean find_quote (gunichar ch, gpointer user_data);
gchar* get_c_url_pure (t_note_page *doc);
t_note_page* doc_ins_to_new (const gchar *newstring);
t_note_page* get_page_text (void);
void set_title (t_note_page *t);
t_note_page* open_file_std (const gchar *f, const gchar *charset);
gboolean doc_search_f_next_ncase (t_note_page *doc);
gboolean doc_search_f_ncase (t_note_page *doc, const gchar *text);
gchar* upcase_each_first_letter_ofw (t_note_page *doc);
gchar* doc_get_word_at_left (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend);
void doc_move_cursor_backw (t_note_page *doc, gint i);
//void doc_move_cursor_forw (t_note_page *doc, gint i);
void doc_move_cursor_backw_middle_tags (t_note_page *doc);

void indent_real(GtkWidget *text_view);
void doc_header_source_switch (t_note_page *doc);
gint cb_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void doc_close_all (void);
void doc_close_all_except_current (void);
GList* add_session_item_composed (GList *list, t_note_page *doc);
GList* add_recent_item_composed (GList *list, t_note_page *doc);
void current_tag_close (t_note_page *doc, gint mode);
void doc_goto_local_label (const gchar *l);
void set_last_dir (t_note_page *doc);
void swap_lines (t_note_page *doc, gboolean with_upper);
void doc_save_all (void);
void doc_search_and_paint (t_note_page *doc, const gchar *str);
void doc_paint_lines_bigger_than (t_note_page *doc, gint val);
void session_save_to_file (const gchar *filename);
void session_open_from_file (const gchar *filename);
void doc_undo (t_note_page *doc);
void doc_redo (t_note_page *doc);
void doc_indent_selection (t_note_page *doc, gboolean unindent);
void doc_save_emergency (void);
gchar* get_c_url2 (t_note_page *doc);
gchar* doc_get_cur_mispelled (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend);
void hide_error_marks (t_note_page *doc);

//#ifndef CHAI_MODE
GtkTextTag *tag_paint;
GtkTextTag *tag_current_line;
GtkTextTag *tag_spell_err;
GtkTextTag *tag_comment;
GtkTextTag *tag_identifier;
GtkTextTag *tag_digit;
GtkTextTag *tag_string;
GtkTextTag *tag_html_tag;
GtkTextTag *tag_preprocessor;
GtkTextTag *tag_type;


GHashTable *c_t_keywords;
GHashTable *c_t_types;
GHashTable *pas_t_keywords;
GHashTable *pas_t_types;
GHashTable *py_t_keywords;
GHashTable *php_t_keywords;
GHashTable *bash_t_keywords;
GHashTable *po_t_keywords;

void document_apply_hl (t_note_page* doc);

void doc_set_text (t_note_page *doc, const gchar *text);
gchar* doc_get_sel (t_note_page *doc);
gboolean doc_replace_from_cursor_ones (t_note_page *doc, const gchar *what_to_find, const gchar *replace_to,
                                       gboolean undoable);
void doc_replace_from_cursor_all (t_note_page *doc, const gchar *what_to_find, const gchar *replace_to);
gchar* doc_get_current_word (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend);
gchar* doc_move_to_next_word (t_note_page *doc);
void modify_cursor_color (GtkWidget *textview, const gchar *color_name);
void doc_smarthomeend (t_note_page *doc, GdkEventKey *event);
gboolean doc_move_to_next_word_sel_if_regexp (t_note_page *doc, const gchar *text);
gboolean doc_search_f_silent (t_note_page *doc, const gchar *text);
void doc_update_statusbar (t_note_page *doc);
void do_hl_spell_check (t_note_page *doc, const gchar *lang);
gboolean doc_save_buffer_to_file_iconv (gpointer text_buffer, const gchar *filename, const gchar *enc);
void doc_set_new_text (t_note_page *doc, GtkTextIter *itstart, GtkTextIter *itend, const gchar *text);
gboolean doc_has_selection (t_note_page *doc);
void update_cursor_position (GtkTextBuffer *text_buffer, gpointer user_data);
void
move_cursor_cb (GtkTextBuffer *buffer,
		GtkTextIter   *cursoriter,
		GtkTextMark   *mark,
		gpointer       user_data);
void doc_spaces_to_tabs (t_note_page *doc, gint tabsize);
void doc_tabs_to_spaces (t_note_page *doc, gint tabsize);
void create_tags (void);
void prepare_hl_all (void);
gchar* get_c_url (t_note_page *doc);
void doc_move_to_pos_bw_quote (t_note_page* doc);
gboolean doc_search_f (t_note_page *doc, const gchar *text);
void apply_hl (t_note_page *doc);
void doc_toggle_images_visibility (t_note_page *p);
void remove_tags (t_note_page *doc);
gboolean doc_search_f_next (t_note_page *doc);
gint find_index_by_page (t_note_page *page);
void make_stats (t_note_page *doc);
void run_unitaz (t_note_page *page, gint sort_type, gboolean case_insensetive);
void run_extract_words (t_note_page *page);
void walk_by_words (gpointer key,
                    gpointer value,
                    gpointer user_data);
void add_recent_internal (t_note_page *doc);
void assign_tags (t_note_page *doc);

gchar* readability_ari_en (t_note_page *page); //The Automated Readability Index (ARI)
void make_readability (t_note_page *page);
void document_do_hl (t_note_page* doc);
gchar* get_full_fname (const gchar *fname, const gchar *linkname);


#endif
