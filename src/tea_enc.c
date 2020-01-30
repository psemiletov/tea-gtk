/***************************************************************************
                          tea_enc.c  -  description
                             -------------------
    begin                : Oct 1 2004
    copyright            : (C) 2004-2006 by Peter 'Roxton' Semiletov
    email                : peter.semiletov@gmail.com
***************************************************************************/

/*
Tarot Osuji <tarot@sdf.lonestar.org> //Leafcode editor,
http://tarot.freeshell.org/leafpad/
*/

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

#include <gtk/gtk.h>
#include <string.h>


#include "tea_text_document.h"
#include "tea_config.h"
#include "rox_strings.h"


gboolean enc_is_finnish (const gchar *s)
{
  gchar *a[8] = {
                 " eri ",
                 " sen ",
                 " ovat ",
                 " voi ",
                 " mutta ",
                 " joka ",
                 " jotta ",
                 " sen "
                 };

  gint i;
  for (i = 0; i <= 7; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_koi8r (const gchar *s)
{
  gchar *a[15] = {
                  " ÎÁ ",
                  " ÎÅ ",
                  " ÂÅÚ ",
                  " ÔÕÔ ",
                  " ÖÅ ",
                  " ÄÏ ",
                  " ÅÝÅ ",
                  " ×Ù ",
                  " ÍÏÊ ",
                  " ÅÇÏ ",
                  " ÅÅ ",
                  " ×ÁÓ ",
                  " ÎÁÓ ",
                  " ÎÉÈ ",
                  " ÉÈ "
                 };

  gint i;

  for (i = 0; i <= 14; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_koi8u (const gchar *s)
{
  gchar *a[15] = {
                  " ×É ",
                  " ÝÅ ",
                  " ÑË¦",
                  " ÁÂÏ ",
                  " Î¦Ö ",
                  " ×¦Î ",
                  " ÝÏ ",
                  " ÊÏÇÏ ",
                  " ¦Ú ",
                  " Í¦Ö ",
                  " ¦Í'Ñ ",
                  " Ð¦Ä ",
                  " ÃÀ ",
                  " ÒÅÞ¦ ",
                  " §È "
                  };

  gint i;

  for (i = 0; i <= 14; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_cp1251 (const gchar *s)
{
  gchar *a[15] = {
                  " íà ",
                  " íå ",
                  " áåç ",
                  " òóò ",
                  " æå ",
                  " äî ",
                  " åùå ",
                  " âû ",
                  " ìîé ",
                  " åãî ",
                  " åå ",
                  " âàñ ",
                  " íàñ ",
                  " íèõ ",
                  " èõ "
                  };

  gint i;
  for (i = 0; i <= 14; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_dos866 (const gchar *s)
{
  gchar *a[15] = {
                  " ­  ",
                  " ­¥ ",
                  " ¡¥§ ",
                  " âãâ ",
                  " ¦¥ ",
                  " ¤® ",
                  " ¥é¥ ",
                  " ¢ë ",
                  " ¬®© ",
                  " ¥£® ",
                  " ¥¥ ",
                  " ¢ á ",
                  " ­ á ",
                  " ­¨å ",
                  " ¨å "
                  };

  gint i;
  for (i = 0; i <= 14; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_serbian (const gchar *s)
{
  gchar *a[7] = {
                 " kako ",
                 " koji ",
                 " druge ",
                 " koja ",
                 " skup ",
                 " ako ",
                 " drugu "
                 };

  gint i;
  for (i = 0; i <= 6; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_german (const gchar *s)
{
  gchar *a[8] = {
                 " der ",
                 " das ",
                 " ich ",
                 " ist ",
                 " uns ",
                 " dem ",
                 " und ",
                 " auf "
                 };

  gint i;
  for (i = 0; i <= 7; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_polish (const gchar *s)
{
  gchar *a[7] = {
                 " nie ",
                 " gdzie ",
                 " sie ",
                 " wam ",
                 " jest ",
                 " mozna ",
                 " moze "
                 };

  gint i;
  for (i = 0; i <= 6; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_latvian (const gchar *s)
{
  gchar *a[6] = {
                 " lai ",
                 " visu ",
                 " visi ",
                 " arî ",
                 " ir ",
                 " vai "
                 };

  gint i;
  for (i = 0; i <= 5; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_slovak (const gchar *s)
{
  gchar *a[6] = {
                 " tomto ",
                 " sa ",
                 " jeho ",
                 " pre ",
                 " bolo ",
                 " ste "
                };

  gint i;
  for (i = 0; i <= 5; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_slovenian (const gchar *s)
{
  gchar *a[6] = {
                 " sta ",
                 " ki ",
                 " kot ",
                 " ter ",
                 " tudi ",
                 " vec "
                 };

  gint i;
  for (i = 0; i <= 5; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_spanish (const gchar *s)
{
  gchar *a[6] = {
                 " los ",
                 " las ",
                 " sobre ",
                 " dar ",
                 " del ",
                 " por "
                 };

  gint i;
  for (i = 0; i <= 5; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_czech (const gchar *s)
{
  gchar *a[6] = {
                 " byl ",
                 " ale ",
                 " jako ",
                 " tak ",
                 " jak ",
                 " jeho "
                 };

  gint i;
  for (i = 0; i <= 5; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_turkish (const gchar *s)
{
  gchar *a[4] = {
                 " bir ",
                 " bu ",
                 " ile ",
                 " olan "
                };

  gint i;
  for (i = 0; i <= 3; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


gboolean enc_is_portuguese (const gchar *s)
{
  gchar *a[4] = {
                 " por ",
                 " pelo ",
                 " um ",
                 " uma "
                };

  gint i;
  for (i = 0; i <= 3; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


//JIS X 0208-1990
gboolean enc_is_iso_2022_jp (const gchar *s)
{
  gchar *a[4] = {
                 "\x1b\x24\x40",
                 "\x1b\x24\x42",
                 "\x1b\x28\x42",
                 "\x1b\x28\x4a",
                };

  gint i;
  for (i = 0; i <= 3; i++)
      if (strstr (s, a[i]))
         return TRUE;

  return FALSE;
}


/*
GB = Guo Biao = Guojia Biaojun = National Standard

JIS = Japanese Industrial Standard
JIS = ISO-2022-JP
Shift-JIS = SJIS or MS Kanji or CP_932
EUC-JP
GB2312 = Chinese Simplified
Big5 = Chinese Traditional
*/


//Leafpad code by Tarot Osuji <tarot@sdf.lonestar.org>
//modified by roxton
gchar* detect_charset_zh (const gchar *text)
{
  guint8 c = *text;
  gchar *charset = g_strdup ("GB2312");

  while ((c = *text++) != '\0')
       {
        if (c >= 0x81 && c <= 0x87)
           {
            charset = ch_str (charset, "GB18030");
            break;
            }
        else
            if (c >= 0x88 && c <= 0xA0)
               {
                c = *text++;
                if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0))
                   {
                    charset = ch_str (charset, "GB18030");
                    break;
                    } //else GBK/Big5-HKSCS cannot determine
               }
             else
                 if ((c >= 0xA1 && c <= 0xC6) || (c >= 0xC9 && c <= 0xF9))
                    {
                     c = *text++;
                     if (c >= 0x40 && c <= 0x7E)
                        charset = ch_str ("Big5", charset);
                     else
                         if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0))
                            {
                             charset = ch_str (charset, "GB18030");
                             break;
                            }
                    }
                  else
                      if (c >= 0xC7)
                         {
                          c = *text++;
                          if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0))
                             {
                              charset = ch_str (charset, "GB18030");
                              break;
                             }
                         }
              }

  return charset;
}


gchar *detect_charset_ja (const gchar *text)
{
  if (enc_is_iso_2022_jp (text))
     return g_strdup ("ISO-2022-JP-2");

  if ( g_utf8_validate (text, -1, 0) )
     return g_strdup ("UTF-8");

  guint8 c = *text;
  gchar *charset = NULL;

  while (charset == NULL && (c = *text++) != '\0')
        {
         if (c >= 0x81 && c <= 0x9F)
            {
             if (c == 0x8E) /* SS2 */
                {
                 c = *text++;
                 if ((c >= 0x40 && c <= 0xA0) || (c >= 0xE0 && c <= 0xFC))
                    charset = ch_str (charset, "Shift_JIS");
                }
            else
                if (c == 0x8F) /* SS3 */
                   {
                    c = *text++;
                    if (c >= 0x40 && c <= 0xA0)
                       charset = ch_str (charset, "Shift_JIS");
                    else
                        if (c >= 0xFD)
                           break;
                   }
                else
                    charset = ch_str (charset, "Shift_JIS");
             }
         else
             if (c >= 0xA1 && c <= 0xDF)
                {
                 c = *text++;
                 if (c <= 0x9F)
                    charset = ch_str (charset, "Shift_JIS");
                 else
                     if (c >= 0xFD)
                        break;
                }
             else
                 if (c >= 0xE0 && c <= 0xEF)
                    {
                     c = *text++;
                     if (c >= 0x40 && c <= 0xA0)
                        charset = ch_str (charset, "Shift_JIS");
                     else
                         if (c >= 0xFD)
                            break;
                     }
                 else
                     if (c >= 0xF0)
                        break;
        }

  if (! charset)
     charset = ch_str (charset, "EUC-JP");

  return charset;
}
//end of the code from Leafpad


GList* create_enc_list (void)
{
  GList *l = NULL;

l = g_list_prepend (l, "YU");
l = g_list_prepend (l, "WS2");
l = g_list_prepend (l, "WINSAMI2");
l = g_list_prepend (l, "WINDOWS-1258");
l = g_list_prepend (l, "WINDOWS-1257");
l = g_list_prepend (l, "WINDOWS-1256");
l = g_list_prepend (l, "WINDOWS-1255");
l = g_list_prepend (l, "WINDOWS-1254");
l = g_list_prepend (l, "WINDOWS-1253");
l = g_list_prepend (l, "WINDOWS-1252");
l = g_list_prepend (l, "WINDOWS-1251");
l = g_list_prepend (l, "WINDOWS-1250");
l = g_list_prepend (l, "WINDOWS-936");
l = g_list_prepend (l, "WINDOWS-31J");
l = g_list_prepend (l, "WINBALTRIM");
l = g_list_prepend (l, "WIN-SAMI-2");
l = g_list_prepend (l, "WCHAR_T");
l = g_list_prepend (l, "VISCII1.1-1");
l = g_list_prepend (l, "VISCII");
l = g_list_prepend (l, "VISCII-1");
l = g_list_prepend (l, "UTF32LE");
l = g_list_prepend (l, "UTF32BE");
l = g_list_prepend (l, "UTF32");
l = g_list_prepend (l, "UTF16LE");
l = g_list_prepend (l, "UTF16BE");
l = g_list_prepend (l, "UTF16");
l = g_list_prepend (l, "UTF8");
l = g_list_prepend (l, "UTF7");
l = g_list_prepend (l, "UTF-32LE");
l = g_list_prepend (l, "UTF-32BE");
l = g_list_prepend (l, "UTF-32");
l = g_list_prepend (l, "UTF-16LE");
l = g_list_prepend (l, "UTF-16BE");
l = g_list_prepend (l, "UTF-16");
l = g_list_prepend (l, "UTF-8");
l = g_list_prepend (l, "UTF-7");
l = g_list_prepend (l, "US");
l = g_list_prepend (l, "US-ASCII");
l = g_list_prepend (l, "UNICODELITTLE");
l = g_list_prepend (l, "UNICODEBIG");
l = g_list_prepend (l, "UNICODE");
l = g_list_prepend (l, "UK");
l = g_list_prepend (l, "UJIS");
l = g_list_prepend (l, "UHC");
l = g_list_prepend (l, "UCS4");
l = g_list_prepend (l, "UCS2");
l = g_list_prepend (l, "UCS-4LE");
l = g_list_prepend (l, "UCS-4BE");
l = g_list_prepend (l, "UCS-4");
l = g_list_prepend (l, "UCS-2LE");
l = g_list_prepend (l, "UCS-2BE");
l = g_list_prepend (l, "UCS-2");
l = g_list_prepend (l, "TSCII");
l = g_list_prepend (l, "TSCII-0");
l = g_list_prepend (l, "TS-5881");
l = g_list_prepend (l, "TIS620");
l = g_list_prepend (l, "TIS620.2533-0");
l = g_list_prepend (l, "TIS620.2529-1");
l = g_list_prepend (l, "TIS620-0");
l = g_list_prepend (l, "TIS-620");
l = g_list_prepend (l, "TCVN5712-1:1993");
l = g_list_prepend (l, "TCVN5712-1");
l = g_list_prepend (l, "TCVN");
l = g_list_prepend (l, "TCVN-5712");
l = g_list_prepend (l, "TATAR-CYR");
l = g_list_prepend (l, "T.618BIT");
l = g_list_prepend (l, "T.61");
l = g_list_prepend (l, "T.61-8BIT");
l = g_list_prepend (l, "ST_SEV_358-88");
l = g_list_prepend (l, "SS636127");
l = g_list_prepend (l, "SJIS");
l = g_list_prepend (l, "SJIS-WIN");
l = g_list_prepend (l, "SJIS-OPEN");
l = g_list_prepend (l, "SHIFT_JISX0213");
l = g_list_prepend (l, "SHIFT_JIS");
l = g_list_prepend (l, "SHIFT-JIS");
l = g_list_prepend (l, "SEN_850200_C");
l = g_list_prepend (l, "SEN_850200_B");
l = g_list_prepend (l, "SE2");
l = g_list_prepend (l, "SE");
l = g_list_prepend (l, "RUSCII");
l = g_list_prepend (l, "ROMAN8");
l = g_list_prepend (l, "R8");
l = g_list_prepend (l, "PT154");
l = g_list_prepend (l, "PT2");
l = g_list_prepend (l, "PT");
l = g_list_prepend (l, "PT-CP154");
l = g_list_prepend (l, "OSF10020417");
l = g_list_prepend (l, "OSF10020402");
l = g_list_prepend (l, "OSF10020396");
l = g_list_prepend (l, "OSF10020388");
l = g_list_prepend (l, "OSF10020387");
l = g_list_prepend (l, "OSF10020370");
l = g_list_prepend (l, "OSF10020367");
l = g_list_prepend (l, "OSF10020366");
l = g_list_prepend (l, "OSF10020365");
l = g_list_prepend (l, "OSF10020364");
l = g_list_prepend (l, "OSF10020360");
l = g_list_prepend (l, "OSF10020359");
l = g_list_prepend (l, "OSF10020357");
l = g_list_prepend (l, "OSF10020354");
l = g_list_prepend (l, "OSF10020352");
l = g_list_prepend (l, "OSF10020129");
l = g_list_prepend (l, "OSF10020122");
l = g_list_prepend (l, "OSF10020118");
l = g_list_prepend (l, "OSF10020116");
l = g_list_prepend (l, "OSF10020115");
l = g_list_prepend (l, "OSF10020111");
l = g_list_prepend (l, "OSF10020025");
l = g_list_prepend (l, "OSF10010001");
l = g_list_prepend (l, "OSF1002037B");
l = g_list_prepend (l, "OSF1002036B");
l = g_list_prepend (l, "OSF1002035F");
l = g_list_prepend (l, "OSF1002035E");
l = g_list_prepend (l, "OSF1002035D");
l = g_list_prepend (l, "OSF1002011D");
l = g_list_prepend (l, "OSF1002011C");
l = g_list_prepend (l, "OSF100203B5");
l = g_list_prepend (l, "OSF100201F4");
l = g_list_prepend (l, "OSF100201B5");
l = g_list_prepend (l, "OSF100201A8");
l = g_list_prepend (l, "OSF100201A4");
l = g_list_prepend (l, "OSF05010001");
l = g_list_prepend (l, "OSF0005000A");
l = g_list_prepend (l, "OSF0004000A");
l = g_list_prepend (l, "OSF00030010");
l = g_list_prepend (l, "OSF00010106");
l = g_list_prepend (l, "OSF00010105");
l = g_list_prepend (l, "OSF00010104");
l = g_list_prepend (l, "OSF00010102");
l = g_list_prepend (l, "OSF00010101");
l = g_list_prepend (l, "OSF00010100");
l = g_list_prepend (l, "OSF00010020");
l = g_list_prepend (l, "OSF0001000A");
l = g_list_prepend (l, "OSF00010009");
l = g_list_prepend (l, "OSF00010008");
l = g_list_prepend (l, "OSF00010007");
l = g_list_prepend (l, "OSF00010006");
l = g_list_prepend (l, "OSF00010005");
l = g_list_prepend (l, "OSF00010004");
l = g_list_prepend (l, "OSF00010003");
l = g_list_prepend (l, "OSF00010002");
l = g_list_prepend (l, "OSF00010001");
l = g_list_prepend (l, "OS2LATIN1");
l = g_list_prepend (l, "NS_45512");
l = g_list_prepend (l, "NS_45511");
l = g_list_prepend (l, "NS_4551-2");
l = g_list_prepend (l, "NS_4551-1");
l = g_list_prepend (l, "NO2");
l = g_list_prepend (l, "NO");
l = g_list_prepend (l, "NF_Z_62010_1973");
l = g_list_prepend (l, "NF_Z_62010");
l = g_list_prepend (l, "NF_Z_62-010_1973");
l = g_list_prepend (l, "NF_Z_62-010_(1973)");
l = g_list_prepend (l, "NF_Z_62-010");
l = g_list_prepend (l, "NC_NC00-10:81");
l = g_list_prepend (l, "NC_NC00-10");
l = g_list_prepend (l, "NC_NC0010");
l = g_list_prepend (l, "NATSSEFI");
l = g_list_prepend (l, "NATSDANO");
l = g_list_prepend (l, "NATS-SEFI");
l = g_list_prepend (l, "NATS-DANO");
l = g_list_prepend (l, "NAPLPS");
l = g_list_prepend (l, "MULELAO-1");
l = g_list_prepend (l, "MS_KANJI");
l = g_list_prepend (l, "MSZ_7795.3");
l = g_list_prepend (l, "MSMACCYRILLIC");
l = g_list_prepend (l, "MSCP1361");
l = g_list_prepend (l, "MSCP949");
l = g_list_prepend (l, "MS936");
l = g_list_prepend (l, "MS932");
l = g_list_prepend (l, "MS-TURK");
l = g_list_prepend (l, "MS-MAC-CYRILLIC");
l = g_list_prepend (l, "MS-HEBR");
l = g_list_prepend (l, "MS-GREEK");
l = g_list_prepend (l, "MS-EE");
l = g_list_prepend (l, "MS-CYRL");
l = g_list_prepend (l, "MS-ARAB");
l = g_list_prepend (l, "MS-ANSI");
l = g_list_prepend (l, "MACUKRAINIAN");
l = g_list_prepend (l, "MACUK");
l = g_list_prepend (l, "MACIS");
l = g_list_prepend (l, "MACINTOSH");
l = g_list_prepend (l, "MACCYRILLIC");
l = g_list_prepend (l, "MAC");
l = g_list_prepend (l, "MAC-UK");
l = g_list_prepend (l, "MAC-SAMI");
l = g_list_prepend (l, "MAC-IS");
l = g_list_prepend (l, "MAC-CYRILLIC");
l = g_list_prepend (l, "LATINGREEK1");
l = g_list_prepend (l, "LATINGREEK");
l = g_list_prepend (l, "LATIN10");
l = g_list_prepend (l, "LATIN8");
l = g_list_prepend (l, "LATIN7");
l = g_list_prepend (l, "LATIN6");
l = g_list_prepend (l, "LATIN5");
l = g_list_prepend (l, "LATIN4");
l = g_list_prepend (l, "LATIN3");
l = g_list_prepend (l, "LATIN2");
l = g_list_prepend (l, "LATIN1");
l = g_list_prepend (l, "LATIN-GREEK");
l = g_list_prepend (l, "LATIN-GREEK-1");
l = g_list_prepend (l, "LATIN-9");
l = g_list_prepend (l, "L10");
l = g_list_prepend (l, "L8");
l = g_list_prepend (l, "L7");
l = g_list_prepend (l, "L6");
l = g_list_prepend (l, "L5");
l = g_list_prepend (l, "L4");
l = g_list_prepend (l, "L3");
l = g_list_prepend (l, "L2");
l = g_list_prepend (l, "L1");
l = g_list_prepend (l, "KSC5636");
l = g_list_prepend (l, "KOI8U");
l = g_list_prepend (l, "KOI8R");
l = g_list_prepend (l, "KOI8");
l = g_list_prepend (l, "KOI8-U");
l = g_list_prepend (l, "KOI8-T");
l = g_list_prepend (l, "KOI8-R");
l = g_list_prepend (l, "KOI8-K");
l = g_list_prepend (l, "KOI-8");
l = g_list_prepend (l, "KOI-7");
l = g_list_prepend (l, "KHMER-NS");
l = g_list_prepend (l, "JUS_I.B1.002");
l = g_list_prepend (l, "JS");
l = g_list_prepend (l, "JP");
l = g_list_prepend (l, "JP-OCR-B");
l = g_list_prepend (l, "JOHAB");
l = g_list_prepend (l, "JIS_C62291984B");
l = g_list_prepend (l, "JIS_C62201969RO");
l = g_list_prepend (l, "JIS_C6229-1984-B");
l = g_list_prepend (l, "JIS_C6220-1969-RO");
l = g_list_prepend (l, "IT");
l = g_list_prepend (l, "ISO_69372");
l = g_list_prepend (l, "ISO_10367BOX");
l = g_list_prepend (l, "ISO_10367-BOX");
l = g_list_prepend (l, "ISO_9036");
l = g_list_prepend (l, "ISO_8859-16:2001");
l = g_list_prepend (l, "ISO_8859-16");
l = g_list_prepend (l, "ISO_8859-15:1998");
l = g_list_prepend (l, "ISO_8859-15");
l = g_list_prepend (l, "ISO_8859-14:1998");
l = g_list_prepend (l, "ISO_8859-14");
l = g_list_prepend (l, "ISO_8859-10:1992");
l = g_list_prepend (l, "ISO_8859-10");
l = g_list_prepend (l, "ISO_8859-9:1989");
l = g_list_prepend (l, "ISO_8859-9");
l = g_list_prepend (l, "ISO_8859-8:1988");
l = g_list_prepend (l, "ISO_8859-8");
l = g_list_prepend (l, "ISO_8859-7:1987");
l = g_list_prepend (l, "ISO_8859-7");
l = g_list_prepend (l, "ISO_8859-6:1987");
l = g_list_prepend (l, "ISO_8859-6");
l = g_list_prepend (l, "ISO_8859-5:1988");
l = g_list_prepend (l, "ISO_8859-5");
l = g_list_prepend (l, "ISO_8859-4:1988");
l = g_list_prepend (l, "ISO_8859-4");
l = g_list_prepend (l, "ISO_8859-3:1988");
l = g_list_prepend (l, "ISO_8859-3");
l = g_list_prepend (l, "ISO_8859-2:1987");
l = g_list_prepend (l, "ISO_8859-2");
l = g_list_prepend (l, "ISO_8859-1:1987");
l = g_list_prepend (l, "ISO_8859-1");
l = g_list_prepend (l, "ISO_6937:1992");
l = g_list_prepend (l, "ISO_6937");
l = g_list_prepend (l, "ISO_6937-2:1983");
l = g_list_prepend (l, "ISO_6937-2");
l = g_list_prepend (l, "ISO_5428:1980");
l = g_list_prepend (l, "ISO_5428");
l = g_list_prepend (l, "ISO_5427EXT");
l = g_list_prepend (l, "ISO_5427:1981");
l = g_list_prepend (l, "ISO_5427");
l = g_list_prepend (l, "ISO_5427-EXT");
l = g_list_prepend (l, "ISO_2033");
l = g_list_prepend (l, "ISO_2033-1983");
l = g_list_prepend (l, "ISO_646.IRV:1991");
l = g_list_prepend (l, "ISO885916");
l = g_list_prepend (l, "ISO885915");
l = g_list_prepend (l, "ISO885914");
l = g_list_prepend (l, "ISO885913");
l = g_list_prepend (l, "ISO885911");
l = g_list_prepend (l, "ISO885910");
l = g_list_prepend (l, "ISO88599");
l = g_list_prepend (l, "ISO88598");
l = g_list_prepend (l, "ISO88597");
l = g_list_prepend (l, "ISO88596");
l = g_list_prepend (l, "ISO88595");
l = g_list_prepend (l, "ISO88594");
l = g_list_prepend (l, "ISO88593");
l = g_list_prepend (l, "ISO88592");
l = g_list_prepend (l, "ISO88591");
l = g_list_prepend (l, "ISO8859-16");
l = g_list_prepend (l, "ISO8859-15");
l = g_list_prepend (l, "ISO8859-14");
l = g_list_prepend (l, "ISO8859-13");
l = g_list_prepend (l, "ISO8859-11");
l = g_list_prepend (l, "ISO8859-10");
l = g_list_prepend (l, "ISO8859-9");
l = g_list_prepend (l, "ISO8859-8");
l = g_list_prepend (l, "ISO8859-7");
l = g_list_prepend (l, "ISO8859-6");
l = g_list_prepend (l, "ISO8859-5");
l = g_list_prepend (l, "ISO8859-4");
l = g_list_prepend (l, "ISO8859-3");
l = g_list_prepend (l, "ISO8859-2");
l = g_list_prepend (l, "ISO8859-1");
l = g_list_prepend (l, "ISO6937");
l = g_list_prepend (l, "ISO2022KR");
l = g_list_prepend (l, "ISO2022JP2");
l = g_list_prepend (l, "ISO2022JP");
l = g_list_prepend (l, "ISO2022CNEXT");
l = g_list_prepend (l, "ISO2022CN");
l = g_list_prepend (l, "ISO646-YU");
l = g_list_prepend (l, "ISO646-US");
l = g_list_prepend (l, "ISO646-SE2");
l = g_list_prepend (l, "ISO646-SE");
l = g_list_prepend (l, "ISO646-PT2");
l = g_list_prepend (l, "ISO646-PT");
l = g_list_prepend (l, "ISO646-NO2");
l = g_list_prepend (l, "ISO646-NO");
l = g_list_prepend (l, "ISO646-KR");
l = g_list_prepend (l, "ISO646-JP");
l = g_list_prepend (l, "ISO646-JP-OCR-B");
l = g_list_prepend (l, "ISO646-IT");
l = g_list_prepend (l, "ISO646-HU");
l = g_list_prepend (l, "ISO646-GB");
l = g_list_prepend (l, "ISO646-FR1");
l = g_list_prepend (l, "ISO646-FR");
l = g_list_prepend (l, "ISO646-FI");
l = g_list_prepend (l, "ISO646-ES2");
l = g_list_prepend (l, "ISO646-ES");
l = g_list_prepend (l, "ISO646-DK");
l = g_list_prepend (l, "ISO646-DE");
l = g_list_prepend (l, "ISO646-CU");
l = g_list_prepend (l, "ISO646-CN");
l = g_list_prepend (l, "ISO646-CA2");
l = g_list_prepend (l, "ISO646-CA");
l = g_list_prepend (l, "ISO-IR-226");
l = g_list_prepend (l, "ISO-IR-209");
l = g_list_prepend (l, "ISO-IR-203");
l = g_list_prepend (l, "ISO-IR-199");
l = g_list_prepend (l, "ISO-IR-197");
l = g_list_prepend (l, "ISO-IR-193");
l = g_list_prepend (l, "ISO-IR-179");
l = g_list_prepend (l, "ISO-IR-166");
l = g_list_prepend (l, "ISO-IR-157");
l = g_list_prepend (l, "ISO-IR-156");
l = g_list_prepend (l, "ISO-IR-155");
l = g_list_prepend (l, "ISO-IR-153");
l = g_list_prepend (l, "ISO-IR-151");
l = g_list_prepend (l, "ISO-IR-150");
l = g_list_prepend (l, "ISO-IR-148");
l = g_list_prepend (l, "ISO-IR-144");
l = g_list_prepend (l, "ISO-IR-143");
l = g_list_prepend (l, "ISO-IR-141");
l = g_list_prepend (l, "ISO-IR-139");
l = g_list_prepend (l, "ISO-IR-138");
l = g_list_prepend (l, "ISO-IR-127");
l = g_list_prepend (l, "ISO-IR-126");
l = g_list_prepend (l, "ISO-IR-122");
l = g_list_prepend (l, "ISO-IR-121");
l = g_list_prepend (l, "ISO-IR-111");
l = g_list_prepend (l, "ISO-IR-110");
l = g_list_prepend (l, "ISO-IR-109");
l = g_list_prepend (l, "ISO-IR-103");
l = g_list_prepend (l, "ISO-IR-101");
l = g_list_prepend (l, "ISO-IR-100");
l = g_list_prepend (l, "ISO-IR-99");
l = g_list_prepend (l, "ISO-IR-98");
l = g_list_prepend (l, "ISO-IR-92");
l = g_list_prepend (l, "ISO-IR-90");
l = g_list_prepend (l, "ISO-IR-89");
l = g_list_prepend (l, "ISO-IR-88");
l = g_list_prepend (l, "ISO-IR-86");
l = g_list_prepend (l, "ISO-IR-85");
l = g_list_prepend (l, "ISO-IR-84");
l = g_list_prepend (l, "ISO-IR-69");
l = g_list_prepend (l, "ISO-IR-61");
l = g_list_prepend (l, "ISO-IR-60");
l = g_list_prepend (l, "ISO-IR-57");
l = g_list_prepend (l, "ISO-IR-55");
l = g_list_prepend (l, "ISO-IR-54");
l = g_list_prepend (l, "ISO-IR-51");
l = g_list_prepend (l, "ISO-IR-50");
l = g_list_prepend (l, "ISO-IR-49");
l = g_list_prepend (l, "ISO-IR-37");
l = g_list_prepend (l, "ISO-IR-27");
l = g_list_prepend (l, "ISO-IR-25");
l = g_list_prepend (l, "ISO-IR-21");
l = g_list_prepend (l, "ISO-IR-19");
l = g_list_prepend (l, "ISO-IR-18");
l = g_list_prepend (l, "ISO-IR-17");
l = g_list_prepend (l, "ISO-IR-16");
l = g_list_prepend (l, "ISO-IR-15");
l = g_list_prepend (l, "ISO-IR-14");
l = g_list_prepend (l, "ISO-IR-11");
l = g_list_prepend (l, "ISO-IR-10");
l = g_list_prepend (l, "ISO-IR-9-1");
l = g_list_prepend (l, "ISO-IR-8-1");
l = g_list_prepend (l, "ISO-IR-6");
l = g_list_prepend (l, "ISO-IR-4");
l = g_list_prepend (l, "ISO-CELTIC");
l = g_list_prepend (l, "ISO-10646/UTF8/");
l = g_list_prepend (l, "ISO-10646/UTF-8/");
l = g_list_prepend (l, "ISO-10646/UCS4/");
l = g_list_prepend (l, "ISO-10646/UCS2/");
l = g_list_prepend (l, "ISO-10646");
l = g_list_prepend (l, "ISO-8859-16");
l = g_list_prepend (l, "ISO-8859-15");
l = g_list_prepend (l, "ISO-8859-14");
l = g_list_prepend (l, "ISO-8859-13");
l = g_list_prepend (l, "ISO-8859-11");
l = g_list_prepend (l, "ISO-8859-10");
l = g_list_prepend (l, "ISO-8859-9E");
l = g_list_prepend (l, "ISO-8859-9");
l = g_list_prepend (l, "ISO-8859-8");
l = g_list_prepend (l, "ISO-8859-7");
l = g_list_prepend (l, "ISO-8859-6");
l = g_list_prepend (l, "ISO-8859-5");
l = g_list_prepend (l, "ISO-8859-4");
l = g_list_prepend (l, "ISO-8859-3");
l = g_list_prepend (l, "ISO-8859-2");
l = g_list_prepend (l, "ISO-8859-1");
l = g_list_prepend (l, "ISO-2022-KR");
l = g_list_prepend (l, "ISO-2022-JP");
l = g_list_prepend (l, "ISO-2022-JP-3");
l = g_list_prepend (l, "ISO-2022-JP-2");
l = g_list_prepend (l, "ISO-2022-CN");
l = g_list_prepend (l, "ISO-2022-CN-EXT");
l = g_list_prepend (l, "ISIRI3342");
l = g_list_prepend (l, "ISIRI-3342");
l = g_list_prepend (l, "ISCII-DEV");
l = g_list_prepend (l, "ISCII-91");
l = g_list_prepend (l, "IS13194:1991");
l = g_list_prepend (l, "IS13194-DEVANAGARI");
l = g_list_prepend (l, "IS13194-DEV");
l = g_list_prepend (l, "INISCYRILLIC");
l = g_list_prepend (l, "INIS8");
l = g_list_prepend (l, "INIS");
l = g_list_prepend (l, "INIS-CYRILLIC");
l = g_list_prepend (l, "INIS-8");
l = g_list_prepend (l, "IEC_P271");
l = g_list_prepend (l, "IEC_P27-1");
l = g_list_prepend (l, "IBM1164");
l = g_list_prepend (l, "IBM1163");
l = g_list_prepend (l, "IBM1162");
l = g_list_prepend (l, "IBM1161");
l = g_list_prepend (l, "IBM1160");
l = g_list_prepend (l, "IBM1133");
l = g_list_prepend (l, "IBM1132");
l = g_list_prepend (l, "IBM1129");
l = g_list_prepend (l, "IBM1124");
l = g_list_prepend (l, "IBM1089");
l = g_list_prepend (l, "IBM1047");
l = g_list_prepend (l, "IBM1046");
l = g_list_prepend (l, "IBM1026");
l = g_list_prepend (l, "IBM1004");
l = g_list_prepend (l, "IBM943");
l = g_list_prepend (l, "IBM939");
l = g_list_prepend (l, "IBM937");
l = g_list_prepend (l, "IBM935");
l = g_list_prepend (l, "IBM933");
l = g_list_prepend (l, "IBM932");
l = g_list_prepend (l, "IBM930");
l = g_list_prepend (l, "IBM922");
l = g_list_prepend (l, "IBM920");
l = g_list_prepend (l, "IBM918");
l = g_list_prepend (l, "IBM916");
l = g_list_prepend (l, "IBM915");
l = g_list_prepend (l, "IBM912");
l = g_list_prepend (l, "IBM905");
l = g_list_prepend (l, "IBM904");
l = g_list_prepend (l, "IBM903");
l = g_list_prepend (l, "IBM891");
l = g_list_prepend (l, "IBM880");
l = g_list_prepend (l, "IBM875");
l = g_list_prepend (l, "IBM874");
l = g_list_prepend (l, "IBM871");
l = g_list_prepend (l, "IBM870");
l = g_list_prepend (l, "IBM869");
l = g_list_prepend (l, "IBM868");
l = g_list_prepend (l, "IBM866NAV");
l = g_list_prepend (l, "IBM866");
l = g_list_prepend (l, "IBM865");
l = g_list_prepend (l, "IBM864");
l = g_list_prepend (l, "IBM863");
l = g_list_prepend (l, "IBM862");
l = g_list_prepend (l, "IBM861");
l = g_list_prepend (l, "IBM860");
l = g_list_prepend (l, "IBM857");
l = g_list_prepend (l, "IBM856");
l = g_list_prepend (l, "IBM855");
l = g_list_prepend (l, "IBM852");
l = g_list_prepend (l, "IBM851");
l = g_list_prepend (l, "IBM850");
l = g_list_prepend (l, "IBM848");
l = g_list_prepend (l, "IBM819");
l = g_list_prepend (l, "IBM813");
l = g_list_prepend (l, "IBM775");
l = g_list_prepend (l, "IBM500");
l = g_list_prepend (l, "IBM437");
l = g_list_prepend (l, "IBM424");
l = g_list_prepend (l, "IBM423");
l = g_list_prepend (l, "IBM420");
l = g_list_prepend (l, "IBM367");
l = g_list_prepend (l, "IBM297");
l = g_list_prepend (l, "IBM290");
l = g_list_prepend (l, "IBM285");
l = g_list_prepend (l, "IBM284");
l = g_list_prepend (l, "IBM281");
l = g_list_prepend (l, "IBM280");
l = g_list_prepend (l, "IBM278");
l = g_list_prepend (l, "IBM277");
l = g_list_prepend (l, "IBM275");
l = g_list_prepend (l, "IBM274");
l = g_list_prepend (l, "IBM273");
l = g_list_prepend (l, "IBM256");
l = g_list_prepend (l, "IBM038");
l = g_list_prepend (l, "IBM037");
l = g_list_prepend (l, "IBM-1164");
l = g_list_prepend (l, "IBM-1163");
l = g_list_prepend (l, "IBM-1162");
l = g_list_prepend (l, "IBM-1161");
l = g_list_prepend (l, "IBM-1160");
l = g_list_prepend (l, "IBM-1133");
l = g_list_prepend (l, "IBM-1132");
l = g_list_prepend (l, "IBM-1129");
l = g_list_prepend (l, "IBM-1124");
l = g_list_prepend (l, "IBM-1047");
l = g_list_prepend (l, "IBM-1046");
l = g_list_prepend (l, "IBM-943");
l = g_list_prepend (l, "IBM-939");
l = g_list_prepend (l, "IBM-937");
l = g_list_prepend (l, "IBM-935");
l = g_list_prepend (l, "IBM-933");
l = g_list_prepend (l, "IBM-932");
l = g_list_prepend (l, "IBM-930");
l = g_list_prepend (l, "IBM-922");
l = g_list_prepend (l, "IBM-856");
l = g_list_prepend (l, "HU");
l = g_list_prepend (l, "HPROMAN8");
l = g_list_prepend (l, "HP-ROMAN8");
l = g_list_prepend (l, "HEBREW");
l = g_list_prepend (l, "GREEKCCITT");
l = g_list_prepend (l, "GREEK8");
l = g_list_prepend (l, "GREEK7OLD");
l = g_list_prepend (l, "GREEK7");
l = g_list_prepend (l, "GREEK7-OLD");
l = g_list_prepend (l, "GREEK");
l = g_list_prepend (l, "GREEK-CCITT");
l = g_list_prepend (l, "GOST_1976874");
l = g_list_prepend (l, "GOST_19768");
l = g_list_prepend (l, "GOST_19768-74");
l = g_list_prepend (l, "GEORGIAN-RS");
l = g_list_prepend (l, "GEORGIAN-PS");
l = g_list_prepend (l, "GEORGIAN-ACADEMY");
l = g_list_prepend (l, "GEO8-PS");
l = g_list_prepend (l, "GEO8-BPG");
l = g_list_prepend (l, "GB_198880");
l = g_list_prepend (l, "GB_1988-80");
l = g_list_prepend (l, "GBK");
l = g_list_prepend (l, "GB18030");
l = g_list_prepend (l, "GB13000");
l = g_list_prepend (l, "GB2312");
l = g_list_prepend (l, "GB");
l = g_list_prepend (l, "FR");
l = g_list_prepend (l, "FI");
l = g_list_prepend (l, "EUCTW");
l = g_list_prepend (l, "EUCKR");
l = g_list_prepend (l, "EUCJP");
l = g_list_prepend (l, "EUCJP-WIN");
l = g_list_prepend (l, "EUCJP-OPEN");
l = g_list_prepend (l, "EUCJP-MS");
l = g_list_prepend (l, "EUCCN");
l = g_list_prepend (l, "EUC-TW");
l = g_list_prepend (l, "EUC-KR");
l = g_list_prepend (l, "EUC-JP");
l = g_list_prepend (l, "EUC-JP-MS");
l = g_list_prepend (l, "EUC-JISX0213");
l = g_list_prepend (l, "EUC-CN");
l = g_list_prepend (l, "ES2");
l = g_list_prepend (l, "ES");
l = g_list_prepend (l, "ELOT_928");
l = g_list_prepend (l, "ECMACYRILLIC");
l = g_list_prepend (l, "ECMA-CYRILLIC");
l = g_list_prepend (l, "ECMA-128");
l = g_list_prepend (l, "ECMA-118");
l = g_list_prepend (l, "ECMA-114");
l = g_list_prepend (l, "EBCDICUS");
l = g_list_prepend (l, "EBCDICUK");
l = g_list_prepend (l, "EBCDICPT");
l = g_list_prepend (l, "EBCDICIT");
l = g_list_prepend (l, "EBCDICISFRISS");
l = g_list_prepend (l, "EBCDICFR");
l = g_list_prepend (l, "EBCDICFISEA");
l = g_list_prepend (l, "EBCDICFISE");
l = g_list_prepend (l, "EBCDICESS");
l = g_list_prepend (l, "EBCDICESA");
l = g_list_prepend (l, "EBCDICES");
l = g_list_prepend (l, "EBCDICDKNOA");
l = g_list_prepend (l, "EBCDICDKNO");
l = g_list_prepend (l, "EBCDICCAFR");
l = g_list_prepend (l, "EBCDICATDEA");
l = g_list_prepend (l, "EBCDICATDE");
l = g_list_prepend (l, "EBCDIC-US");
l = g_list_prepend (l, "EBCDIC-UK");
l = g_list_prepend (l, "EBCDIC-PT");
l = g_list_prepend (l, "EBCDIC-JP-KANA");
l = g_list_prepend (l, "EBCDIC-JP-E");
l = g_list_prepend (l, "EBCDIC-IT");
l = g_list_prepend (l, "EBCDIC-IS-FRISS");
l = g_list_prepend (l, "EBCDIC-INT1");
l = g_list_prepend (l, "EBCDIC-INT");
l = g_list_prepend (l, "EBCDIC-GREEK");
l = g_list_prepend (l, "EBCDIC-FR");
l = g_list_prepend (l, "EBCDIC-FI-SE");
l = g_list_prepend (l, "EBCDIC-FI-SE-A");
l = g_list_prepend (l, "EBCDIC-ES");
l = g_list_prepend (l, "EBCDIC-ES-S");
l = g_list_prepend (l, "EBCDIC-ES-A");
l = g_list_prepend (l, "EBCDIC-DK-NO");
l = g_list_prepend (l, "EBCDIC-DK-NO-A");
l = g_list_prepend (l, "EBCDIC-CYRILLIC");
l = g_list_prepend (l, "EBCDIC-CP-YU");
l = g_list_prepend (l, "EBCDIC-CP-WT");
l = g_list_prepend (l, "EBCDIC-CP-US");
l = g_list_prepend (l, "EBCDIC-CP-TR");
l = g_list_prepend (l, "EBCDIC-CP-SE");
l = g_list_prepend (l, "EBCDIC-CP-ROECE");
l = g_list_prepend (l, "EBCDIC-CP-NO");
l = g_list_prepend (l, "EBCDIC-CP-NL");
l = g_list_prepend (l, "EBCDIC-CP-IT");
l = g_list_prepend (l, "EBCDIC-CP-IS");
l = g_list_prepend (l, "EBCDIC-CP-HE");
l = g_list_prepend (l, "EBCDIC-CP-GR");
l = g_list_prepend (l, "EBCDIC-CP-GB");
l = g_list_prepend (l, "EBCDIC-CP-FR");
l = g_list_prepend (l, "EBCDIC-CP-FI");
l = g_list_prepend (l, "EBCDIC-CP-ES");
l = g_list_prepend (l, "EBCDIC-CP-DK");
l = g_list_prepend (l, "EBCDIC-CP-CH");
l = g_list_prepend (l, "EBCDIC-CP-CA");
l = g_list_prepend (l, "EBCDIC-CP-BE");
l = g_list_prepend (l, "EBCDIC-CP-AR2");
l = g_list_prepend (l, "EBCDIC-CP-AR1");
l = g_list_prepend (l, "EBCDIC-CA-FR");
l = g_list_prepend (l, "EBCDIC-BR");
l = g_list_prepend (l, "EBCDIC-BE");
l = g_list_prepend (l, "EBCDIC-AT-DE");
l = g_list_prepend (l, "EBCDIC-AT-DE-A");
l = g_list_prepend (l, "E13B/");
l = g_list_prepend (l, "DS_2089");
l = g_list_prepend (l, "DS2089");
l = g_list_prepend (l, "DK");
l = g_list_prepend (l, "DIN_66003");
l = g_list_prepend (l, "DECMCS");
l = g_list_prepend (l, "DEC");
l = g_list_prepend (l, "DEC-MCS");
l = g_list_prepend (l, "DE");
l = g_list_prepend (l, "CYRILLIC");
l = g_list_prepend (l, "CYRILLIC-ASIAN");
l = g_list_prepend (l, "CWI");
l = g_list_prepend (l, "CWI-2");
l = g_list_prepend (l, "CUBA");
l = g_list_prepend (l, "CSWINDOWS31J");
l = g_list_prepend (l, "CSUNICODE");
l = g_list_prepend (l, "CSUCS4");
l = g_list_prepend (l, "CSSHIFTJIS");
l = g_list_prepend (l, "CSPCP852");
l = g_list_prepend (l, "CSPC862LATINHEBREW");
l = g_list_prepend (l, "CSPC850MULTILINGUAL");
l = g_list_prepend (l, "CSPC775BALTIC");
l = g_list_prepend (l, "CSPC8CODEPAGE437");
l = g_list_prepend (l, "CSN_369103");
l = g_list_prepend (l, "CSNATSSEFI");
l = g_list_prepend (l, "CSNATSDANO");
l = g_list_prepend (l, "CSMACINTOSH");
l = g_list_prepend (l, "CSKSC5636");
l = g_list_prepend (l, "CSKOI8R");
l = g_list_prepend (l, "CSISOLATINHEBREW");
l = g_list_prepend (l, "CSISOLATINGREEK");
l = g_list_prepend (l, "CSISOLATINCYRILLIC");
l = g_list_prepend (l, "CSISOLATINARABIC");
l = g_list_prepend (l, "CSISOLATIN6");
l = g_list_prepend (l, "CSISOLATIN5");
l = g_list_prepend (l, "CSISOLATIN4");
l = g_list_prepend (l, "CSISOLATIN3");
l = g_list_prepend (l, "CSISOLATIN2");
l = g_list_prepend (l, "CSISOLATIN1");
l = g_list_prepend (l, "CSISO10367BOX");
l = g_list_prepend (l, "CSISO5428GREEK");
l = g_list_prepend (l, "CSISO5427CYRILLIC1981");
l = g_list_prepend (l, "CSISO5427CYRILLIC");
l = g_list_prepend (l, "CSISO2033");
l = g_list_prepend (l, "CSISO2022KR");
l = g_list_prepend (l, "CSISO2022JP2");
l = g_list_prepend (l, "CSISO2022JP");
l = g_list_prepend (l, "CSISO2022CN");
l = g_list_prepend (l, "CSISO646DANISH");
l = g_list_prepend (l, "CSISO153GOST1976874");
l = g_list_prepend (l, "CSISO151CUBA");
l = g_list_prepend (l, "CSISO150GREEKCCITT");
l = g_list_prepend (l, "CSISO150");
l = g_list_prepend (l, "CSISO143IECP271");
l = g_list_prepend (l, "CSISO141JUSIB1002");
l = g_list_prepend (l, "CSISO139CSN369103");
l = g_list_prepend (l, "CSISO122CANADIAN2");
l = g_list_prepend (l, "CSISO121CANADIAN1");
l = g_list_prepend (l, "CSISO111ECMACYRILLIC");
l = g_list_prepend (l, "CSISO103T618BIT");
l = g_list_prepend (l, "CSISO99NAPLPS");
l = g_list_prepend (l, "CSISO92JISC62991984B");
l = g_list_prepend (l, "CSISO90");
l = g_list_prepend (l, "CSISO89ASMO449");
l = g_list_prepend (l, "CSISO88GREEK7");
l = g_list_prepend (l, "CSISO86HUNGARIAN");
l = g_list_prepend (l, "CSISO85SPANISH2");
l = g_list_prepend (l, "CSISO84PORTUGUESE2");
l = g_list_prepend (l, "CSISO69FRENCH");
l = g_list_prepend (l, "CSISO61NORWEGIAN2");
l = g_list_prepend (l, "CSISO60NORWEGIAN1");
l = g_list_prepend (l, "CSISO60DANISHNORWEGIAN");
l = g_list_prepend (l, "CSISO58GB1988");
l = g_list_prepend (l, "CSISO51INISCYRILLIC");
l = g_list_prepend (l, "CSISO50INIS8");
l = g_list_prepend (l, "CSISO49INIS");
l = g_list_prepend (l, "CSISO27LATINGREEK1");
l = g_list_prepend (l, "CSISO25FRENCH");
l = g_list_prepend (l, "CSISO21GERMAN");
l = g_list_prepend (l, "CSISO19LATINGREEK");
l = g_list_prepend (l, "CSISO18GREEK7OLD");
l = g_list_prepend (l, "CSISO17SPANISH");
l = g_list_prepend (l, "CSISO16PORTUGESE");
l = g_list_prepend (l, "CSISO15ITALIAN");
l = g_list_prepend (l, "CSISO14JISC6220RO");
l = g_list_prepend (l, "CSISO11SWEDISHFORNAMES");
l = g_list_prepend (l, "CSISO10SWEDISH");
l = g_list_prepend (l, "CSISO4UNITEDKINGDOM");
l = g_list_prepend (l, "CSIBM11621162");
l = g_list_prepend (l, "CSIBM1164");
l = g_list_prepend (l, "CSIBM1163");
l = g_list_prepend (l, "CSIBM1161");
l = g_list_prepend (l, "CSIBM1160");
l = g_list_prepend (l, "CSIBM1133");
l = g_list_prepend (l, "CSIBM1132");
l = g_list_prepend (l, "CSIBM1129");
l = g_list_prepend (l, "CSIBM1124");
l = g_list_prepend (l, "CSIBM1026");
l = g_list_prepend (l, "CSIBM943");
l = g_list_prepend (l, "CSIBM939");
l = g_list_prepend (l, "CSIBM937");
l = g_list_prepend (l, "CSIBM935");
l = g_list_prepend (l, "CSIBM933");
l = g_list_prepend (l, "CSIBM932");
l = g_list_prepend (l, "CSIBM930");
l = g_list_prepend (l, "CSIBM922");
l = g_list_prepend (l, "CSIBM918");
l = g_list_prepend (l, "CSIBM905");
l = g_list_prepend (l, "CSIBM904");
l = g_list_prepend (l, "CSIBM903");
l = g_list_prepend (l, "CSIBM891");
l = g_list_prepend (l, "CSIBM880");
l = g_list_prepend (l, "CSIBM871");
l = g_list_prepend (l, "CSIBM870");
l = g_list_prepend (l, "CSIBM869");
l = g_list_prepend (l, "CSIBM868");
l = g_list_prepend (l, "CSIBM866");
l = g_list_prepend (l, "CSIBM865");
l = g_list_prepend (l, "CSIBM864");
l = g_list_prepend (l, "CSIBM863");
l = g_list_prepend (l, "CSIBM860");
l = g_list_prepend (l, "CSIBM857");
l = g_list_prepend (l, "CSIBM856");
l = g_list_prepend (l, "CSIBM855");
l = g_list_prepend (l, "CSIBM851");
l = g_list_prepend (l, "CSIBM500");
l = g_list_prepend (l, "CSIBM424");
l = g_list_prepend (l, "CSIBM423");
l = g_list_prepend (l, "CSIBM420");
l = g_list_prepend (l, "CSIBM297");
l = g_list_prepend (l, "CSIBM290");
l = g_list_prepend (l, "CSIBM285");
l = g_list_prepend (l, "CSIBM284");
l = g_list_prepend (l, "CSIBM281");
l = g_list_prepend (l, "CSIBM280");
l = g_list_prepend (l, "CSIBM278");
l = g_list_prepend (l, "CSIBM277");
l = g_list_prepend (l, "CSIBM275");
l = g_list_prepend (l, "CSIBM274");
l = g_list_prepend (l, "CSIBM273");
l = g_list_prepend (l, "CSIBM038");
l = g_list_prepend (l, "CSIBM037");
l = g_list_prepend (l, "CSHPROMAN8");
l = g_list_prepend (l, "CSGB2312");
l = g_list_prepend (l, "CSEUCPKDFMTJAPANESE");
l = g_list_prepend (l, "CSEUCKR");
l = g_list_prepend (l, "CSEBCDICUS");
l = g_list_prepend (l, "CSEBCDICUK");
l = g_list_prepend (l, "CSEBCDICPT");
l = g_list_prepend (l, "CSEBCDICIT");
l = g_list_prepend (l, "CSEBCDICFR");
l = g_list_prepend (l, "CSEBCDICFISEA");
l = g_list_prepend (l, "CSEBCDICFISE");
l = g_list_prepend (l, "CSEBCDICESS");
l = g_list_prepend (l, "CSEBCDICESA");
l = g_list_prepend (l, "CSEBCDICES");
l = g_list_prepend (l, "CSEBCDICDKNOA");
l = g_list_prepend (l, "CSEBCDICDKNO");
l = g_list_prepend (l, "CSEBCDICCAFR");
l = g_list_prepend (l, "CSEBCDICATDEA");
l = g_list_prepend (l, "CSEBCDICATDE");
l = g_list_prepend (l, "CSDECMCS");
l = g_list_prepend (l, "CSA_Z243.419852");
l = g_list_prepend (l, "CSA_Z243.419851");
l = g_list_prepend (l, "CSA_Z243.4-1985-2");
l = g_list_prepend (l, "CSA_Z243.4-1985-1");
l = g_list_prepend (l, "CSA_T500");
l = g_list_prepend (l, "CSA_T500-1983");
l = g_list_prepend (l, "CSASCII");
l = g_list_prepend (l, "CSA7-2");
l = g_list_prepend (l, "CSA7-1");
l = g_list_prepend (l, "CPIBM861");
l = g_list_prepend (l, "CP10007");
l = g_list_prepend (l, "CP1361");
l = g_list_prepend (l, "CP1258");
l = g_list_prepend (l, "CP1257");
l = g_list_prepend (l, "CP1256");
l = g_list_prepend (l, "CP1255");
l = g_list_prepend (l, "CP1254");
l = g_list_prepend (l, "CP1253");
l = g_list_prepend (l, "CP1252");
l = g_list_prepend (l, "CP1251_TT");
l = g_list_prepend (l, "CP1251");
l = g_list_prepend (l, "CP1250");
l = g_list_prepend (l, "CP1164");
l = g_list_prepend (l, "CP1163");
l = g_list_prepend (l, "CP1162");
l = g_list_prepend (l, "CP1161");
l = g_list_prepend (l, "CP1160");
l = g_list_prepend (l, "CP1133");
l = g_list_prepend (l, "CP1132");
l = g_list_prepend (l, "CP1129");
l = g_list_prepend (l, "CP1125");
l = g_list_prepend (l, "CP1124");
l = g_list_prepend (l, "CP1089");
l = g_list_prepend (l, "CP1084");
l = g_list_prepend (l, "CP1081");
l = g_list_prepend (l, "CP1079");
l = g_list_prepend (l, "CP1070");
l = g_list_prepend (l, "CP1047");
l = g_list_prepend (l, "CP1046");
l = g_list_prepend (l, "CP1026");
l = g_list_prepend (l, "CP1004");
l = g_list_prepend (l, "CP950");
l = g_list_prepend (l, "CP949");
l = g_list_prepend (l, "CP939");
l = g_list_prepend (l, "CP937");
l = g_list_prepend (l, "CP936");
l = g_list_prepend (l, "CP935");
l = g_list_prepend (l, "CP933");
l = g_list_prepend (l, "CP932");
l = g_list_prepend (l, "CP930");
l = g_list_prepend (l, "CP922");
l = g_list_prepend (l, "CP920");
l = g_list_prepend (l, "CP918");
l = g_list_prepend (l, "CP916");
l = g_list_prepend (l, "CP915");
l = g_list_prepend (l, "CP912");
l = g_list_prepend (l, "CP905");
l = g_list_prepend (l, "CP904");
l = g_list_prepend (l, "CP903");
l = g_list_prepend (l, "CP891");
l = g_list_prepend (l, "CP880");
l = g_list_prepend (l, "CP875");
l = g_list_prepend (l, "CP874");
l = g_list_prepend (l, "CP871");
l = g_list_prepend (l, "CP870");
l = g_list_prepend (l, "CP869");
l = g_list_prepend (l, "CP868");
l = g_list_prepend (l, "CP866NAV");
l = g_list_prepend (l, "CP866");
l = g_list_prepend (l, "CP865");
l = g_list_prepend (l, "CP864");
l = g_list_prepend (l, "CP863");
l = g_list_prepend (l, "CP862");
l = g_list_prepend (l, "CP861");
l = g_list_prepend (l, "CP860");
l = g_list_prepend (l, "CP857");
l = g_list_prepend (l, "CP856");
l = g_list_prepend (l, "CP855");
l = g_list_prepend (l, "CP852");
l = g_list_prepend (l, "CP851");
l = g_list_prepend (l, "CP850");
l = g_list_prepend (l, "CP819");
l = g_list_prepend (l, "CP813");
l = g_list_prepend (l, "CP775");
l = g_list_prepend (l, "CP737");
l = g_list_prepend (l, "CP500");
l = g_list_prepend (l, "CP437");
l = g_list_prepend (l, "CP424");
l = g_list_prepend (l, "CP423");
l = g_list_prepend (l, "CP420");
l = g_list_prepend (l, "CP367");
l = g_list_prepend (l, "CP297");
l = g_list_prepend (l, "CP290");
l = g_list_prepend (l, "CP285");
l = g_list_prepend (l, "CP284");
l = g_list_prepend (l, "CP282");
l = g_list_prepend (l, "CP281");
l = g_list_prepend (l, "CP280");
l = g_list_prepend (l, "CP278");
l = g_list_prepend (l, "CP275");
l = g_list_prepend (l, "CP274");
l = g_list_prepend (l, "CP273");
l = g_list_prepend (l, "CP154");
l = g_list_prepend (l, "CP038");
l = g_list_prepend (l, "CP037");
l = g_list_prepend (l, "CP-HU");
l = g_list_prepend (l, "CP-GR");
l = g_list_prepend (l, "CP-AR");
l = g_list_prepend (l, "CN");
l = g_list_prepend (l, "CN-GB");
l = g_list_prepend (l, "CN-BIG5");
l = g_list_prepend (l, "CA");
l = g_list_prepend (l, "BS_4730");
l = g_list_prepend (l, "BIGFIVE");
l = g_list_prepend (l, "BIG5HKSCS");
l = g_list_prepend (l, "BIG5");
l = g_list_prepend (l, "BIG5-HKSCS");
l = g_list_prepend (l, "BIG-FIVE");
l = g_list_prepend (l, "BIG-5");
l = g_list_prepend (l, "BALTIC");
l = g_list_prepend (l, "ASMO_449");
l = g_list_prepend (l, "ASMO-708");
l = g_list_prepend (l, "ASCII");
l = g_list_prepend (l, "ARMSCII-8");
l = g_list_prepend (l, "ARABIC7");
l = g_list_prepend (l, "ARABIC");
l = g_list_prepend (l, "ANSI_X3.110");
l = g_list_prepend (l, "ANSI_X3.110-1983");
l = g_list_prepend (l, "ANSI_X3.4");
l = g_list_prepend (l, "ANSI_X3.4-1986");
l = g_list_prepend (l, "ANSI_X3.4-1968");
l = g_list_prepend (l, "10646-1:1993/UCS4/");
l = g_list_prepend (l, "10646-1:1993");
l = g_list_prepend (l, "8859_9");
l = g_list_prepend (l, "8859_8");
l = g_list_prepend (l, "8859_7");
l = g_list_prepend (l, "8859_6");
l = g_list_prepend (l, "8859_5");
l = g_list_prepend (l, "8859_4");
l = g_list_prepend (l, "8859_3");
l = g_list_prepend (l, "8859_2");
l = g_list_prepend (l, "8859_1");
l = g_list_prepend (l, "1047");
l = g_list_prepend (l, "1046");
l = g_list_prepend (l, "1026");
l = g_list_prepend (l, "904");
l = g_list_prepend (l, "874");
l = g_list_prepend (l, "869");
l = g_list_prepend (l, "866NAV");
l = g_list_prepend (l, "866");
l = g_list_prepend (l, "865");
l = g_list_prepend (l, "864");
l = g_list_prepend (l, "863");
l = g_list_prepend (l, "862");
l = g_list_prepend (l, "861");
l = g_list_prepend (l, "860");
l = g_list_prepend (l, "857");
l = g_list_prepend (l, "856");
l = g_list_prepend (l, "855");
l = g_list_prepend (l, "852");
l = g_list_prepend (l, "851");
l = g_list_prepend (l, "850");
l = g_list_prepend (l, "500V1");
l = g_list_prepend (l, "500");
l = g_list_prepend (l, "437");
  return l;
}
