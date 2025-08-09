/* SignToolExDll.dll
*  =================
*  Hooks the time related functions to automagically permit
*  using expired certificates with SignTool.exe. This is the
*  detours library function code for changing expiry time to
*  make a certificate appear valid and then spoofing the system
*  time check to sign a file using an expired certificate.
* 
*  -- HF @ https://hacker.house
*/
#include <windows.h>
#include <detours.h>

// Original function pointers
static BOOL(WINAPI* TrueFileTimeToSystemTime)(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime) = FileTimeToSystemTime;
static LONG(WINAPI* TrueCompareFileTime)(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) = CompareFileTime;

// Detour functions
BOOL WINAPI DetourFileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime) {
    BOOL result = TrueFileTimeToSystemTime(lpFileTime, lpSystemTime);
    if (result) {
        SYSTEMTIME currentSystemTime;
        GetSystemTime(&currentSystemTime);
        FILETIME currentFileTime;
        if (SystemTimeToFileTime(&currentSystemTime, &currentFileTime)) {
            ULARGE_INTEGER uli;
            uli.LowPart = currentFileTime.dwLowDateTime;
            uli.HighPart = currentFileTime.dwHighDateTime;
            const ULONGLONG oneDayIntervals = 864000000000ULL;
            uli.QuadPart += oneDayIntervals;
            FILETIME newFileTime;
            newFileTime.dwLowDateTime = uli.LowPart;
            newFileTime.dwHighDateTime = uli.HighPart;
            TrueFileTimeToSystemTime(&newFileTime, lpSystemTime);
        }
    }
    return result;
}

LONG WINAPI DetourCompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) {
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            DetourRestoreAfterWith();
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)TrueFileTimeToSystemTime, DetourFileTimeToSystemTime);
            DetourAttach(&(PVOID&)TrueCompareFileTime, DetourCompareFileTime);
            DetourTransactionCommit();
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID&)TrueFileTimeToSystemTime, DetourFileTimeToSystemTime);
            DetourDetach(&(PVOID&)TrueCompareFileTime, DetourCompareFileTime);
            DetourTransactionCommit();
            break;
        }
    }
    return TRUE;
}