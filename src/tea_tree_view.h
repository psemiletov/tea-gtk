#ifndef TEA_TREE_VIEW_H
#define TEA_TREE_VIEW_H

enum
{
  COL_TEXT = 0,
  NUM_COLS
};

enum
{
  COL_IMG_CELL = 0,
  COL_TEXT_CELL,
  NUM_COLS_IMG
};

GtkTreeView* tv_create_framed (GtkContainer *c, const gchar *caption, GtkSelectionMode mode);

GList* tv_get_selected_multiply (GtkTreeView *tree_view);
void tv_fill_with_glist (GtkTreeView *tree_view, GList *l);
gchar* tv_get_selected_single (GtkTreeView *tree_view);
GtkTreeView* tv_create (GtkSelectionMode mode);
GList* tv_get_list_data (GtkTreeView *tree_view);
////////////////
void tv_fill_with_glist_img (GtkTreeView *tree_view, GList *l);
GtkTreeView* tv_create_img (GtkSelectionMode mode);
GtkTreeView* tv_create_framed_img (GtkContainer *c, const gchar *caption, GtkSelectionMode mode);
void tv_add_new_item_img (GtkTreeView *tree_view, const gchar *filename, GdkPixbuf *pixbuf);
GtkTreeView* tv_create_tree (GtkContainer *c, GtkSelectionMode mode);
GtkTreeView* tv_create_non_framed (GtkContainer *c, GtkSelectionMode mode);
void tv_clear (GtkTreeView *tv);
void tv_sel_by_name (GtkTreeView *tree_view, const gchar *name);
void model_add_new_item (GtkTreeModel *model, const gchar *s);
void model_fill_with_glist (GtkTreeModel *model, GList *l);
void model_fill_from_file (GtkTreeModel *model, const gchar *filename);
void model_save_to_file (GtkTreeModel *model, const gchar *filename);
gchar* request_item_from_tvlist (GList *list);

#endif
