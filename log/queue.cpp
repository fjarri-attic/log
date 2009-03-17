#include <Windows.h>
#include "queue.h"

WinApiPipe::WinApiPipe(size_t buffer_size)
{
	ReadEnd = NULL;
	WriteEnd = NULL;

	InitializeCriticalSection(&CS);
	CreatePipe(&ReadEnd, &WriteEnd, NULL, (DWORD)buffer_size);
}

WinApiPipe::~WinApiPipe()
{
	CloseHandle(ReadEnd);
	CloseHandle(WriteEnd);
	DeleteCriticalSection(&CS);
}

void WinApiPipe::Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2)
{
	DWORD dw;
	size_t size = size1 + size2;
	EnterCriticalSection(&CS);
	WriteFile(WriteEnd, &size, sizeof(size_t), &dw, NULL);
	WriteFile(WriteEnd, buffer1, (DWORD)size1, &dw, NULL);
	if(size2)
		WriteFile(WriteEnd, buffer2, (DWORD)size2, &dw, NULL);
	LeaveCriticalSection(&CS);
}

void WinApiPipe::Pop(void *header, size_t header_size, Buffer &buffer)
{
	DWORD dw;
	size_t size;
	ReadFile(ReadEnd, &size, sizeof(size_t), &dw, NULL);
	if(header_size)
		ReadFile(ReadEnd, header, (DWORD)header_size, &dw, NULL);
	buffer.Resize(size - header_size);
	ReadFile(ReadEnd, buffer.GetPtr(), (DWORD)size, &dw, NULL);
}

//

struct QueueElement
{
	QueueElement *Next;
	size_t BufferSize;
	QueueElement() { Next = NULL; BufferSize = 0; }
};

RingBuffer::RingBuffer(size_t buffer_size)
{
	InitializeCriticalSection(&CS);

	BufSize = buffer_size;
	Buf = new char[BufSize];
	ZeroMemory(Buf, BufSize);

	PushCursor = Buf;
	PopCursor = Buf;
	((QueueElement *)PushCursor)->Next = NULL;
}

RingBuffer::~RingBuffer()
{
	DeleteCriticalSection(&CS);
	if(Buf)
		delete[] Buf;
}

void RingBuffer::Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2)
{
	char *cur;

	size_t total_size = sizeof(QueueElement) + size1 + size2;

	EnterCriticalSection(&CS);
	if(((PushCursor < PopCursor) && ((size_t)(PopCursor - PushCursor) < total_size)) ||
		((PushCursor > PopCursor) && (BufSize - (PushCursor - Buf) < total_size + sizeof(QueueElement)) && ((size_t)(PopCursor - Buf) < total_size)) ||
		((PushCursor == PopCursor) && (((QueueElement *)PopCursor)->Next != NULL)))
	{
		char *new_buf = new char[BufSize * 2];
		ZeroMemory(new_buf, BufSize * 2);

		if(PushCursor > PopCursor)
		{
			memcpy(new_buf, Buf, BufSize);		

			PopCursor = new_buf + (PopCursor - Buf);
			PushCursor = new_buf + (PushCursor - Buf);

			QueueElement *temp = (QueueElement *)PopCursor;
			while(temp->Next)
			{
				temp->Next = (QueueElement *)(new_buf + ((char *)temp->Next - Buf));
				temp = temp->Next;
			}
		}
		else
		{
			memcpy(new_buf, Buf, PushCursor - Buf);
			memcpy(new_buf + BufSize + (PopCursor - Buf), PopCursor, BufSize - (PopCursor - Buf));

			QueueElement *OldPushCursor = (QueueElement *)PushCursor;
			PopCursor = new_buf + (PopCursor - Buf) + BufSize;
			PushCursor = new_buf + (PushCursor - Buf);

			QueueElement *temp = (QueueElement *)PopCursor;
			while(temp->Next)
			{
				if(temp->Next <= OldPushCursor)
					temp->Next = (QueueElement *)(new_buf + ((char *)temp->Next - Buf));
				else
					temp->Next = (QueueElement *)(new_buf + ((char *)temp->Next - Buf) + BufSize);
				temp = temp->Next;
			}
		}

		delete[] Buf;
		Buf = new_buf;
		BufSize *= 2;
	}

	if(BufSize - (PushCursor - Buf) < total_size + sizeof(QueueElement))
	{
		((QueueElement *)PushCursor)->Next = (QueueElement *)Buf;
		((QueueElement *)PushCursor)->BufferSize = (size_t)(-1);
		PushCursor = (char *)Buf;
	}

	cur = PushCursor;
	PushCursor += total_size;

	LeaveCriticalSection(&CS);

	QueueElement header;
	((QueueElement *)cur)->BufferSize = size1 + size2;
	memcpy(cur + sizeof(QueueElement), buffer1, size1);
	memcpy(cur + sizeof(QueueElement) + size1, buffer2, size2);

	EnterCriticalSection(&CS);
	((QueueElement *)cur)->Next = (QueueElement *)(cur + total_size);
	LeaveCriticalSection(&CS);
}

void RingBuffer::Pop(void *header, size_t header_size, Buffer &buffer)
{
	QueueElement *params;

	char *cur = NULL;

	while(!cur)
	{
		EnterCriticalSection(&CS);
		if(((QueueElement *)PopCursor)->Next)
		{
			cur = PopCursor;
			if(((QueueElement *)cur)->BufferSize == (size_t)(-1))
			{
				PopCursor = (char*)((QueueElement *)PopCursor)->Next;
				cur = NULL;
			}
		}
		LeaveCriticalSection(&CS);

		if(!cur)
			Sleep(0);
	}

	params = (QueueElement *)cur;
	if(header_size)
		memcpy(header, cur + sizeof(QueueElement), header_size);
	buffer.Resize(((QueueElement *)cur)->BufferSize - header_size);
	memcpy(buffer.GetPtr(), cur + sizeof(QueueElement) + header_size, buffer.GetDataSize());

	EnterCriticalSection(&CS);
	PopCursor = (char *)(params->Next);
	LeaveCriticalSection(&CS);
}