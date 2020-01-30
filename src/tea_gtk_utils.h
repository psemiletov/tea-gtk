GtkWidget *mni_temp;

GtkWidget* tea_text_entry (GtkWidget *container, const gchar *caption, const gchar *value);
GtkWidget* tea_color_selector (GtkWidget *container, const gchar *caption, const gchar *value);
GtkWidget* tea_checkbox (GtkWidget *container, const gchar *caption, gboolean value);
GtkWidget* tea_spinbutton (GtkWidget *container, const gchar *caption, gdouble value);

GtkWidget *tea_scale_widget (GtkWidget *container, const gchar *caption, gdouble min, gdouble max, gdouble value); 

void logmemo_set_pos (GtkTextView *v, guint pos);
GtkWidget* tea_dir_selector (GtkWidget *container, const gchar *caption, const gchar *value);
GtkWidget* tea_combo (GtkWidget *container, GList *strings, const gchar *caption, const gchar *value);
void widget_set_colors (GtkWidget *w, GdkColor *background, GdkColor *text, GdkColor *selection, GdkColor *sel_text);
gboolean dlg_question (const gchar *caption, const gchar *prompt, const gchar *val);
void dlg_info (const gchar *caption, const gchar *prompt, const gchar *val);
GtkWidget* tea_button_at_box (GtkWidget *container, const gchar *caption, gpointer callback, gpointer user_data);
void cb_on_bt_close (gpointer *w);
GtkWidget* find_menuitem (GtkContainer *widget, const gchar *widget_name);
GtkFileFilter* tea_file_filter_with_patterns (const gchar *name, gchar *pattern1, ...);
GtkWidget* daisho_add_del (gpointer *f1, gpointer f2);
GtkWidget* new_menu_item_with_udata (const gchar *label, GtkWidget *parent, gpointer f, gpointer data);
GtkWidget* new_menu_sep (GtkWidget *parent);
GtkWidget* new_menu_item (const gchar *label, GtkWidget *parent, gpointer f);
GtkWidget* new_menu_tof (GtkWidget *parent);
GtkWidget* new_menu_submenu (GtkWidget *parent);
void window_destroy (GtkWidget *windowname);
gboolean win_key_handler (GtkWidget *widget, GdkEventKey *event, GtkWidget *w);
void set_cursor_blink (gboolean value);
void fill_combo_with_glist (GtkComboBox *combo, GList *l);
GtkWidget* tea_gtk_toolbar_prepend_space (GtkToolbar *toolbar);
GtkWidget* tea_font_button (GtkWidget *container, const gchar *caption, const gchar *value);
GtkWidget* tea_stock_button (const gchar *stock_id);
GtkWidget* tea_combo_combo (GtkWidget *container, GList *strings, const gchar *caption, gint value,
gpointer on_changed, gpointer user_data);

gchar* dlg_question_char (const gchar *prompt, const gchar *defval);
gboolean dlg_key_handler (GtkWidget *widget, GdkEventKey *event, GtkWidget *w);
void build_menu_from_ht (GHashTable *hash_table, gpointer menu, gpointer callback);
void build_menu_from_glist (GList *list, gpointer menu, gpointer callback);
void build_menu_wudata_from_glist (GList *list, gpointer menu, gpointer callback, gpointer udata);
void tea_widget_show (GtkWidget *w, gboolean visible);
GtkWidget* tea_color_selector_simple (GtkWidget *container, const gchar *caption);
gchar* get_text_buf (gpointer text_buffer);
GtkWidget* tea_shortcut_entry (void);
GtkWidget* tea_text_entry_hotkey (GtkWidget *container, const gchar *caption, const gchar *value);
GtkWidget* tea_button_list_items (
                                  GtkWidget *container,
                                  GList *list,
                                  const gchar *caption,
                                  const gchar *defval
                                  );
