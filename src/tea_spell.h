#ifndef TEA_SPELL
#define TEA_SPELL


void spell_checker_init (const gchar *lang);
void spell_checker_done (void);
gboolean spell_checker_check (const gchar *word);
GList* get_suggestions_list (const gchar *s);


#ifdef HAVE_LIBASPELL
GList* get_speller_modules_list (void);
void dict_add_to_user_dict (const gchar *word);
void spell_checker_ch_lang (const gchar *lang);
gboolean spell_checker_check (const gchar *word);
#endif


#endif