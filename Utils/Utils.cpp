#include "Utils.h"

void itos(int in, std::string &out)
{
    std::stringstream ss;
    ss << in;
    ss >> out;
}

int stoi(const std::string &in)
{
    std::stringstream ss;
    int out;
    ss << in;
    ss >> out;
    return out;
}

std::string itos(int in)
{
    std::stringstream ss;
    std::string out;
    ss << in;
    ss >> out;
    return out;
}

const char *ReadSubstring(const char *buff)
{
    if (!buff) return NULL;

    // trim front
    while (*buff && (*buff == ' ' || *buff == '\t')) ++buff;

    static char resBuff[255];

    // if empty, return empty
    if (!*buff) { resBuff[0] = 0; return resBuff; };

    // check if we want white spaces
    char delim = 0;
    if (buff[0] == '"' || buff[0] == '\'') delim = *buff;

    // if we want single word, return it
    if (!delim)
    {
        const char * pchr = strpbrk(buff+1, " \t");
        if (!pchr) return buff;

        size_t len  = pchr - buff;
        memcpy(resBuff, buff, len);
        resBuff[len] = 0;
        return resBuff;
    }

    // find delimetered string double delim is returned as delim
    const char *posC, *posB = buff+1;
    char       *res = resBuff;
    size_t      len;

    posC = strchr(posB+1, delim);
    while (posC) {
        len = posC - posB;
        memcpy (res, posB, len);
        posB = posC + 1;
        res += len;

        if (*posB != delim) break;
        posC = strchr(posB+1, delim);
    }

    if (!posC) // not properly closed string, assume [delim] at the end
    {
        len = strlen(posB);
        memcpy (res, posB, len);
        res += len;
    }

    *res = 0;
    return resBuff;
}

const char *ReadSubstring(const char *buff, const char *&last)
{
    if (!buff) return NULL;

    // trim front
    while (*buff && (*buff == ' ' || *buff == '\t')) ++buff;

    static char resBuff[255];

    // if empty, return empty
    if (!*buff) { last = buff; resBuff[0] = 0; return resBuff; };

    // check if we want white spaces
    char delim = 0;
    if (buff[0] == '"' || buff[1] == '\'') delim = *buff;

    // if we want single word, return it
    if (!delim)
    {
        last = strpbrk(buff+1, " \t");
        if (!last) return buff;

        size_t len  = last - buff;
        memcpy(resBuff, buff, len);
        resBuff[len] = 0;
        return resBuff;
    }

    // find delimetered string double delim is returned as delim
    const char *posC, *posB = buff+1;
    char       *res = resBuff;
    size_t      len;

    posC = strchr(posB+1, delim);
    while (posC) {
        len = posC - posB;
        memcpy (res, posB, len);
        posB = posC + 1;
        res += len;

        if (*posB != delim) break;
        posC = strchr(posB+1, delim);
    }

    if (!posC) // not properly closed string, assume [delim] at the end
    {
        len = strlen(posB);
        memcpy (res, posB, len);
        res += len;
        last = posB+len;
    }
    else
        last = posB;

    *res = 0;
    return resBuff;
}
