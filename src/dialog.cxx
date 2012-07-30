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
#include <commctrl.h>
#include <mbstring.h>
#include "comm-arc.h"
#include "util.h"
#include "dialog.h"

static void
center_window (HWND hwnd)
{
  HWND owner = GetWindow (hwnd, GW_OWNER);
  if (!owner)
    owner = GetParent (hwnd);
  if (!owner)
    owner = GetDesktopWindow ();

  RECT dr, or;
  GetWindowRect (hwnd, &dr);
  GetWindowRect (owner, &or);

  LONG left = (or.left + (or.right - or.left) / 3
               - (dr.right - dr.left) / 3);
  LONG top = (or.top + (or.bottom - or.top) / 3
              - (dr.bottom - dr.top) / 3);

  RECT work;
  SystemParametersInfo (SPI_GETWORKAREA, 0, &work, 0);

  left = min (max (left, work.left), work.right - (dr.right - dr.left));
  top = min (max (top, work.top), work.bottom - (dr.bottom - dr.top));

  SetWindowPos (hwnd, 0, left, top, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static char *
insert_comma (char *b, int l)
{
  char *f = b + l, *te = f + (l - 1) / 3, *t = te;
  *te = 0;
  b += 3;
  while (f > b)
    {
      *--t = *--f;
      *--t = *--f;
      *--t = *--f;
      *--t = ',';
    }
  return te;
}

static void
set_size (HWND hwnd, int id, const int64 &size)
{
  char buf[64];
  strcpy (insert_comma (buf, sprintf (buf, "%I64u", size)), " bytes");
  SetDlgItemText (hwnd, id, buf);
}

static void
set_date (HWND hwnd, int id, DWORD date)
{
  char buf[32];
  sprintf (buf, "%d/%02d/%02d %02d:%02d:%02d",
           (date >> 25) + 1980,
           (date >> 21) & 15,
           (date >> 16) & 31,
           (date >> 11) & 31,
           (date >> 5) & 63,
           (date & 31) * 2);
  SetDlgItemText (hwnd, id, buf);
}

static INT_PTR CALLBACK
replace_dlgproc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      {
        center_window (hwnd);
        replace_param *rp = (replace_param *)lparam;
        char *sl = find_last_slash (rp->name);
        if (sl)
          {
            SetDlgItemText (hwnd, IDC_NAME, sl + 1);
            *sl = 0;
            SetDlgItemText (hwnd, IDC_PATH, rp->name);
            *sl = '\\';
          }
        else
          {
            SetDlgItemText (hwnd, IDC_NAME, rp->name);
            SetDlgItemText (hwnd, IDC_PATH, "");
          }
        set_size (hwnd, IDC_OLDSIZE, rp->old_size);
        set_date (hwnd, IDC_OLDDATE, rp->old_date);
        set_size (hwnd, IDC_NEWSIZE, rp->new_size);
        set_date (hwnd, IDC_NEWDATE, rp->new_date);
        SendMessage (GetDlgItem (hwnd, IDC_Q), STM_SETICON,
                     WPARAM (LoadIcon (0, IDI_QUESTION)), 0);
        return 1;
      }
    case WM_COMMAND:
      switch (LOWORD (wparam))
        {
        case IDYES:
        case IDNO:
        case IDC_ALL:
        case IDCANCEL:
          EndDialog (hwnd, LOWORD (wparam));
          return 1;
        }
      break;
    }
  return 0;
}

INT_PTR
replace_dialog (HWND hwnd_parent, const replace_param &rp)
{
  return DialogBoxParam (lstate.hinst, MAKEINTRESOURCE (IDD_REPLACE),
                         hwnd_parent, replace_dlgproc, LPARAM (&rp));
}

void
doevents ()
{
  MSG msg;
  while (PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage (&msg);
      DispatchMessage (&msg);
    }
}

INT_PTR CALLBACK
progress_dlg::progress_dlgproc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  progress_dlg *p;
  if (msg == WM_INITDIALOG)
    {
      SetWindowLongPtr (hwnd, DWLP_USER, lparam);
      p = (progress_dlg *)lparam;
      p->m_hwnd = hwnd;
    }
  else
    p = (progress_dlg *)GetWindowLongPtr (hwnd, DWLP_USER);
  return p ? p->wndproc (msg, wparam, lparam) : 0;
}

BOOL
progress_dlg::wndproc (UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      center_window (m_hwnd);
      return 1;

    case WM_COMMAND:
      if (LOWORD (wparam) == IDCANCEL)
        {
          DestroyWindow (m_hwnd);
          m_hwnd = 0;
          return 1;
        }
      break;
    }
  return 0;
}

int
progress_dlg::create (HWND hwnd_parent)
{
  if (!CreateDialogParam (lstate.hinst, MAKEINTRESOURCE (IDD_INPROG),
                          hwnd_parent, progress_dlgproc, LPARAM (this)))
    return 0;
  ShowWindow (m_hwnd, SW_SHOW);
  doevents ();
  return 1;
}

static void
set_prog_size (HWND hwnd, int id, const int64 &size, const int64 &total)
{
  char buf[128];
  char *b = insert_comma (buf, sprintf (buf, "%I64u", size));
  *b++ = '/';
  strcpy (insert_comma (b, sprintf (b, "%I64u", total)), " bytes");
  SetDlgItemText (hwnd, id, buf);
}

int
progress_dlg::init (const char *path, unsigned maxl, unsigned maxh)
{
  doevents ();
  if (!m_hwnd)
    return 0;
  m_max_bytes.s.l = maxl;
  m_max_bytes.s.h = maxh;
  SendDlgItemMessage (m_hwnd, IDC_PROG, PBM_SETRANGE, 0, MAKELPARAM (0, PROGRESS_MAX));
  SendDlgItemMessage (m_hwnd, IDC_PROG, PBM_SETPOS, 0, 0);
  char *sl = find_last_slash (path);
  if (sl)
    {
      *sl = 0;
      SetDlgItemText (m_hwnd, IDC_DSTPATH, path);
      *sl = '\\';
      SetDlgItemText (m_hwnd, IDC_NAME, sl + 1);
    }
  else
    {
      SetDlgItemText (m_hwnd, IDC_DSTPATH, "");
      SetDlgItemText (m_hwnd, IDC_NAME, path);
    }
  SetDlgItemText (m_hwnd, IDC_PERCENT, "  0%");
  int64 zero;
  zero.d = 0;
  set_prog_size (m_hwnd, IDC_FILESIZE, zero, m_max_bytes);
  return 1;
}

int
progress_dlg::update (const int64 &n) const
{
  doevents ();
  if (!m_hwnd)
    return 0;
  int percent = m_max_bytes.d ? int (PROGRESS_MAX * double (n.d) / m_max_bytes.d) : PROGRESS_MAX;
  SendDlgItemMessage (m_hwnd, IDC_PROG, PBM_SETPOS, percent, 0);

  set_prog_size (m_hwnd, IDC_FILESIZE, n, m_max_bytes);
  char buf[32];
  sprintf (buf, "%3d%%", percent);
  SetDlgItemText (m_hwnd, IDC_PERCENT, buf);
  return 1;
}

static wchar_t passwd[128];

static INT_PTR CALLBACK
askpass_dlgproc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      center_window (hwnd);
      return 1;

    case WM_COMMAND:
      switch (LOWORD (wparam))
        {
        case IDOK:
          GetDlgItemTextW (hwnd, IDC_PASSWD, passwd, sizeof passwd - 1);
          /* fall thru... */
        case IDCANCEL:
          EndDialog (hwnd, LOWORD (wparam));
          return 1;
        }
      break;
    }
  return 0;
}

wchar_t *
askpass_dialog (HWND hwnd_parent)
{
  return DialogBoxW (lstate.hinst, MAKEINTRESOURCEW (IDD_ASKPASSWD),
                    hwnd_parent, askpass_dlgproc) == IDOK ? passwd : 0;
}

static char *vol_name;

static INT_PTR CALLBACK
changevol_dlgproc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      center_window (hwnd);
      SetDlgItemText (hwnd, IDC_PATH, vol_name);
      return 1;

    case WM_COMMAND:
      switch (LOWORD (wparam))
        {
        case IDC_REF:
          {
            char buf[MAX_PATH];
            char filter[256];
            LoadString (lstate.hinst, IDS_FILTER, filter, sizeof filter);
            for (char *p = filter; (p = (char *)_mbschr ((u_char *)p, '|')); p++)
              *p = 0;
            GetDlgItemText (hwnd, IDC_PATH, buf, sizeof buf);
            OPENFILENAME of;
            memset (&of, 0, sizeof of);
            of.lStructSize = sizeof of;
            of.hwndOwner = hwnd;
            of.lpstrFilter = filter;
            of.nFilterIndex = 1;
            of.lpstrFile = buf;
            of.nMaxFile	= sizeof buf;
            of.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR
                        | OFN_PATHMUSTEXIST | OFN_EXPLORER);
            if (GetOpenFileName (&of))
              SetDlgItemText (hwnd, IDC_PATH, buf);
            return 1;
          }

        case IDOK:
          {
            char buf[MAX_PATH];
            GetDlgItemText (hwnd, IDC_PATH, buf, sizeof buf);
            if (!*buf)
              return 1;
            strcpy (vol_name, buf);
          }
          /* fall thru... */
        case IDCANCEL:
          EndDialog (hwnd, LOWORD (wparam));
          return 1;
        }
      break;
    }
  return 0;
}

int
change_vol_dialog (HWND hwnd_parent, char *path)
{
  vol_name = path;
  return (DialogBox (lstate.hinst, MAKEINTRESOURCE (IDD_CHANGEVOL),
    hwnd_parent, changevol_dlgproc) == IDOK) ? 1 : -1;
}
