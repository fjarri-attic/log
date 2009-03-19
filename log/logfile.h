#ifndef _LOGFILE_H
#define _LOGFILE_H

#include "queue.h"
#include "file.h"

struct MessageHeader
{
	bool Unicode;
};

class LogFile
{
private:
	bool ReplaceLF;

	HANDLE StopEvent;
	HANDLE LoggerThread;

	Queue MessageQueue;
	File TargetFile;

public:
	LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed);
	~LogFile();

	DWORD Write(LPCVOID buf, size_t size);
	void Push(const MessageHeader *header, const void *message, size_t message_size);
	void Pop(MessageHeader *header, Buffer &buffer);
	void Stop();
	int Start();
	static DWORD WINAPI LogThreadProc(PVOID context);
};


class LogFilesPool
{
	Buffer LogFiles;
public:
	LogFilesPool() {}
	~LogFilesPool();

	int Start(size_t index, const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed);
	void Stop(size_t index);
	LogFile *Get(size_t index);
};


#endif