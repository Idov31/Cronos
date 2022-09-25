#include "Utils.h"

BOOL bCompare(const BYTE *pData, const BYTE *bMask, const char *szMask)
{
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
            return FALSE;

    return TRUE;
}

DWORD_PTR findPattern(DWORD_PTR dwAddress, DWORD dwLen, PBYTE bMask, PCHAR szMask)
{
    for (DWORD i = 0; i < dwLen; i++)
        if (bCompare((PBYTE)(dwAddress + i), bMask, szMask))
            return (DWORD_PTR)(dwAddress + i);

    return 0;
}

DWORD_PTR findInModule(LPCSTR moduleName, PBYTE bMask, PCHAR szMask)
{
    PIMAGE_DOS_HEADER ImageBase = (PIMAGE_DOS_HEADER)GetModuleHandleA(moduleName);
    PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)ImageBase + ImageBase->e_lfanew);
    DWORD_PTR section_offset = (DWORD_PTR)ImageBase + ImageBase->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    PIMAGE_SECTION_HEADER text_section = (PIMAGE_SECTION_HEADER)(section_offset);
    DWORD_PTR dwAddress = findPattern((DWORD_PTR)ImageBase + text_section->VirtualAddress, text_section->SizeOfRawData, bMask, szMask);
    return dwAddress;
}

PVOID findGadget(PBYTE hdrParserFuncB, PCHAR hdrParserFunctMask)
{
    HANDLE hProcess;
    BOOL result;
    HMODULE *moduleList;
    DWORD bytesNeeded;
    DWORD nModules = 0;
    LPSTR moduleName = NULL;
    DWORD_PTR ptr = 0;
    // PBYTE hdrParserFuncB = (PBYTE)"\x48\x89\x22\xc3";

    hProcess = GetCurrentProcess();

    moduleList = malloc(SIZE_MODULE_LIST * sizeof(HMODULE));
    result = EnumProcessModules(hProcess, moduleList, SIZE_MODULE_LIST * sizeof(HMODULE), &bytesNeeded);
    if (bytesNeeded > SIZE_MODULE_LIST * sizeof(HMODULE))
    {
        moduleList = realloc(moduleList, bytesNeeded);
        result = EnumProcessModules(hProcess, moduleList, bytesNeeded, &bytesNeeded);
    }

    if (!result)
        goto end;
    for (int iModule = 1; iModule < (bytesNeeded / sizeof(HMODULE)); iModule++)
    {
        moduleName = malloc(MAX_MODULE_NAME * sizeof(CHAR));
        if (GetModuleFileNameExA(hProcess, moduleList[iModule], moduleName, MAX_MODULE_NAME * sizeof(CHAR)) == 0)
            goto end;
        ptr = findInModule(moduleName, hdrParserFuncB, hdrParserFunctMask);
        if (ptr)
        {   
            break;
        }
    }
end:
    if (moduleList)
        free(moduleList);

    if (moduleName)
        free(moduleName);
    if (hProcess)
        CloseHandle(hProcess);
    return (PVOID)ptr;
}
