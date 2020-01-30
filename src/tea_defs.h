/***************************************************************************
                          tea_defs.h  -  description
                             -------------------
    begin                : Mon Dec 8 2003
    copyright            : (C) 2003, 2004, 2005 by Peter 'Roxton' Semiletov
    email                : peter.semiletov@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TEA_DEFS_H
#define TEA_DEFS_H

#include <libintl.h>
#include <locale.h>
 
#define eol_LF 0
#define eol_CRLF 1
#define eol_CR 2

#define CHAI_BASE_KEY "/apps/chai"
#define CHAI_PREFS_DIR CHAI_BASE_KEY "/preferences"
#define CHAI_SYNTAX_HL_DIR CHAI_PREFS_DIR "/syntax_hl"

#define cTEA_TEMP "tea_temp_file"
#define ERRO g_print("%s\n", strerror (errno))
#define DBM(x) g_print("%s\n", x)
#define STRMAXLEN G_MAXINT
#define dbm(x) g_print("%s\n", x)
#define CURR_LOCALE "current locale"

#define CHARSET_JAPANESE "Japanese autodetect"
#define CHARSET_CHINESE "Chinese autodetect"
#define CHARSET_AUTODETECT "autodetect"

#define LM_ERROR 13
#define LM_NORMAL 14
#define LM_ADVICE 15
#define LM_GREET 16
#define LM_BOLD 17

#define opt_ht_to_glist_full 1
#define opt_ht_to_glist_keys 2

#define MENUITEMUTIL "-x-"

#define MM_HTML 0
#define MM_XHTML 1
#define MM_WIKI 2
#define MM_TEX 3
#define MM_DOCBOOK 4

#define glist_from_string(x) glist_from_string_sep(x,"\n")
#define string_from_glist(x) string_from_glist_sep(x,"\n")

#define file_exists(x) g_file_test (x, G_FILE_TEST_EXISTS)

#define is_css(x) check_ext(x,".css")
#define is_rtf(x) check_ext(x,".rtf")
#define is_sxw(x) check_ext(x,".sxw")
#define is_kwd(x) check_ext(x,".kwd")
#define is_office(x) is_ext(x,".kwd",".sxw",".abw",".odt",NULL)
#define is_format_readonly(x) is_ext(x,".kwd",".sxw",".abw",".rtf",".gz",".zip",".bz2",".odt",".doc",NULL)

#define UI_PACKFACTOR 1

#define HL_NONE "none"
#define HL_C "C/C++"
#define HL_PASCAL "Pascal"
#define HL_PYTHON "Python"
#define HL_PHP "PHP"
#define HL_MARKUP "[X]HTML"
#define HL_BASH "BASH Script"
#define HL_PO "gettext PO file"
#define HL_TEX "LaTeX"


#ifdef FOR_DEBIAN

#define TEA_RUN_CMD "Exec=teaed"
#define TEA_DOC_DIR "/usr/share/doc/tea-data/"
#define TEA_PIX_MAINLOGO "/usr/share/pixmaps/tea_logo.jpg"
#define TEA_PIX_MAINICON "/usr/share/pixmaps/tea_icon_v2.png"
#define TEA_PIX_DIR "/usr/share/pixmaps/"

#endif

#define TEA_PIX_MAINLOGO PACKAGE_DATA_DIR"/images/tea_logo.jpg"
#define TEA_PIX_MAINICON PACKAGE_DATA_DIR"/images/tea_icon.png"
#define TEA_PIX_DIR PACKAGE_DATA_DIR"/images/"
#define TEA_RUN_CMD "Exec=teagtk"


#define doc_get_buf(x) get_text_buf(x)
#define get_tag_by_name(doc,name) gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(doc->text_buffer),name)

#define DSK_UNKNOWN 0
#define DSK_KDE 1
#define DSK_GNOME 2
#define DSK_XFCE 3

#define DCM_NONE 0
#define DCM_GREP 1
#define DCM_MAKE 2
#define DCM_LATEX 3

#define combo_get_value(combo,list) g_list_nth_data (list,gtk_combo_box_get_active(combo))

#endif