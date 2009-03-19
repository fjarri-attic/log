// log.cpp : Defines the entry point for the DLL application.
//

#include <Windows.h>
#include <tchar.h>
#include "log.h"
#include "misc.h"
#include "queue.h"

#include <stdio.h>

struct MessageHeader
{
	bool Unicode;
};

class File
{
	HANDLE FileHandle;
	void *FileName;
	bool FileNameIsUnicode;
	bool KeepClosed;
public:
	File(const void *file_name, bool name_is_unicode, bool keep_closed);
	~File();
	int Open();
	void Close();
	int Write(const void *buffer, size_t buffer_size);
};

File::File( const void *file_name, bool name_is_unicode, bool keep_closed )
{
	size_t buffer_size;

	FileHandle = INVALID_HANDLE_VALUE;
	FileName = NULL;
	FileNameIsUnicode = name_is_unicode;
	KeepClosed = keep_closed;

	if(name_is_unicode)
		buffer_size = (wcsnlen((const wchar_t *)file_name, MAX_PATH) + 1) * sizeof(wchar_t);
	else
		buffer_size = (strnlen((const char *)file_name, MAX_PATH) + 1) * sizeof(char);

	FileName = new char[buffer_size];
	memcpy(FileName, file_name, buffer_size);
}

File::~File()
{
	if(FileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);

	if(FileName)
		delete[] FileName;
}

int File::Open()
{
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		if(FileNameIsUnicode)
			FileHandle = CreateFileW((wchar_t *)FileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		else
			FileHandle = CreateFileA((char *)FileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if(FileHandle == INVALID_HANDLE_VALUE) 
			return GetLastError();
	}

	return 0;
}

void File::Close()
{
	if(KeepClosed && (FileHandle != INVALID_HANDLE_VALUE))
	{
		CloseHandle(FileHandle);
		FileHandle = INVALID_HANDLE_VALUE;
	}
}

int File::Write( const void *buffer, size_t buffer_size )
{
	DWORD res = Open();
	if(res)
		return res;

	DWORD size_written;
	if(!WriteFile(FileHandle, buffer, (DWORD)buffer_size, &size_written, NULL) || size_written != (DWORD)buffer_size)
	{
		Close();
		return GetLastError();
	}

	Close();
	return 0;
}


struct LogFile
{
	bool ReplaceLF;

	HANDLE StopEvent;
	HANDLE LoggerThread;

	Queue MessageQueue;
	File TargetFile;

	LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed) : 
		MessageQueue(buffer_size), TargetFile(file_name, name_is_unicode, keep_closed)
	{ 
		ReplaceLF = true;
		StopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~LogFile() 
	{ 
		if(StopEvent)
			CloseHandle(StopEvent);
	}

	DWORD Write(LPCVOID buf, size_t size)
	{
		return TargetFile.Write(buf, size);
	}

	void Push(const MessageHeader *header, const void *message, size_t message_size)
	{
		MessageQueue.Push(header, sizeof(*header), message, message_size);
	}

	void Pop(MessageHeader *header, Buffer &buffer)
	{
		MessageQueue.Pop(header, sizeof(*header), buffer);
	}

	void Stop()
	{
		SetEvent(StopEvent);
		if(WaitForSingleObject(LoggerThread, 10000) == WAIT_TIMEOUT)
		{
			TerminateThread(LoggerThread, (DWORD)-1);
		}
	}

	int Start()
	{
		DWORD dw;
		// Create logging thread
		LoggerThread = CreateThread(NULL, 0, LogThreadProc, (PVOID)this, 0, &dw);
		if(LoggerThread == INVALID_HANDLE_VALUE)
			return GetLastError();

		return 0;
	}
};


void SwapBuffers(Buffer *&buf1, Buffer *&buf2)
{
	Buffer *temp = buf1;
	buf1 = buf2;
	buf2 = temp;
}

//
DWORD WINAPI LogThreadProc(PVOID context)
{
	LogFile *lf = (LogFile *)context;
	DWORD res;

	Buffer buf1, buf2;

	DWORD counter = 0;
	MessageHeader header;

	while(WaitForSingleObject(lf->StopEvent, 0) != WAIT_OBJECT_0)
	{
		Buffer *primary_buf = &buf1;
		Buffer *secondary_buf = &buf2;

		lf->Pop(&header, *primary_buf);

		if(header.Unicode)
			UnicodeToMbStr(*primary_buf, *secondary_buf);
		SwapBuffers(primary_buf, secondary_buf);

		// Replace LF -> CRLF
		if(lf->ReplaceLF)
			ExpandLF(*primary_buf, *secondary_buf);
		SwapBuffers(primary_buf, secondary_buf);

		res = lf->Write(primary_buf->GetPtr(), primary_buf->GetDataSize());
		if(res)
			return res;

		counter++;

		Sleep(0);
	}

	_tprintf(_T("Logged: %d\n"), counter);

	return 0;
}


class LogFilesPool
{
	Buffer LogFiles;
public:
	LogFilesPool() {}
	~LogFilesPool()
	{
		for(size_t i = 0; i < LogFiles.GetDataSize() / sizeof(LogFile *); i++)
		{
			LogFile *lf = Get(i);
			if(lf != NULL)
			{
				lf->Stop();
				delete lf;
			}
		}
	}

	int Start(size_t index, const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed)
	{
		if(index >= LogFiles.GetDataSize() / sizeof(LogFile *))
			LogFiles.Resize((index + 1) * sizeof(LogFile *), true);
		else
			Stop(index);
		
		LogFile **lf = (LogFile **)LogFiles.GetPtr() + index;
		LogFile *new_lf = new LogFile(file_name, name_is_unicode, buffer_size, keep_closed);
		*lf = new_lf;

		return new_lf->Start();
	}

	void Stop(size_t index)
	{
		if(index > LogFiles.GetDataSize() / sizeof(LogFile *))
			return;

		LogFile *lf = (LogFile *)LogFiles.GetPtr() + index;

		if(lf)
		{
			lf->Stop();
			delete lf;
			lf = NULL;
		}
	}

	LogFile *Get(size_t index)
	{
		if(index >= LogFiles.GetDataSize() / sizeof(LogFile *))
			return NULL;

		return *((LogFile **)LogFiles.GetPtr() + index);
	}
};

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

