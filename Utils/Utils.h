#ifndef __incl_Utils_h
#define __incl_Utils_h

#ifdef WIN32
#   define strcasecmp  stricmp
#endif

#define SAFE_DELETE(obj)          { delete   obj; obj = 0; }
#define SAFE_DELETE_ARRAY(obj)    { delete[] obj; obj = 0; }
#define SAFE_DELETE_IF(obj)       if (obj) { delete   obj; obj = 0; }
#define SAFE_DELETE_ARRAY_IF(obj) if (obj) { delete[] obj; obj = 0; }
#define SAFE_begin   do
#define SAFE_return      return
#define SAFE_catch   while (false);

#define SAFE_fread(variable, count, file) \
    if (fread(&variable, sizeof(variable), count, file) != count) break;
#define SAFE_fwrite(variable, count, file) \
    if (fwrite(&variable, sizeof(variable), count, file) != count) break;
#define SAFE_fwrite_ex(variable_ptr, size, count, file) \
    if (fwrite(variable_ptr, size, count, file) != count) break;

#include <sstream>
#include <string>
#include <cstring>

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
