#ifndef __incl_Utils_h
#define __incl_Utils_h

#ifdef WIN32
#   define strcasecmp  stricmp
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
int stoi(const std::string &in);

inline bool StartsWith(const char *buff, const char *string)
{
    if (!string || !buff) return false;

    for(; *string && *buff; ++string, ++buff)
        if (tolower(*string) != tolower(*buff))
            return false;
    return !*string;
}

inline char *MakeLowerCase(char *buff)
{
    if (!buff) return buff;

    char *iter = buff;
    for(; *iter; ++iter)
        *iter = tolower(*iter);
    return buff;
}

const char *ReadSubstring(const char *buff);
const char *ReadSubstring(const char *buff, const char *&last);

#endif
