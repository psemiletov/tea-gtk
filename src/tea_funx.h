/***************************************************************************
                          tea_funx.h  -  description
                             -------------------
    begin                : Fri Dec 12 2003
    copyright            : (C) 2003-2006 by Peter 'Roxton' Semiletov
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


typedef gboolean (*DirForeachFunc) (const gchar *path, const gchar *basename, gpointer user_data);

//double get_value (double total, double perc);
double get_percent (double total, double value);


gchar* get_noname_name (void);

int get_value (int total, int perc);
//int get_percent (int total, int value);

gboolean change_dir (gchar *filename);
gchar* create_full_path (const gchar *filename, const gchar *basedir);
void handle_file (const gchar *filename, const gchar *charset, gint mode, gboolean kwas);
gboolean is_image (const gchar *filename);
gboolean copy_file (const gchar *src, const gchar *dest);
void create_empty_file (const gchar *filename, const gchar *first_line);
gchar* get_time (const gchar *format);
gint get_file_size (const gchar *filename);
gchar* get_8_filename (const gchar *filename);
gchar* get_l_filename (const gchar *filename);
gchar* get_lang_name (void);
gchar* find_doc_index_name (void);
void run_doc_in_browser (void);
gchar* get_clipboard_text (void);
gchar* get_tea_doc_compose_name (const gchar *f);
gchar* get_tea_doc_dir (void);
GList* read_dir_files (const gchar *path);
gchar* get_hl_name (const gchar *file_name);
gboolean is_po (const gchar *f);
void insert_link (const gchar *a_filename);
gboolean parse_error_line (const gchar *line, gchar **filename, gint *lineno);
void handle_file_ide (const gchar *filename, gint line);
gboolean check_ext (const gchar *filename, const gchar *ext);
void handle_file_enc (const gchar *filename, const gchar *enc);
void clipboard_put_text (const gchar *s);
GList* read_dir_to_glist (const gchar *path);
gint get_markup_mode (void);
gboolean is_ext (const gchar *filename, gchar *ext1, ...);
gchar* run_process_and_wait (const gchar *command);
gboolean is_tex (const gchar *filename);
void delete_dir (const gchar *path);
gchar* filename_from_xuri (const gchar *uri);
gchar* create_relative_link (const gchar *doc_filename, const gchar *img_filename);
gboolean is_ext_arr (const gchar *filename, const gchar *exts);
gboolean check_ext_wo_dot (const gchar *filename, const gchar *ext);
gboolean is_writable (const gchar *filename);
gboolean is_readable (const gchar *filename);
gboolean has_ext (const gchar *filename);
gboolean dir_foreach (const gchar *path, DirForeachFunc function, gpointer user_data, GError **error);
void read_dir_files_recurse (const gchar *path, const gchar *pattern);
gchar* get_build_info (void);
gchar* file_replace_path (const gchar *filename, const gchar *new_dir);
gchar* file_combine_path (const gchar *dir, const gchar *filename);
gchar* add_slash_if_need (const gchar *dir);
gboolean is_markup (const gchar *f);
gboolean is_c (const gchar *f);
gboolean is_pascal (const gchar *f);
gchar* compose_browser_cmd (const gchar *filename);
GList* image_formats_fill (void);
void insert_image (const gchar *a_filename, gint width, gint height, gboolean read_xy_from_file);
gboolean is_prefixed (const gchar *filename, gchar *prefix1, ...);
gchar* path_get_last_element (const gchar *s);
gchar* path_up (const gchar *filename);
double get_percent (double total, double value);
//double get_value (double total, double perc);
gint get_desktop_name (void);
GList* read_dir_files_rec (const gchar *path);
GList* read_dir_files_rec_pat (const gchar *path, const gchar *pattern);
void zip_create_archive (const gchar *filename);
gboolean parse_grep_line (const gchar *line, gchar **filename, gint *lineno);
