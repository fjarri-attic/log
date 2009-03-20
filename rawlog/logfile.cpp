#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "misc.h"
#include "logfile.h"

void SwapBuffers(Buffer *&buf1, Buffer *&buf2)
{
	Buffer *temp = buf1;
	buf1 = buf2;
	buf2 = temp;
}

//
DWORD WINAPI LogFile::LogThreadProc(PVOID context)
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
		{
			UnicodeToMbStr(*primary_buf, *secondary_buf);
			SwapBuffers(primary_buf, secondary_buf);
		}

		// Replace LF -> CRLF
		if(lf->ReplaceLF)
		{
			ExpandLF(*primary_buf, *secondary_buf);
			SwapBuffers(primary_buf, secondary_buf);
		}

		res = lf->Write(primary_buf->GetPtr(), primary_buf->GetDataSize());
		if(res)
			return res;

		counter++;

		Sleep(0);
	}

	_tprintf(_T("Logged: %d\n"), counter);

	return 0;
}

LogFile::LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed) : 
	MessageQueue(buffer_size), TargetFile(file_name, name_is_unicode, keep_closed)
{
	ReplaceLF = true;
	StopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

LogFile::~LogFile()
{
	if(StopEvent)
		CloseHandle(StopEvent);
}

int LogFile::Write(const void *buf, size_t size)
{
	return TargetFile.Write(buf, size);
}

void LogFile::Push(const MessageHeader *header, const void *message, size_t message_size)
{
	MessageQueue.Push(header, sizeof(*header), message, message_size);
}

void LogFile::Pop(MessageHeader *header, Buffer &buffer)
{
	MessageQueue.Pop(header, sizeof(*header), buffer);
}

void LogFile::Stop()
{
	SetEvent(StopEvent);
	if(WaitForSingleObject(LoggerThread, 10000) == WAIT_TIMEOUT)
	{
		_ftprintf(stderr, _T("Logging thread timeouted, terminating\n"));
		TerminateThread(LoggerThread, (DWORD)-1);
	}
}

int LogFile::Start()
{
	DWORD dw;

	// Create logging thread
	LoggerThread = CreateThread(NULL, 0, LogThreadProc, (PVOID)this, 0, &dw);
	if(LoggerThread == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		_ftprintf(stderr, _T("Failed to create logging thread\n"));
		return err;
	}

	return 0;
}


LogFilesPool::~LogFilesPool()
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

int LogFilesPool::Start(size_t index, const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed)
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

void LogFilesPool::Stop(size_t index)
{
	LogFile *lf = Get(index);

	if(lf)
	{
		lf->Stop();
		delete lf;
		lf = NULL;
	}
}

LogFile * LogFilesPool::Get(size_t index)
{
	if(index >= LogFiles.GetDataSize() / sizeof(LogFile *))
	{
		_ftprintf(stderr, _T("Log file index out of range: %d\n"), index);
		return NULL;
	}

	return *((LogFile **)LogFiles.GetPtr() + index);
}