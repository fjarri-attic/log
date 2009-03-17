#ifndef _QUEUE_H
#define _QUEUE_H

#include <Windows.h>
#include "misc.h"

class Queue
{
public:
	virtual void Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2) = 0;
	virtual void Pop(void *header, size_t header_size, Buffer &buffer) = 0;
};

class WinApiPipe: public Queue
{
	HANDLE WriteEnd;
	HANDLE ReadEnd;
	CRITICAL_SECTION CS;
public:
	WinApiPipe(size_t buffer_size);
	~WinApiPipe();
	void Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2);
	void Pop(void *header, size_t header_size, Buffer &buffer);
};

class RingBuffer: public Queue
{
	CRITICAL_SECTION CS;
	char *Buf;
	size_t BufSize;

	char *PushCursor;
	char *PopCursor;
public:
	RingBuffer(size_t buffer_size);
	~RingBuffer();
	void Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2);
	void Pop(void *header, size_t header_size, Buffer &buffer);
};

#endif