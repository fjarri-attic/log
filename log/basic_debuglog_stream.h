#ifndef BASIC_DEBUGLOG_STREAM_H
#define BASIC_DEBUGLOG_STREAM_H

#include <sstream>
#include <streambuf>

/*!
    \brief Functor definition for output policy
    \param charT type of characters to handle (char or wchar_t)
*/
template<class charT>
struct basic_log_function 
{
    typedef void result_type;                           // no result
    typedef const charT * const first_argument_type;    // context
    typedef const charT * const second_argument_type;   // message
};

/*!
    \brief Userdefined stream buffer for debug logging
    \param charT type of characters to handle (char or wchar_t)
    \param logfunction Functor type for output policy
    \param traits Character traits 
*/
template
<
    class charT,                                    // character type
    class logfunction,                              // logfunction type
    class traits = std::char_traits<charT>          // character traits 
>
class basic_debuglog_buf : public std::basic_streambuf<charT, traits>
{
    typedef std::basic_string<charT, traits> string_type;   // shortcut for a string 

public:
    virtual ~basic_debuglog_buf()
    {
        sync(); // flush buffer
    }

    //! \brief Set a context to be displayed before the messages 
    void setContext(const string_type &context)
    {
        context_ = context;
    }

protected:

    /*!
        \brief Reimplementation, called when a new character is inserted into a full buffer.
        \param c Current character shiftet to the stream
    */
    virtual typename traits::int_type overflow (typename traits::int_type c)
    {
        if (!traits::eq_int_type(c, traits::eof())) // if current character is not EOF 
        {
            buffer_ += traits::to_char_type(c);     // add current character to buffer
//            if (c == traits::to_int_type('\n'))     // if current character is a new line 
//              sync();                             // flush buffer			
        }
        return traits::not_eof(c);              
    }

    /*!
        \brief Reimplementation, sends the buffer to the Debuglogfunction, and clears it.
    */
    virtual int sync()
    {
        if (!buffer_.empty())   // if characters in the buffer  
            sendToDebugLog();   // send them to the debuglogfunction 
        buffer_.clear();        // empty buffer
        return 0;
    }

private:
    string_type buffer_, context_;
	logfunction func_;					//! the logfunction functor

    //! \brief send the current stream content to the output policy 
    void sendToDebugLog()
    {
        func_(context_.c_str(), buffer_.c_str());   // send context and message 
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
    class charT,                                // character type 
    class logfunction,                          // logfunction type
    class traits = std::char_traits<charT>      // character traits 
>
class basic_debuglog_stream : public std::basic_ostream<charT, traits>
{
    typedef std::basic_string<charT, traits> string_type;                   // shortcut for a string
    typedef basic_debuglog_buf<charT, logfunction, traits> buffer_type;     // shortcut for the streambuffer
    typedef std::basic_ostream<charT, traits> stream_type;                  // shortcut for the stream 
    typedef std::basic_ostringstream<charT, traits> stringstream_type;      // shortcut for a stringstream 

public:
    /*!
        \brief the default constructor
        \param file the filename where the debuglogging occured
        \param line the linenumber where the debuglogging occured
    */
    basic_debuglog_stream(const char *file = 0, int line = -1)
        : stream_type(&buf_), file_(file), line_(line)
    {
        buildContext(); // build the whole context and set it to the streambuffer
    }

    /*!
        \brief A constructor for providing a context string 
        \param context the context string 
        \param file the filename where the debuglogging occured
        \param line the linenumber where the debuglogging occured
    */
    basic_debuglog_stream(const string_type &context, const char *file = 0, int line = -1)  
        : stream_type(&buf_), file_(file), line_(line), context_(context)
    {
        buildContext(); // build the whole context and set it to the streambuffer
    }

    // allow deriving 
    virtual ~basic_debuglog_stream() {}

    /*!
        \brief set the prefix string for the debugoutput
        \param context the context string for the stream 
    */
    void setContext(const string_type &context)
    {
        context_ = context;
        buildContext();
    }

    /*!
        \brief  retrieve the prefix string for the debugoutput
        \returns the context string of the stream 
    */
    const string_type getContext() const
    {
        return context_;
    }

    /*!
        \brief get a reference to the stream (for temporaries)
        \returns a reference to the stream 
    */
    basic_debuglog_stream &get() {return *this;}

private:
    // don't allow copying of the stream
    basic_debuglog_stream(const basic_debuglog_stream &);
    basic_debuglog_stream &operator=(const basic_debuglog_stream &);

    /*!
        \brief  Build the prefix for the debugmessage
                [<filename>][(<linenumber>) : ][<context> : ]
    */
    void buildContext()
    {
        stringstream_type os;			// format the prefix via stringstream 

        if (file_)                      // if a filename is given
            os << file_;                // show it first
        if (line_ >= 0)                 // if a linenumber is given
            os << "(" << line_ << ")";  // show it in brackets
        if (file_ || line_ >= 0)        // if filename or linenumber given
            os << " : ";                // separate it by a double colon from context or message
        if (!context_.empty())          // if a context string is given
            os << context_ << " : ";    // show it at last separate it by a double colon from the message
        buf_.setContext(os.str());      // set the context to the streambuffer
    }

    const char *file_;					//! name of the sourcefile where the debuglogger was instantiated.
    const int line_;					//! number of the line in file_ where the debuglogger was instantiated.
    string_type context_;				//! additional context for output (after file_ and line_)
    buffer_type buf_;					//! the streambuffer which sends it's content to OutputDebugString
};

#endif 
