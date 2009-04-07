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

char *LogLevelStrings[] = {
	"[Error] ", "[Warning] ", "[Message] ", "[Debug] "
};

//
template<class charT>
void AddLineFolding(std::basic_string<charT> &target);

template<>
void AddLineFolding(std::basic_string<char> &target)
{
	target += '\n';
}

template<>
void AddLineFolding(std::basic_string<wchar_t> &target)
{
	target += L'\n';
}

/*!
	\brief Functor definition for output policy
	\param charT type of characters to handle (char or wchar_t)
*/
template<class charT>
class log_functor 
{
	virtual void operator()(std::basic_string<charT> &message, LogLevel level) = 0;
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
	void setContext(LogLevel level)
	{
		level_ = level;
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
		if(!buffer_.empty())
		{
			AddLineFolding<charT>(buffer_);
			func_(buffer_, level_);   // send context and message 
			buffer_.clear();
		}
		
		return 0;
	}

private:
	string_type buffer_;
	logfunction func_;					//! the logfunction functor
	LogLevel level_;
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
	log_ostream(LogLevel level, const char *file = 0, int line = -1) :
		stream_type(&buf_)
	{
		if (file)					  // if a filename is given
			*this << file;				// show it first
		if (line >= 0)				 // if a linenumber is given
			*this << "(" << line << ")";  // show it in brackets
		if (file || line >= 0)		// if filename or linenumber given
			*this << ": ";				// separate it by a double colon from context or message

		*this << LogLevelStrings[level];
		buf_.setContext(level);
	}

	// allow deriving 
	virtual ~log_ostream() {}

	log_ostream &get() {return *this;}

private:
	// don't allow copying of the stream
	log_ostream(const log_ostream &);
	log_ostream &operator=(const log_ostream &);

	buffer_type buf_;					//! the streambuffer which sends it's content to OutputDebugString
};

#endif 
