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

int LogFile::Flush(LogFile *lf, const MessageHeader &header, Buffer &main_buf, Buffer &temp_buf)
{
	Buffer *primary_buf = &main_buf;
	Buffer *secondary_buf = &temp_buf;

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

	// Write string without ending zero symbol
	int res = lf->Write(primary_buf->GetPtr(), primary_buf->GetDataSize() - 1);
	if(res)
		return res;

	return 0;
}

//
DWORD WINAPI LogFile::LogThreadProc(PVOID context)
{
	LogFile *lf = (LogFile *)context;
	Buffer buf1, buf2;
	int res, counter = 0;
	MessageHeader header;

	while(WaitForSingleObject(lf->StopEvent, 0) != WAIT_OBJECT_0)
	{
		Sleep(0);

		if(!lf->Pop(&header, buf1))
			continue;

		res = Flush(lf, header, buf1, buf2);
		if(res)
			return res;

		counter++;
	}

	_tprintf(_T("Logged: %d\n"), counter);

	counter = 0;

	if(lf->FlushRemains)
	{
		while(lf->Pop(&header, buf1))
		{
			res = Flush(lf, header, buf1, buf2);
			if(res)
				return res;
			counter++;
		}
	}

	_tprintf(_T("Flushed: %d\n"), counter);

	return 0;
}

LogFile::LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed) : 
	MessageQueue(buffer_size), TargetFile(file_name, name_is_unicode, keep_closed)
{
	Running = false;
	ReplaceLF = true;
	FlushRemains = false;
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

bool LogFile::Pop(MessageHeader *header, Buffer &buffer)
{
	return MessageQueue.Pop(header, sizeof(*header), buffer);
}

void LogFile::Stop(bool flush_remains)
{
	if(Running)
	{
		DWORD timeout = (flush_remains ? INFINITE : 10000);
		FlushRemains = flush_remains;

		SetEvent(StopEvent);
		if(WaitForSingleObject(LoggerThread, timeout) == WAIT_TIMEOUT)
		{
			_ftprintf(stderr, _T("Logging thread timeouted, terminating\n"));
			TerminateThread(LoggerThread, (DWORD)-1);
		}

		Running = false;
	}
}

int LogFile::Start()
{
	if(!Running)
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

		Running = true;
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
			lf = NULL;
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