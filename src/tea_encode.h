#ifndef TEA_ENCODE_H
#define TEA_ENCODE_H

typedef struct
{
  GtkTreeView *tv_charsets_from;
  GtkTreeView *tv_charsets_to;

  GtkCheckButton *chb_case_insensetive;
  GtkWidget *tb_control;
  GtkWidget *ent_dir_where_to_find;
  GtkWidget *ent_pattern;

  GtkWidget *pb_progress_bar;
  GtkWidget *ffstatusbar;
  GtkCheckButton *bt_convert_eol;
  GtkRadioButton *bt_lf;
  GtkRadioButton *bt_cr;
  GtkRadioButton *bt_crlf;
  guint id_pb;
} t_mass_encode_tool;

GtkWidget* create_wnd_encode (void);

#endif
