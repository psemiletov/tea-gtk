/***************************************************************************
                          rox_strings.h  -  description
                             -------------------
    begin                : Fri Oct 17 12:08:36 EEST 2003
    copyleft            : 2003-2013 by Peter Semiletov
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

#ifndef ROX_STRINGS_H
#define ROX_STRINGS_H

typedef struct
{
  gchar *a;
  gchar *b;
  gchar *c;
  gchar *d;
} t_str_quad;


typedef struct {
               gchar *word;
               gint count;
              } t_struct_word;


typedef gboolean (FilterStrFunc) (const gchar *value);

gchar* str_after_sep (const gchar *str, const gchar *sep);
gchar* str_before_sep (const gchar *str, const gchar *sep);
gchar* str_face_quotes (const gchar *buf, const gchar *q1, const gchar *q2);
t_str_quad* parse_ctags_line (const gchar *line);
t_str_quad* str_quad_new (const gchar *a, const gchar *b, const gchar *c, const gchar *d);
void str_quad_print (t_str_quad *t);
void str_quad_free (t_str_quad *quad);
gchar* str_extract_between (const gchar *source, const gchar *a, const gchar *b);

//now playing: Dom & Kemal - Back for the Future - Moulin Rouge

gchar* text_load_msdoc (const gchar *filename);
gchar* get_file_ver_name (const gchar *name, const gchar *format);

size_t tNumber2Roman (guint uiNumber, int bUpperCase, gchar *szOutput);
GList* add_to_glist_combined_st (GList *list, const gchar* key, const gchar *value);
GList* add_to_glist_combined_int (GList *list, const gchar* key, gint value);
GList* remove_blank_lines (GList *lines);
void glist_strings_free (GList *list);
GList* glist_strings_sort (GList *list);
GList* glist_trim_l (GList* list, gint n);
GList* glist_trim_l_on_char (GList* list, gchar n);
void glist_print (GList *list);
void glist_save_to_file (GList *list, const gchar *filename);
GList* load_file_to_glist (const gchar *filename);
GList* str_in_glist (GList *list, const gchar *st);
gchar* rep_all_s (const gchar *s, const gchar *r);gchar* str_file_read (const gchar *filename);
GList* glist_shuffle (GList *list);
gchar* glist_enum (const gchar *buf, const gchar *format_str);
gboolean save_string_to_file (const gchar *filename, const gchar *s);
gchar* g_str_replace (const gchar *input, const gchar *replace, const gchar *with);
gchar* str_replace_all (const gchar *s, const gchar *what, const gchar *to);
GList* glist_repl (GList* list, const gchar *forma);
GHashTable* load_file_to_hashtable (const gchar *filename);
void print_ht (GHashTable *ht);
gchar* str_crackerize (const gchar *buf);
gint get_gchar_array_size (const gchar **a);
gchar* morse_encode (const gchar *s);
gchar* morse_decode (const gchar *s);
GList* ht_to_glist (GHashTable *hash_table, gint option);
gchar* ch_str (gchar *s, const gchar *new_val);
GList* each_line_remove_lt_spaces (GList *lines, gint mode);
gchar* get_c_header_fname (const gchar *f);
gchar* get_c_source_fname (const gchar *f);
gchar* ht_get_gchar_value (GHashTable *hash_table, const gchar *key, const gchar *def);
GList* str_remove_from_list (GList *list, const gchar *s);
gchar* str_to_html (const gchar *buf, gboolean xhtml);
gchar* text_load_gzip (const gchar *filename);
gchar* locale_to_utf8 (const gchar *s);
gchar* quotes_to_tex (const gchar *buf);
gchar* tex_paras (const gchar *buf);
gchar* string_from_glist_sep (GList *list, const gchar *sep);
gchar* change_file_ext (const gchar *filename, const gchar *new_ext);
gboolean str_in (const gchar *s, gchar *ext1, ...);
gchar* read_xml_text (const gchar *filename, const gchar *xml_file, const gchar *para);
gchar* read_abw_text (const gchar *filename, const gchar *para);
gchar* separated_table_to_latex_tabular (const gchar *st, const gchar *sep);
gint find_str_index (GList *l, const gchar *s);
GList* filter_list (GList *list, const gchar *phrase, gboolean incld);
gchar* kill_str_by_size (const gchar *s, gint val, gboolean killlesser);
gchar* copied_from_browser_to_ftext (const gchar *s, const gchar *pattern);
gchar* get_charset_from_meta (const gchar *text);
void gslist_strings_free (GSList *list);
GList* parse_string_to_list_sep (const gchar *s, const gchar *sep);
gchar* string_split_by (const gchar *source, const gchar *delimeter);
gchar* tea_convert_charset (const gchar *text, const gchar *from, const gchar *to);
gchar* recent_item_compose (const gchar *file_name_utf8, const gchar *charset, gint pos);
gboolean check_is_number (const gchar *value);
gchar* extract_each_line_bewteen (const gchar *text, const gchar *a, const gchar *b);
t_str_quad* str_triplex_test (void);
gchar* str_each_line_after_sep (const gchar *text, const gchar *sep);
gchar* str_each_line_before_sep (const gchar *text, const gchar *sep);
GList* glist_from_string_sep (const gchar *string, const gchar *sep);
gchar* kill_formatting (const gchar *s);
gchar* wrap_raw (const gchar *s, gint str_len);
gchar* kill_formatting_on_each_line (const gchar *s);
GList* arr_to_glist (const gchar **a);
gchar* xml_to_plain_text (const gchar *xml_text, const gchar *para);
gchar* strinfile_office (const gchar *filename, const gchar *text_to_find);
gchar* strinfile (const gchar *filename, const gchar *text_to_find);
gchar* linkify_text (const gchar *text);
gchar* strip_html (const gchar *text);
gchar* str_kill_dups (const gchar *s);
GList* sort_list_case_insensetive (GList *list);
void wrap_on_spaces (gchar *str, gint width);
GList* glist_word_sort_mode (GList *list, gint mode);
gchar* text_load_rtf (const gchar *filename);
gchar* enc_guess (const gchar *s);
GList* add_to_glist_combined_str (GList *list, const gchar *key, const gchar *value);
GList* glist_copy_with_data (GList *source, gint count);
GList* glist_copy_with_filter (GList *source, FilterStrFunc func);
void free_word_data (gpointer data);
gint sort_node_m1 (t_struct_word *a, t_struct_word *b);
gint sort_node_m2 (t_struct_word *a, t_struct_word *b);
gint sort_node_m4 (const gchar *a, const gchar *b);
GList* glist_word_sort_mode (GList *list, gint mode);
gchar* apply_repl_table (const gchar *text, const gchar *table_filename);
void print_ht (GHashTable *ht);
void save_ht_to_file (GHashTable *hash_table, const gchar *filename);
gchar* strinfile_charset (const gchar *filename, const gchar *text_to_find, const gchar *charset);
GList* filter_list_pattern (GList *list, const gchar *pattern, gboolean incld);
gchar* read_file_from_zip (const gchar *archive_name, const gchar *file_name);
gchar* read_single_file_from_zip (const gchar *archive_name);
gint get_eol_mode (const gchar *buf);
gchar* str_eol_to_lf (const gchar *buf);
gchar* str_eol_to_cr (const gchar *buf);
gchar* str_eol_to_crlf (const gchar *buf);
long romanToDecimal (const gchar *roman);
gchar* extract_filename (const gchar *filename);
gchar* extract_ext (const gchar *filename);

#endif
