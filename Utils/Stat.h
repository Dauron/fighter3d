#ifndef __incl_Utils_Stat_h
#define __incl_Utils_Stat_h

#include "Singleton.h"
#include "Profiler.h"

typedef std::vector<std::string> Vec_string;

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

struct Stat_BoolPtr : public Stat_Base {
protected:
    bool *BoolP;
    char  buff[255];

public:
    Stat_BoolPtr() { BoolP = NULL; }

    void Create(const std::string &Name, bool &pBool)
    {
        assert (Name.size() && "Stat_BoolPtr::Create : Name is empty");

        this->Name   = Name;
        this->BoolP = &pBool;
    }

    virtual const char* Print()
    {
        assert (Name.size() && "Stat_BoolPtr::Print : Name is empty");
        assert (BoolP && "Stat_BoolPtr::Print : BoolP is NULL");

        sprintf(buff, "%s = %s", Name.c_str(), *BoolP ? "true" : "false");
        return buff;
    }
};

struct Stat_FloatPtr : public Stat_Base {
protected:
    float *FloatP;
    char   buff[255];

public:
    Stat_FloatPtr() { FloatP = NULL; }

    void Create(const std::string &Name, float &Float)
    {
        assert (Name.size() && "Stat_FloatPtr::Create : Name is empty");

        this->Name   = Name;
        this->FloatP = &Float;
    }

    virtual const char* Print()
    {
        assert (Name.size() && "Stat_FloatPtr::Print : Name is empty");
        assert (FloatP && "Stat_FloatPtr::Print : FloatP is NULL");

        sprintf(buff, "%s = %f", Name.c_str(), *FloatP);
        return buff;
    }
};

#define g_StatMgr   StatMgr::GetSingleton()

struct StatPage_Base
{
    std::string Name;

    virtual const Vec_string &GetLines() = 0;

    virtual void Destroy() { Name.clear(); }
    virtual ~StatPage_Base() { Destroy(); }
};

struct StatPage : public StatPage_Base
{
private:
    Vec_string lines;

    typedef std::vector<Stat_Base*> Vec_Stats;
    Vec_Stats stats;
public:

    StatPage()
    { Name = "Unnamed stat page"; }

    void Add(Stat_Base &stat)
    { stats.push_back(&stat); }

    virtual const Vec_string &GetLines()
    {
        lines.clear();
        Vec_Stats::iterator ST_curr = stats.begin(),
                            ST_last = stats.end();
        for (; ST_curr != ST_last; ++ST_curr)
            lines.push_back((**ST_curr).Print());

        return lines;
    }

    virtual void Destroy()
    {
        StatPage_Base::Destroy();

        Vec_Stats::iterator ST_curr = stats.begin(),
                            ST_last = stats.end();
        for (; ST_curr != ST_last; ++ST_curr)
            delete *ST_curr;
        stats.clear();
    }
};

struct ProfilerPage : public StatPage_Base
{
public:

    ProfilerPage() { Name = "Profiler"; }

    virtual const Vec_string &GetLines()
    { return g_Profiler.GetLines(); }
};

class StatMgr : public Singleton<StatMgr> {
public:
    typedef std::vector<StatPage_Base*> Vec_Pages;
    Vec_Pages pages;

    void Add(StatPage_Base &page)
    {
        pages.push_back(&page);
    }

    void Remove(StatPage_Base &page)
    {
        Vec_Pages::iterator PG_curr = pages.begin(),
                            PG_last = pages.end();
        for (; PG_curr != PG_last; ++PG_curr)
            if (*PG_curr == &page)
            {
                delete *PG_curr;
                pages.erase(PG_curr);
                return;
            }
    }

    void Destroy()
    {
        Vec_Pages::iterator PG_curr = pages.begin(),
                            PG_last = pages.end();
        for (; PG_curr != PG_last; ++PG_curr)
            delete *PG_curr;
        pages.clear();
    }

    ~StatMgr() { Destroy(); }
};

#endif

