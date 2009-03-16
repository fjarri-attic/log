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

void WinApiPipe::Push(void *buffer, size_t size)
{
	DWORD dw;
	EnterCriticalSection(&CS);
	WriteFile(WriteEnd, &size, sizeof(size_t), &dw, NULL);
	WriteFile(WriteEnd, buffer, (DWORD)size, &dw, NULL);
	LeaveCriticalSection(&CS);
}

void WinApiPipe::Pop(Buffer &buffer)
{
	DWORD dw;
	size_t size;
	ReadFile(ReadEnd, &size, sizeof(size_t), &dw, NULL);
	buffer.Resize(size);
	ReadFile(ReadEnd, buffer.GetPtr(), (DWORD)size, &dw, NULL);
}