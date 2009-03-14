// log.cpp : Defines the entry point for the DLL application.
//

#include <Windows.h>
#include <tchar.h>
#include "log.h"
#include "misc.h"

struct MessageParameters
{
	MessageParameters *NextMessage;
	size_t BufferSize;
	bool Unicode;
	bool Redirector;

	MessageParameters() { NextMessage = NULL; Redirector = false; }
};

struct LogFile
{
	TCHAR FileName[MAX_PATH];
	HANDLE File;

	BOOLEAN KeepClosed;
	BOOLEAN ReplaceLF;

	CRITICAL_SECTION cs;

	char *Buf;
	size_t BufSize;

	char *PushCursor;
	char *PopCursor;

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
		InitializeCriticalSection(&cs); 
		KeepClosed = TRUE;
		ReplaceLF = TRUE;
		File = INVALID_HANDLE_VALUE;

		BufSize = 1024 * 1024;
		Buf = new char[BufSize];
		ZeroMemory(Buf, BufSize);

		PushCursor = Buf;
		PopCursor = Buf;
		((MessageParameters *)PushCursor)->NextMessage = NULL;
	}

	~LogFile() 
	{ 
		DeleteCriticalSection(&cs); 
		if(File != INVALID_HANDLE_VALUE)
			CloseHandle(File);
	}

	VOID Push(const MessageParameters *params, const void *message)
	{
		char *cur;

		size_t message_size = sizeof(*params) + params->BufferSize;

		EnterCriticalSection(&cs);
		if(BufSize - (PushCursor - Buf) < message_size + sizeof(*params))
		{
			((MessageParameters *)PushCursor)->NextMessage = (MessageParameters *)Buf;
			((MessageParameters *)PushCursor)->Redirector = true;
			PushCursor = (char *)Buf;
		}

		cur = PushCursor;
		PushCursor += message_size;
		LeaveCriticalSection(&cs);

		memcpy(cur, params, sizeof(*params));
		memcpy(cur + sizeof(*params), message, params->BufferSize);
		
		EnterCriticalSection(&cs);
		((MessageParameters *)cur)->NextMessage = (MessageParameters *)(cur + message_size);
		LeaveCriticalSection(&cs);
	}

	VOID Pop(Buffer &dst)
	{
		MessageParameters params;

		char *cur = NULL;

		while(!cur)
		{
			EnterCriticalSection(&cs);
			if(((MessageParameters *)PopCursor)->NextMessage)
				cur = PopCursor;
			LeaveCriticalSection(&cs);

			if(cur)
			{
				if(((MessageParameters *)cur)->Redirector)
					cur = NULL;
			}
			else 
			{
				Sleep(0);
			}
		}

		memcpy(&params, cur, sizeof(params));
		dst.Resize(params.BufferSize);
		memcpy(dst.GetPtr(), cur + sizeof(params), dst.GetDataSize());

		PopCursor = (char *)params.NextMessage;
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

	while(1)
	{
		log_file->Pop(raw_buf);

		// Replace LF -> CRLF
		if(log_file->ReplaceLF)
			ExpandLF(raw_buf, expanded_buf);

		res = log_file->Write(buf.GetPtr(), buf.GetDataSize());
		if(res)
			return res;

		Sleep(0);
	}

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


//
LOG_API VOID LogWrite(const char *message)
{
	MessageParameters params;

	params.BufferSize = strlen(message) * sizeof(char);
	params.Unicode = false;
	log_file.Push(&params, (const void *)message);
}


//
LOG_API VOID LogWrite(const wchar_t *message)
{
	MessageParameters params;

	params.BufferSize = wcslen(message) * sizeof(wchar_t);
	params.Unicode = true;
	log_file.Push(&params, (const void *)message);
}

