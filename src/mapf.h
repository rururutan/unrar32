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

#ifndef _MAPF_H_
# define _MAPF_H_

class mapf
{
public:
  mapf ()
    {init ();}
  ~mapf ()
    {close ();}
  bool open (const char *path, int mode = FILE_FLAG_SEQUENTIAL_SCAN)
    {
      close ();
      mb2wide pathw(path);
      m_hf = CreateFileW (pathw.getstring(), GENERIC_READ, FILE_SHARE_READ, 0,
                          OPEN_EXISTING, mode, 0);
      if (m_hf == INVALID_HANDLE_VALUE)
        return false;

      m_size = GetFileSize (m_hf, 0);
      if (m_size)
        {
          m_hm = CreateFileMapping (m_hf, 0, PAGE_READONLY, 0, 0, 0);
          if (!m_hm)
            return false;

          m_base = MapViewOfFile (m_hm, FILE_MAP_READ, 0, 0, 0);
          if (!m_base)
            return false;
        }
      return true;
    }
  void close ()
    {
      if (m_base)
        UnmapViewOfFile (m_base);
      if (m_hm)
        CloseHandle (m_hm);
      if (m_hf != INVALID_HANDLE_VALUE)
        CloseHandle (m_hf);
      init ();
    }
  const void *base () const
    {return m_base;}
  DWORD size () const
    {return m_size;}
  operator HANDLE () const
    {return m_hf;}

private:
  HANDLE m_hf;
  HANDLE m_hm;
  DWORD m_size;
  void *m_base;

  void init ()
    {
      m_hf = INVALID_HANDLE_VALUE;
      m_hm = 0;
      m_base = 0;
      m_size = 0;
    }
};
#endif
