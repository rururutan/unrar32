/*
 *   Copyright (c) 1998-2004 T. Kamei (kamei@jsdlab.co.jp)
 *
 *   Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose is hereby granted provided
 * that the above copyright notice and this permission notice appear
 * in all copies of the software and related documentation.
 *
 *                          NO WARRANTY
 *
 *   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY WARRANTIES;
 * WITHOUT EVEN THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS
 * FOR A PARTICULAR PURPOSE.
 */

#include <windows.h>
#include <stdio.h>
#include "comm-arc.h"
#include "util.h"
#include "mapf.h"

void
init_table ()
{
  int i;
  for (i = 0; i < 256; i++)
    {
      translate_table[i] = u_char (i);
      mblead_table[i] = IsDBCSLeadByte (BYTE (i));
    }
  for (i = 'a'; i <= 'z'; i++)
    translate_table[i] = u_char (i - 'a' + 'A');
}

#define SEPCHAR_P(C) ((C) == '/' || (C) == '\\')

char *
find_last_slash (const char *p)
{
  u_char *x = 0;
  for (u_char *s = (u_char *)p; *s;)
    {
#ifdef KANJI
      if (iskanji (*s) && s[1])
        s += 2;
      else
#endif
        {
          if (SEPCHAR_P (*s))
            x = s;
          s++;
        }
    }
  return (char *)x;
}

char *
find_slash (const char *p)
{
  for (u_char *s = (u_char *)p; *s;)
    {
#ifdef KANJI
      if (iskanji (*s) && s[1])
        s += 2;
      else
#endif
        {
          if (SEPCHAR_P (*s))
            return (char *)s;
          s++;
        }
    }
  return 0;
}

void
slash2backsl (char *p)
{
  for (; p = find_slash (p); *p++ = '\\')
    ;
}

char *
trim_root (const char *path)
{
  const char *last;
  do
    {
      last = path;
      for (; isalpha (u_char (*path)) && path[1] == ':'; path += 2)
        ;
      for (; SEPCHAR_P (*path); path++)
        ;
    }
  while (path != last);
  return (char *)path;
}

void
sanitize_path (char *path)
{
  char *p = path;
  char *b = path;
  for (;;)
    {
      p = trim_root (p);
      if (!*p)
        break;
      if (*p == '.')
        {
          if (!p[1] || SEPCHAR_P (p[1]))
            {
              p++;
              continue;
            }
          if (p[1] == '.')
            {
              if (!p[2] || SEPCHAR_P (p[2]))
                {
                  if (b > path)
                    b--;
                  for (; b > path && b[-1]; b--)
                    ;
                  p += 2;
                  continue;
                }
              else
                {
                  char *q = p + 2;
                  for (; *q == '.'; q++)
                    ;
                  if (!*q || SEPCHAR_P (*q))
                    {
                      p = q;
                      continue;
                    }
                }
            }
        }
      while (*p && !SEPCHAR_P (*p))
        *b++ = *p++;
      if (!*p++)
        break;
      *b++ = 0;
    }
  *b = 0;
  for (p = path; p < b; p++)
    if (!*p)
      *p = '\\';
}

size_t
strlcpy (char *d, const char *s, size_t n)
{
  if (!n)
    return strlen (s);

  const char *src = s;
  n--;
  size_t i;
  for (i = 0; i < n && (d[i] = s[i]); i++)
    ;
  if (i == n)
    {
      d[i] = 0;
      for (; s[i]; i++)
        ;
    }
  return i;
}

char *
stpcpy (char *d, const char *s)
{
  int i;
  for (i = 0; (d[i] = s[i]); i++)
    ;
  return d + i;
}

void
cmdline::discard ()
{
  for (int i = 0; i < m_ac; i++)
    if (m_av[i])
      free (m_av[i]);
  if (m_av)
    free (m_av);
  m_ac = 0;
  m_av = 0;
  m_max = 0;
}

char *
cmdline::copyarg (const u_char *s, const u_char *se, int dq)
{
  int l = se - s - dq;
  if (!l)
    return "";
  char *d0 = (char *)malloc (l + 1);
  if (!d0)
    return 0;
  char *d = d0;
  for (; s < se; s++)
    if (*s != '"')
      *d++ = *s;
  *d = 0;
  return d0;
}

int
cmdline::parse (const char *cmdline, int l, bool resp_ok)
{
  if (!cmdline)
    return 0;
  const u_char *cp = (const u_char *)cmdline;
  const u_char *ce = cp + l;
  const u_char *c0 = 0;
  int dq = 0;
  for (;; cp++)
    {
      int c = cp == ce ? EOF : *cp;
      switch (c)
        {
        case ' ':
        case '\t':
          if (dq & 1)
            goto normal_char;
          /* fall thru... */
        case EOF:
        case '\r':
        case '\n':
          if (c0)
            {
              char *arg = copyarg (c0, cp, dq);
              if (!arg)
                return ERROR_ENOUGH_MEMORY;
              if (resp_ok && *arg == '@')
                {
                  mapf mf;
                  int e = mf.open (arg + 1);
                  free (arg);
                  if (!e)
                    return ERROR_RESPONSE_READ;
                  e = parse ((const char *)mf.base (), mf.size (), 0);
                  if (e)
                    return e;
                }
              else if (*arg)
                {
                  if (m_ac == m_max)
                    {
                      m_max += 32;
                      char **x = (char **)realloc (m_av, sizeof *x * m_max);
                      if (!x)
                        {
                          free (arg);
                          return ERROR_ENOUGH_MEMORY;
                        }
                      m_av = x;
                    }
                  m_av[m_ac++] = arg;
                }
              dq = 0;
              c0 = 0;
            }
          if (c == EOF)
            return 0;
          break;

        case '"':
          dq++;
          /* fall thru... */
        default:
        normal_char:
          if (!c0)
            c0 = cp;
          break;
        }
    }
}

bool
glob::match (const char *pat, const char *str, bool recursive)
{
  const u_char *p = reinterpret_cast <const u_char *> (pat);
  const u_char *s = reinterpret_cast <const u_char *> (str);

  for (;;)
    {
      int c = *p++;
      switch (c)
        {
        case 0:
          return !*s || (recursive && SEPCHAR_P (*s));

        case '?':
          if (!*s || SEPCHAR_P (*s))
            return false;
#ifdef KANJI
          if (iskanji (*s) && s[1])
            s++;
#endif
          s++;
          break;

        case '*':
          for (; *p == '*'; p++)
            ;
          if (!*p)
            return recursive || !find_slash (reinterpret_cast <const char *> (s));
          for (;;)
            {
              if (match (reinterpret_cast <const char *> (p),
                         reinterpret_cast <const char *> (s),
                         recursive))
                return true;
              if (!*s || SEPCHAR_P (*s))
                return false;
#ifdef KANJI
              if (iskanji (*s) && s[1])
                s++;
#endif
              s++;
            }

        case '/':
        case '\\':
          if (!SEPCHAR_P (*s))
            return false;
          s++;
          break;

        default:
#ifdef KANJI
          if (iskanji (c) && *p)
            {
              if (u_char (c) != *s++)
                return false;
              if (*p++ != *s++)
                return false;
            }
          else
#endif
            {
              if (translate (c) != translate (*s))
                return false;
              s++;
            }
        }
    }
}

bool
glob::match (const char *file, bool strict, bool recursive) const
{
  if (!m_npat)
    return true;
  if (strict)
    {
      for (int i = 0; i < m_npat; i++)
        if (match (m_pat[i], file, recursive))
          return true;
    }
  else
    {
      const char *name = find_last_slash (file);
      name = name ? name + 1 : file;
      for (int i = 0; i < m_npat; i++)
        if (match (m_pat[i], find_slash (m_pat[i]) ? file : name, false))
          return true;
    }
  return false;
}

void
glob::set_pattern (int ac, char **av)
{
  m_npat = ac;
  m_pat = av;
  for (int i = 0; i < m_npat; i++)
    {
      char *p = find_last_slash (m_pat[i]);
      if (p && !p[1])
        *p = 0;
    }
}

int
check_kanji_trail (const char *string, u_int off)
{
  const u_char *const s0 = reinterpret_cast <const u_char *> (string);
  const u_char *const se = s0 + off;
  const u_char *s = se;
  for (; s-- > s0 && iskanji (*s);)
    ;
  return !((se - s) & 1);
}

int
ostrbuf::formatv (const char *fmt, va_list ap)
{
  if (space () <= 0)
    return 0;
  int l = _vsnprintf (m_buf, space (), fmt, ap);
  if (l > 0)
    {
      m_buf += l;
      m_size -= l;
      *m_buf = 0;
    }
  else
    {
      l = space ();
      if (check_kanji_trail (m_buf, l))
        l--;
      m_buf[l] = 0;
      m_size = 0;
    }
  return space () > 0;
}

int
ostrbuf::format (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int x = formatv (fmt, ap);
  va_end (ap);
  return x;
};
