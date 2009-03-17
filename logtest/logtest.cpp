#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\log\log.h"

HANDLE stop_event;

DWORD WINAPI TesterThread(PVOID context)
{
	DWORD num = (DWORD)(ULONG_PTR)context;

	DWORD min_time = 999999, max_time = 0;
	LARGE_INTEGER time1, time2, freq;
	float mean = 0;


	TCHAR str[] = _T("Logging thread #\n");
	//CHAR str[] = "Logging thread #\n";
	str[15] = (TCHAR)(num + 0x30);
	//str[15] = (CHAR)(num + 0x30);

	DWORD count = 0;

	while(WaitForSingleObject(stop_event, 0) != WAIT_OBJECT_0)
	{
		count ++;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&time1);
		LogWrite(str);
		QueryPerformanceCounter(&time2);

		DWORD t = (time2.QuadPart - time1.QuadPart) * 1000000 / freq.QuadPart;

		if(t < min_time)
			min_time = t;

		if(t > max_time)
			max_time = t;

		mean = (mean * (count - 1) + t) / count;

		Sleep(0);
	}

	_tprintf(_T("%d, %d, %d, %f: %s"), count, min_time, max_time, mean, str);

	return 0;
}

int _tmain(int argc, TCHAR *argv[])
{
	LoadLibrary(_T("log.dll"));

	if(LogInit(argv[1]))
		return 1;

	stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE threads[10];

	for(int i = 0; i < 10; i++)
	{
		DWORD dw;
		threads[i] = CreateThread(NULL, 0, TesterThread, (LPVOID)i, 0, &dw);
	}

	Sleep(10000);
	SetEvent(stop_event);
	LogStop();
	WaitForMultipleObjects(10, threads, TRUE, INFINITE);

	return 0;
}