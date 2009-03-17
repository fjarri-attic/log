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

struct LogFile
{
	TCHAR FileName[MAX_PATH];
	HANDLE File;

	BOOLEAN KeepClosed;
	BOOLEAN ReplaceLF;

	HANDLE StopEvent;

	Queue *MessageQueue;

	DWORD Open()
	{
		if(File == INVALID_HANDLE_VALUE)
		{
			File = CreateFile(FileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(File == INVALID_HANDLE_VALUE) 
				return GetLastError();
		}

		return 0;
	}

	VOID Close()
	{
		if(KeepClosed && (File != INVALID_HANDLE_VALUE))
		{
			CloseHandle(File);
			File = INVALID_HANDLE_VALUE;
		}
	}

	LogFile() { 
		KeepClosed = TRUE;
		ReplaceLF = TRUE;
		File = INVALID_HANDLE_VALUE;

		StopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		MessageQueue = new RingBuffer(1024);
	}

	~LogFile() 
	{ 
		if(File != INVALID_HANDLE_VALUE)
			CloseHandle(File);
		if(StopEvent)
			CloseHandle(StopEvent);
		if(MessageQueue)
			delete MessageQueue;
	}

	DWORD Write(LPCVOID buf, size_t size)
	{
		DWORD res = Open();
		if(res)
			return res;

		DWORD size_written;
		if(!WriteFile(File, buf, (DWORD)size, &size_written, NULL) || size_written != size)
		{
			Close();
			return GetLastError();
		}

		Close();
		return 0;
	}

	void Push(const MessageHeader *header, const void *message, size_t message_size)
	{
		MessageQueue->Push(header, sizeof(*header), message, message_size);
	}

	void Pop(MessageHeader *header, Buffer &buffer)
	{
		MessageQueue->Pop(header, sizeof(*header), buffer);
	}
};



LogFile log_file;

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
	LogFile *log_file = (LogFile *)context;
	DWORD res;

	Buffer raw_buf, expanded_buf;
	Buffer &buf = (log_file->ReplaceLF ? expanded_buf : raw_buf);

	DWORD counter = 0;
	MessageHeader header;

	while(WaitForSingleObject(log_file->StopEvent, 0) != WAIT_OBJECT_0)
	{
		log_file->Pop(&header, raw_buf);

		// Replace LF -> CRLF
		if(log_file->ReplaceLF)
			ExpandLF(raw_buf, expanded_buf);

		res = log_file->Write(buf.GetPtr(), buf.GetDataSize());
		if(res)
			return res;

		counter++;

		Sleep(0);
	}

	_tprintf(_T("Logged: %d\n"), counter);

	return 0;
}


//
LOG_API DWORD LogInit(const TCHAR *file_name)
{
	DWORD dw;

	// Create logfile
	_tcscpy_s(log_file.FileName, MAX_PATH, file_name);
	dw = log_file.Open();
	if(dw)
	{
		log_file.Close();
		return dw;
	}

	// Create logging thread
	HANDLE logger_thread = CreateThread(NULL, 0, LogThreadProc, (PVOID)&log_file, 0, &dw);
	if(logger_thread == INVALID_HANDLE_VALUE)
		return GetLastError();

	return 0;
}


LOG_API VOID LogStop()
{
	SetEvent(log_file.StopEvent);
}

//
LOG_API VOID LogWrite(const char *message)
{
	MessageHeader header;
	header.Unicode = false;
	log_file.Push(&header, (const void *)message, strlen(message) * sizeof(char));
}


//
LOG_API VOID LogWrite(const wchar_t *message)
{
	MessageHeader header;
	header.Unicode = true;
	log_file.Push(&header, (const void *)message, wcslen(message) * sizeof(wchar_t));
}

