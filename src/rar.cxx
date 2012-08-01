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
#include <mbstring.h>
#include <stdio.h>
#include "comm-arc.h"
#include "util.h"
#include "unrarapi.h"
#include "rar.h"
#include "dialog.h"
#include "arcinfo.h"

int
UnRAR::open_err (int e) const
{
  switch (e)
    {
    case ERAR_NO_MEMORY:
      format (IDS_NOT_ENOUGH_MEMORY);
      return ERROR_ENOUGH_MEMORY;

    case ERAR_BAD_DATA:
      format (IDS_ARCHIVE_HEADER_BROKEN, m_path);
      return ERROR_HEADER_BROKEN;

    case ERAR_BAD_ARCHIVE:
      format (IDS_NOT_VALID_RAR_ARCHIVE, m_path);
      return ERROR_FILE_STYLE;

    case ERAR_EOPEN:
      format (IDS_FILE_OPEN_ERROR, m_path);
      return ERROR_NOT_FIND_ARC_FILE;

    case ERAR_UNKNOWN:	//FALLTHROUGH
    default:
      format (IDS_UNDOCUMENTED, e, m_path);
      return ERROR_UNEXPECTED;
    }
}

int
UnRAR::process_err (int e,const wchar_t *path, const rarData &rd) const
{
  switch (e)
    {
    case ERAR_BAD_DATA:
      format (IDS_CRC_ERROR, path);
      return ERROR_FILE_CRC;

    case ERAR_BAD_ARCHIVE:
      format (IDS_NOT_VALID_RAR_VOLUME, path);
      return ERROR_FILE_STYLE;

    case ERAR_UNKNOWN_FORMAT:
      format (IDS_UNKNOWN_ARCHIVE_FORMAT, path);
      return ERROR_FILE_STYLE;

    case ERAR_EOPEN:
      format (IDS_VOLUME_OPEN, path);
      return ERROR_ARC_FILE_OPEN;

    case ERAR_ECREATE:
      format (IDS_FILE_CREATE_ERROR, path);
      return ERROR_FILE_OPEN;

    case ERAR_ECLOSE:
      format (IDS_FILE_CLOSE_ERROR, path);
      return ERROR_CANNOT_WRITE;

    case ERAR_EREAD:
      format (IDS_READ_ERROR, path);
      return ERROR_CANNOT_READ;

    case ERAR_EWRITE:
      format (IDS_WRITE_ERROR, path);
      return ERROR_CANNOT_WRITE;

    case ERAR_UNKNOWN:	//FALLTHROUGH
    default:
      if(rd.is_missing_password){
        format (IDS_MISSING_PASSWORD, path);
        return ERROR_PASSWORD_FILE;
      }else{
        format (IDS_UNDOCUMENTED, e, path);
        return ERROR_UNEXPECTED;
      }
    }
}

int
UnRAR::header_err (int e,const rarData &rd) const
{
  switch (e)
    {
    case ERAR_END_ARCHIVE:
      return 0;

    case ERAR_BAD_DATA:
      format (IDS_FILE_HEADER_BROKEN, m_path);
      return ERROR_HEADER_BROKEN;

    case ERAR_UNKNOWN:	//FALLTHROUGH
    default:
      if(rd.is_missing_password){
        format (IDS_MISSING_PASSWORD, m_path);
        return ERROR_PASSWORD_FILE;
      }else{
        format (IDS_UNDOCUMENTED, e, m_path);
        return ERROR_UNEXPECTED;
      }
    }
}

int
UnRAR::format (const wchar_t *fmt, ...) const
{
  va_list ap;
  va_start (ap, fmt);
  int x = m_ostr.formatv (fmt, ap);
  va_end (ap);
  return x;
}

int
UnRAR::format (int id, ...) const
{
  wchar_t fmt[1024];
  if (!LoadStringW (lstate.hinst, id, fmt, sizeof fmt))
    return format (L"Unable to load message string: %d\n", id);
  else
    {
      va_list ap;
      va_start (ap, id);
      int x = m_ostr.formatv (fmt, ap);
      va_end (ap);
      return x;
    }
}

int
UnRAR::parse_opt (int ac, char **av)
{
  m_cmd = C_NOTDEF;
  m_opt = 0;
  m_type = UT_ASK;
  m_dest[0] = 0;
  if (m_passwd)
    delete [] m_passwd;
  m_passwd = 0;
  if (m_path)
    delete [] m_path;
  m_path = 0;
  m_security_level = 2;

  if (!ac)
    {
      format (IDS_NO_ARCHIVE_FILE);
      return ERROR_NOT_ARC_FILE;
    }

  int c = av[0][0] == '-' ? av[0][1] : av[0][0];
  switch (c)
    {
    case 'x':
      m_cmd = C_EXTRACT;
      break;

    case 'e':
      m_cmd = C_EXTRACT_NODIR;
      break;

    case 't':
      m_cmd = C_TEST;
      break;

    case 'p':
      m_cmd = C_PRINT;
      break;

    case 'l':
      m_cmd = C_LIST;
      break;

    case 'v':
      m_cmd = C_VLIST;
      break;

    case 'c':
      m_cmd = C_COMMENT;
      break;

    default:
      format (IDS_UNRECOGNIZED_OPTION, c);
      return ERROR_COMMAND_NAME;
    }

  int i;
  for (i = 1; i < ac && av[i][0] == '-'; i++)
    switch (av[i][1])
      {
      case 'r':
        m_opt |= O_RECURSIVE;
        break;

      case 'y':
        m_opt |= O_YES;
        break;

      case 'o':
        m_type = av[i][2] == '-' ? UT_SKIP : UT_OVWRT;
        break;

      case 'f':
        m_type = UT_EXISTING;
        break;

      case 'u':
        m_type = UT_NEWER;
        break;

      case 's':
        m_opt |= O_STRICT;
        break;

      case 'p':
        if (av[i][2])
        {
          mb2wide passw(&av[i][2]);
          wchar_t *pass = new wchar_t[passw.getsize()];
          wcsncpy(pass, passw.getstring(), passw.getsize());
          m_passwd = pass;
        }
        else if (++i < ac)
        {
          mb2wide passw(av[i]);
          wchar_t *pass = new wchar_t[passw.getsize()];
          wcsncpy(pass, passw.getstring(), passw.getsize());
          m_passwd = pass;
        }
        else
          {
            format (IDS_OPTION_REQ_ARGS, 'p');
            return ERROR_COMMAND_NAME;
          }
        break;

      case 'q':
        m_opt |= O_QUIET;
        break;

      case 'e':
        if (av[i][2])
          m_security_level = av[i][2];
        else if (++i < ac)
          m_security_level = av[i][0];
        else
          {
            format (IDS_OPTION_REQ_ARGS, 'e');
            return ERROR_COMMAND_NAME;
          }
        if (m_security_level != '1' && m_security_level != '2')
          {
            format (IDS_INVALID_SECURITY_LEVEL, m_security_level);
            return ERROR_COMMAND_NAME;
          }
        m_security_level -= '0';
        break;

      case 'h':
        m_opt |= O_NOT_ASK_PASSWORD;
        break;

      case '-':
        i++;
        goto optend;

      default:
        format (IDS_UNRECOGNIZED_OPTION, av[i][1]);
        return ERROR_COMMAND_NAME;
      }
optend:
  if (i >= ac)
    {
      format (IDS_NO_ARCHIVE_FILE);
      return ERROR_NOT_ARC_FILE;
    }
  mb2wide pathw(av[i++]);
  m_path = new wchar_t[pathw.getsize()];
  wcscpy( const_cast<wchar_t*>(m_path), pathw.getstring() );

  if (i < ac)
    {
      char *sl = find_last_slash (av[i]);
      if (sl && !sl[1])
        {
          wcscpy(m_dest, mb2wide(av[i++]).getstring());
          if (wcslen (m_dest) >= FNAME_MAX32)
            {
              format (IDS_FILE_NAME_TOO_LONG, m_dest);
              return ERROR_LONG_FILE_NAME;
            }
        }
    }

  m_glob.set_pattern (ac - i, av + i);

  return 0;
}

class dyn_handle
{
public:
  dyn_handle () : m_handle (INVALID_HANDLE_VALUE) {}
  dyn_handle (HANDLE h) : m_handle (h) {}
  ~dyn_handle ()
    {if (is_valid ()) CloseHandle (m_handle);}
  bool is_valid () const
    {return m_handle != INVALID_HANDLE_VALUE;}
  operator HANDLE () const
    {return m_handle;}
  void attach (HANDLE h) {m_handle = h;}
  void close ()
    {
      if (is_valid ())
        {
          CloseHandle (m_handle);
          m_handle = INVALID_HANDLE_VALUE;
        }
    }
private:
  void operator = (const dyn_handle &);
  dyn_handle (const dyn_handle &);
  HANDLE m_handle;
};

class write_handle: public dyn_handle
{
public:
  write_handle (const wchar_t *path)
       : m_complete (false), m_delete_if_fail (false), m_path (path)
    {}
  ~write_handle ()
    {
      if (is_valid ())
        {
          close ();
          if (!m_complete && m_delete_if_fail)
            DeleteFileW (m_path);
        }
    }
  void complete ()
    {m_complete = true;}
  bool ensure_room (LONG low, LONG high)
    {
      if ((SetFilePointer (*this, low, &high, FILE_BEGIN) == DWORD (~0)
           && GetLastError () != NO_ERROR)
          || !SetEndOfFile (*this))
        return false;

      m_delete_if_fail = true;

      if (SetFilePointer (*this, 0, 0, FILE_BEGIN) == -1)
        return false;
      return true;
    }
  bool open ()
    {
      attach (CreateFileW (m_path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS,
                          FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN, 0));
      if (!is_valid ())
        return false;
      m_delete_if_fail = GetLastError () == NO_ERROR;
      return true;
    }
private:
  bool m_complete;
  bool m_delete_if_fail;
  const wchar_t *m_path;
};

int
UnRAR::check_timestamp (const wchar_t *path, const rarHeaderDataEx &hde)
{
  WIN32_FIND_DATAW fd;
  HANDLE h = FindFirstFileW (path, &fd);
  if (h != INVALID_HANDLE_VALUE)
    FindClose (h);

  switch (m_type)
    {
    case UT_ASK:
      if (h != INVALID_HANDLE_VALUE && !(m_opt & O_YES))
        {
          replace_param r;
          r.name = path;
          FILETIME ft;
          FileTimeToLocalFileTime (&fd.ftLastWriteTime, &ft);
          WORD d, t;
          FileTimeToDosDateTime (&ft, &d, &t);
          r.old_date = (DWORD (d) << 16) + t;
          r.old_size.s.l = fd.nFileSizeLow;
          r.old_size.s.h = fd.nFileSizeHigh;
          r.new_date = hde.FileTime;
          r.new_size.s.l = hde.UnpSize;
          r.new_size.s.h = hde.UnpSizeHigh;
          switch (replace_dialog (m_hwnd, r))
            {
            case IDYES:
              return 1;
            case IDNO:
              return 0;
            case IDC_ALL:
              m_opt |= O_YES;
              return 1;
            default:
              return -1;
            }
        }
      break;

    case UT_OVWRT:
      return 1;

    case UT_SKIP:
      return h == INVALID_HANDLE_VALUE;

    case UT_EXISTING:
      if (h == INVALID_HANDLE_VALUE)
        return 0;
      /* fall thru... */
    case UT_NEWER:
      if (h != INVALID_HANDLE_VALUE)
        {
          FILETIME ft;
          FileTimeToLocalFileTime (&fd.ftLastWriteTime, &ft);
          WORD d, t;
          FileTimeToDosDateTime (&ft, &d, &t);
          if ((DWORD (d) << 16) + t >= hde.FileTime)
            return 0;
        }
      break;
    }
  return 1;
}

struct extract_info
{
  const progress_dlg *progress;
  HWND hwnd_owner;
  HANDLE h;
  const rarHeaderDataEx *hde;
  const wchar_t *path;
  bool canceled;
  bool error;
  int64 nbytes;
  EXTRACTINGINFOEX *xex;
};

static extract_info *xtract_info;
static const UINT UWM_ARCEXTRACT = RegisterWindowMessage (WM_ARCEXTRACT);

static LONG_PTR
run_callback (int mode, EXTRACTINGINFOEX &ex)
{
  return (lstate.has_callback
          ? (lstate.callback
             ? !lstate.callback (lstate.hwnd_owner, UWM_ARCEXTRACT,
                                 mode, &ex)
             : SendMessage (lstate.hwnd_owner, UWM_ARCEXTRACT,
                            mode, LPARAM (&ex)))
          : 0);
}

static int __cdecl
extract_helper (void *, u_char *data, int nbytes)
{
  if (!xtract_info)
    return 1;

  DWORD nwritten;
  if (!WriteFile (xtract_info->h, data, nbytes, &nwritten, 0)
      || nwritten != DWORD (nbytes))
    {
      xtract_info->error = true;
      xtract_info = 0;
      return 0;
    }

  xtract_info->nbytes.d += nbytes;
  if (xtract_info->progress
      && !xtract_info->progress->update (xtract_info->nbytes))
    {
      xtract_info->canceled = true;
      xtract_info = 0;
      return 0;
    }
  if (lstate.has_callback)
    {
      xtract_info->xex->exinfo.dwWriteSize =
        !xtract_info->nbytes.s.h ? xtract_info->nbytes.s.l : -1;
      if (run_callback (ARCEXTRACT_INPROCESS, *xtract_info->xex))
        {
          xtract_info->canceled = true;
          xtract_info = 0;
          return 0;
        }
    }

  return 1;
}

static int __cdecl
change_volume (void *, wchar_t *path, int mode)
{
  if (mode != RAR_VOL_ASK)
    return 1;
  return change_vol_dialog (xtract_info ? xtract_info->hwnd_owner : 0, path);
}

int CALLBACK rar_event_handler(UINT msg,LPARAM UserData,LPARAM P1,LPARAM P2)
{
  switch(msg)
    {
    case UCM_CHANGEVOLUME:
      return change_volume(NULL,mb2wide((char*)P1).getstring(),(int)P2);
    case UCM_CHANGEVOLUMEW:
      return change_volume(NULL,(wchar_t*)P1,(int)P2);
    case UCM_PROCESSDATA:
      return extract_helper(NULL,(u_char*)P1,(int)P2);
    case UCM_NEEDPASSWORD:
      {
        const wchar_t* pwd=NULL;
        rarData* prd=(rarData*)UserData;
        if(!prd)return -1;
        if(prd->can_ask_password){
          if(prd->pUserData){
            pwd=((UnRAR*)prd->pUserData)->get_password();
          }else{
            pwd=askpass_dialog (0);
          }
        }
        if(pwd){
          wide2mb pwda(pwd);
          strncpy((char*)P1,pwda.getstring(),P2);
        }else{
          prd->is_missing_password=true;
          *((char*)P1)='\0';
          return -1;
        }
      }
      return 0;
    case UCM_NEEDPASSWORDW:
      {
        const wchar_t* pwd=NULL;
        rarData* prd=(rarData*)UserData;
        if(!prd)return -1;
        if(prd->can_ask_password){
          if(prd->pUserData){
            pwd=((UnRAR*)prd->pUserData)->get_password();
          }else{
            pwd=askpass_dialog (0);
          }
        }
        if(pwd){
          wcsncpy((wchar_t*)P1,pwd,P2);
          return 0;
        }else{
          prd->is_missing_password=true;
          *((wchar_t*)P1)=L'\0';
          return -1;
        }
      }
    default:
      return 0;
    }
}

int CALLBACK rar_openarc_handler(UINT msg,LPARAM UserData,LPARAM P1,LPARAM P2)
{
  switch(msg)
    {
    case UCM_CHANGEVOLUME:
      return change_volume(NULL,mb2wide((char*)P1).getstring(),(int)P2);
    case UCM_CHANGEVOLUMEW:
      return change_volume(NULL,(wchar_t*)P1,(int)P2);
    case UCM_PROCESSDATA:
      return extract_helper(NULL,(u_char*)P1,(int)P2);
    case UCM_NEEDPASSWORD:
      {
        const wchar_t* pwd=NULL;
        arcinfo* pInfo=(arcinfo*)UserData;
        if(!pInfo)return -1;
        if(!(pInfo->m_mode & M_ERROR_MESSAGE_OFF)){
          pwd=askpass_dialog (0);
        }
        if(pwd){
          wide2mb pwda(pwd);
          strncpy((char*)P1,pwda.getstring(),P2);
        }else{
          pInfo->m_is_missing_password=true;
          *((char*)P1)='\0';
          return -1;
        }
      }
      return 0;
    case UCM_NEEDPASSWORDW:
      {
        const wchar_t* pwd=NULL;
        arcinfo* pInfo=(arcinfo*)UserData;
        if(!pInfo)return -1;
        if(!(pInfo->m_mode & M_ERROR_MESSAGE_OFF)){
          pwd=askpass_dialog (0);
        }
        if(pwd){
          wcsncpy((wchar_t*)P1,pwd,P2);
        }else{
          pInfo->m_is_missing_password=true;
          *((wchar_t*)P1)=L'\0';
          return -1;
        }
      }
    default:
      return 0;
    }
}

static void
init_exinfo (EXTRACTINGINFOEX &ex, const rarHeaderDataEx &hde,
             const wchar_t *path)
{
  ex.exinfo.dwFileSize = !hde.UnpSizeHigh ? hde.UnpSize : -1;
  ex.exinfo.dwWriteSize = 0;
  strlcpy (ex.exinfo.szSourceFileName, wide2mb(hde.FileNameW).getstring(), sizeof ex.exinfo.szSourceFileName);
  strlcpy (ex.exinfo.szDestFileName, wide2mb(path).getstring(), sizeof ex.exinfo.szDestFileName);
  ex.dwCompressedSize = hde.PackSize;
  ex.dwCRC = hde.FileCRC;
  ex.uOSType = os_type (hde.HostOS);
  ex.wRatio = calc_ratio (hde.PackSizeHigh, hde.PackSize, hde.UnpSizeHigh, hde.UnpSize);
  ex.wDate = HIWORD (hde.FileTime);
  ex.wTime = LOWORD (hde.FileTime);
  strcpy (ex.szAttribute, attr_string (hde.FileAttr));
  strcpy (ex.szMode, method_string (hde.Method));
}

int
UnRAR::canceled () const
{
  format (IDS_CANCELED);
  return ERROR_USER_CANCEL;
}

int
UnRAR::skip (rarData &rd, const wchar_t *path) const
{
  format (IDS_SKIPPING, path);
  int e = rd.skip ();
  return e ? process_err (e, path,rd) : -1;
}

int
UnRAR::extract (rarData &rd, const wchar_t *path, const rarHeaderDataEx &hde,
                progress_dlg &progress)
{
  if (progress.m_hwnd)
    progress.init (path, hde.UnpSize, hde.UnpSizeHigh);

  int e = check_timestamp (path, hde);
  if (e < 0)
    return canceled ();
  if (!e)
    return skip (rd, path);

  write_handle w (path);
  if (!w.open ())
    {
      format (IDS_CANNOT_CREATE, path);
      return skip (rd, path);
    }
  if (!w.ensure_room (hde.UnpSize, hde.UnpSizeHigh))
    {
      format (IDS_DISK_FULL);
      return skip (rd, path);
    }

  extract_info xinfo;
  xinfo.progress = progress.m_hwnd ? &progress : 0;
  xinfo.hwnd_owner = m_hwnd;
  xinfo.h = w;
  xinfo.hde = &hde;
  xinfo.path = path;
  xinfo.canceled = false;
  xinfo.error = false;
  xinfo.nbytes.d = 0;
  xinfo.xex = &m_ex;
  xtract_info = &xinfo;
  format (IDS_EXTRACTING, path);

  if (lstate.has_callback)
    {
      init_exinfo (*xinfo.xex, hde, path);
      if (run_callback (ARCEXTRACT_BEGIN, *xinfo.xex))
        return canceled ();
    }

  e = rd.test ();
  xtract_info = 0;
  if (xinfo.canceled)
    return canceled ();
  if (e)
    return process_err (e, path,rd);
  if (xinfo.error)
    {
      format (IDS_WRITE_ERROR, path);
      return -1;
    }
  if (!SetEndOfFile (w))
    {
      format (IDS_CANNOT_SET_EOF);
      return -1;
    }

  w.complete ();

  FILETIME lo, ft;
  DosDateTimeToFileTime (hde.FileTime >> 16, hde.FileTime, &lo);
  LocalFileTimeToFileTime (&lo, &ft);
  SetFileTime (w, 0, 0, &ft);
  SetFileAttributesW (path, hde.FileAttr);

  return 0;
}

const wchar_t* UnRAR::get_password()
{
  if (m_passwd) return m_passwd;
  else{
      return askpass_dialog (m_hwnd);
    }
}

int
UnRAR::mkdirhier (const wchar_t *path)
{
  if (CreateDirectoryW (path, 0))
    {
      format (IDS_CREATING, path);
      return 1;
    }
  DWORD a = GetFileAttributesW (path);
  if (a != -1 && a & FILE_ATTRIBUTE_DIRECTORY)
    return 1;
  wchar_t buf[FNAME_MAX32 + FRAR_PATH_MAX + 1];
  wcscpy (buf, path);
  for (wchar_t *p = buf; p = find_slash (p); *p++ = L'\\')
    {
      *p = 0;
      if (CreateDirectoryW (buf, 0))
        format (IDS_CREATING, buf);
    }
  if (CreateDirectoryW (path, 0))
    {
      format (IDS_CREATING, path);
      return 1;
    }
  a = GetFileAttributesW (path);
  if (a != -1 && a & FILE_ATTRIBUTE_DIRECTORY)
    return 1;
  format (IDS_CANNOT_CREATE, path);
  return 0;
}

int
UnRAR::extract1 ()
{
  format (IDS_EXTRACTING_FROM, m_path);

  rarData rd;
  if (!rd.open (m_path, RAR_OM_EXTRACT))
    return open_err (rd.oade.OpenResult);

  rd.pUserData=this;
  if(m_opt & O_NOT_ASK_PASSWORD){
    rd.can_ask_password=false;
  }
  rarSetCallback(rd.h,rar_event_handler,(LPARAM)&rd);

  progress_dlg progress;
  if (!lstate.has_callback && !(m_opt & O_QUIET))
    progress.create (m_hwnd);

  wchar_t dest[FNAME_MAX32 + FRAR_PATH_MAX + 1];
  wchar_t *de = stpcpy (dest, const_cast<wchar_t*>(m_dest));
  slash2backsl (dest);

  int nerrors = 0;
  int e;
  for (;;)
    {
      e = rd.read_header ();
      if (e)
        {
          e = header_err (e,rd);
          return e ? e : nerrors;
        }

      if (!m_glob.match (rd.hde.FileNameW, (m_opt & O_STRICT) != 0, (m_opt & O_RECURSIVE) != 0))
        {
          e = rd.skip ();
          if (e)
            return process_err (e, rd.hde.FileNameW,rd);
        }
      else
        {
          const wchar_t *name = trim_root (rd.hde.FileNameW);
          if (m_cmd == C_EXTRACT)
            {
              wcscpy (de, name);
              if (m_security_level >= 2)
                sanitize_path (de);
            }
          else
            {
              wchar_t *sl = find_last_slash (name);
              wcscpy (de, sl ? sl + 1 : name);
            }
          if (!*de)
            {
              e = rd.skip ();
              if (e)
                return process_err (e, dest,rd);
            }
          //else if (rd.hd.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
          else if ((rd.hde.Flags & 0xE0) == 0xE0)  //Directory check modified:Not with rd.hd.FileAttr,but with rd.hd.Flags
            {
              if (m_cmd == C_EXTRACT && !mkdirhier (dest))
                return ERROR_DIRECTORY;
              e = rd.skip ();
              if (e)
                return process_err (e, dest,rd);
            }
          else
            {
              wchar_t *p = find_last_slash (dest);
              if (p)
                {
                  *p = 0;
                  if (!mkdirhier (dest))
                    return ERROR_DIRECTORY;
                  *p = '\\';
                }
              e = extract (rd, dest, rd.hde, progress);
              if (e)
                {
                  if (e > 0)
                    return e;
                  nerrors++;
                }
            }
        }
    }
}

int
UnRAR::extract ()
{
  if (lstate.has_callback)
    {
      memset (&m_ex, 0, sizeof m_ex);
      strlcpy (m_ex.exinfo.szSourceFileName, wide2mb(m_path).getstring(), sizeof m_ex.exinfo.szSourceFileName);
      if (run_callback (ARCEXTRACT_OPEN, m_ex))
        return canceled ();
    }

  int e = extract1 ();
  if (lstate.has_callback)
    run_callback (ARCEXTRACT_END, m_ex);

  return e;
}

int
UnRAR::test ()
{
  format (IDS_TEST_NOT_IMPL);
  return ERROR_NOT_SUPPORT;
}

int
UnRAR::list ()
{
  rarData rd;
  if (!rd.open (m_path, RAR_OM_LIST))
    return open_err (rd.oade.OpenResult);

  rd.pUserData=this;
  if(m_opt & O_NOT_ASK_PASSWORD){
    rd.can_ask_password=false;
  }
  rarSetCallback(rd.h,rar_event_handler,(LPARAM)&rd);

  format (L"  Name         Original   Packed  Ratio   Date     Time   Attr Method  CRC\n");
  format (L"-------------- -------- -------- ------ -------- -------- ---- ------- --------\n");
  int nfiles = 0;
  int64 org_sz, comp_sz;
  int e;

  org_sz.d = comp_sz.d = 0;
  for (;;)
    {
      e = rd.read_header ();
      if (e)
        {
          if (e == ERAR_END_ARCHIVE)
            break;
          return header_err (e,rd);
        }
      if (m_glob.match (rd.hde.FileNameW, (m_opt & O_STRICT) != 0, (m_opt & O_RECURSIVE) != 0))
        {
          nfiles++;
          if (m_cmd == C_VLIST)
            format (L"%s\n%15c", rd.hde.FileNameW, L' ');
          else
            {
              wchar_t *p = find_last_slash (rd.hde.FileNameW);
              format (L"%-14s ", p ? p + 1 : rd.hde.FileNameW);
            }
          int ratio = calc_ratio (rd.hde.PackSizeHigh, rd.hde.PackSize,
                                  rd.hde.UnpSizeHigh, rd.hde.UnpSize);
          int64 u, p;
          u.s.l = rd.hde.UnpSize;
          u.s.h = rd.hde.UnpSizeHigh;
          p.s.l = rd.hde.PackSize;
          p.s.h = rd.hde.PackSizeHigh;
          org_sz.d += u.d;
          comp_sz.d += p.d;
          format (L"%8I64d %8I64d%c%3d.%d%%%c%02d-%02d-%02d %02d:%02d:%02d %s %-7s %08x\n",
                  u, p,
                  rd.hde.Flags & FRAR_PREVVOL ? L'<' : L' ',
                  ratio / 10, ratio % 10,
                  rd.hde.Flags & FRAR_NEXTVOL ? L'>' : L' ',
                  ((rd.hde.FileTime >> 25) + 80) % 100,
                  (rd.hde.FileTime >> 21) & 15,
                  (rd.hde.FileTime >> 16) & 31,
                  (rd.hde.FileTime >> 11) & 31,
                  (rd.hde.FileTime >> 5) & 63,
                  (rd.hde.FileTime & 31) * 2,
                  mb2wide(attr_string (rd.hde.FileAttr)).getstring(),
                  mb2wide(method_string (rd.hde.Method)).getstring(),
                  rd.hde.FileCRC);
        }
      e = rd.skip ();
      if (e)
        return process_err (e, rd.hde.FileNameW,rd);
    }

  if (nfiles)
    {
      format (L"-------------- -------- -------- ------\n");
      wchar_t b[32];
      swprintf (b, 32, L"%d File%s", nfiles, nfiles == 1 ? L"" : L"s");
      int ratio = calc_ratio (comp_sz, org_sz);
      format (L"%12s   %8I64d %8I64d %3d.%d%%\n",
              b, org_sz, comp_sz, ratio / 10, ratio % 10);
    }
  return 0;
}

int
UnRAR::print ()
{
  format (IDS_PRINT_NOT_IMPL);
  return ERROR_NOT_SUPPORT;
}

int
UnRAR::comment ()
{
  format (IDS_COMMENT_NOT_IMPL);
  return ERROR_NOT_SUPPORT;
}

int
UnRAR::xmain (int ac, char **av)
{
  int e = parse_opt (ac, av);
  if (e)
    return e;

  switch (m_cmd)
    {
    case C_EXTRACT:
    case C_EXTRACT_NODIR:
      return extract ();

    case C_PRINT:
      return print ();

    case C_LIST:
    case C_VLIST:
      return list ();

    case C_TEST:
      return test ();

    case C_COMMENT:
      return comment ();
    }
  return 0;
}

