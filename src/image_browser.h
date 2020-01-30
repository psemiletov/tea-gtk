#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

typedef struct
{
  GtkWidget *wnd_imageplane;
  gchar *directory;
  GList *gl_thumbs;
  GtkTreeView *tree_view;
  GtkWidget *pb; //progress bar
} t_image_plane;


typedef struct
{
  GList *files_list;
  GList *files_list_current;

  GtkWidget *wnd_image;
  GtkImage *image;
  GtkEventBox *event_box;
  gchar *filename;
  gboolean fullscreen;
//  gint rotation_mode;
} t_image_viewer;



GtkCheckButton *cb_imgplane_mode_sw;
GtkWidget* create_wnd_imgviewer (t_image_viewer *instance);
t_image_viewer* image_viewer_create (const gchar *filename);
GtkWidget* create_image_browser (t_image_plane *instance);
void image_browser_update (t_image_plane *instance);
void image_viewer_free (t_image_viewer *instance);
t_image_plane* create_image_plane (const gchar *dir);

#endif