#include <Windows.h>
#include "misc.h"

class Queue
{
public:
	virtual void Push(void *buffer, size_t size) = 0;
	virtual void Pop(Buffer &buffer) = 0;
};

class WinApiPipe: public Queue
{
	HANDLE WriteEnd;
	HANDLE ReadEnd;
	CRITICAL_SECTION CS;
public:
	WinApiPipe(size_t buffer_size);
	~WinApiPipe();
	void Push(void *buffer, size_t size);
	void Pop(Buffer &buffer);
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
	void Push(void *buffer, size_t size);
	void Pop(Buffer &buffer);
};