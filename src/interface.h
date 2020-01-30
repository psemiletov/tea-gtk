/***************************************************************************
                          interface.h  -  description
                             -------------------
    begin                : Mon Dec 1 2003
    copyleft            : 2003-2013 by Peter Semiletov
    email                : tea@list.ru
 ***************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H

GtkWidget *bt_close_current_doc;
GtkWidget *mni_suggestions;
GtkWidget *mni_ext_programs;
GtkWidget *sr_wnd;
GtkWidget *tea_main_window;
GtkWidget *mainvbox;
GtkWidget *ent_search;
GtkWidget *dlg_colorsel;
GtkNotebook *notebook1;
GtkWidget *menubar1;
GtkWidget *mni_file_menu;
GtkWidget *mni_mark_menu;
GtkWidget *mni_print;
GtkWidget *mni_reptables_menu;
GtkWidget *mni_reptables;

GtkWidget *mni_profiles_menu;
GtkWidget *mni_profiles;


GtkWidget *mni_file_configs_menu;
GtkWidget *mni_autocomp;
GtkWidget *mni_ide;
GtkWidget *mni_tools;
GtkWidget *mni_tools_menu;

GtkWidget *mni_ide_menu;
GtkWidget *mni_view_menu;
GtkWidget *mni_recent_list;
GtkWidget *mni_recent_menu;
GtkWidget *mni_sessions;
GtkWidget *mni_sessions_menu;
GtkWidget *mni_templates;
GtkWidget *mni_templates_menu;
GtkWidget *mni_latex;
GtkWidget *mni_latex_menu;
GtkWidget *mni_wiki_menu;
GtkWidget *mni_docbook_menu;
GtkWidget *mni_list_menu;
GtkWidget *mni_morse_menu;
GtkVPaned *vpaned1;
GtkWidget *vbox2;
GtkWidget *scrolledwindow3;
GtkWidget *tv_logmemo;
GtkTextBuffer *log_memo_textbuffer;
GtkWidget *mni_edit_menu;
GtkWidget *mni_edit_undo;
GtkWidget *mni_edit_redo;
GtkWidget *mni_hl_mode_menu;
GtkWidget *mni_markup_menu;
GtkWidget *mni_insert_doctype_menu;
GtkWidget *mni_insert_char_entities_menu;
GtkWidget *mni_format_menu;
GtkWidget *mni_quest_menu;
GtkWidget *mni_ins_menu;
GtkWidget *mni_eol_menu;
GtkWidget *mni_savedifferent_menu;
GtkWidget *mni_opendifferent_menu;
GtkWidget *mni_markup_header_menu;
GtkWidget *mni_markup_header_item;
GtkWidget *mni_markup_align;
GtkWidget *mni_markup_align_menu;
GtkWidget *mni_what_menu;
GtkWidget *mni_functions;
GtkWidget *mni_spellcheck_menu;
GtkWidget *mni_spellcheck;

GtkWidget *mni_last;

  GtkWidget *mni_table_menu;
  GtkWidget *mni_functions_menu;
  GtkWidget *mni_functions_case;
  GtkWidget *mni_functions_case_menu;
  GtkWidget *mni_filter_menu;
  GtkWidget *mni_functions_number;
  GtkWidget *mni_functions_number_menu;
  GtkWidget *mni_functions_string;
  GtkWidget *mni_functions_string_menu;

  GtkWidget *mni_snippets_menu;
  GtkWidget *mni_snippets;

  GtkWidget *mni_html_menu;
  GtkWidget *mni_align_menu;

  GtkWidget *mni_co_menu;
  GtkWidget *mni_co;

  GtkWidget *mni_mm_menu;



  GtkWidget *mni_ext_programs_menu;

  GtkWidget *mni_links;
  GtkMenuItem *mni_links_menu;

  GtkWidget *mni_sort_menu;

  GtkWidget *mni_tabs;
  GtkWidget *mni_tabs_menu;


  GtkWidget *mni_utils_menu;
  GtkWidget *mni_utils_cc_menu;

GtkWidget *mni_utils_unitaz_menu;


GtkWidget *mni_bookmarks_root;
GtkWidget *mni_bookmarks_menu;
GtkWidget *mni_bookmarks_cc_menu;
GtkWidget *mni_add_to;

GtkWidget *tb_main_toolbar;

GtkWidget *mni_nav_menu;

GtkWidget *mni_scripts;

GtkWidget *mni_scripts_menu;

GtkWidget *mni_file_new;
GtkWidget *mni_file_kwas;
GtkWidget *mni_file_crapbook;
GtkWidget *mni_file_open;
GtkWidget *mni_file_save;
GtkWidget *mni_file_save_ver;
GtkWidget *mni_file_save_backup;
GtkWidget *mni_file_close_current;
GtkWidget *mni_file_quit;
GtkWidget *mni_edit_line_up;
GtkWidget *mni_edit_line_down;
GtkWidget *mni_edit_delete_cur_line;
GtkWidget *mni_find_now;
GtkWidget *mni_find_now_caseinsens;
GtkWidget *mni_find_next;
GtkWidget *mni_find_next_caseinsens;
GtkWidget *mni_goto_line;

GtkWidget *mni_bold;
GtkWidget *mni_italic;
GtkWidget *mni_comment;
GtkWidget *mni_para;
GtkWidget *mni_br;
GtkWidget *mni_nbp;
GtkWidget *mni_link;
GtkWidget *mni_color;
GtkWidget *mni_align_center;
GtkWidget *mni_stats;
GtkWidget *mni_upcase;
GtkWidget *mni_locase;
GtkWidget *mni_apply_tpl_to_each_ln;
GtkWidget *mni_block_start;
GtkWidget *mni_block_end;
GtkWidget *mni_open_at_cursor;
GtkWidget *mni_refresh_hl;
GtkWidget *mni_word_wrap;


GtkWidget *statusbar1;
GtkAccelGroup *accel_group;

GtkTextTag *temp_tag;

GtkTextTag *tag_lm_error;
GtkWidget *pb_status;

guint id_mpb;

GtkWidget* create_tea_main_window (void);
void statusbar_msg (const gchar *s);

void mpb_start (void);
void mpb_stop (void);
void ui_update (void);

#endif