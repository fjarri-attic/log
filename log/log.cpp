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
	File(const void *file_name, bool name_is_unicode, bool keep_closed)
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

	~File()
	{
		if(FileHandle != INVALID_HANDLE_VALUE)
			CloseHandle(FileHandle);

		if(FileName)
			delete[] FileName;
	}

	DWORD Open()
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

	void Close()
	{
		if(KeepClosed && (FileHandle != INVALID_HANDLE_VALUE))
		{
			CloseHandle(FileHandle);
			FileHandle = INVALID_HANDLE_VALUE;
		}
	}

	DWORD Write(const void *buffer, size_t buffer_size)
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
};

struct LogFile
{
	BOOLEAN KeepClosed;
	BOOLEAN ReplaceLF;

	HANDLE StopEvent;

	Queue MessageQueue;
	File TargetFile;

	LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed) : 
		MessageQueue(buffer_size), TargetFile(file_name, name_is_unicode, keep_closed)
	{ 
		ReplaceLF = TRUE;
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
};

LogFile *log_file;

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
DWORD WINAPI LogThreadProc(PVOID context)
{
	LogFile *lf = (LogFile *)context;
	DWORD res;

	Buffer raw_buf, expanded_buf;
	Buffer &buf = (log_file->ReplaceLF ? expanded_buf : raw_buf);

	DWORD counter = 0;
	MessageHeader header;

	while(WaitForSingleObject(log_file->StopEvent, 0) != WAIT_OBJECT_0)
	{
		lf->Pop(&header, raw_buf);

		// Replace LF -> CRLF
		if(log_file->ReplaceLF)
			ExpandLF(raw_buf, expanded_buf);

		res = lf->Write(buf.GetPtr(), buf.GetDataSize());
		if(res)
			return res;

		counter++;

		Sleep(0);
	}

	_tprintf(_T("Logged: %d\n"), counter);

	return 0;
}


//
DWORD LogInitInternal(const void *file_name, bool unicode)
{
	DWORD dw;

	// Create logfile
	log_file = new LogFile(file_name, unicode, 1024 * 1024, true);

	// Create logging thread
	HANDLE logger_thread = CreateThread(NULL, 0, LogThreadProc, (PVOID)log_file, 0, &dw);
	if(logger_thread == INVALID_HANDLE_VALUE)
		return GetLastError();

	return 0;
}

LOG_API DWORD LogInitA(const char *file_name)
{
	return LogInitInternal(file_name, false);
}

LOG_API DWORD LogInitW(const wchar_t *file_name)
{
	return LogInitInternal(file_name, true);
}

LOG_API VOID LogStop()
{
	SetEvent(log_file->StopEvent);
}

//
LOG_API VOID LogWriteA(const char *message)
{
	MessageHeader header;
	header.Unicode = false;
	log_file->Push(&header, (const void *)message, strlen(message) * sizeof(char));
}


//
LOG_API VOID LogWriteW(const wchar_t *message)
{
	MessageHeader header;
	header.Unicode = true;
	log_file->Push(&header, (const void *)message, wcslen(message) * sizeof(wchar_t));
}

