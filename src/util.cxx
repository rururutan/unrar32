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
      translate_table[i] = u_short (i);
      mblead_table[i] = IsDBCSLeadByte (BYTE (i));
    }
  for (i = L'a'; i <= L'z'; i++)
    translate_table[i] = u_short (i - L'a' + L'A');
}

#define SEPCHAR_P(C) ((C) == '/' || (C) == '\\')
#define SEPCHAR_PW(C) ((C) == L'/' || (C) == L'\\')

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

wchar_t *
find_last_slash (const wchar_t *p)
{
  WORD *x = 0;
  for (WORD *s = (WORD *)p; *s;)
    {
      if (IS_HIGH_SURROGATE (*s) && s[1])
        s += 2;
      else {
        if (SEPCHAR_PW (*s))
          x = s;
        s++;
      }
    }
  return (wchar_t *)x;
}

wchar_t *
find_slash (const wchar_t *p)
{
  for (WORD *s = (WORD *)p; *s;)
    {
      if (IS_HIGH_SURROGATE (*s) && s[1])
        s += 2;
      else {
        if (SEPCHAR_PW (*s))
          return (wchar_t *)s;
        s++;
      }
    }
  return 0;
}

void
slash2backsl (wchar_t *p)
{
  for (; p = find_slash (p); *p++ = L'\\')
    ;
}

wchar_t *
trim_root (const wchar_t *path)
{
  const wchar_t *last;
  do
    {
      last = path;
      for (; iswalpha (u_short (*path)) && path[1] == L':'; path += 2)
        ;
      for (; SEPCHAR_PW (*path); path++)
        ;
    }
  while (path != last);
  return (wchar_t *)path;
}

void
sanitize_path (wchar_t *path)
{
  wchar_t *p = path;
  wchar_t *b = path;
  for (;;)
    {
      p = trim_root (p);
      if (!*p)
        break;
      if (*p == L'.')
        {
          if (!p[1] || SEPCHAR_PW (p[1]))
            {
              p++;
              continue;
            }
          if (p[1] == L'.')
            {
              if (!p[2] || SEPCHAR_PW (p[2]))
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
                  wchar_t *q = p + 2;
                  for (; *q == L'.'; q++)
                    ;
                  if (!*q || SEPCHAR_PW (*q))
                    {
                      p = q;
                      continue;
                    }
                }
            }
        }
      while (*p && !SEPCHAR_PW (*p))
        *b++ = *p++;
      if (!*p++)
        break;
      *b++ = 0;
    }
  *b = 0;
  for (p = path; p < b; p++)
    if (!*p)
      *p = L'\\';
}

size_t
strlcpy (char *d, const char *s, size_t n)
{
  if (!n)
    return strlen (s);

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

wchar_t *
stpcpy (wchar_t *d, const wchar_t *s)
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
  size_t l = se - s - dq;
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
cmdline::parse (const char *cmdline, size_t l, bool resp_ok)
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
glob::match (const wchar_t *pat, const wchar_t *str, bool recursive)
{
  const u_short *p = reinterpret_cast <const u_short *> (pat);
  const u_short *s = reinterpret_cast <const u_short *> (str);

  for (;;)
    {
      int c = *p++;
      switch (c)
        {
        case 0:
          return !*s || (recursive && SEPCHAR_PW (*s));

        case L'?':
          if (!*s || SEPCHAR_PW (*s))
            return false;
          if (IS_HIGH_SURROGATE (*s) && s[1])
            s ++;
          s++;
          break;

        case L'*':
          for (; *p == L'*'; p++)
            ;
          if (!*p)
            return recursive || !find_slash (reinterpret_cast <const wchar_t *> (s));
          for (;;)
            {
              if (match (reinterpret_cast <const wchar_t *> (p),
                         reinterpret_cast <const wchar_t *> (s),
                         recursive))
                return true;
              if (!*s || SEPCHAR_PW (*s))
                return false;
              if (IS_HIGH_SURROGATE (*s) && s[1])
                s ++;
              s++;
            }

        case L'/':
        case L'\\':
          if (!SEPCHAR_PW (*s))
            return false;
          s++;
          break;

        default:
          if (iswalpha (u_short (c)))
            {
              if (translate (c) != translate (*s))
                return false;
              s++;
            }
          else
            {
              if (u_short (c) != *s++)
                return false;
              if (*p++ != *s++)
                return false;
              if (IS_HIGH_SURROGATE (*s)) {
                if (*p++ != *s++)
                  return false;
              }
            }
        }
    }
}

bool
glob::match (const wchar_t *file, bool strict, bool recursive) const
{
  if (!m_npat)
    return true;
  if (strict)
    {
      for (int i = 0; i < m_npat; i++)
        if (match (mb2wide(m_pat[i]).getstring(), file, recursive))
          return true;
    }
  else
    {
      const wchar_t *name = find_last_slash (file);
      name = name ? name + 1 : file;
      for (int i = 0; i < m_npat; i++)
        if (match (mb2wide(m_pat[i]).getstring(),
                           find_slash (mb2wide(m_pat[i]).getstring()) ? file : name, false))
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

#if 0
int
check_kanji_trail (const wchar_t *string, u_int off)
{
  const u_short *const s0 = reinterpret_cast <const u_short *> (string);
  const u_short *const se = s0 + off;
  const u_short *s = se;
  for (; s-- > s0 && IS_SURROGATE_PAIR(*s, *s[1]);)
    ;
  return !((se - s) & 1);
}
#endif

int
ostrbuf::formatv (const wchar_t *fmt, va_list ap)
{
  if (space () <= 0)
    return 0;
  int l = _vsnwprintf (m_bufp, space (), fmt, ap);
  wide2mb bufa(m_bufp);
  strncpy( m_buf, bufa.getstring(), space() );
  if (l > 0)
    {
      m_buf += (bufa.getsize() - 1);
      m_bufp += l;
      m_size -= (bufa.getsize() - 1);
      *m_buf = 0;
      *m_bufp = 0;
    }
  else
    {
      l = space ();
#if 0
      if (check_kanji_trail (m_buf, l))
        l--;
#endif
      m_buf[l] = 0;
      m_bufp[l] = 0;
      m_size = 0;
    }
  return space () > 0;
}

int
ostrbuf::format (const wchar_t *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int x = formatv (fmt, ap);
  va_end (ap);
  return x;
};
