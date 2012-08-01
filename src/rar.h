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

#ifndef _rar_h_
# define _rar_h_

class UnRAR
{
public:
  enum unrar_update_type
    {UT_ASK, UT_OVWRT, UT_SKIP, UT_NEWER, UT_EXISTING};

  enum unrar_cmd
    {
      C_NOTDEF,
      C_EXTRACT,
      C_EXTRACT_NODIR,
      C_TEST,
      C_PRINT,
      C_LIST,
      C_VLIST,
      C_COMMENT
    };

  enum unrar_opt
    {
      O_RECURSIVE = 1,
      O_YES = 2,
      O_STRICT = 8,
      O_QUIET = 16,
	  O_NOT_ASK_PASSWORD = 32,
    };

public:
  int xmain (int argc, char **argv);
  UnRAR (HWND hwnd, ostrbuf &ostr)
       : m_hwnd (hwnd), m_ostr (ostr), m_passwd (0), m_path(0)
    {}
  ~UnRAR ()
    {
      delete [] m_passwd;
      delete [] m_path;
    }

  const wchar_t* get_password();

private:
  unrar_cmd m_cmd;
  unrar_update_type m_type;
  int m_opt;
  int m_security_level;
  const wchar_t *m_path;
  const wchar_t *m_passwd;
  wchar_t m_dest[FNAME_MAX32 + 1];

  HWND m_hwnd;

  ostrbuf &m_ostr;
  glob m_glob;
  EXTRACTINGINFOEX m_ex;

  int mkdirhier (const wchar_t *path);
  int check_timestamp (const wchar_t *path, const rarHeaderDataEx &hde);
  int parse_opt (int ac, char **av);
  int extract (rarData &rd, const wchar_t *path, const rarHeaderDataEx &hde,
               class progress_dlg &process);
  int extract ();
  int extract1 ();
  int print ();
  int list ();
  int test ();
  int comment ();

  int format (const wchar_t *fmt, ...) const;
  int format (int id, ...) const;

  int open_err (int e) const;
  int header_err (int e,const rarData &rd) const;
  int process_err (int e,const wchar_t *path, const rarData &rd) const;
  int canceled () const;
  int skip (rarData &rd, const wchar_t *path) const;
};

//Callback for UnRAR.DLL
int CALLBACK rar_event_handler(UINT msg,LPARAM UserData,LPARAM P1,LPARAM P2);
int CALLBACK rar_openarc_handler(UINT msg,LPARAM UserData,LPARAM P1,LPARAM P2);	//for UnrarOpenArchiveEx()

#endif
