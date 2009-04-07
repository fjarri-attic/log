#ifndef _LOG_H
#define _LOG_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\rawlog\rawlog.h"

#include "log_ostream.h"

#include <iostream>
#include <fstream>

// Output to file
template<class charT, int FileID> 
class log_to_file : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message, LogLevel level);
};

template<int FileID> 
class log_to_file<char, FileID> : public log_functor<char>
{
public:
	void operator()(std::basic_string<char> &message, LogLevel level)
	{
		RawLogWriteA(FileID, message.c_str());
	}	
};

template<int FileID> 
class log_to_file<wchar_t, FileID> : public log_functor<wchar_t>
{
public:
	void operator()(std::basic_string<wchar_t> &message, LogLevel level)
	{
		RawLogWriteW(FileID, message.c_str());
	}	
};


// Output to stderr
template<class charT>
class log_to_console : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message, LogLevel level);
};

void log_to_console<char>::operator()(std::basic_string<char> &message, LogLevel level)
{
	if(level == Error || level == Warning)
		std::cerr << message;
	else
		std::cout << message;
}

void log_to_console<wchar_t>::operator()(std::basic_string<wchar_t> &message, LogLevel level)
{
	if(level == Error || level == Warning)
		std::wcerr << message;
	else
		std::wcout << message;
}

#endif
