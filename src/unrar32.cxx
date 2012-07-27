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

#include "comm-arc.h"
#include <commctrl.h>
#include <stdio.h>
#define EXTERN /* empty */
#include "unrarapi.h"
#include "util.h"
#include "arcinfo.h"
#include "rar.h"
#include "unrar32.h"
#include "resource.h"
#include "dialog.h"

#define UNRAR32_VERSION 12

class in_progress
{
  static LONG lock;
  LONG non_zero;
public:
  in_progress () {non_zero = InterlockedIncrement (&lock);}
  ~in_progress () {InterlockedDecrement (&lock);}
  bool is_locked () const {return non_zero != 0L;}
};

LONG in_progress::lock = -1;

static void
no_unrar_dll (HWND hwnd)
{
  char buf[1024];
  if (!LoadString (lstate.hinst, IDS_UNRAR_NOT_LOADED, buf, sizeof buf))
    strcpy (buf, "Unable to load UnRAR.DLL");
  MessageBox (hwnd, buf, 0, MB_ICONHAND);
}

#define IN_API(not_loaded, busy) \
  if (!lstate.hrardll) return (not_loaded); \
  in_progress in_progress__; \
  if (in_progress__.is_locked ()) return (busy)

WORD WINAPI
UnrarGetVersion ()
{
  return lstate.hrardll ? UNRAR32_VERSION : 0;
}

BOOL WINAPI
UnrarGetRunning ()
{
  IN_API (1, 1);
  return 0;
}

BOOL WINAPI
UnrarGetBackGroundMode ()
{
  return lstate.s_bg_mode;
}

BOOL WINAPI
UnrarSetBackGroundMode (BOOL mode)
{
  IN_API (0, 0);
  lstate.s_bg_mode = mode;
  return 1;
}

BOOL WINAPI
UnrarGetCursorMode ()
{
  return lstate.s_cursor_mode;
}

BOOL WINAPI
UnrarSetCursorMode (BOOL cursor_mode)
{
  IN_API (0, 0);
  lstate.s_cursor_mode = cursor_mode;
  return 1;
}

WORD WINAPI
UnrarGetCursorInterval ()
{
  return lstate.s_cursor_interval;
}

BOOL WINAPI
UnrarSetCursorInterval (WORD interval)
{
  IN_API (0, 0);
  lstate.s_cursor_interval = interval;
  return 1;
}

int WINAPI
Unrar (HWND hwnd, LPCSTR args, LPSTR buf, DWORD size)
{
  if (!lstate.hrardll)
    no_unrar_dll (hwnd);

  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);

  cmdline cl;
  int e = cl.parse (args, 1);
  if (e)
    return e;

  bool disable = !hwnd || EnableWindow (hwnd, 0);

  ostrbuf obuf (buf, size);
  UnRAR unrar (hwnd, obuf);
  int x = unrar.xmain (cl.argc (), cl.argv ());
  if (!disable)
    EnableWindow (hwnd, 1);
  return x;
}

BOOL WINAPI
UnrarCheckArchive (const char *path, int mode)
{
  IN_API (0, 0);

  if ((mode & (CHECKARCHIVE_MASK | CHECKARCHIVE_ALL))
      == (CHECKARCHIVE_RAPID | CHECKARCHIVE_ALL))
    mode = (mode & ~CHECKARCHIVE_MASK) | CHECKARCHIVE_BASIC;

  rarData rd;
  if (!rd.open (path,
                ((mode & CHECKARCHIVE_MASK) == CHECKARCHIVE_FULLCRC
                 ? RAR_OM_EXTRACT : RAR_OM_LIST)))
    return 0;

  if(mode & CHECKARCHIVE_NOT_ASK_PASSWORD){
    rd.can_ask_password=false;
  }
  rarSetCallback(rd.h,rar_event_handler,(ULONG)&rd);

  int e, nfiles = 0;
  while (!(e = rd.read_header ())
         && !(rd.hd.Flags & FRAR_NEXTVOL)
         && ((mode & CHECKARCHIVE_MASK) != CHECKARCHIVE_FULLCRC)
         && (!(e = rd.test ())
             || (e == ERAR_BAD_DATA
                 && mode & CHECKARCHIVE_RECOVERY)))
    if ((mode & CHECKARCHIVE_MASK) == CHECKARCHIVE_RAPID
        && ++nfiles == 3)
      {
        e = ERAR_END_ARCHIVE;
        break;
      }
  if (e && e != ERAR_END_ARCHIVE){
    if(ERAR_UNKNOWN==e && rd.is_missing_password)return ERROR_PASSWORD_FILE;
    else return 0;
  }

  if (mode & CHECKARCHIVE_SFX
      && file_executable_p (path))
    return 0x8000 + SFX_WIN32_UNKNOWN;

  return 1;
}

int WINAPI
UnrarGetFileCount (const char *path)
{
  IN_API (-1, -1);

  rarData rd;
  if (!rd.open (path, RAR_OM_LIST))
    return -1;

  rarSetCallback(rd.h,rar_event_handler,(ULONG)&rd);

  int e, nfiles;
  for (nfiles = 0; !(e = rd.read_header ()) && !(e = rd.skip ()); nfiles++)
    ;
  return !e || e == ERAR_END_ARCHIVE ? nfiles : -1;
}

BOOL WINAPI
UnrarQueryFunctionList (int i)
{
  switch (i)
    {
    case ISARC:
    case ISARC_GET_VERSION:
    case ISARC_GET_CURSOR_INTERVAL:
    case ISARC_SET_CURSOR_INTERVAL:
    case ISARC_GET_BACK_GROUND_MODE:
    case ISARC_SET_BACK_GROUND_MODE:
    case ISARC_GET_CURSOR_MODE:
    case ISARC_SET_CURSOR_MODE:
    case ISARC_GET_RUNNING:
    case ISARC_CHECK_ARCHIVE:
    //case ISARC_CONFIG_DIALOG:
    case ISARC_GET_FILE_COUNT:
    case ISARC_QUERY_FUNCTION_LIST:
    case ISARC_GET_ARC_FILE_INFO:
    case ISARC_OPEN_ARCHIVE:
    case ISARC_CLOSE_ARCHIVE:
    case ISARC_FIND_FIRST:
    case ISARC_FIND_NEXT:
    case ISARC_GET_ARC_FILE_NAME:
    case ISARC_GET_ARC_FILE_SIZE:
    case ISARC_GET_ARC_ORIGINAL_SIZE:
    case ISARC_GET_ARC_COMPRESSED_SIZE:
    case ISARC_GET_ARC_RATIO:
    case ISARC_GET_ARC_DATE:
    case ISARC_GET_ARC_TIME:
    case ISARC_GET_ARC_OS_TYPE:
    case ISARC_GET_ARC_IS_SFX_FILE:
    case ISARC_GET_FILE_NAME:
    case ISARC_GET_ORIGINAL_SIZE:
    case ISARC_GET_COMPRESSED_SIZE:
    case ISARC_GET_RATIO:
    case ISARC_GET_DATE:
    case ISARC_GET_TIME:
    case ISARC_GET_CRC:
    case ISARC_GET_ATTRIBUTE:
    case ISARC_GET_OS_TYPE:
    case ISARC_GET_METHOD:
    //case ISARC_GET_WRITE_TIME:
    //case ISARC_GET_CREATE_TIME:
    //case ISARC_GET_ACCESS_TIME:
      return 1;
    }
  return 0;
}

BOOL WINAPI
UnrarConfigDialog (HWND hwnd, LPSTR szOptionBuffer, int iMode)
{
  if (!lstate.hrardll)
    no_unrar_dll (hwnd);
  IN_API (0, ERROR_ALREADY_RUNNING);
  char buf[256];
  sprintf (buf, "UNRAR32.DLL Version %d.%02d",
           UNRAR32_VERSION / 100, UNRAR32_VERSION % 100);
  MessageBox (hwnd, buf, "Info", MB_OK | MB_ICONINFORMATION);
  return 0;
}

int WINAPI
UnrarExtractMem (HWND hwnd, LPCSTR szCmdLine,
                 LPBYTE szBuffer, DWORD dwSize, time_t *lpTime,
                 LPWORD lpwAttr, LPDWORD lpdwWriteSize)
{
  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);
  return ERROR_NOT_SUPPORT;
}

int WINAPI
UnrarCompressMem (HWND hwnd, LPCSTR szCmdLine,
                  const BYTE *szBuffer, DWORD dwSize,
                  const time_t *_lpTime, const WORD *lpwAttr,
                  LPDWORD lpdwWriteSize)
{
  return ERROR_NOT_SUPPORT;
}

HARC WINAPI
UnrarOpenArchive (HWND hwnd, LPCSTR path, DWORD mode)
{
  if (!lstate.hrardll){
    if(!(mode & M_ERROR_MESSAGE_OFF)){
      no_unrar_dll (hwnd);
    }
  }
  IN_API (0, 0);
  arcinfo *info = 0;
  try {info = new arcinfo;} catch (...) {}
  if (!info)
    return 0;
  if (!info->open (path, mode))
    {
      delete info;
      return 0;
    }

  rarSetCallback(info->m_hunrar,rar_openarc_handler,(ULONG)info);

  return HARC (info);
}

int WINAPI
UnrarCloseArchive (HARC harc)
{
  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);
  arcinfo *info = arcinfo::find (harc);
  if (!info)
    return ERROR_HARC_ISNOT_OPENED;
  info->close ();
  delete info;
  return 0;
}

int WINAPI
UnrarFindFirst (HARC harc, LPCSTR pattern, INDIVIDUALINFO *vinfo)
{
  IN_API (-1, -1);
  arcinfo *info = arcinfo::find (harc);
  if (!info || !info->m_is_first_time)
    return -1;

  if (info->m_cl.parse (pattern, 0))
    return -1;
  info->m_glob.set_pattern (info->m_cl.argc (), info->m_cl.argv ());
  info->m_is_first_time = false;
  return info->findnext (vinfo, 0);
}

int WINAPI
UnrarFindNext (HARC harc, INDIVIDUALINFO *vinfo)
{
  IN_API (-1, -1);
  arcinfo *info = arcinfo::find (harc);
  if (!info || info->m_is_first_time)
    return -1;
  return info->findnext (vinfo, 1);
}

int WINAPI
UnrarGetArcFileName (HARC harc, LPSTR buf, int size)
{
  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);
  arcinfo *info = arcinfo::find (harc);
  if (!info)
    return ERROR_HARC_ISNOT_OPENED;
  if (int (strlen (info->m_arcpath)) >= size)
    return ERROR_BUF_TOO_SMALL;
  strcpy (buf, info->m_arcpath);
  return 0;
}

DWORD WINAPI
UnrarGetArcFileSize (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && !info->m_arcsize.s.h ? info->m_arcsize.s.l : -1;
}

DWORD WINAPI
UnrarGetArcOriginalSize (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && !info->m_orig_sz.s.h ? info->m_orig_sz.s.l : -1;
}

DWORD WINAPI
UnrarGetArcCompressedSize (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && !info->m_comp_sz.s.h ? info->m_comp_sz.s.l : -1;
}

WORD WINAPI
UnrarGetArcRatio (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info ? calc_ratio (info->m_comp_sz, info->m_orig_sz) : -1;
}

WORD WINAPI
UnrarGetArcDate (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info ? info->m_arcdate : -1;
}

WORD WINAPI
UnrarGetArcTime (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info ? info->m_arctime : -1;
}

UINT WINAPI
UnrarGetArcOSType (HARC harc)
{
  IN_API (UINT (-1), UINT (-1));
  arcinfo *info = arcinfo::find (harc);
  return info ? OSTYPE_UNKNOWN : -1;
}

int WINAPI
UnrarIsSFXFile (HARC harc)
{
  IN_API (-1, -1);
  arcinfo *info = arcinfo::find (harc);
  return info ? info->m_sfx : -1;
}

int WINAPI
UnrarGetFileName (HARC harc, LPSTR buf, int size)
{
  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);
  arcinfo *info = arcinfo::find (harc);
  if (!info)
    return ERROR_HARC_ISNOT_OPENED;
  if (!info->m_is_valid)
    return ERROR_NOT_SEARCH_MODE;
  if (int (strlen (info->m_hd.FileName)) >= size)
    return ERROR_BUF_TOO_SMALL;
  strcpy (buf, info->m_hd.FileName);
  return 0;
}

int WINAPI
UnrarGetMethod (HARC harc, LPSTR buf, int size)
{
  IN_API (ERROR_NOT_SUPPORT, ERROR_ALREADY_RUNNING);
  arcinfo *info = arcinfo::find (harc);
  if (!info)
    return ERROR_HARC_ISNOT_OPENED;
  if (!info->m_is_valid)
    return ERROR_NOT_SEARCH_MODE;
  const char *method = method_string (info->m_hd.Method);
  if (int (strlen (method)) >= size)
    return ERROR_BUF_TOO_SMALL;
  strcpy (buf, method);
  return 0;
}

DWORD WINAPI
UnrarGetOriginalSize (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid && !info->m_hd.UnpSizeHigh ? info->m_hd.UnpSize : -1;
}

DWORD WINAPI
UnrarGetCompressedSize (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid && !info->m_hd.PackSizeHigh ? info->m_hd.PackSize : -1;
}

WORD WINAPI
UnrarGetRatio (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return (info && info->m_is_valid
          ? calc_ratio (info->m_hd.PackSizeHigh, info->m_hd.PackSize,
                        info->m_hd.UnpSizeHigh, info->m_hd.UnpSize)
          : -1);
}

WORD WINAPI
UnrarGetDate (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid ? HIWORD (info->m_hd.FileTime) : -1;
}

WORD WINAPI
UnrarGetTime (HARC harc)
{
  IN_API (WORD (-1), WORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid ? LOWORD (info->m_hd.FileTime) : -1;
}

DWORD WINAPI
UnrarGetWriteTime (HARC harc)
{
  return DWORD (-1);
}

DWORD WINAPI
UnrarGetCreateTime (HARC harc)
{
  return DWORD (-1);
}

DWORD WINAPI
UnrarGetAccessTime (HARC harc)
{
  return DWORD (-1);
}

DWORD WINAPI
UnrarGetCRC (HARC harc)
{
  IN_API (DWORD (-1), DWORD (-1));
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid ? info->m_hd.FileCRC : -1;
}

int WINAPI
UnrarGetAttribute (HARC harc)
{
  IN_API (-1, -1);
  arcinfo *info = arcinfo::find (harc);
  return info && info->m_is_valid ? info->m_hd.FileAttr : -1;
}

UINT WINAPI
UnrarGetOSType (HARC harc)
{
  IN_API (UINT (-1), UINT (-1));
  arcinfo *info = arcinfo::find (harc);
  if (!info || !info->m_is_valid)
    return UINT (-1);
  return os_type (info->m_hd.HostOS);
}

BOOL WINAPI
UnrarSetOwnerWindow (HWND hwnd)
{
  IN_API (0, 0);
  if (lstate.has_callback)
    return 0;
  lstate.has_callback = true;
  lstate.hwnd_owner = hwnd;
  lstate.callback = 0;
  return 1;
}

BOOL WINAPI
UnrarClearOwnerWindow ()
{
  IN_API (0, 0);
  lstate.has_callback = false;
  lstate.hwnd_owner = 0;
  lstate.callback = 0;
  return 1;
}

BOOL WINAPI
UnrarSetOwnerWindowEx (HWND hwnd, LPARCHIVERPROC proc)
{
  IN_API (0, 0);
  if (lstate.has_callback)
    return 0;
  lstate.has_callback = true;
  lstate.hwnd_owner = hwnd;
  lstate.callback = proc;
  return 1;
}

BOOL WINAPI
UnrarKillOwnerWindowEx (HWND hwnd)
{
  IN_API (0, 0);
  if (!lstate.has_callback || hwnd != lstate.hwnd_owner)
    return 0;
  lstate.has_callback = false;
  lstate.hwnd_owner = 0;
  lstate.callback = 0;
  return 1;
}

int WINAPI
DllMain (HINSTANCE hinst, DWORD reason, VOID *)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      lstate.hinst = hinst;
      lstate.hrardll = load_rarapi ();
      init_table ();
      InitCommonControls ();
      break;

    case DLL_PROCESS_DETACH:
      arcinfo::cleanup ();
      if (lstate.hrardll)
        FreeLibrary (lstate.hrardll);
      break;
    }
  return 1;
}
