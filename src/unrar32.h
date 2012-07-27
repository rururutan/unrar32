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

#ifndef _UNRAR32_H_
#define _UNRAR32_H_

#include "comm-arc.h"

#ifdef __cplusplus
extern "C" {
#endif

WORD WINAPI UnrarGetVersion ();
BOOL WINAPI UnrarGetRunning ();
BOOL WINAPI UnrarGetBackGroundMode ();
BOOL WINAPI UnrarSetBackGroundMode (BOOL mode);
BOOL WINAPI UnrarGetCursorMode ();
BOOL WINAPI UnrarSetCursorMode (BOOL cursor_mode);
WORD WINAPI UnrarGetCursorInterval ();
BOOL WINAPI UnrarSetCursorInterval (WORD interval);
int WINAPI Unrar (HWND hwnd, LPCSTR args, LPSTR buf, DWORD size);
BOOL WINAPI UnrarCheckArchive (const char *path, int mode);
int WINAPI UnrarGetFileCount (const char *path);
BOOL WINAPI UnrarQueryFunctionList (int i);
BOOL WINAPI UnrarConfigDialog (HWND hwnd, LPSTR szOptionBuffer, int iMode);
int WINAPI UnrarExtractMem (HWND hwnd, LPCSTR szCmdLine,
                            LPBYTE szBuffer, DWORD dwSize, time_t *lpTime,
                            LPWORD lpwAttr, LPDWORD lpdwWriteSize);
int WINAPI UnrarCompressMem (HWND hwnd, LPCSTR szCmdLine,
                             const BYTE *szBuffer, DWORD dwSize,
                             const time_t *lpTime, const WORD *lpwAttr,
                             LPDWORD lpdwWriteSize);
HARC WINAPI UnrarOpenArchive (HWND hwnd, LPCSTR path, DWORD mode);
int WINAPI UnrarCloseArchive (HARC harc);
int WINAPI UnrarFindFirst (HARC harc, LPCSTR pattern, INDIVIDUALINFO *vinfo);
int WINAPI UnrarFindNext (HARC harc, INDIVIDUALINFO *vinfo);
int WINAPI UnrarGetArcFileName (HARC harc, LPSTR buf, int size);
DWORD WINAPI UnrarGetArcFileSize (HARC harc);
BOOL WINAPI UnrarGetArcFileSizeEx (HARC harc, ULHA_INT64 *_lpllSize);
DWORD WINAPI UnrarGetArcOriginalSize (HARC harc);
BOOL WINAPI UnrarGetArcOriginalSizeEx (HARC harc, ULHA_INT64 *_lpllSize);
DWORD WINAPI UnrarGetArcCompressedSize (HARC harc);
BOOL WINAPI UnrarGetArcCompressedSizeEx (HARC harc, ULHA_INT64 *_lpllSize);
WORD WINAPI UnrarGetArcRatio (HARC harc);
WORD WINAPI UnrarGetArcDate (HARC harc);
WORD WINAPI UnrarGetArcTime (HARC harc);
UINT WINAPI UnrarGetArcOSType (HARC harc);
int WINAPI UnrarIsSFXFile (HARC harc);
int WINAPI UnrarGetFileName (HARC harc, LPSTR buf, int size);
int WINAPI UnrarGetMethod (HARC harc, LPSTR buf, int size);
DWORD WINAPI UnrarGetOriginalSize (HARC harc);
BOOL WINAPI UnrarGetOriginalSizeEx (HARC harc, ULHA_INT64 *_lpllSize);
DWORD WINAPI UnrarGetCompressedSize (HARC harc);
BOOL WINAPI UnrarGetCompressedSizeEx (HARC harc, ULHA_INT64 *_lpllSize);
WORD WINAPI UnrarGetRatio (HARC harc);
WORD WINAPI UnrarGetDate (HARC harc);
WORD WINAPI UnrarGetTime (HARC harc);
DWORD WINAPI UnrarGetWriteTime (HARC harc);
DWORD WINAPI UnrarGetCreateTime (HARC harc);
DWORD WINAPI UnrarGetAccessTime (HARC harc);
DWORD WINAPI UnrarGetCRC (HARC harc);
int WINAPI UnrarGetAttribute (HARC harc);
UINT WINAPI UnrarGetOSType (HARC harc);
BOOL WINAPI UnrarSetOwnerWindow (HWND hwnd);
BOOL WINAPI UnrarClearOwnerWindow ();
BOOL WINAPI UnrarSetOwnerWindowEx (HWND hwnd, LPARCHIVERPROC proc);
BOOL WINAPI UnrarKillOwnerWindowEx (HWND hwnd);

#ifdef __cplusplus
}
#endif

#endif /* _UNRAR32_H_ */
