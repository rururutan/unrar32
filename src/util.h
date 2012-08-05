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

EXTERN u_short translate_table[256];
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
#ifdef _UTF8_SUPPORT
  bool utf8_mode;
#endif
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
  glob () : m_npat (0), m_pat(0) {}
  bool match (const wchar_t *filename, bool strict, bool recursive) const;
  void set_pattern (int ac, char **av);

private:
  int m_npat;
  char **m_pat;

  static bool match (const wchar_t *pat, const wchar_t *str, bool recursive);
};

class ostrbuf
{
public:
  ostrbuf (char *b, int size)
       : m_buf (b), m_bufw(0), m_size (b ? size : 0)  {
    m_bufw = new wchar_t[m_size*2+1];
    m_bufp = m_bufw;
    ZeroMemory( m_bufw, sizeof(wchar_t) * (m_size+1) );
  }
  ~ostrbuf() { delete [] m_bufw; }
  int format (const wchar_t *fmt, ...);
  int formatv (const wchar_t *fmt, va_list);
  int space () const
    {return m_size - 1;}
private:
  char *m_buf;		// input data
  wchar_t *m_bufw;	// for delete[]
  wchar_t *m_bufp;	// current pointer
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

class mb2wide
{
public:
	mb2wide(const char *data) : utf16string(0), utf16len(0) {
#ifdef _UTF8_SUPPORT
		UINT codepage = (lstate.utf8_mode) ? CP_UTF8 : CP_THREAD_ACP;
#else
		UINT codepage = CP_THREAD_ACP;
#endif
		utf16len = ::MultiByteToWideChar(codepage, 0, data, -1, NULL, 0);
		utf16string = new WCHAR[utf16len+1];
		::ZeroMemory(utf16string, sizeof(WCHAR) * (utf16len+1));
		::MultiByteToWideChar(codepage, 0, data, -1, utf16string, utf16len);
	}
	~mb2wide() {
		delete [] utf16string;
	}
	int getsize() { return utf16len; }
	LPWSTR getstring() { return utf16string; }

private:
	LPWSTR utf16string;
	int utf16len;
};

class wide2mb
{
public:
	wide2mb(const wchar_t *data) : ansistring(0), ansilen(0) {
#ifdef _UTF8_SUPPORT
		UINT codepage = (lstate.utf8_mode) ? CP_UTF8 : CP_THREAD_ACP;
#else
		UINT codepage = CP_THREAD_ACP;
#endif
		ansilen = ::WideCharToMultiByte(codepage, 0, data, -1, NULL, 0, NULL, NULL);
		ansistring = new char[ansilen+1];
		::ZeroMemory(ansistring, sizeof(char) * (ansilen+1));
		::WideCharToMultiByte(codepage, 0, data, -1, ansistring, ansilen, NULL, NULL);
	}
	~wide2mb() {
		delete [] ansistring;
	}
	int getsize() { return ansilen; }
	char* getstring() { return ansistring; }

private:
	char* ansistring;
	int ansilen;
};

void init_table ();
char *find_last_slash (const char *p);
wchar_t *find_last_slash (const wchar_t *p);
wchar_t *find_slash (const wchar_t *p);
void slash2backsl (wchar_t *p);
size_t strlcpy (char *d, const char *s, size_t n);
wchar_t *stpcpy (wchar_t *d, const wchar_t *s);
wchar_t *trim_root (const wchar_t *path);
void sanitize_path (wchar_t *path);

#endif
