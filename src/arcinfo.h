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

#ifndef _arcinfo_h_
#define _arcinfo_h_

class arcinfo
{
public:
  rarHeaderDataEx m_hde;
  cmdline m_cl;
  glob m_glob;
  HANDLE m_hunrar;
  DWORD m_mode;
  int64 m_arcsize;
  int64 m_orig_sz;
  int64 m_comp_sz;
  int m_sfx;
  WORD m_arcdate;
  WORD m_arctime;
  bool m_is_first_time;
  bool m_is_valid;
  bool m_is_eof;
  char m_arcpath[1024 + 1];
  bool m_is_missing_password;

  arcinfo ();
  ~arcinfo ();
  bool open (const char *filename, DWORD mode);
  int close ();
  int findnext (INDIVIDUALINFO *vinfo, bool skip);
  static arcinfo *find (HARC);
  static void cleanup ();

private:
  arcinfo *m_prev;
  arcinfo *m_next;
  static arcinfo *m_chain;
};

#endif /* _arcinfo_h_ */
