/*
 *   Copyright (c) 1998-2002 T. Kamei (kamei@jsdlab.co.jp)
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
#include "mapf.h"

void
mapf::close ()
{
  if (mf_base)
    UnmapViewOfFile (mf_base);
  if (mf_hm)
    CloseHandle (mf_hm);
  if (mf_hf != INVALID_HANDLE_VALUE)
    CloseHandle (mf_hf);
  init ();
}

int
mapf::open (const char *path, int mode)
{
  close ();
  mf_hf = CreateFile (path, GENERIC_READ, FILE_SHARE_READ, 0,
                      OPEN_EXISTING, mode, 0);
  if (mf_hf == INVALID_HANDLE_VALUE)
    return 0;

  mf_size = GetFileSize (mf_hf, 0);
  if (mf_size)
    {
      mf_hm = CreateFileMapping (mf_hf, 0, PAGE_READONLY, 0, 0, 0);
      if (!mf_hm)
        return 0;

      mf_base = MapViewOfFile (mf_hm, FILE_MAP_READ, 0, 0, 0);
      if (!mf_base)
        return 0;
    }
  return 1;
}
