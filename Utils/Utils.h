#ifndef __incl_Utils_h
#define __incl_Utils_h

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#define strcasecmp  stricmp
#endif

#include <sstream>
#include <string>

// case-insensitive string comparison predicate
struct istring_less
{
    bool operator () ( const std::string& l, const std::string& r ) const
        {  return ( ::strcasecmp( l.c_str(), r.c_str() ) < 0 );  }
};

void itos(int in, std::string &out);
std::string itos(int in);

inline bool StartsWith(const char *buff, const char *string)
{
    if (!string || !buff) return false;

    for(; *string && *buff; ++string, ++buff)
        if (tolower(*string) != tolower(*buff))
            return false;
    return !*string;
}

#endif
