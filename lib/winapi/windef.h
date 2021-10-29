#ifndef __WINDEF_H__
#define __WINDEF_H__

#include <xboxkrnl/xboxdef.h>

#define far
#define FAR far

#define WINAPI __stdcall
#define CALLBACK WINAPI
#define PASCAL WINAPI

#define MAX_PATH 260

typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HINSTANCE HMODULE;

typedef int (FAR WINAPI *FARPROC)(void);

#define MAKELONG(wLow, wHigh) ((((DWORD)((wHigh) & 0xffff)) << 16) | ((wLow) & 0xffff))
#define HIWORD(dwValue) ((WORD)((wValue) >> 16) & 0xffff)
#define LOWORD(dwValue) ((WORD)(wValue) & 0xffff)
#define MAKEWORD(bLow, bHigh) ((((WORD)((bHigh) & 0xff)) << 8) | ((bLow) & 0xff))
#define HIBYTE(wValue) ((BYTE)((wValue) >> 8) & 0xff)
#define LOBYTE(wValue) ((BYTE)(wValue) & 0xff)

#endif
