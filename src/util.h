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

#ifndef _util_h_
# define _util_h_

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN u_char translate_table[256];
#define translate(c) (translate_table[(c) & 0xff])

EXTERN u_char mblead_table[256];
#define iskanji(c) (mblead_table[(c) & 0xff])

struct lib_state
{
  HINSTANCE hinst;
  HINSTANCE hrardll;
  int s_bg_mode;
  int s_cursor_mode;
  WORD s_cursor_interval;
  bool has_callback;
  HWND hwnd_owner;
  LPARCHIVERPROC callback;
};

EXTERN lib_state lstate;

class cmdline
{
public:
  cmdline ()
       : m_ac (0), m_av (0), m_max (0)
    {}
  ~cmdline ()
    {discard ();}
  int parse (const char *cmdline, size_t l, bool resp_ok);
  int parse (const char *cmdline, bool resp_ok)
    {return parse (cmdline, cmdline ? strlen (cmdline) : 0, resp_ok);}
  int argc () const {return m_ac;}
  char **argv () const {return m_av;}

private:
  int m_ac;
  char **m_av;
  int m_max;

  static char *copyarg (const u_char *s, const u_char *se, int dq);
  void discard ();
};

class glob
{
public:
  glob () : m_npat (0) {}
  bool match (const char *filename, bool strict, bool recursive) const;
  void set_pattern (int ac, char **av);

private:
  int m_npat;
  char **m_pat;

  static bool match (const char *pat, const char *str, bool recursive);
};

class ostrbuf
{
public:
  ostrbuf (char *b, int size)
       : m_buf (b), m_size (b ? size : 0) {}
  int format (const char *fmt, ...);
  int formatv (const char *fmt, va_list);
  int space () const
    {return m_size - 1;}
private:
  char *m_buf;
  int m_size;
};

union int64
{
  __int64 d;
  struct
    {
      unsigned long l;
      long h;
    } s;
};

void init_table ();
char *find_last_slash (const char *p);
char *find_slash (const char *p);
void slash2backsl (char *p);
size_t strlcpy (char *d, const char *s, size_t n);
char *stpcpy (char *d, const char *s);
char *trim_root (const char *path);
void sanitize_path (char *path);

#endif
