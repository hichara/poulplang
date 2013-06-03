#ifndef __LOG_H__
#define __LOG_H__

#include <iomanip>
#include <iostream>
#include <string>

extern int lineNumber;

class NullOutputBuffer : public std::streambuf {
    public:
        virtual std::streamsize xsputn (const char * s, std::streamsize n) {
            return n;
        }
        virtual int overflow (int c) {
            return 1;
        }
};

class NullOutputStream : public std::ostream {
    public:
        NullOutputStream() : std::ostream (&buf) {}
    private:
        NullOutputBuffer buf;
};

static NullOutputStream null;

class Log{
public:
    static bool isDebugLevel;

    static std::ostream& Debug()
    {
        if( !isDebugLevel ){
            return null;
        } else {
            return std::cout;
        }
    }
    
    static std::ostream& Info()
    {
        return std::cout;
    }

    static std::ostream& Error()
    {
        std::cout << "ERROR ";
        return std::cout;
    }
};

#endif
