#ifndef _QUEUE_H
#define _QUEUE_H

#include <Windows.h>
#include "misc.h"

class Queue
{
	CRITICAL_SECTION CS;
	char *Buf;
	size_t BufSize;

	char *PushCursor;
	char *PopCursor;
public:
	Queue(size_t buffer_size);
	~Queue();
	void Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2);
	void Pop(void *header, size_t header_size, Buffer &buffer);
};

#endif