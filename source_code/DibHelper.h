//------------------------------------------------------------------------------
// File: DibHelper.H
//
// Desc: DirectShow sample code - Helper code for bitmap manipulation
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <vfw.h> // needed, I guess for all the aviXX type calls
#include <assert.h>

#define HDIB HANDLE

void LocalOutput(const char *str, ...);
void LocalOutput(const wchar_t *str, ...);

#define NOT_SET_IN_REGISTRY 0 // match ruby file

/* DIB macros */
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)   ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)  ((lpRect)->bottom - (lpRect)->top)

// Function prototypes
HDIB BitmapToDIB (HBITMAP hBitmap, HPALETTE hPal);
HDIB ChangeBitmapFormat (HBITMAP	hBitmap,
                         WORD     wBitCount,
                         DWORD    dwCompression,
                         HPALETTE hPal);
HDIB ChangeDIBFormat (HDIB hDIB, WORD wBitCount, DWORD dwCompression);

HDIB CopyScreenToDIB (LPRECT);
HBITMAP CopyWindowToBitmap (HWND, WORD);
HDIB CopyWindowToDIB (HWND, WORD);

HPALETTE CreateDIBPalette (HDIB);
HDIB CreateDIB(DWORD, DWORD, WORD);
WORD DestroyDIB (HDIB);

void DIBError (int ErrNo);
DWORD DIBHeight (LPSTR lpDIB);
WORD DIBNumColors (LPSTR lpDIB);
HBITMAP DIBToBitmap (HDIB hDIB, HPALETTE hPal);
DWORD DIBWidth (LPSTR lpDIB);

LPSTR FindDIBBits (LPSTR lpDIB);
HPALETTE GetSystemPalette (void);
HDIB LoadDIB (LPSTR);

BOOL PaintBitmap (HDC, LPRECT, HBITMAP, LPRECT, HPALETTE);
BOOL PaintDIB (HDC, LPRECT, HDIB, LPRECT, HPALETTE);

int PalEntriesOnDevice (HDC hDC);
WORD PaletteSize (LPSTR lpDIB);
WORD SaveDIB (HDIB, LPSTR);

int read_config_setting(LPCTSTR szValueName, int default);
boolean is_config_set_to_1(LPCTSTR szValueName);
HRESULT set_config_string_setting(LPCTSTR szValueName, wchar_t *szToThis );

void WarmupCounter();
__int64 StartCounter();
long double GetCounterSinceStartMillis(__int64 start);


typedef struct {
  PAVISTREAM handle;
  AVISTREAMINFO info;
  DWORD read;
  LONG chunck_size;
  LONG chunck_samples;
} AVISynthStream;

extern BITMAPINFO savedVideoFormat;
extern AVISynthStream *stream;
int avisynth_read_header();
void avisynth_read_close();
int avisynth_read_packet(BYTE *pData, LONG pDataSize);
wchar_t *read_config_filepath();