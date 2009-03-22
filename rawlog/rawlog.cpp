// log.cpp : Defines the entry point for the DLL application.
//

#include <Windows.h>
#include <tchar.h>
#include "rawlog.h"
#include "logfile.h"

LogFilesPool Pool;

//
int RawLogStartInternal(size_t index, const void *file_name, bool unicode)
{
	return Pool.Start(index, file_name, unicode, 1024 * 1024, true);
}

int RawLogStartA(size_t index, const char *file_name)
{
	return RawLogStartInternal(index, file_name, false);
}

int RawLogStartW(size_t index, const wchar_t *file_name)
{
	return RawLogStartInternal(index, file_name, true);
}

VOID RawLogStop(size_t index)
{
	Pool.Get(index)->Stop();	

}

VOID RawLogWriteInternal(size_t index, const void *message, size_t size, bool unicode)
{
	MessageHeader header;
	header.Unicode = unicode;

	LogFile *lf = Pool.Get(index);
	if(lf)
		Pool.Get(index)->Push(&header, message, size);
}

//
VOID RawLogWriteA(size_t index, const char *message)
{
	RawLogWriteInternal(index, (const void *)message, (strlen(message) + 1) * sizeof(char), false);
}


//
VOID RawLogWriteW(size_t index, const wchar_t *message)
{
	RawLogWriteInternal(index, (const void *)message, (wcslen(message) + 1) * sizeof(wchar_t), true);
}

