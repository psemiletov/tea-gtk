/***************************************************************************
                          interface.c  -  description
                             -------------------
    begin                : 2003
    copyleft            : 2003-2013 by Peter 'Roxton' Semiletov
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


#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <libintl.h>
#include <locale.h>

#include "tea_text_document.h"
#include "callbacks.h"
#include "interface.h"
#include "tea_defs.h"
#include "tea_config.h"
#include "tea_funx.h"
#include "tea_gtk_utils.h"
#include "tea_tree_view.h"
#include "rox_strings.h" // Needed for fill_entities_special_menu


void ui_update (void)
{
  tea_widget_show (bt_close_current_doc, confile.common_closing_button_visible);
}

static gboolean mpb_bounce (gpointer data)
{
  gtk_progress_bar_pulse (pb_status);
  return TRUE;
}


void mpb_start (void)
{
  id_mpb = g_timeout_add (150, mpb_bounce, NULL);
  gtk_widget_show (pb_status);
  gtk_progress_bar_pulse (pb_status);
}


void mpb_stop (void)
{
  gtk_progress_bar_set_fraction (pb_status, 0.0);
  g_source_remove (id_mpb);
  gtk_widget_hide (pb_status);
}


//from Owen Taylor <otaylor src gnome org> at https://mail.gnome.org/archives/commits-list/2009-August/msg09366.html
static void
insert_stock_button (GtkWidget          *toolbar,
                     const gchar        *stock_id,
                     const gchar        *text,
                     GCallback           callback,
                     gpointer            user_data,
                     gint position
                    )
{
  GtkToolItem *button = gtk_tool_button_new_from_stock (stock_id);
  
  if (! button)
    return; 
  
  gtk_tool_item_set_tooltip_text (button, text);
  
  g_signal_connect (G_OBJECT (button),
                    "clicked",
                    callback,
                    user_data);
  
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
                      button,
                      position);
  
  gtk_widget_show (button);
  
}


static GtkWidget* create_hardcoded_toolbar (void)
{
  GtkWidget *toolbar = gtk_toolbar_new();

  insert_stock_button (toolbar, GTK_STOCK_PREFERENCES, NULL, show_options, NULL, 0);
  
  GtkToolItem *sep = gtk_separator_tool_item_new();
  gtk_widget_show (sep);
  
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), sep, 0);  
  
  
  insert_stock_button (toolbar, GTK_STOCK_PASTE, NULL, edit_paste, NULL, 0);
  insert_stock_button (toolbar, GTK_STOCK_CUT, NULL, edit_cut, NULL, 0);
  insert_stock_button (toolbar, GTK_STOCK_COPY, NULL, edit_copy, NULL, 0);

  sep = gtk_separator_tool_item_new();
  gtk_widget_show (sep);
  
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), sep, 0);  
  
  insert_stock_button (toolbar, GTK_STOCK_SAVE_AS, _("Save under a different name"), file_save_as, NULL, 0);
  insert_stock_button (toolbar, GTK_STOCK_SAVE, _("Save the current file"), file_save, NULL, 0);
  insert_stock_button (toolbar, GTK_STOCK_OPEN, _("Open a file"), tea_file_open_dialog, NULL, 0);
  insert_stock_button (toolbar, GTK_STOCK_NEW, _("Create a new file"), file_new, NULL, 0);
  
  gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_LARGE_TOOLBAR);

  if (confile.do_show_main_toolbar)
     gtk_widget_show (toolbar);
  else
      gtk_widget_hide (toolbar);

  return toolbar;
}


void statusbar_msg (const gchar *s)
{
  if (s)
     gtk_statusbar_push (statusbar1, gtk_statusbar_get_context_id (statusbar1, s), s);
}


void fill_entities_special_menu (void)
{
  fill_entities ();
  GList *l = ht_to_glist (ht_entities, opt_ht_to_glist_keys);
  l = glist_strings_sort (l);
  build_menu_from_glist (l, mni_insert_char_entities_menu, on_mni_entity);
  glist_strings_free (l);
}


GtkWidget* create_tea_main_window (void)
{
  confile_reload ();

  accel_group = gtk_accel_group_new ();

  tea_main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (tea_main_window, "tea_main_window");
  gtk_window_set_title (GTK_WINDOW (tea_main_window), "TEA");

  gtk_window_resize (GTK_WINDOW (tea_main_window), 800, 600);

  g_signal_connect ((gpointer) tea_main_window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect ((gpointer) tea_main_window, "destroy_event",
                    G_CALLBACK (gtk_false),
                    NULL);

  g_signal_connect ((gpointer) tea_main_window, "delete_event",
                    G_CALLBACK (on_tea_window_delete_event),
                    tea_main_window);

  mainvbox = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (mainvbox);
  gtk_container_add (GTK_CONTAINER (tea_main_window), mainvbox);

  GtkWidget *hb_topbox = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (hb_topbox);
  gtk_box_pack_start (GTK_BOX (mainvbox), hb_topbox, FALSE, FALSE, UI_PACKFACTOR);

  bt_close_current_doc = tea_stock_button (GTK_STOCK_CLOSE);
  g_signal_connect ((gpointer) bt_close_current_doc, "clicked",
                    G_CALLBACK (on_bt_close_current_doc),
                    NULL);

  menubar1 = gtk_menu_bar_new ();

  gtk_widget_set_name (menubar1, "menubar1");
  gtk_widget_show (menubar1);

  gtk_box_pack_start (GTK_BOX (hb_topbox), menubar1, FALSE, FALSE, UI_PACKFACTOR);
  gtk_box_pack_start (GTK_BOX (hb_topbox), bt_close_current_doc, FALSE, FALSE, UI_PACKFACTOR);

  
   gtk_widget_set_tooltip_text (bt_close_current_doc,
                        _("Close the current document"));

  mni_temp = new_menu_item (_("File"), menubar1, NULL);
  mni_file_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_file_menu);

  mni_file_new = new_menu_item (_("New"), mni_file_menu, on_mni_new_file_activate);
  mni_file_kwas = new_menu_item (_("New Kwas (file manager)"), mni_file_menu, on_mni_new_kwas);


//#ifdef HACKING_MODE
  //mni_temp = new_menu_item ("test", mni_file_menu, on_mni_test);
 // mni_temp = new_menu_item ("test24", mni_file_menu, on_mni_test2);
//#endif


  mni_file_crapbook = new_menu_item (_("Crapbook"), mni_file_menu, on_mni_file_crapbook);

#ifdef PRINTING_SUPPORTED
  mni_print = new_menu_item (_("Print"), mni_file_menu, on_mni_print);

#endif

  mni_temp = new_menu_item (_("Add to..."), mni_file_menu, NULL);
  mni_add_to = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_add_to);
  mni_temp = new_menu_item (_("Add to Bookmarks"), mni_add_to, on_mni_file_add_bookmark);
  mni_temp = new_menu_item (_("Add to the autosaving list"), mni_add_to, on_mni_add_to_autosave_list);

  mni_temp = new_menu_sep (mni_file_menu);

  mni_file_open = new_menu_item (_("Open"), mni_file_menu, on_mni_file_open_activate);

  mni_temp = new_menu_item (_("The last opened file"), mni_file_menu, on_mni_load_last_file);

  mni_temp = new_menu_item (_("Open different"), mni_file_menu, NULL);
  mni_opendifferent_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_opendifferent_menu);
  mni_temp = new_menu_item (_("Open movie"), mni_opendifferent_menu, on_mni_open_open_movie);

  mni_temp = new_menu_item (_("Open the autosaved session"), mni_opendifferent_menu, on_mni_load_autosaved_session);
  mni_temp = new_menu_item (_("Open from the Famous text entry"), mni_opendifferent_menu, on_mni_fam_open);

  mni_temp = new_menu_sep (mni_file_menu);

  mni_recent_list = new_menu_item (_("Recent files"), mni_file_menu, NULL);
  mni_recent_menu = new_menu_submenu (mni_recent_list);

  mni_bookmarks_root = new_menu_item (_("Bookmarks"), mni_file_menu, NULL);
  mni_bookmarks_menu = new_menu_submenu (mni_bookmarks_root);

  mni_sessions = new_menu_item (_("Sessions"), mni_file_menu, NULL);
  mni_sessions_menu = new_menu_submenu (mni_sessions);

  mni_templates = new_menu_item (_("Templates"), mni_file_menu, NULL);
  mni_templates_menu = new_menu_submenu (mni_templates);

  mni_temp = new_menu_sep (mni_file_menu);

  mni_file_save = new_menu_item (_("Save"), mni_file_menu, on_mni_file_save_activate);
  mni_temp = new_menu_item (_("Save as"), mni_file_menu, on_mni_file_save_as_activate);

  mni_temp = new_menu_item (_("Save different"), mni_file_menu, NULL);
  mni_savedifferent_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_savedifferent_menu);

  mni_temp = new_menu_item (_("Save a session"), mni_savedifferent_menu, on_mni_file_save_session);

  mni_temp = new_menu_item (_("Save as a template"), mni_savedifferent_menu, on_mni_file_save_as_template_activate);
  mni_temp = new_menu_item (_("Save as a snippet"), mni_savedifferent_menu, on_mni_file_save_as_snippet_activate);
  mni_temp = new_menu_item (_("Save as a replacement table"), mni_savedifferent_menu, file_save_as_reptable);

  mni_file_save_ver = new_menu_item (_("Save version"), mni_savedifferent_menu, on_mni_file_save_version);
  mni_file_save_backup = new_menu_item (_("Backup"), mni_savedifferent_menu, on_mni_file_backup);

  mni_temp = new_menu_item (_("Save packed"), mni_savedifferent_menu, on_mni_save_packed);

  mni_temp = new_menu_item (_("Save all"), mni_file_menu, on_mni_save_all);

  mni_temp = new_menu_item (_("Revert to saved"), mni_file_menu, on_mni_file_revert_to_saved);

  mni_temp = new_menu_sep (mni_file_menu);

  mni_temp = new_menu_item (_("Manage utility files"), mni_file_menu, NULL);
  mni_file_configs_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_file_configs_menu);
  mni_temp = new_menu_item (_("Autoreplace words file"), mni_file_configs_menu, on_mni_autorep_file_open);
  mni_temp = new_menu_item (_("Autosaving list"), mni_file_configs_menu, on_mni_autosave_list_open);
  mni_temp = new_menu_item (_("Main config"), mni_file_configs_menu , on_mni_config_open);
  mni_temp = new_menu_item (_("Hotkeys config"), mni_file_configs_menu, on_mni_open_hotkeys);
  mni_temp = new_menu_item (_("External programs list config"), mni_file_configs_menu, on_mni_ext_programs_file_open);
  mni_temp = new_menu_item (_("Bookmarks file"), mni_file_configs_menu, on_mni_file_open_bookmark);
  mni_temp = new_menu_item (_("Open the crashfile"), mni_file_configs_menu, on_mni_crashfile_open);

  mni_temp = new_menu_item (_("Preferences"), mni_file_menu, on_mni_show_options);

  mni_temp = new_menu_sep (mni_file_menu);

  mni_file_close_current = new_menu_item (_("Close current"), mni_file_menu, on_mni_file_close_current);
  mni_temp = new_menu_item (_("Close all"), mni_file_menu, on_mni_file_close_all);

  mni_file_quit = new_menu_item (_("Quit"), mni_file_menu, on_mni_out_activate);


  mni_temp = new_menu_item (_("Edit"), menubar1, NULL);
  mni_edit_menu =  new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_edit_menu);

  mni_temp = new_menu_item (_("Indent"), mni_edit_menu, on_mni_indent);
  mni_temp = new_menu_item (_("Unindent"), mni_edit_menu, on_mni_unindent);

  mni_temp = new_menu_sep (mni_edit_menu);

  mni_temp = new_menu_item (_("Copy"), mni_edit_menu, on_mni_edit_copy);
  mni_temp = new_menu_item (_("Copy all"), mni_edit_menu, on_mni_edit_copy_all);
  mni_temp = new_menu_item (_("Copy the current filename"), mni_edit_menu, on_mni_copy_filename);

  mni_temp = new_menu_item (_("Cut!"), mni_edit_menu, on_mni_edit_cut);
  mni_temp = new_menu_item (_("Paste"), mni_edit_menu, on_mni_paste);
  mni_temp = new_menu_item (_("Swop"), mni_edit_menu,on_mni_edit_replace_with_clipboard);

  mni_temp = new_menu_sep (mni_edit_menu);
  mni_edit_line_up = new_menu_item (_("Move a line up"), mni_edit_menu, on_mni_move_line_up);
  gtk_widget_add_accelerator (mni_edit_line_up, "activate", accel_group, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  mni_edit_line_down = new_menu_item (_("Move a line down"), mni_edit_menu, on_mni_move_line_down);
  gtk_widget_add_accelerator (mni_edit_line_down, "activate", accel_group, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

  mni_temp = new_menu_sep (mni_edit_menu);

  mni_temp = new_menu_item (_("Cut to new"), mni_edit_menu, on_mni_cut_to_new);
  mni_temp = new_menu_item (_("Copy to new"), mni_edit_menu, on_mni_copy_to_new);
  mni_temp = new_menu_item (_("Paste to new"), mni_edit_menu, on_mni_paste_to_new);
  mni_temp = new_menu_item (_("Add to Crapbook"), mni_edit_menu, on_mni_edit_append_to_crapbook);

  mni_temp = new_menu_item (_("Select all"), mni_edit_menu, on_mni_edit_select_all);

  mni_edit_delete_cur_line = new_menu_item (_("Delete current line"), mni_edit_menu, on_mni_edit_delete_current_line);
  mni_temp = new_menu_item (_("Delete"), mni_edit_menu, on_mni_edit_delete);

  mni_temp = new_menu_sep (mni_edit_menu);

  mni_edit_undo = new_menu_item (_("Undo"), mni_edit_menu, on_mni_undo);
  mni_edit_redo = new_menu_item (_("Redo"), mni_edit_menu, on_mni_redo);


  mni_temp = new_menu_item (_("Search"), menubar1, NULL);

  mni_quest_menu =  new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_quest_menu);

  mni_temp = new_menu_item (_("Marking"), mni_quest_menu, NULL);
  mni_mark_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_mark_menu);

  mni_temp = new_menu_item (_("Mark"), mni_mark_menu, on_mni_paint);
  mni_temp = new_menu_item (_("Copy marked"), mni_mark_menu, on_mni_copy_painted);
  mni_temp = new_menu_item (_("Unmark all"), mni_mark_menu, on_mni_unpaint_all);
  mni_temp = new_menu_item (_("Mark each n-th line"), mni_mark_menu, on_mni_mark_each_n_line);
  mni_temp = new_menu_item (_("Mark lines containing the string"), mni_mark_menu, on_mni_mark_all_lines_with_string);
  mni_temp = new_menu_item (_("Mark by regexp"), mni_mark_menu, on_mni_mark_on_regexp);
  mni_temp = new_menu_item (_("Mark all fuzzy and empty translations"), mni_mark_menu, on_mni_mark_lines_with_fuzzy_nontrans);
  mni_temp = new_menu_item (_("Find and mark"), mni_mark_menu, on_mni_search_and_paint);
  mni_temp = new_menu_item (_("Mark lines > n characters"), mni_mark_menu, on_mni_paint_lines_bigger_than);

  mni_temp = new_menu_item (_("Find in files"), mni_quest_menu, on_mni_find_in_files);
  mni_temp = new_menu_item (_("Search and Replace (tool)"), mni_quest_menu, on_mni_find_and_replace_wnd);
  mni_find_now = new_menu_item (_("Find"), mni_quest_menu, on_mni_quest_find);
  mni_find_now_caseinsens = new_menu_item (_("Find case insensitive"), mni_quest_menu, on_mni_quest_find_ncase);
  mni_find_next = new_menu_item (_("Find next"), mni_quest_menu, on_mni_quest_find_next);
  mni_find_next_caseinsens = new_menu_item (_("Find next case insensitive"), mni_quest_menu, on_mni_quest_find_next_ncase);
  mni_temp = new_menu_item (_("Replace"), mni_quest_menu, on_mni_replace_in_place);
  mni_goto_line = new_menu_item (_("Goto line"), mni_quest_menu, on_mni_goto_line);
  mni_temp = new_menu_item (_("Replace all"), mni_quest_menu, on_mni_search_repall);

  mni_temp = new_menu_item (_("Mark broken local links"), mni_quest_menu, scan_missing_local_links);
  mni_temp = new_menu_item (_("Scan for local links"), mni_quest_menu, on_mni_get_links);
  mni_temp = new_menu_item (_("Scan for SRC"), mni_quest_menu, on_mni_get_src);
  mni_temp = new_menu_item (_("Run grep"), mni_quest_menu, on_mni_grep);



  mni_temp = new_menu_item (_("Markup"), menubar1, NULL);
  mni_markup_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_markup_menu);

  mni_temp = new_menu_item (_("Markup mode"), mni_markup_menu, NULL);
  mni_mm_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_mm_menu);

  mni_temp = new_menu_item ("HTML", mni_mm_menu, on_mni_mm_change);
  mni_temp = new_menu_item ("XHTML", mni_mm_menu, on_mni_mm_change);
  mni_temp = new_menu_item ("Wikipedia", mni_mm_menu, on_mni_mm_change);
  mni_temp = new_menu_item ("LaTeX", mni_mm_menu, on_mni_mm_change);
  mni_temp = new_menu_item ("Docbook", mni_mm_menu, on_mni_mm_change);

  mni_temp = new_menu_item (_("Wikipedia elements"), mni_markup_menu, NULL);
  mni_wiki_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_wiki_menu);

  mni_temp = new_menu_item (_("Internal link"), mni_wiki_menu, on_mni_markup_wiki_internal_link);

  mni_temp = new_menu_item (_("LaTeX elements"), mni_markup_menu, NULL);
  mni_latex_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_latex_menu);

  mni_temp = new_menu_item ("***", mni_latex_menu, on_mni_ins_ast);
  mni_temp = new_menu_item (_("Begin/end block"), mni_latex_menu, on_mni_markup_tex_begend);
  mni_temp = new_menu_item ("author", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("bibitem", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("date", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("documentclass", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("footnote", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("geometry", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("label", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("ldots", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("linebreak", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("listoftables", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("listoffigures", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("maketitle", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("marginpar", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("pagestyle", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("part", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("ref", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("renewcommand", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("rule", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("setlength", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("subsection", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("tableofcontents", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("textcopyright", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("textit", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("title", mni_latex_menu, on_mni_markup_tex_common);
  mni_temp = new_menu_item ("usepackage", mni_latex_menu, on_mni_markup_tex_common);

  mni_temp = new_menu_item (_("Docbook elements"), mni_markup_menu, NULL);
  mni_docbook_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_docbook_menu);

  mni_temp = new_menu_item ("application", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("caution", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("citation", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("command", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("example", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("filename", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("important", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("indexterm", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("itemizedlist", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("literallayout", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("markup", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("primary", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("programlisting", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("screen", mni_docbook_menu, on_mni_markup_common);
  mni_temp = new_menu_item ("title", mni_docbook_menu, on_mni_markup_common);

  mni_temp = new_menu_item (_("HTML tools"), mni_markup_menu, NULL);
  mni_html_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_html_menu);

  mni_temp = new_menu_item (_("Built-in template"), mni_html_menu, on_mni_html_default_template);

  mni_temp = new_menu_item (_("Strip tags"), mni_html_menu, on_mni_html_strip_tags);
  mni_temp = new_menu_item (_("Enclose selected link into tags"), mni_html_menu, on_mni_html_enclose_link);
  mni_temp = new_menu_item (_("Convert tags to entities"), mni_html_menu, on_mni_convert_ascii_2_html);
  mni_temp = new_menu_item (_("Document weight"), mni_html_menu, on_mni_html_calc_weight);
  mni_temp = new_menu_item (_("Text to HTML"), mni_html_menu, on_mni_text_to_html_fmt);

  mni_temp = new_menu_sep (mni_markup_menu);

  mni_bold = new_menu_item (_("Bold"), mni_markup_menu, on_mni_Markup_bold_activate);
  mni_italic = new_menu_item (_("Italic"), mni_markup_menu, on_mni_Markup_italic_activate);

  mni_temp = new_menu_item (_("Emphasis"), mni_markup_menu, on_mni_markup_em);
  mni_temp = new_menu_item (_("Underline"), mni_markup_menu, on_mni_Markup_underline_activate);

  mni_temp = new_menu_sep (mni_markup_menu);

  mni_temp = new_menu_item (_("Close the current tag"), mni_markup_menu, on_mni_close_current_tag);
  mni_comment = new_menu_item (_("Comment"), mni_markup_menu, on_mni_Markup_comment);
  mni_temp = new_menu_item (_("Chapter"), mni_markup_menu, on_mni_markup_chap);
  mni_temp = new_menu_item (_("Section"), mni_markup_menu, on_mni_markup_section);
  mni_para = new_menu_item (_("Paragraph"), mni_markup_menu, on_mni_Markup_para_activate);
  mni_link = new_menu_item (_("Link"), mni_markup_menu, on_mni_Markup_link_activate);
  mni_color = new_menu_item (_("Color"), mni_markup_menu, on_mni_markup_select_color_activate);
  mni_temp = new_menu_item (_("Image"), mni_markup_menu, on_mni_image_activate);
  mni_br = new_menu_item (_("Line break"), mni_markup_menu, on_mni_Markup_br);
  gtk_widget_add_accelerator (mni_br, "activate", accel_group, GDK_KEY_Return, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  mni_nbp = new_menu_item (_("Non-breaking space"), mni_markup_menu, on_mni_Markup_nbsp);
  gtk_widget_add_accelerator (mni_nbp, "activate", accel_group,
                              GDK_KEY_space, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  mni_temp = new_menu_sep (mni_markup_menu);

  mni_temp = new_menu_item (_("List stuff"), mni_markup_menu, NULL);
  mni_list_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_list_menu);

  mni_temp = new_menu_item (_("Itemized list"), mni_list_menu, on_mni_markup_itemizedlist);
  mni_temp = new_menu_item (_("Enumerated list"), mni_list_menu, on_mni_markup_enumeratedlist);
  mni_temp = new_menu_item (_("List item"), mni_list_menu, on_mni_markup_list_item);

  mni_temp = new_menu_item (_("Table stuff"), mni_markup_menu, NULL);
  mni_table_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_table_menu);

  mni_temp = new_menu_item (_("Make a table"), mni_table_menu, on_mni_html_make_table);
  mni_temp = new_menu_item (_("Table"), mni_table_menu, on_mni_markup_table);
  mni_temp = new_menu_item (_("Row"), mni_table_menu, on_mni_markup_row);
  mni_temp = new_menu_item (_("Cell"), mni_table_menu, on_mni_markup_col);

  mni_temp = new_menu_item (_("H"), mni_markup_menu, NULL);
  mni_markup_header_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_markup_header_menu);

  gint c;
  gchar *h;

  for (c = 1; c <= 6; c++)
      {
       h = g_strdup_printf ("h%d", c);
       mni_temp = new_menu_item (h, mni_markup_header_menu, on_mni_markup_common);
       g_free (h);
      }

  mni_temp = new_menu_item (_("Align"), mni_markup_menu, NULL);
  mni_align_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_align_menu);

  mni_align_center = new_menu_item ("Center", mni_align_menu, on_mni_markup_align);
  mni_temp = new_menu_item ("Left", mni_align_menu, on_mni_markup_align);
  mni_temp = new_menu_item ("Right", mni_align_menu, on_mni_markup_align);
  mni_temp = new_menu_item ("Justify", mni_align_menu, on_mni_markup_align);

  mni_temp = new_menu_item (_("Functions"), menubar1, NULL);
  mni_functions_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_functions_menu);

  mni_temp = new_menu_item (_("Calendar"), mni_functions_menu, on_mni_calendar);

  mni_stats = new_menu_item (_("Document stats"), mni_functions_menu, on_mni_utils_stats);
  mni_stats = new_menu_item (_("Document readability"), mni_functions_menu, on_mni_utils_readability);

  mni_temp = new_menu_sep (mni_functions_menu);
#ifdef HAVE_LIBASPELL
  mni_spellcheck = new_menu_item (_("Spell-checker languages"), mni_functions_menu, NULL);
  mni_spellcheck_menu = new_menu_submenu (mni_spellcheck);
  mni_temp = new_menu_item (_("Spell check"), mni_functions_menu, on_mni_spellcheck_def);
  mni_temp = new_menu_item (_("Suggest a word"), mni_functions_menu, on_mni_suggest);
  mni_temp = new_menu_item (_("Add to dictionary"), mni_functions_menu, on_mni_add_to_dict );
  mni_temp = new_menu_sep (mni_functions_menu);
#endif

#ifdef ENCHANT_SUPPORTED
  mni_temp = new_menu_item (_("Spell check"), mni_functions_menu, on_mni_spellcheck_def);
  mni_temp = new_menu_item (_("Suggest a word"), mni_functions_menu, on_mni_suggest);
  mni_temp = new_menu_item (_("Add to dict"), mni_functions_menu, on_mni_add_to_dict );
  mni_temp = new_menu_sep (mni_functions_menu);
#endif

  mni_latex = new_menu_item ("LaTeX", mni_functions_menu, NULL);
  mni_latex_menu = new_menu_submenu (mni_latex);
  mni_temp = new_menu_tof (mni_latex_menu);

  mni_temp = new_menu_item (_("Process with LaTeX"), mni_latex_menu, on_mni_latex_process);
  mni_temp = new_menu_item (_("Process with pdflatex"), mni_latex_menu, on_mni_pdflatex_process);

  mni_temp = new_menu_item (_("dvi to PS"), mni_latex_menu, on_mni_dvi_to_ps);
  mni_temp = new_menu_item (_("dvi to PDF"), mni_latex_menu, on_mni_dvi_to_pdf);
  mni_temp = new_menu_item (_("View PDF"), mni_latex_menu, on_mni_view_pdf);
  mni_temp = new_menu_item (_("View dvi"), mni_latex_menu, on_mni_view_dvi);
  mni_temp = new_menu_item (_("View PS"), mni_latex_menu, on_mni_view_ps);

  mni_ide = new_menu_item (_("IDE"), mni_functions_menu, NULL);
  mni_ide_menu = new_menu_submenu (mni_ide);
  mni_temp = new_menu_tof (mni_ide_menu);

  mni_temp = new_menu_item (_("New project"), mni_ide_menu, on_mni_project_new);
  mni_temp = new_menu_item (_("Open project"), mni_ide_menu, on_mni_project_open);
  mni_temp = new_menu_item (_("Save project"), mni_ide_menu, on_mni_show_project_save);
  mni_temp = new_menu_item (_("Save project as"), mni_ide_menu, on_mni_project_save_as);
  mni_temp = new_menu_item (_("Project properties"), mni_ide_menu, on_mni_show_project_props);
  mni_temp = new_menu_item (_("Make"), mni_ide_menu, on_mni_project_make);
  mni_temp = new_menu_item (_("Execute"), mni_ide_menu, on_mni_show_project_run);
  mni_temp = new_menu_item (_("Refresh tags with ctags"), mni_ide_menu, on_mni_refresh_ctags);

  mni_temp = new_menu_item (_("UNIversal Text AnalyZer"), mni_functions_menu, NULL);
  mni_utils_unitaz_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_utils_unitaz_menu);
  mni_temp = new_menu_item (_("Call UNITAZ with sorting abc"), mni_utils_unitaz_menu, on_mni_tools_unitaz_sort);
  mni_temp = new_menu_item (_("Call UNITAZ with sorting by count"), mni_utils_unitaz_menu, on_mni_tools_unitaz_sort_by_count);
  mni_temp = new_menu_item (_("Call UNITAZ with sorting abc case insensitive"), mni_utils_unitaz_menu, on_mni_tools_unitaz_sort_caseinsens);
  mni_temp = new_menu_item (_("Call UNITAZ with sorting by count case insensitive"), mni_utils_unitaz_menu, on_mni_tools_unitaz_sort_by_count_caseinsens);
  mni_temp = new_menu_item (_("Call UNITAZ plain"), mni_utils_unitaz_menu, on_mni_tools_unitaz_plain);
  mni_temp = new_menu_item (_("Extract words"), mni_utils_unitaz_menu, on_mni_extract_words);

  mni_temp = new_menu_sep (mni_functions_menu);

  mni_reptables = new_menu_item (_("Replacement tables"), mni_functions_menu, NULL);
  mni_reptables_menu = new_menu_submenu (mni_reptables);

  mni_scripts = new_menu_item (_("Scripts"), mni_functions_menu, NULL);
  mni_scripts_menu = new_menu_submenu (mni_scripts);

  mni_snippets = new_menu_item (_("Snippets"), mni_functions_menu, NULL);
  mni_snippets_menu = new_menu_submenu (mni_snippets);

  mni_tools = new_menu_item (_("Tools"), mni_functions_menu, NULL);
  mni_tools_menu = new_menu_submenu (mni_tools);
  mni_temp = new_menu_tof (mni_tools_menu);

  mni_temp = new_menu_item (_("Mass encode tool"), mni_tools_menu, on_mni_mass_encode);

  mni_temp = new_menu_sep (mni_functions_menu);

  mni_temp = new_menu_item (_("Insert"), mni_functions_menu, NULL);
  mni_ins_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_ins_menu);

  mni_temp = new_menu_item (_("Insert entity"), mni_ins_menu, NULL);
  mni_insert_char_entities_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_insert_char_entities_menu);

  fill_entities_special_menu ();

  mni_temp = new_menu_item (_("Insert DOCTYPE"), mni_ins_menu, NULL);
  mni_insert_doctype_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_insert_doctype_menu);

  mni_temp = new_menu_item ("HTML 4.1 Transitional", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("HTML 4.1 Strict", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("HTML 4.1 Frameset", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("XHTML 1.0 Strict", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("XHTML 1.0 Transitional", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("XHTML 1.0 Frameset", mni_insert_doctype_menu, on_mni_insert_doctype);
  mni_temp = new_menu_item ("WML 1.1", mni_insert_doctype_menu, on_mni_insert_doctype);

  mni_temp = new_menu_item (_("Lorem ipsum"), mni_ins_menu, on_mni_lorem_ipsum);
  mni_temp = new_menu_item (_("Date/Time"), mni_ins_menu, on_mni_funx_ins_time);
  mni_temp = new_menu_item (_("Dump menu"), mni_ins_menu, on_mni_dump_menu);


  mni_temp = new_menu_item (_("Case"), mni_functions_menu, NULL);
  mni_functions_case_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_functions_case_menu);

  mni_upcase = new_menu_item (_("UPCASE"), mni_functions_case_menu, on_mni_case_upcase_activate);
  gtk_widget_add_accelerator (mni_upcase, "activate", accel_group, GDK_KEY_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  mni_locase = new_menu_item (_("lowcase"), mni_functions_case_menu, on_mni_case_locase_activate);
  gtk_widget_add_accelerator (mni_locase, "activate", accel_group, GDK_KEY_Down, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  mni_temp = new_menu_item (_("Upcase The First Letter Of Each Word"), mni_functions_case_menu, on_mni_upcase_each_first_letter_ofw);

  mni_temp = new_menu_item (_("cRAcKeriZE"), mni_functions_case_menu, on_mni_crackerize);

  mni_temp = new_menu_item (_("Numbers"), mni_functions_menu, NULL);
  mni_functions_number_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_functions_number_menu);

  mni_temp = new_menu_item (_("Calculate"), mni_functions_number_menu, on_mni_calc);
  mni_temp = new_menu_item (_("Arabian to Roman"), mni_functions_number_menu, on_mni_Functions_number_arabian2roman);
  mni_temp = new_menu_item (_("Roman to Arabian"), mni_functions_number_menu, on_mni_Functions_number_roman2arabian);

  mni_temp = new_menu_item (_("HEX to decimal"), mni_functions_number_menu, on_mni_Functions_number_hex2dec);
  mni_temp = new_menu_item (_("Decimal to hex"), mni_functions_number_menu, on_mni_dec2hex);
  mni_temp = new_menu_item (_("Binary to decimal"), mni_functions_number_menu, on_mni_Functions_number_bin2dec);
  mni_temp = new_menu_item (_("Counter"), mni_functions_number_menu, on_mni_make_numbering);

  mni_temp = new_menu_item (_("Format"), mni_functions_menu, NULL);
  mni_format_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_format_menu);

  mni_temp = new_menu_item (_("Browser to text"), mni_format_menu, on_mni_copied_html_to_ftext);

  mni_temp = new_menu_item (_("Kill formatting"), mni_format_menu, on_mni_kill_formatting);
  mni_temp = new_menu_item (_("Kill formatting on each line"), mni_format_menu, on_mni_kill_formatting_on_each_line);

  mni_temp = new_menu_item (_("Wrap raw at position"), mni_format_menu, on_mni_wrap_raw);
  mni_temp = new_menu_item (_("Wrap on spaces at position"), mni_format_menu, on_mni_wrap_on_spaces);

  mni_temp = new_menu_item (_("Kill leading whitespace on each line"), mni_format_menu, on_mni_strings_remove_leading_whitespaces);
  mni_temp = new_menu_item (_("Kill trailing whitespace on each line"), mni_format_menu, on_mni_strings_remove_trailing_whitespaces);

  mni_temp = new_menu_item (_("Sort"), mni_functions_menu, NULL);
  mni_sort_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_sort_menu);

  mni_temp = new_menu_item (_("Sort lines"), mni_sort_menu, on_mni_func_strings_sort);
  mni_temp = new_menu_item (_("Sort lines by length"), mni_sort_menu, on_mni_func_strings_sort_by_q);
  mni_temp = new_menu_item (_("Sort lines case insensitive"), mni_sort_menu, on_mni_sort_case_insensetive);
  mni_temp = new_menu_item (_("Reverse order of lines"), mni_sort_menu, on_mni_func_strings_rev);

  mni_temp = new_menu_item (_("String"), mni_functions_menu, NULL);
  mni_functions_string_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_functions_string_menu);

  mni_temp = new_menu_item (_("Double quotes to facing quotes"), mni_functions_string_menu, on_mni_quotes_to_facing_quotes);

  mni_temp = new_menu_item (_("CSV-like table to LaTeX table"), mni_functions_string_menu, on_mni_csv_to_latex_table);
  mni_temp = new_menu_item (_("Convert usual quotes to TeX quotes"), mni_functions_string_menu, on_mni_quotes_to_tex);
  mni_temp = new_menu_item (_("Escape and quote"), mni_functions_string_menu, on_mni_quote_string);
  mni_temp = new_menu_item (_("Unescape and unquote"), mni_functions_string_menu, on_mni_string_unquote);
  mni_temp = new_menu_item (_("Reverse"), mni_functions_string_menu, on_mni_reverse);
  mni_temp = new_menu_item (_("Antispam e-mail"), mni_functions_string_menu, on_mni_antispam);
  mni_temp = new_menu_item (_("Remove blank lines"), mni_functions_string_menu, on_mni_func_strings_removeblanks);
  mni_temp = new_menu_item (_("Kill duplicates"), mni_functions_string_menu, on_mni_str_kill_dups);
  mni_temp = new_menu_item (_("Numerate lines"), mni_functions_string_menu, on_mni_count_string_list);

  mni_temp = new_menu_item (_("Shuffle lines"), mni_functions_string_menu, on_mni_shuffle_strings);
  mni_temp = new_menu_item (_("Convert tabs to spaces"), mni_functions_string_menu, on_mni_tabs_to_spaces);
  mni_temp = new_menu_item (_("Convert spaces to tabs"), mni_functions_string_menu, on_mni_spaces_to_tabs);
  mni_temp = new_menu_item (_("Split to TeX paragraphs"), mni_functions_string_menu, on_mni_split_to_tex_paragraphs);
  mni_temp = new_menu_item (_("Split by the delimiter"), mni_functions_string_menu, on_mni_split_by);
  mni_temp = new_menu_item (_("Split after the delimiter"), mni_functions_string_menu, on_mni_split_after_delimiter);

  mni_apply_tpl_to_each_ln = new_menu_item (_("Apply a template to each line"), mni_functions_string_menu, on_mni_func_strings_enclose);

  mni_temp = new_menu_item (_("Filter"), mni_functions_menu, NULL);
  mni_filter_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_filter_menu);

  mni_temp = new_menu_item (_("Extract at the each line"), mni_filter_menu, on_mni_extract_at_each_line);
  mni_temp = new_menu_item (_("Extract at the each line before a separator"), mni_filter_menu, on_mni_extract_at_each_line_bef_sep);
  mni_temp = new_menu_item (_("Extract at the each line after a separator"), mni_filter_menu, on_mni_extract_at_each_line_aft_sep);

  mni_temp = new_menu_item (_("Kill lines containing the phrase"), mni_filter_menu, on_mni_func_filter_exclude_from_list);
  mni_temp = new_menu_item (_("Kill all lines except with the phrase"), mni_filter_menu, on_mni_func_filter_antiexclude_from_list);

  mni_temp = new_menu_item (_("Kill lines containing a regexp"), mni_filter_menu, on_mni_func_filter_regexp_exclude_from_list);
  mni_temp = new_menu_item (_("Kill all lines without a regexp"), mni_filter_menu, on_mni_func_filter_regexp_incl_to_list);

  mni_temp = new_menu_item (_("Kill all lines <= N characters"), mni_filter_menu, on_mni_filter_kill_lesser);
  mni_temp = new_menu_item (_("Kill all lines > N characters"), mni_filter_menu, on_mni_filter_kill_more_than);

  mni_temp = new_menu_item (_("End of line"), mni_functions_menu, NULL);
  mni_eol_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_eol_menu);
  mni_temp = new_menu_item (_("Convert to LF"), mni_eol_menu, on_mni_eol_to_lf);
  mni_temp = new_menu_item (_("Convert to CR"), mni_eol_menu, on_mni_eol_to_cr);
  mni_temp = new_menu_item (_("Convert to CR/LF"), mni_eol_menu, on_mni_eol_to_crlf);

  mni_temp = new_menu_item (_("Morse code"), mni_functions_menu, NULL);
  mni_morse_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_morse_menu);
  mni_temp = new_menu_item (_("Encode to Morse code EN"), mni_morse_menu, on_mni_morse_encode);
  mni_temp = new_menu_item (_("Decode from Morse code EN"), mni_morse_menu, on_mni_morse_decode);

  mni_temp = new_menu_item (_("Last used"), mni_functions_menu, on_mni_repeat_the_last);

  mni_tabs = new_menu_item (_("Tabs"), menubar1, NULL);
  mni_tabs_menu = new_menu_submenu (mni_tabs);

  mni_ext_programs = new_menu_item (_("Run"), menubar1, NULL);
  mni_ext_programs_menu = new_menu_submenu (mni_ext_programs);

  mni_temp = new_menu_item (_("Nav"), menubar1, NULL);
  mni_nav_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_nav_menu);

  mni_links = new_menu_item (_("Links"), mni_nav_menu, NULL);
  mni_links_menu = new_menu_submenu (mni_links);

  mni_temp = new_menu_item (_("Save the position"), mni_nav_menu, on_mni_save_pos);
  mni_temp = new_menu_item (_("Jump to the saved position"), mni_nav_menu, on_mni_jump_to_saved_pos);

  mni_block_start = new_menu_item (_("Go to the block start {"), mni_nav_menu, on_mni_nav_block_start);
  gtk_widget_add_accelerator (mni_block_start, "activate", accel_group,
                              GDK_KEY_leftarrow, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  mni_block_end = new_menu_item (_("Go to the block end }"), mni_nav_menu, on_mni_nav_block_end);
  gtk_widget_add_accelerator (mni_block_end, "activate", accel_group,
                              GDK_KEY_rightarrow, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  mni_temp = new_menu_item (_("Go to recent tab"), mni_nav_menu, on_mni_nav_goto_recent_tab);
  mni_temp = new_menu_item (_("Go to next tab"), mni_nav_menu, on_mni_nav_goto_next_tab);
  mni_temp = new_menu_item (_("Go to previous tab"), mni_nav_menu, on_mni_nav_goto_prev_tab);
  mni_temp = new_menu_item (_("Go to first tab"), mni_nav_menu, on_mni_nav_goto_first_tab);
  mni_temp = new_menu_item (_("Go to last tab"), mni_nav_menu, on_mni_nav_goto_last_tab);
  mni_temp = new_menu_item (_("Go to selection"), mni_nav_menu, on_mni_nav_goto_selection);

  mni_temp = new_menu_item (_("Focus the FAMOUS text entry"), mni_nav_menu, on_mni_nav_focus_to_famous);
  mni_temp = new_menu_item (_("Focus the text"), mni_nav_menu, on_mni_nav_focus_to_text);

  mni_open_at_cursor = new_menu_item (_("Open at cursor"), mni_nav_menu, on_mni_file_open_at_cursor);

  mni_temp = new_menu_item (_("Edit at cursor"), mni_nav_menu, on_mni_file_edit_at_cursor);
  mni_temp = new_menu_item (_("Browse the symbol's definition"), mni_nav_menu, on_mni_browse_symbol_decl);

  mni_temp = new_menu_item (_("View"), menubar1, NULL);
  mni_view_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_view_menu);

  mni_co = new_menu_item (_("Encoding"), mni_view_menu, NULL);
  mni_co_menu = new_menu_submenu (mni_co);

  
  mni_profiles = new_menu_item (_("Profiles"), mni_view_menu, NULL);
  mni_profiles_menu = new_menu_submenu (mni_profiles);
  mni_temp = new_menu_item (_("Save profile"), mni_view_menu, on_mni_file_save_profile);

  mni_temp = new_menu_item (_("Hide error marks"), mni_view_menu, on_mni_hide_error_marks);
  mni_word_wrap = new_menu_item (_("Word wrap"), mni_view_menu, on_mni_view_wrap);
  mni_temp = new_menu_item (_("Line numbers"), mni_view_menu, on_mni_view_show_line_numbers);
  mni_temp = new_menu_item (_("Toggle image visibility"), mni_view_menu, on_mni_show_images_in_text);
  mni_temp = new_menu_item ("Imageplane", mni_view_menu, on_mni_imageplane);
  mni_temp = new_menu_item (_("Switch header/source"), mni_view_menu, on_source_header_switch);
  mni_temp = new_menu_item (_("Preview with Mplayer"), mni_view_menu, on_mni_nav_mplayer_jump);
  mni_temp = new_menu_item (_("Fullscreen"), mni_view_menu, on_mni_fullscreen);
  mni_temp = new_menu_item (_("Un-fullscreen"), mni_view_menu, on_mni_unfullscreen);
  mni_temp = new_menu_item (_("Window on top"), mni_view_menu, on_mni_toggle_topmost);


  mni_temp = new_menu_item (_("Help"), menubar1, NULL);
  mni_what_menu = new_menu_submenu (mni_temp);
  mni_temp = new_menu_tof (mni_what_menu);

  mni_temp = new_menu_item (_("About"), mni_what_menu, on_mni_about_activate);

  mni_temp = new_menu_item (_("Read the fine manual"), mni_what_menu, on_mni_rtfm);
  mni_temp = new_menu_item (_("Read the fine English manual anyway"), mni_what_menu, on_mni_rtfm_en);
  mni_temp = new_menu_item (_("Self check"), mni_what_menu, on_mni_self_check);

  mni_temp = new_menu_sep (mni_what_menu);
  mni_temp = new_menu_item ("NEWS", mni_what_menu, on_mni_help_item);
  mni_temp = new_menu_item ("ChangeLog", mni_what_menu, on_mni_help_item);
  mni_temp = new_menu_item ("TODO", mni_what_menu, on_mni_help_item);
  mni_temp = new_menu_item ("COPYING", mni_what_menu, on_mni_help_item);

  tb_main_toolbar = create_hardcoded_toolbar();
  gtk_box_pack_start (GTK_BOX (mainvbox), tb_main_toolbar, FALSE, TRUE, UI_PACKFACTOR);

  vpaned1 = gtk_vpaned_new();
  gtk_widget_show (vpaned1);
  gtk_box_pack_start (GTK_BOX (mainvbox), vpaned1, TRUE, TRUE, UI_PACKFACTOR);
  gtk_paned_set_position (GTK_PANED (vpaned1), 0);

  notebook1 = gtk_notebook_new ();
  gtk_widget_set_name (notebook1, "notebook1");
  gtk_widget_show (notebook1);

  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook1), TRUE);

  gtk_paned_pack1 (GTK_PANED (vpaned1), notebook1, TRUE, TRUE);

  vbox2 = gtk_vbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (vbox2);
  gtk_paned_pack2 (GTK_PANED (vpaned1), vbox2, TRUE, TRUE);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow3);

  gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow3, TRUE, TRUE, UI_PACKFACTOR);

  tv_logmemo = gtk_text_view_new ();
  gtk_widget_show (tv_logmemo);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), tv_logmemo);

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv_logmemo), GTK_WRAP_WORD);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_paned_set_position (vpaned1, 300);

  GtkWidget *fam_hbox = gtk_hbox_new (FALSE, UI_PACKFACTOR);
  gtk_widget_show (fam_hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), fam_hbox, FALSE, FALSE, UI_PACKFACTOR);

  ent_search = gtk_entry_new ();
  gtk_widget_show (ent_search);
  gtk_box_pack_start (GTK_BOX (fam_hbox), ent_search, TRUE, TRUE, UI_PACKFACTOR);

  //GtkEntryCompletion *comp_fte = gtk_entry_completion_new ();
  //GtkTreeModel *completion_model = gtk_list_store_new (1, G_TYPE_STRING);
  //gtk_entry_completion_set_model (comp_fte, completion_model);
  //gtk_entry_set_completion (ent_search, comp_fte);
  //gtk_entry_completion_set_text_column (comp_fte, 0);
//  gtk_entry_completion_set_minimum_key_length (comp_fte, confile.fte_min_prefix_len);
  //g_object_unref (completion_model);
  //g_object_unref (comp_fte);

  GtkWidget* bt_fam = tea_stock_button (GTK_STOCK_FIND);
  gtk_widget_show (bt_fam);
  gtk_box_pack_start (GTK_BOX (fam_hbox), bt_fam, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_fam, "clicked",
                    G_CALLBACK (on_doit_button),
                    NULL);

  GtkWidget* bt_open = tea_stock_button (GTK_STOCK_OPEN);
  gtk_widget_show (bt_open);
  gtk_box_pack_start (GTK_BOX (fam_hbox), bt_open, FALSE, FALSE, UI_PACKFACTOR);

  g_signal_connect ((gpointer) bt_open, "clicked",
                    G_CALLBACK (on_bt_open),
                    NULL);

  
  gtk_widget_set_tooltip_text (bt_fam,
                        _("Click here to search this through the text"));
  
    
  gtk_widget_set_tooltip_text (bt_open,
                        _("Click here to open a file"));
  
  g_signal_connect ((gpointer) ent_search, "key_press_event",
                    G_CALLBACK (on_ent_search_key_press_event),
                    NULL);

  statusbar1 = gtk_statusbar_new ();
  gtk_widget_show (statusbar1);
  gtk_box_pack_start (GTK_BOX (vbox2), statusbar1, FALSE, FALSE, UI_PACKFACTOR);

  pb_status = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (statusbar1), pb_status, FALSE, FALSE, UI_PACKFACTOR);
  gtk_progress_bar_set_pulse_step (pb_status, 0.05);


 // gtk_progress_bar_set_ellipsize (pb_status, PANGO_ELLIPSIZE_END);

  g_signal_connect ((gpointer) notebook1, "switch_page",
                    G_CALLBACK (on_notebook1_switch_page),
                    NULL);

  gtk_window_add_accel_group (GTK_WINDOW (tea_main_window), accel_group);

  set_dnd_accept_notebook (notebook1);

  log_memo_textbuffer = gtk_text_view_get_buffer (tv_logmemo);

  gtk_text_buffer_create_tag (log_memo_textbuffer, "lm_greet",
                              "foreground", "#6c0606", NULL);

  tag_lm_error = gtk_text_buffer_create_tag (log_memo_textbuffer, "lm_error",
                                             "foreground", "red", NULL);

  gtk_text_buffer_create_tag (log_memo_textbuffer, "lm_normal",
                              "foreground", confile.text_foreground, NULL);

  gtk_text_buffer_create_tag (log_memo_textbuffer, "lm_advice",
                              "foreground", "navy", NULL);

  gtk_text_buffer_create_tag (log_memo_textbuffer, "lm_bold",
                              "foreground", confile.text_foreground, "font", "bold", NULL);


  set_lm_colors ();
  widget_apply_colors (tv_logmemo);
  widget_apply_colors (ent_search);

  g_signal_connect ((gpointer) tv_logmemo, "button-press-event",
                    G_CALLBACK (cb_logmemo_button_press_event),
                    tv_logmemo);


  log_to_memo (_("PLEASE read the manual :)"), NULL, LM_GREET);
  log_to_memo ("2000-2013 Peter Semiletov <tea@list.ru>", NULL, LM_GREET);

  gchar* teavers;


  teavers = g_strconcat ("TEA ", VERSION, " @ http://www.semiletov.org/tea", NULL);

  log_to_memo (teavers, NULL, LM_GREET);

  g_free (teavers);

  mni_last = NULL;
  ui_init();

  
  gtk_widget_set_tooltip_text (ent_search, _("It is the Famous text entry... Read the Manual for more!"));
  

  if (g_file_test (TEA_PIX_MAINICON, G_FILE_TEST_EXISTS))
     gtk_window_set_default_icon_from_file (TEA_PIX_MAINICON, NULL);

  return tea_main_window;
}
