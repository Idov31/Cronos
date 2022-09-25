#ifndef CRONUS_H
#define CRONUS_H

#include <stdio.h>
#include <windows.h>

// Macros
#define TIMER_SLEEP 2000
#define InitializeTimerMs(ft, sec)                                                  \
    {                                                                                            \
        (ft)->HighPart = (DWORD)(((ULONGLONG) - ((sec) * 1000 * 10 * 1000)) >> 32);                               \
        (ft)->LowPart  = (DWORD)(((ULONGLONG) - ((sec) * 1000 * 10 * 1000)) & 0xffffffff);                          \
    }

// Struct definitions.    
typedef struct _CRYPT_BUFFER {
    DWORD Length;
    DWORD MaximumLength;
    PVOID Buffer;
} CRYPT_BUFFER, *PCRYPT_BUFFER, DATA_KEY, *PDATA_KEY, CLEAR_DATA, * PCLEAR_DATA, CYPHER_DATA, * PCYPHER_DATA;


// Functions.
typedef NTSTATUS(WINAPI* tSystemFunction032)(PCRYPT_BUFFER pData, PDATA_KEY pKey);

extern void QuadSleep(PVOID, PVOID, PVOID, PVOID);
void CronosSleep(int ticks);

#endif