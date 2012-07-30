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

#ifndef _dialog_h_
# define _dialog_h_

# include "resource.h"

struct replace_param
{
  const wchar_t *name;
  DWORD old_date;
  int64 old_size;
  DWORD new_date;
  int64 new_size;
};

INT_PTR replace_dialog (HWND hwnd_parent, const replace_param &rp);

class progress_dlg
{
  enum {PROGRESS_MAX = 100};
public:
  HWND m_hwnd;
  int64 m_max_bytes;
  progress_dlg ()
       : m_hwnd (0)
    {m_max_bytes.d = 0;}
  ~progress_dlg ()
    {if (m_hwnd) DestroyWindow (m_hwnd);}
  int create (HWND hwnd_parent);
  int init (const wchar_t *path, unsigned maxl, unsigned maxh);
  int update (const int64 &n) const;
  static INT_PTR CALLBACK progress_dlgproc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  BOOL wndproc (UINT msg, WPARAM wparam, LPARAM lparam);
};

wchar_t *askpass_dialog (HWND hwnd_parent);
int change_vol_dialog (HWND hwnd_parent, wchar_t *path);

#endif
