// log.cpp : Defines the entry point for the DLL application.
//

#include <Windows.h>
#include <tchar.h>
#include "log.h"
#include "misc.h"
#include "queue.h"
#include "file.h"
#include "logfile.h"

#include <stdio.h>

LogFilesPool Pool;

//
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

//
int LogInitInternal(size_t index, const void *file_name, bool unicode)
{
	return Pool.Start(index, file_name, unicode, 1024 * 1024, true);
}

LOG_API int LogInitA(size_t index, const char *file_name)
{
	return LogInitInternal(index, file_name, false);
}

LOG_API int LogInitW(size_t index, const wchar_t *file_name)
{
	return LogInitInternal(index, file_name, true);
}

LOG_API VOID LogStop(size_t index)
{
	Pool.Get(index)->Stop();	

}

VOID LogWriteInternal(size_t index, const void *message, size_t size, bool unicode)
{
	MessageHeader header;
	header.Unicode = unicode;

	LogFile *lf = Pool.Get(index);
	if(lf)
		Pool.Get(index)->Push(&header, message, size);
}

//
LOG_API VOID LogWriteA(size_t index, const char *message)
{
	LogWriteInternal(index, (const void *)message, strlen(message) * sizeof(char), false);
}


//
LOG_API VOID LogWriteW(size_t index, const wchar_t *message)
{
	LogWriteInternal(index, (const void *)message, wcslen(message) * sizeof(wchar_t), true);
}

