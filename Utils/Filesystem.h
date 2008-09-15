#ifndef __incl_Filesystem_h
#define __incl_Filesystem_h

#include "Debug.h"
#include <vector>
#include <string>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
// Linux Version based on http://developer.novell.com/wiki/index.php/Programming_to_the_Linux_Filesystem

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <cassert>

class LinuxFileInfo
{
public:
    static bool canRead(const std::string& strPath)
    {
        return canAccess(strPath, R_OK);
    }
    static bool canExecute(const std::string& strPath)
    {
        return canAccess(strPath, X_OK);
    }
    static bool canWrite(const std::string& strPath)
    {
        return canAccess(strPath, W_OK);
    }
    static bool exists(const std::string& strPath)
    {
        return canAccess(strPath, F_OK);
    }
    static bool isDirectory(const std::string& strPath)
    {
        return S_ISDIR(getMode(strPath));
    }
    static bool isRegularFile(const std::string& strPath)
    {
        return S_ISREG(getMode(strPath));
    }

protected:
    static bool canAccess(const std::string& strPath, int mode)
    {
        assert(mode == F_OK || mode == R_OK || mode == W_OK || mode == X_OK);
        return 0 == access(strPath.c_str(), mode);
    }
    static mode_t getMode(const std::string& strPath)
    {
        struct stat _stat;
        if ( 0 > lstat(strPath.c_str(), &_stat) )
        {
            //throw invalid_argument("Unable to stat file");
        }
        return _stat.st_mode;
    }
};

static int one (const struct dirent *unused)
{
    return 1;
}

#endif

class Filesystem
{
public:

    // WorkingDirectory should be initialized on application start (in main)
    static std::string WorkingDirectory;

    typedef std::vector<std::string> Vec_string;

    static std::string GetSystemWorkingDirectory()
    {
        char buff[255];
#ifdef WIN32
        _getcwd(buff, 255);
#else
        getcwd(buff, 255);
#endif
        return buff;
    }

    static void SetSystemWorkingDirectory(const std::string &path)
    {
#ifdef WIN32
        _chdir(path.c_str());
#else
        chdir(path.c_str());
#endif
    }

    static std::string  GetParentDir(const std::string &path)
    {
        int pos = path.find_last_of("\\/");
        return path.substr(0, pos);
    }

    static std::string  GetFullPath(const std::string &path)
    {
        if (path[1] != ':' && path[0] != '/')
            return WorkingDirectory + "/" + path;
        return path;
    }

    static std::string  GetFileName(const std::string &file)
    {
        int pos = file.find_last_of("\\/");
        return file.substr(pos+1);
    }
    static std::string  GetFileExt(const std::string &file)
    {
        int pos  = file.find_last_of(".");
        int pos2 = file.find_last_of("\\/");
        if (pos <= pos2) // if . is before /, or no . at all
            return std::string();
        return file.substr(pos+1);
    }
    static std::string  ChangeFileExt(const std::string &file, const char *ext)
    {
        int pos  = file.find_last_of(".");
        int pos2 = file.find_last_of("\\/");
        if (pos <= pos2) // if . is before /, or no . at all
            pos = -1;
        if (ext == NULL) // remove ext
            return (pos >= 0) ? file.substr(0, pos) : file;

        std::string res = (pos >= 0) ? file.substr(0, pos+1) : file + '.';
        return res + ext;
    }

    static Vec_string GetDirectories(const std::string &path)
    {
        Vec_string vec;
        std::string p = path;
        if (!p.length() || p[p.length()-1] != '/')
            p += '/';

#ifdef WIN32
        p += '*';

        WIN32_FIND_DATA f;
        HANDLE h = FindFirstFile(p.c_str(), &f);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
                if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                    && strcmp(f.cFileName,".") )
                    vec.push_back(std::string(f.cFileName));
            while(FindNextFile(h, &f));
            FindClose(h);
        }
#else
        dirent **eps;
        dirent **dirp;
        int n = scandir (p.c_str(), &eps, one, alphasort);
        if (n >= 0)
        {
            dirp = eps;
            for (int cnt = n; cnt; --cnt, ++dirp)
            {
                if ( 0 != strcmp( ".", (*dirp)->d_name ) )// && // skip . and ..
                     //0 != strcmp( "..",(*dirp)->d_name ) )
                {
                    std::string candidate( p + (*dirp)->d_name );
                    if ( LinuxFileInfo::isDirectory(candidate) )
                        vec.push_back( (*dirp)->d_name );
                }
                delete *dirp;
           }
           delete[] eps;
        }
#endif

        return vec;
    }

    static Vec_string GetFiles(const std::string &path, const char *mask)
    {
        Vec_string vec;
        std::string p = path;
        if (!p.length() || p[p.length()-1] != '/')
            p += '/';

#ifdef WIN32
        if (mask) p += mask;
        else      p += '*';

        WIN32_FIND_DATA f;
        HANDLE h = FindFirstFile(p.c_str(), &f);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
                if (! (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    vec.push_back(std::string(f.cFileName));
            while(FindNextFile(h, &f));
            FindClose(h);
        }
#else
        dirent **eps;
        dirent **dirp;
        int n = scandir (p.c_str(), &eps, one, alphasort);
        if (n >= 0)
        {
            dirp = eps;
            for (int cnt = n; cnt; --cnt, ++dirp)
            {
                if ( 0 != strcmp( ".", (*dirp)->d_name ) && // skip . and ..
                     0 != strcmp( "..",(*dirp)->d_name ) )
                {
                    std::string candidate( p + (*dirp)->d_name );
                    if ( LinuxFileInfo::isRegularFile(candidate) && LinuxFileInfo::canRead(candidate) &&
                         MatchWildcards(mask, candidate.c_str()) )
                        vec.push_back( (*dirp)->d_name );
                }
                delete *dirp;
           }
           delete[] eps;
        }
#endif
        return vec;
    }


    // Written by Jack Handy - jakkhandy@hotmail.com
    static int MatchWildcards(const char *wild, const char *string)
    {
        const char *cp = NULL, *mp = NULL;

        while ((*string) && (*wild != '*')) {
            if ((*wild != *string) && (*wild != '?'))
                return 0;
            ++wild;
            ++string;
        }

        while (*string) {
            if (*wild == '*') {
                if (!*++wild)
                    return 1;
                mp = wild;
                cp = string+1;
            }
            else if ((*wild == *string) || (*wild == '?')) {
                ++wild;
                ++string;
            }
            else {
                wild = mp;
                string = cp++;
            }
        }

        while (*wild == '*')
            wild++;
        return !*wild;
    }
};

#endif
