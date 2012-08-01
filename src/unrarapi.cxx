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
#include "unrarapi.h"
#include "util.h"

HINSTANCE
load_rarapi ()
{
#ifdef _WIN64
  HINSTANCE h = LoadLibrary ("unrar64.dll");
#else
  HINSTANCE h = LoadLibrary ("unrar.dll");
#endif
  if (!h)
    return 0;
  if ((rarOpenArchiveEx = RAROPENARCHIVEEX (GetProcAddress (h, "RAROpenArchiveEx")))
      && (rarCloseArchive = RARCLOSEARCHIVE (GetProcAddress (h, "RARCloseArchive")))
      && (rarProcessFileW = RARPROCESSFILEW (GetProcAddress (h, "RARProcessFileW")))
      && (rarSetCallback = RARSETCALLBACK (GetProcAddress (h, "RARSetCallback")))
      && (rarReadHeaderEx = RARREADHEADEREX (GetProcAddress (h, "RARReadHeaderEx"))))
    {
      return h;
    }

  FreeLibrary (h);
  return 0;
}

bool
rarData::open (const wchar_t *filename, int mode, char *buf, int size)
{
  oade.ArcName = 0;
  oade.ArcNameW = (wchar_t *)filename;
  oade.CmtBuf = buf;
  oade.CmtBufSize = size;
  oade.OpenMode = mode;
  oade.Callback = 0;
  oade.UserData = 0;
  ZeroMemory(oade.Reserved, sizeof(oade.Reserved));
  hde.CmtBuf = 0;
  hde.CmtBufSize = 0;
  h = rarOpenArchiveEx (&oade);
  return h != 0;
}

int
rarData::close ()
{
  if (!h)
    return 0;
  int x = rarCloseArchive (h);
  h = 0;
  return x;
}

bool
file_executable_p (const wchar_t *path)
{
  bool f = false;
  HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile != INVALID_HANDLE_VALUE)
  {
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS nt;
    DWORD readsize;
    f = (ReadFile(hFile, &dos, sizeof(dos), &readsize, NULL) == TRUE &&
         dos.e_magic == IMAGE_DOS_SIGNATURE &&
         SetFilePointer (hFile, dos.e_lfanew, NULL, FILE_BEGIN) == dos.e_lfanew &&
         ReadFile(hFile, &nt, sizeof(nt), &readsize, NULL) == TRUE &&
         nt.Signature == IMAGE_NT_SIGNATURE);
    CloseHandle(hFile);
  }
  return f;
}

int
calc_ratio (u_long comp_sz_high, u_long comp_sz_low,
            u_long orig_sz_high, u_long orig_sz_low)
{
  int64 comp_sz, orig_sz;
  comp_sz.s.h = comp_sz_high;
  comp_sz.s.l = comp_sz_low;
  orig_sz.s.h = orig_sz_high;
  orig_sz.s.l = orig_sz_low;
  return calc_ratio (comp_sz, orig_sz);
}

int
calc_ratio (const int64 &comp_sz, const int64 &orig_sz)
{
  return orig_sz.d ? int (double (comp_sz.d) / orig_sz.d * 1000) : 0;
}

const char *
method_string (int method)
{
  switch (method)
    {
    case 0x30:
      return "storing";
    case 0x31:
      return "fastest";
    case 0x32:
      return "fast";
    case 0x33:
      return "normal";
    case 0x34:
      return "good";
    case 0x35:
      return "best";
    default:
      return "unknown";
    }
}

int
os_type (int os)
{
  switch (os)
    {
    case 0:
      return OSTYPE_MSDOS;
    case 1:
      return OSTYPE_OS2;
    case 2:
      return OSTYPE_WINDOWS95; // Win32
    case 3:
      return OSTYPE_UNIX;
    default:
      return OSTYPE_UNKNOWN;
    }
}

const char *
attr_string (int attr)
{
  static char b[5];
  b[0] = (attr & FILE_ATTRIBUTE_DIRECTORY
          ? 'd'
          : (attr & FILE_ATTRIBUTE_ARCHIVE
             ? 'a' : '-'));
  b[1] = attr & FILE_ATTRIBUTE_SYSTEM ? 's' : '-';
  b[2] = attr & FILE_ATTRIBUTE_HIDDEN ? 'h' : '-';
  b[3] = attr & FILE_ATTRIBUTE_READONLY ? '-' : 'w';
  return b;
}
