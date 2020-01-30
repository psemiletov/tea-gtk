/***************************************************************************
                          latex_support.c  -  description
                             -------------------
    begin                : 17.02.2005
    copyleft            :  2005-2007 by Peter Semiletov
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

#include <libintl.h>
#include <locale.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <unistd.h> // needed for chdir

#include "tea_text_document.h" // Needed for log_to_memo
#include "interface.h"
#include "tea_config.h"
#include "tea_funx.h"
#include "tea_defs.h"
#include "rox_strings.h" // Needed for change_file_ext
#include "latex_support.h"


//Now playing: Guano Apes - Living In A Lie
void latex_dvi_to_ps (const gchar *filename)
{
  if (! filename)
     return;

  gchar *dir = g_path_get_dirname (filename);
  if (dir)
      chdir (dir);
  g_free (dir);

  gchar *f = change_file_ext (filename, "dvi");
  gchar *cm = str_replace_all (confile.cm_dvi_to_ps, "%s", f);

  gchar *r = run_process_and_wait (cm);

  log_to_memo (gettext("Done"), NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (r, NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (gettext("dvi to PS results for %s:"), f, LM_NORMAL);

  g_free (r);
  g_free (cm);
  g_free (f);
}


void latex_process (const gchar *filename)
{
  if (! filename)
     return;

  gchar *dir = g_path_get_dirname (filename);
  if (dir)
      chdir (dir);
  g_free (dir);

  gchar *cm = str_replace_all (confile.cm_run_latex, "%s", filename);
  gchar *r = run_process_and_wait (cm);

  log_to_memo (gettext("Done"), NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (r, NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (gettext("LaTeX processing results for %s:"), filename, LM_NORMAL);

  g_free (r);
  g_free (cm);
}


void pdflatex_process (const gchar *filename)
{
  if (! filename)
     return;

  gchar *dir = g_path_get_dirname (filename);
  if (dir)
      chdir (dir);
  g_free (dir);

  gchar *cm = str_replace_all (confile.cm_pdflatex, "%s", filename);
  gchar *r = run_process_and_wait (cm);

  log_to_memo (gettext("Done"), NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (r, NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (gettext("pdflatex processing results for %s:"), filename, LM_NORMAL);

  g_free (r);
  g_free (cm);
}


void latex_dvi_to_pdf (const gchar *filename)
{
  if (! filename)
     return;

  gchar *dir = g_path_get_dirname (filename);
  if (dir)
      chdir (dir);
  g_free (dir);

  gchar *f = change_file_ext (filename, "dvi");
  gchar *cm = str_replace_all (confile.cm_dvi_to_pdf, "%s", f);

  gchar *r = run_process_and_wait (cm);

  log_to_memo (gettext("Done"), NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (r, NULL, LM_NORMAL);
  log_to_memo ("-------", NULL, LM_NORMAL);
  log_to_memo (gettext("dvi to PDF results for %s:"), f, LM_NORMAL);

  g_free (r);
  g_free (cm);
  g_free (f);
}

//Now playing Guano Apes - Proud Like A God
void view_dvi (const gchar *filename)
{
  if (! filename)
     return;

  gchar *f = change_file_ext (filename, "dvi");
  gchar *cm = str_replace_all (confile.cm_dvi_view, "%s", f);
  g_spawn_command_line_async (cm, NULL);
  g_free (cm);
  g_free (f);
}


void view_ps (const gchar *filename)
{
  if (! filename)
     return;

  gchar *f = change_file_ext (filename, "dvi.ps");
  gchar *cm = str_replace_all (confile.cm_pdf_view, "%s", f);
  g_spawn_command_line_async (cm, NULL);
  g_free (cm);
  g_free (f);
}


void view_pdf (const gchar *filename)
{
  if (! filename)
     return;

  gchar *f = change_file_ext (filename, "pdf");
  gchar *cm = str_replace_all (confile.cm_pdf_view, "%s", f);
  g_spawn_command_line_async (cm, NULL);
  g_free (cm);
  g_free (f);
}
