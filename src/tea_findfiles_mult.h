#ifndef TEA_FINDFILES_MULT_H
#define TEA_FINDFILES_MULT_H

typedef struct
{
  GtkComboBox *cmb_charset;
  GtkWidget *l_text_to_replace;
  GtkWidget *ent_text_to_replace;
  GtkWidget *ent_text_to_find;

  GtkCheckButton *chb_case_insensetive;
  GtkWidget *chb_whole_words;
  GtkWidget *tb_control;
  GtkWidget *ent_dir_where_to_find;
  GtkWidget *ent_pattern;

  GtkWidget *pb_progress_bar;
  GtkWidget *ffstatusbar;

  guint id_pb;
  GtkTreeView *tv_found_files;
} t_find_in_files_mult;


GtkWidget* create_findfiles_mult (void);

#endif
