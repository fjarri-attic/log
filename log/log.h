#ifndef _LOG_H
#define _LOG_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\rawlog\rawlog.h"

#include "basic_debuglog_stream.h"

#include <iostream>
#include <fstream>

template<class charT>
class log_to_file : public basic_log_function<charT>
{
public:
	typename basic_log_function<charT>::result_type operator()(typename basic_log_function<charT>::second_argument_type context, typename basic_log_function<charT>::second_argument_type output)
	{    
		std::basic_string<charT> str = context;
		str += output;
		RawLogWrite(0, str.c_str());
	}
};

typedef basic_debuglog_stream<TCHAR, log_to_file<TCHAR> > DebugLogger;


#endif
