//============================================================================//
// A exception class for PRad Event Viewer                                    //
//                                                                            //
// Chao Peng                                                                  //
// 02/27/2016                                                                 //
//============================================================================//

#include "GemException.h"

using namespace std;

GemException::GemException(GemExceptionType typ, const string &txt, const string &aux) 
  : type(typ), text(txt), auxText(aux)
{
}

GemException::GemException(GemExceptionType typ, const string &txt, const string &file, const string &func, int line) 
  : type(typ), text(txt)
{
    ostringstream oss;
    oss <<  "    evioException occured in file " << file << ", function " << func << ", line " << line;
    auxText=oss.str();
}


string GemException::FailureDesc(void) const throw()
{
    ostringstream oss;
    oss << text << endl
        << endl
        << auxText << dec;
    return(oss.str());
}

string GemException::FailureType(void) const throw()
{
    string oss;
    switch(type) {
    case ET_OPEN_ERROR:
        oss = "ET OPEN ERROR";
        break;
    case ET_CONFIG_ERROR:
        oss = "ET CONFIG ERROR";
        break;
    case ET_STATION_CONFIG_ERROR:
        oss = "ET STATION CONFIG ERROR";
        break;
    case ET_STATION_CREATE_ERROR:
        oss = "ET STATION CREATE ERROR";
        break;
    case ET_STATION_ATTACH_ERROR:
        oss = "ET ATTACH ERROR";
        break;
    case ET_READ_ERROR:
        oss = "ET READ ERROR";
        break;
    case ET_PUT_ERROR:
        oss = "ET PUT ERROR";
        break;
    default:
        oss = "UNKNOWN ERROR";
        break;
    }
    return(oss);
}
