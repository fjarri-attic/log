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
	void operator()(std::basic_string<charT> &message);
};

template<int FileID> 
class log_to_file<char, FileID> : public log_functor<char>
{
public:
	void operator()(std::basic_string<char> &message)
	{
		RawLogWriteA(FileID, message.c_str());
	}	
};

template<int FileID> 
class log_to_file<wchar_t, FileID> : public log_functor<wchar_t>
{
public:
	void operator()(std::basic_string<wchar_t> &message)
	{
		RawLogWriteW(FileID, message.c_str());
	}	
};


// Output to stderr
template<class charT>
class log_to_stderr : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message);
};

void log_to_stderr<char>::operator()(std::basic_string<char> &message)
{
	std::cerr << message << std::endl;
}

void log_to_stderr<wchar_t>::operator()(std::basic_string<wchar_t> &message)
{
	std::wcerr << message << std::endl;
}

// Output to stdout
template<class charT>
class log_to_stdout : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message);
};

void log_to_stdout<char>::operator()(std::basic_string<char> &message)
{
	std::cout << message << std::endl;
}

void log_to_stdout<wchar_t>::operator()(std::basic_string<wchar_t> &message)
{
	std::wcout << message << std::endl;
}


typedef log_ostream<TCHAR, log_to_file<TCHAR, 0> > DebugLogger;
//typedef log_ostream<TCHAR, log_to_stderr<TCHAR> > DebugLogger;

#define LogMsg DebugLogger(3, __FILE__, __LINE__)

#endif
