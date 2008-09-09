#ifndef __incl_Utils_Stat_h
#define __incl_Utils_Stat_h

#include "Singleton.h"
#include <string>
#include <vector>

struct Stat_Base {
    std::string Name;

    virtual const char* Print() = 0;
};

struct Stat_Float3Ptr : public Stat_Base {
protected:
    float *Float3;
    char   buff[255];

public:
    Stat_Float3Ptr() { Float3 = NULL; }

    void Create(const std::string &Name, float *Float3)
    {
        assert (Name.size() && "Stat_Float3Ptr::Create : Name is empty");
        assert (Float3 && "Stat_Float3Ptr::Create : Float3 is NULL");

        this->Name   = Name;
        this->Float3 = Float3;
    }

    virtual const char* Print()
    {
        assert (Name.size() && "Stat_Float3Ptr::Print : Name is empty");
        assert (Float3 && "Stat_Float3Ptr::Print : Float3 is NULL");

        sprintf(buff, "%s = (%3.3f, %3.3f, %3.3f)", Name.c_str(), Float3[0], Float3[1], Float3[2]);
        return buff;
    }
};

struct Stat_Float3PtrAndLen : public Stat_Float3Ptr {
    virtual const char* Print()
    {
        assert (Name.size() && "Stat_Float3PtrAndLen::Print : Name is empty");
        assert (Float3 && "Stat_Float3PtrAndLen::Print : Float3 is NULL");

        float lenSqr = Float3[0]*Float3[0] + Float3[1]*Float3[1] + Float3[2]*Float3[2];
        sprintf(buff, "%s = (%8.3f, %8.3f, %8.3f) = %10.3f", Name.c_str(), Float3[0], Float3[1], Float3[2], lenSqr);
        return buff;
    }
};

#define g_StatMgr   StatMgr::GetSingleton()

class StatMgr : public Singleton<StatMgr> {
public:
    typedef std::vector<Stat_Base*> Vec_Stats;
    Vec_Stats stats;

    void Add(Stat_Base &stat)
    {
        stats.push_back(&stat);
    }

    void Destroy()
    {
        Vec_Stats::iterator ST_curr = stats.begin(),
                            ST_last = stats.end();
        for (; ST_curr != ST_last; ++ST_curr)
            delete *ST_curr;
        stats.clear();
    }

    ~StatMgr() { Destroy(); }
};

#endif

