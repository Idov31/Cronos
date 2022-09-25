#ifndef UTILS_H
#define UTILS_H

#include "Cronos.h"
#include <psapi.h>

#define SIZE_MODULE_LIST 2
#define MAX_MODULE_NAME 100

BOOL        bCompare (const BYTE *pData, const BYTE *bMask, const char *szMask);
DWORD_PTR   findPattern (DWORD_PTR dwAddress, DWORD dwLen, PBYTE bMask, PCHAR szMask);
DWORD_PTR   findInModule (LPCSTR moduleName, PBYTE bMask, PCHAR szMask);
PVOID       findGadget (PBYTE hdrParserFuncB, PCHAR hdrParserFunctMask);

#endif