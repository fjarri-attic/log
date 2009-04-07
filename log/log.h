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
template<class charT>
void WriteToFile(int file_id, std::basic_string<charT> &message);

template<class charT, int FileID> 
class log_to_file : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message, LogLevel level)
	{
		WriteToFile(FileID, message);
	}
};

// Output to stderr
template<class charT>
class log_to_console : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message, LogLevel level);
};

#endif
