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
#include "unrarapi.h"
#include "util.h"
#include "arcinfo.h"

arcinfo *arcinfo::m_chain;

arcinfo *
arcinfo::find (HARC harc)
{
  for (arcinfo *p = m_chain; p; p = p->m_next)
    if (reinterpret_cast <arcinfo *> (harc) == p)
      return p;
  return 0;
}

void
arcinfo::cleanup ()
{
  while (m_chain)
    delete m_chain;
}

arcinfo::arcinfo ()
     : m_hunrar (0), m_prev (0), m_next (m_chain),m_is_missing_password(false)
{
  if (m_next)
    m_next->m_prev = this;
  m_chain = this;
}

arcinfo::~arcinfo ()
{
  if (m_prev)
    m_prev->m_next = m_next;
  else
    m_chain = m_next;
  if (m_next)
    m_next->m_prev = m_prev;
  close ();
}

bool
arcinfo::open (const char *filename, DWORD mode)
{
  WIN32_FIND_DATAW fd;
  mb2wide fnamew(filename);
  HANDLE h = FindFirstFileW (fnamew.getstring(), &fd);
  if (!h)
    return 0;
  FindClose (h);
  strcpy (m_arcpath, filename);
  m_mode = mode;
  m_arcsize.s.l = fd.nFileSizeLow;
  m_arcsize.s.h = fd.nFileSizeHigh;
  FILETIME lt;
  FileTimeToLocalFileTime (&fd.ftLastWriteTime, &lt);
  if (!FileTimeToDosDateTime (&lt, &m_arcdate, &m_arctime))
    m_arcdate = m_arctime = WORD (-1);
  m_sfx = file_executable_p (filename) ? SFX_WIN32_UNKNOWN : 0;

  m_orig_sz.d = 0;
  m_comp_sz.d = 0;
  m_is_first_time = true;
  m_is_valid = false;
  m_is_eof = false;

  rarOpenArchiveDataEx oade (fnamew.getstring(), RAR_OM_LIST);
  m_hunrar = rarOpenArchiveEx (&oade);
  return m_hunrar != 0;
}

int
arcinfo::close ()
{
  if (!m_hunrar)
    return 0;
  int e = rarCloseArchive (m_hunrar);
  if (e)
    return e;
  m_hunrar = 0;
  return 0;
}

int
arcinfo::findnext (INDIVIDUALINFO *vinfo, bool skip)
{
  do
    {
      if (m_is_eof
          || (skip && rarProcessFile (m_hunrar, RAR_SKIP, 0, 0))
          || rarReadHeaderEx (m_hunrar, &m_hde))
        {
          m_is_eof = true;
          m_is_valid = false;
          if(m_is_missing_password){
            return ERROR_PASSWORD_FILE;
          }else{
            return -1;
          }
        }
      skip = true;
    }
  while (!m_glob.match (m_hde.FileName, (m_mode & M_CHECK_ALL_PATH) != 0, 0));

  m_orig_sz.d += m_hde.UnpSize;
  m_orig_sz.s.h += m_hde.UnpSizeHigh;
  m_comp_sz.d += m_hde.PackSize;
  m_comp_sz.s.h += m_hde.PackSizeHigh;
  m_is_valid = true;

  if (vinfo)
    {
      vinfo->dwOriginalSize = !m_hde.UnpSizeHigh ? m_hde.UnpSize : -1;
      vinfo->dwCompressedSize = !m_hde.PackSizeHigh ? m_hde.PackSize : -1;
      vinfo->dwCRC = m_hde.FileCRC;
      vinfo->uFlag = 0;
      vinfo->uOSType = os_type (m_hde.HostOS);
      vinfo->wRatio = WORD (calc_ratio (m_hde.PackSizeHigh, m_hde.PackSize,
                                        m_hde.UnpSizeHigh, m_hde.UnpSize));
      vinfo->wDate = HIWORD (m_hde.FileTime);
      vinfo->wTime = LOWORD (m_hde.FileTime);
      strlcpy (vinfo->szFileName, wide2mb(m_hde.FileNameW).getstring(), sizeof vinfo->szFileName);
      strcpy (vinfo->szAttribute, attr_string (m_hde.FileAttr));
      strcpy (vinfo->szMode, method_string (m_hde.Method));
    }
  return 0;
}
