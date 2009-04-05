#ifndef BASIC_DEBUGLOG_STREAM_H
#define BASIC_DEBUGLOG_STREAM_H

#include <sstream>
#include <streambuf>

enum LogLevel
{
	Error,
	Warning,
	Message,
	Debug
};

char *LogLevelStringsA[] = {
	"[Error] ", "[Warning] ", "[Message] ", "[Debug] "
};

wchar_t *LogLevelStringsW[] = {
	L"[Error] ", L"[Warning] ", L"[Message] ", L"[Debug] "
};

//
template<class charT>
void AddLevelStr(LogLevel level, std::basic_string<charT> &target);

template<>
void AddLevelStr<char>(LogLevel level, std::basic_string<char> &target)
{
	target += LogLevelStringsA[level];
}

template<>
void AddLevelStr<wchar_t>(LogLevel level, std::basic_string<wchar_t> &target)
{
	target += LogLevelStringsW[level];
}

//
template<class charT>
std::basic_string<charT> GetLineFolding();

template<>
std::basic_string<char> GetLineFolding()
{
	return std::basic_string<char>("\n");
}

template<>
std::basic_string<wchar_t> GetLineFolding()
{
	return std::basic_string<wchar_t>(L"\n");
}

/*!
	\brief Functor definition for output policy
	\param charT type of characters to handle (char or wchar_t)
*/
template<class charT>
class log_functor 
{
	virtual void operator()(std::basic_string<charT> &message) = 0;
};

/*!
	\brief Userdefined stream buffer for debug logging
	\param charT type of characters to handle (char or wchar_t)
	\param logfunction Functor type for output policy
	\param traits Character traits 
*/
template
<
	class charT,									// character type
	class logfunction,							  // logfunction type
	class traits = std::char_traits<charT>		  // character traits 
>
class log_streambuf : public std::basic_streambuf<charT, traits>
{
	typedef std::basic_string<charT, traits> string_type;   // shortcut for a string 

public:
	virtual ~log_streambuf()
	{
		sync();
	}

	//! \brief Set a context to be displayed before the messages 
	void setContext(const string_type &context)
	{
		context_ = context;
	}

	void finishLine()
	{
		if(buffer_.empty())
			return;

		full_line += context_ + buffer_ + GetLineFolding<charT>();
		buffer_.clear();
	}

protected:

	virtual typename traits::int_type overflow (typename traits::int_type c)
	{
		if (!traits::eq_int_type(c, traits::eof())) // if current character is not EOF 
			buffer_ += traits::to_char_type(c);	 // add current character to buffer

		return traits::not_eof(c);			  
	}

	virtual int sync()
	{
		finishLine();
				
		if(!full_line.empty())
			sendToDebugLog();   // send them to the debuglogfunction 
		return 0;
	}

private:
	string_type buffer_, context_, full_line;
	logfunction func_;					//! the logfunction functor

	//! \brief send the current stream content to the output policy 
	void sendToDebugLog()
	{
		func_(full_line);   // send context and message 
	}
};


/*!
	\brief User defined stream type for debuglogging 
	\param charT type of characters to handle (char or wchar_t)
	\param logfunction Functor type for output policy
	\param traits Character traits 
*/
template
<
	class charT,								// character type 
	class logfunction,						  // logfunction type
	class traits = std::char_traits<charT>	  // character traits 
>
class log_ostream : public std::basic_ostream<charT, traits>
{
	typedef std::basic_string<charT, traits> string_type;				   // shortcut for a string
	typedef log_streambuf<charT, logfunction, traits> buffer_type;	 // shortcut for the streambuffer
	typedef std::basic_ostream<charT, traits> stream_type;				  // shortcut for the stream 
	typedef std::basic_ostringstream<charT, traits> stringstream_type;	  // shortcut for a stringstream 

public:
	/*!
		\brief the default constructor
		\param file the filename where the debuglogging occured
		\param line the linenumber where the debuglogging occured
	*/
	log_ostream(const char *file = 0, int line = -1)
		: stream_type(&buf_), file_(file), line_(line)
	{
		level_ = Message;
		buildContext(); 
	}

	log_ostream &operator<<(LogLevel level)
	{
		level_ = level;
		string_type context = context_;
		AddLevelStr(level, context);
		buf_.setContext(context);	  // set the context to the streambuffer
		buf_.finishLine();
		return *this;
	}
	

	// allow deriving 
	virtual ~log_ostream() {}

	log_ostream &get() {return *this;}

private:
	// don't allow copying of the stream
	log_ostream(const log_ostream &);
	log_ostream &operator=(const log_ostream &);

	/*!
		\brief  Build the prefix for the debugmessage
				[<filename>][(<linenumber>) : ][<context> : ]
	*/
	void buildContext()
	{
		stringstream_type os;			// format the prefix via stringstream 

		if (file_)					  // if a filename is given
			os << file_;				// show it first
		if (line_ >= 0)				 // if a linenumber is given
			os << "(" << line_ << ")";  // show it in brackets
		if (file_ || line_ >= 0)		// if filename or linenumber given
			os << ": ";				// separate it by a double colon from context or message
		context_ = os.str();
	}

	LogLevel level_;
	const char *file_;					//! name of the sourcefile where the debuglogger was instantiated.
	const int line_;					//! number of the line in file_ where the debuglogger was instantiated.
	buffer_type buf_;					//! the streambuffer which sends it's content to OutputDebugString
	string_type context_;
};

#endif 
