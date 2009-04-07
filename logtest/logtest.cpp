#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\log\log.h"

//typedef log_ostream<TCHAR, log_to_file<TCHAR, 0> > DebugLogger;
#define SystemLog(lev) log_ostream<TCHAR, log_to_console<TCHAR> >(lev, __FILE__, __LINE__).get()

int _tmain(int argc, TCHAR *argv[])
{
	RawLogStart(0, _T("debug.log"));
	SystemLog(Error) << _T("Test\n") << _T("TTT");
	SystemLog(Message) << _T("Test2\n") << _T("TTT2");
	SystemLog(Warning) << _T("Test3");
	SystemLog(Debug) << _T("Test4");
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
