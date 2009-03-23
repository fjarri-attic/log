#ifndef _LOG_H
#define _LOG_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "..\rawlog\rawlog.h"

#include "log_ostream.h"

#include <iostream>
#include <fstream>

template<class charT>
class log_to_file : public log_functor<charT>
{
public:
	void operator()(std::basic_string<charT> &message)
	{
		RawLogWrite(0, message.c_str());
	}
};

typedef log_ostream<TCHAR, log_to_file<TCHAR> > DebugLogger;


#endif
