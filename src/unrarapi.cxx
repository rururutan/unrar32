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

static int __stdcall
ReadHeaderEx (HANDLE h, RARHeaderDataEx *hde)
{
  RARHeaderData hd;
  hd.CmtBuf = hde->CmtBuf;
  hd.CmtBufSize = hde->CmtBufSize;

  int r = rarReadHeader (h, &hd);

  strlcpy (hde->ArcName, hd.ArcName, sizeof hd.ArcName);
  *hde->ArcNameW = 0;
  strlcpy (hde->FileName, hd.FileName, sizeof hd.FileName);
  *hde->FileNameW = 0;
  hde->Flags = hd.Flags;
  hde->PackSize = hd.PackSize;
  hde->PackSizeHigh = 0;
  hde->UnpSize = hd.UnpSize;
  hde->UnpSizeHigh = 0;
  hde->HostOS = hd.HostOS;
  hde->FileCRC = hd.FileCRC;
  hde->FileTime = hd.FileTime;
  hde->UnpVer = hd.UnpVer;
  hde->Method = hd.Method;
  hde->FileAttr = hd.FileAttr;
  hde->CmtBuf = hd.CmtBuf;
  hde->CmtBufSize = hd.CmtBufSize;
  hde->CmtSize = hd.CmtSize;
  hde->CmtState = hd.CmtState;

  return r;
}

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
  if ((rarOpenArchive = RAROPENARCHIVE (GetProcAddress (h, "RAROpenArchive")))
      && (rarCloseArchive = RARCLOSEARCHIVE (GetProcAddress (h, "RARCloseArchive")))
      && (rarProcessFile = RARPROCESSFILE (GetProcAddress (h, "RARProcessFile")))
      && (rarSetCallback = RARSETCALLBACK (GetProcAddress (h, "RARSetCallback"))))
    {
      rarReadHeaderEx = RARREADHEADEREX (GetProcAddress (h, "RARReadHeaderEx"));
      if (rarReadHeaderEx)
        return h;
      rarReadHeader = RARREADHEADER (GetProcAddress (h, "RARReadHeader"));
      if (rarReadHeader)
        {
          rarReadHeaderEx = ReadHeaderEx;
          return h;
        }
    }

  FreeLibrary (h);
  return 0;
}

bool
rarData::open (const char *filename, int mode, char *buf, int size)
{
  oad.ArcName = (char *)filename;
  oad.CmtBuf = buf;
  oad.CmtBufSize = size;
  oad.OpenMode = mode;
  hd.CmtBuf = 0;
  hd.CmtBufSize = 0;
  h = rarOpenArchive (&oad);
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
file_executable_p (const char *path)
{
  bool f = false;
  FILE *fp = fopen (path, "rb");
  if (fp)
    {
      IMAGE_DOS_HEADER dos;
      IMAGE_NT_HEADERS nt;
      f = (fread (&dos, sizeof dos, 1, fp) == 1
           && dos.e_magic == IMAGE_DOS_SIGNATURE
           && !fseek (fp, dos.e_lfanew, SEEK_SET)
           && fread (&nt, sizeof nt, 1, fp) == 1
           && nt.Signature == IMAGE_NT_SIGNATURE);
      fclose (fp);
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
