/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file. - thanks :)

//Peter Semiletov
28.11.2003, n.p. Nirvana "You Know You're Right"
and this project is starting
 */

/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Nov 28 2003
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



#include <gtk/gtk.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#include <fcntl.h>
#include <errno.h>


#include <glib/gi18n.h>

#include "tea_text_document.h" // Needs to be above for t_note_page struct
#include "callbacks.h"
#include "tea_config.h"
#include "tea_defs.h"
#include "interface.h" // Needed for create_tea_main_window


void sig_handler (int i)
{

  switch (i)
         {
          case SIGTERM:
          case SIGHUP:
          case SIGABRT:
          case SIGINT:
                      dbm ("sig_handler");
                      if (confile.do_save_all_on_termination)
                         doc_save_all();

                      if (confile.do_crash_file)
                         doc_save_emergency();

                      gtk_main_quit();
         }
}


int main (int argc, char *argv[])
{
  GtkWidget *app_main_window;

  /*
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif
*/

  setlocale (LC_ALL, "");

  
  //bindtextdomain ("teagtk", "/usr/local/share/locale");
 
  bindtextdomain ("teagtk", TEA_TRANS_DIR);
  bind_textdomain_codeset ("teagtk", "UTF-8");
  textdomain ("teagtk");  
  
// dbm (gettext("my mrga test"));
//  dbm (_("hiiiiii"));
//  gtk_set_locale ();

  g_argv = argv;
  g_argc = argc;

  gtk_init (&argc, &argv);

  signal (SIGHUP, sig_handler);
  signal (SIGABRT, sig_handler);
  signal (SIGTERM, sig_handler);
  signal (SIGINT, sig_handler);

  tea_start();

  app_main_window = create_tea_main_window();
  gtk_widget_show (app_main_window);

  //gtk_init_add (tea_init, NULL);

#ifdef SOUNDS_MODE
  sounds_init();
#endif

  update_enc_menu();
  reload_ext_programs();
  bmx_reload();
  reload_reptables();
  reload_scripts();
  reload_snippets();
  spell_checker_init (confile.def_spell_lang);
  reload_dicts();
  reload_sessions();
  reload_templates();
  reload_autosave();
  hotkeys_initial_check();
  reload_hotkeys();
  reload_autoreplace();
  reload_profiles();
  update_recent_list_menu (TRUE);

  tea_init (0); 
  
  gtk_main();
  tea_done();

  return 0;
}
