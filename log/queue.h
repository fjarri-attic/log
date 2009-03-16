#include <misc.h>

class Queue
{
public:
	void Push(void *buffer, size_t size) = 0;
	void Pop(Buffer &buffer) = 0;
};

class WinApiPipe: public Queue
{
public:
	WinApiPipe(size_t buffer_size);
	~WinApiPipe();
	void Push(void *buffer, size_t size);
	void Pop(Buffer &buffer);
};

class RingBuffer: public Queue
{
public:
	RingBuffer(size_t buffer_size);
	~RingBuffer();
	void Push(void *buffer, size_t size);
	void Pop(Buffer &buffer);
};