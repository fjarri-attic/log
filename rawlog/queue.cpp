#include <Windows.h>
#include "queue.h"

struct QueueElement
{
	QueueElement *Next;
	size_t BufferSize;
	QueueElement() { Next = NULL; BufferSize = 0; }
};

Queue::Queue(size_t buffer_size)
{
	InitializeCriticalSection(&CS);

	BufSize = buffer_size;
	Buf = new char[BufSize];
	ZeroMemory(Buf, BufSize);

	PushCursor = Buf;
	PopCursor = Buf;
	((QueueElement *)PushCursor)->Next = NULL;
}

Queue::~Queue()
{
	DeleteCriticalSection(&CS);
	if(Buf)
		delete[] Buf;
}

void Queue::Push(const void *buffer1, size_t size1, const void *buffer2, size_t size2)
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

bool Queue::Pop(void *header, size_t header_size, Buffer &buffer)
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
			return false;
	}

	params = (QueueElement *)cur;
	if(header_size)
		memcpy(header, cur + sizeof(QueueElement), header_size);
	buffer.Resize(((QueueElement *)cur)->BufferSize - header_size);
	memcpy(buffer.GetPtr(), cur + sizeof(QueueElement) + header_size, buffer.GetDataSize());

	EnterCriticalSection(&CS);
	PopCursor = (char *)(params->Next);
	LeaveCriticalSection(&CS);

	return true;
}