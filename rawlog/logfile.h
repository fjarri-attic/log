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
	bool FlushRemains;

	HANDLE StopEvent;
	HANDLE LoggerThread;

	bool Running; // FIXME: make setting/unsetting of this variable thread-safe

	Queue MessageQueue;
	File TargetFile;

	int Write(const void *buf, size_t size);
	bool Pop(MessageHeader *header, Buffer &buffer);

	static int Flush(LogFile *lf, const MessageHeader &header, Buffer &main_buf, Buffer &temp_buf);
	static DWORD WINAPI LogThreadProc(PVOID context);

public:
	LogFile(const void *file_name, bool name_is_unicode, size_t buffer_size, bool keep_closed);
	~LogFile();

	void Push(const MessageHeader *header, const void *message, size_t message_size);
	void Stop(bool flush_remains = true);
	int Start();
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