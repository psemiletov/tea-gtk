/***************************************************************************
                          tea_gtk_utils.c  -  description
                             -------------------
    begin                : aug. 2004
    copyleft            : 2004-2013 by Peter Semiletov
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

#include "tea_defs.h"

#ifdef HAVE_LIBASPELL

#include "aspell.h"


static AspellConfig *spell_config;
static AspellCanHaveError *ret;
static AspellSpeller *speller;


void spell_checker_ch_lang (const gchar *lang)
{
  aspell_config_replace (spell_config, "lang", lang);
  aspell_config_replace (spell_config, "encoding", "UTF-8");

  if (speller)
    delete_aspell_speller (speller);

  ret = new_aspell_speller (spell_config);

  if (aspell_error (ret) != 0)
     {
      g_print ("Error: %s\n", aspell_error_message (ret));
      delete_aspell_can_have_error (ret);
      return;
     }

  speller = to_aspell_speller (ret);
}



void dict_add_to_user_dict (const gchar *lang, const gchar *word)
{
  if (! lang || ! word)
     return;

  aspell_speller_add_to_personal (speller, word, strlen (word));
  aspell_speller_save_all_word_lists (speller);
}


GList* get_speller_modules_list (void)
{
  GList *l = NULL;
  AspellDictInfoList *dlist;
  AspellDictInfoEnumeration *dels;
  const AspellDictInfo *entry;

  dlist = get_aspell_dict_info_list (spell_config);

  dels = aspell_dict_info_list_elements (dlist);

  while ((entry = aspell_dict_info_enumeration_next (dels)) != 0)
        if (entry)
           l = g_list_prepend (l, g_strdup (entry->name));

  delete_aspell_dict_info_enumeration (dels);

  return l;
}
#endif




void spell_checker_init (const gchar *lang)
{
#ifdef HAVE_LIBASPELL

  spell_config = new_aspell_config ();

  aspell_config_replace (spell_config, "lang", lang);
  aspell_config_replace (spell_config, "encoding", "UTF-8");

  ret = new_aspell_speller (spell_config);

  if (aspell_error (ret) != 0)
     {
      g_print ("Error: %s\n", aspell_error_message (ret));
      delete_aspell_can_have_error (ret);
      return;
     }

  speller = to_aspell_speller (ret);

#endif

}


void spell_checker_done (done)
{
#ifdef HAVE_LIBASPELL

  if (speller)
    delete_aspell_speller (speller);
  if (spell_config)
    delete_aspell_config (spell_config);

#endif
}



gboolean spell_checker_check (const gchar *word)
{
   if (! word)
      return FALSE;

#ifdef HAVE_LIBASPELL
   return aspell_speller_check (speller, word, -1);
#endif

}



GList* get_suggestions_list (const gchar *s)
{
  if (! s)
     return NULL;

#ifdef HAVE_LIBASPELL

  GList *result = NULL;

  AspellWordList *suggestions = aspell_speller_suggest (speller, s, -1);

  if (! suggestions)
     return NULL;

  AspellStringEnumeration *elements = aspell_word_list_elements (suggestions);
  const char *word;

  while (word = aspell_string_enumeration_next (elements))
        result = g_list_prepend (result, g_strdup (word));

  delete_aspell_string_enumeration (elements);

  return g_list_reverse (result);

#endif


 return NULL;
}
