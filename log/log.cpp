#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\rawlog\rawlog.h"
#include "log.h"


void log_to_console<wchar_t>::operator()( std::basic_string<wchar_t> &message, LogLevel level )
{
	if(level == Error || level == Warning)
		std::wcerr << message;
	else
		std::wcout << message;
}

void log_to_console<char>::operator()( std::basic_string<char> &message, LogLevel level )
{
	if(level == Error || level == Warning)
		std::cerr << message;
	else
		std::cout << message;
}

template<>
void WriteToFile<char>(int file_id, std::basic_string<char> &message)
{
	RawLogWriteA(file_id, message.c_str());
}	

template<>
void WriteToFile<wchar_t>(int file_id, std::basic_string<wchar_t> &message)
{
	RawLogWriteW(file_id, message.c_str());
}	