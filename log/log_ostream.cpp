#include "log_ostream.h"
#include <string>
#include <sstream>

char *LogLevelStrings[] = {
	"[Error] ", "[Warning] ", "[Message] ", "[Debug] "
};


void AddLineFolding(std::basic_string<char> &target)
{
	target += '\n';
}

void AddLineFolding(std::basic_string<wchar_t> &target)
{
	target += L'\n';
}

std::string AddContext(LogLevel level, const char *file, int line)
{
	std::stringstream ss;

	if (file)					  // if a filename is given
		ss << file;				// show it first
	if (line >= 0)				 // if a linenumber is given
		ss << "(" << line << ")";  // show it in brackets
	if (file || line >= 0)		// if filename or linenumber given
		ss << ": ";				// separate it by a double colon from context or message

	ss << LogLevelStrings[level];
	return ss.str();
}