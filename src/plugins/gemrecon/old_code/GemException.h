#ifndef PRAD_EXCEPTION_H
#define PRAD_EXCEPTION_H


#include <stdlib.h>
#include <string.h>
#include <exception>
#include <string>
#include <sstream>

class GemException : public std::exception
{
public:
    enum GemExceptionType
    {
        UNKNOWN_ERROR,
        ET_OPEN_ERROR,
        ET_CONFIG_ERROR,
        ET_STATION_CONFIG_ERROR,
        ET_STATION_CREATE_ERROR,
        ET_STATION_ATTACH_ERROR,
        ET_READ_ERROR,
        ET_PUT_ERROR,
    };
    GemException(GemExceptionType typ = UNKNOWN_ERROR, const std::string &txt = "", const std::string &aux = "");
    GemException(GemExceptionType typ, const std::string &txt, const std::string &file, const std::string &func, int line);
    virtual ~GemException(void) throw() {};
    virtual std::string FailureDesc(void) const throw();
    virtual std::string FailureType(void) const throw();


public:
    GemExceptionType type;             // exception type
    std::string text;     // primary text
    std::string auxText;  // auxiliary text
};

#endif


