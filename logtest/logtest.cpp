#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\log\log.h"

enum LogFilesEnum
{
	MainLogFile,
	SystemLogFile
};

#define SystemLog (Log << SystemLogFile)
#define MainLog (Log << MainLogFile)

int _tmain(int argc, TCHAR *argv[])
{
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
