#ifndef TEA_KWAS
#define TEA_KWAS

typedef struct
              {
               GHashTable* hash_filelist;
               gchar *path;
               GtkWidget *tree_view;
               gpointer *opposite_panel;
               gpointer *kwas_win;
               gint pos;
              }
               t_kwas_panel;


typedef struct
              {
               t_kwas_panel *p1;
               t_kwas_panel *p2;
               t_kwas_panel *focused_panel;
               GtkWindow *window;
               GtkWidget *cm_entry;
               GtkComboBox *cb_charset;
               GtkWidget *status_bar;
               GtkWidget *log_memo;
               GtkWidget *mni_bookmarks;
               GtkWidget *mni_bookmarks_menu;
               gint log_to_memo_counter;
               gint msg_counter;
              }
               t_kwas_win;


GtkWidget *kwas_mn_functions;
GtkWidget *kwas_mn_boomarks;
GtkAccelGroup *kwas_accel_group;

t_kwas_win* create_kwas_window (void);
gchar* kwas_get_current_filename (t_kwas_win *win);
void free_kwas_panel (t_kwas_panel *p);
void free_kwas_window (t_kwas_win *w);
t_kwas_panel* get_current_panel (t_kwas_win *win);
void fill_filelist (t_kwas_panel *p, const gchar *path);

#endif
