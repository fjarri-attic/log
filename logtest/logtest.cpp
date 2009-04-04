#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\log\log.h"

int _tmain(int argc, TCHAR *argv[])
{
	RawLogStart(0, _T("debug.log"));
	LogMsg << _T("Test\n") << _T("TTT\n");
	RawLogStop(0);
/*
	Log.Init(MainLog, "mainlog.txt");
	Log.Init(SystemLog, "systemlog.txt");

	SystemLog << Message << "Message";
	SystemLog 
		<< Header << "Header"
		<< Message << "Message after header";

	MainLog << Warning << "Warning";	
*/
	return 0;
}
