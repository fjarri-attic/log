// log.cpp : Defines the entry point for the DLL application.
//

#include <Windows.h>
#include <tchar.h>
#include "log.h"
#include "misc.h"

struct MessageParameters
{
	size_t BufferSize;
	bool Unicode;
};

struct LogFile
{
	HANDLE WriteEnd;
	HANDLE ReadEnd;
	TCHAR FileName[MAX_PATH];
	HANDLE File;

	BOOLEAN KeepClosed;
	BOOLEAN ReplaceLF;

	CRITICAL_SECTION cs;

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
	}

	~LogFile() 
	{ 
		DeleteCriticalSection(&cs); 
		CloseHandle(ReadEnd);
		CloseHandle(WriteEnd);
		CloseHandle(File);
	}

	VOID Push(const MessageParameters *params, const void *message)
	{
		DWORD dw;
		EnterCriticalSection(&cs);
		WriteFile(WriteEnd, (PVOID)params, sizeof(*params), &dw, NULL);
		WriteFile(WriteEnd, message, (DWORD)params->BufferSize, &dw, NULL);
		LeaveCriticalSection(&cs);
	}

	DWORD Pop(Buffer &dst)
	{
		MessageParameters params;
		DWORD size_read;

		if(!ReadFile(ReadEnd, &params, sizeof(params), &size_read, NULL) || size_read != sizeof(params))
			return GetLastError();

		dst.Resize(params.BufferSize);

		if(!ReadFile(ReadEnd, dst.GetPtr(), (DWORD)dst.GetDataSize(), &size_read, NULL) || size_read != dst.GetDataSize())
			return GetLastError();

		return 0;
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
		res = log_file->Pop(raw_buf);
		if(res)
			return res;

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
	HANDLE hRead, hWrite;

	// Create logfile
	_tcscpy_s(log_file.FileName, MAX_PATH, file_name);
	dw = log_file.Open();
	if(dw)
	{
		log_file.Close();
		return dw;
	}

	// Create pipe
	if(!CreatePipe(&hRead, &hWrite, NULL, 10000000))
		return GetLastError();

	log_file.WriteEnd = hWrite;
	log_file.ReadEnd = hRead;

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

