#ifndef __incl_Utils_Profiler_h
#define __incl_Utils_Profiler_h

#include "Singleton.h"
#include <string>
#include <cstring>
#include <vector>
#include <ctime>

inline float GetTick()
{
    static float __TimerScale = 1000.f / CLOCKS_PER_SEC;
    return clock() * __TimerScale;
}

typedef unsigned int uint;
typedef std::vector<std::string> Vec_string;

#define g_Profiler Profiler::GetSingleton()

class Profiler : public Singleton<Profiler>
{
    struct ProfileSample {
        const char *Name;
        bool        FL_valid;

        uint        I_ProfileInstances;
        uint        I_OpenProfiles;

        float       T_Start;
        float       T_Samples;
        float       T_ChildrenSamples;

        uint        I_NumParents;
    } *Samples;

    struct ProfileSampleHistory {
        const char *Name;
        bool        FL_valid;
        float       T_AvgPrcnt;
        float       T_MinPrcnt;
        float       T_MaxPrcnt;
    } *History;

    uint  I_MaxSamples;
    float T_FrameStart;
    float T_FrameEnd;

    Vec_string lines;

public:
    static void CreateS(uint i_MaxSamples)
    {
        new Profiler();
        g_Profiler.Create(i_MaxSamples);
    }

    Profiler() : I_MaxSamples(0) {}
    ~Profiler() { Destroy(); }

    void Create(uint i_MaxSamples)
    {
        assert (!I_MaxSamples && "Profiler::Create() : Profiler hasn't been destroyed");
        assert (i_MaxSamples && "Profiler::Create() : i_MaxSamples is 0");

        I_MaxSamples = i_MaxSamples;

        Samples   = new ProfileSample[i_MaxSamples];
        History   = new ProfileSampleHistory[i_MaxSamples];

        for (uint i = 0; i < i_MaxSamples; ++i)
        {
            Samples[i].FL_valid = false;
            History[i].FL_valid = false;
        }

        T_FrameStart = GetTick();
    }
    void Destroy()
    {
        if (I_MaxSamples)
        {
            delete[] Samples;
            delete[] History;
            I_MaxSamples = 0;
        }
        lines.clear();
    }

    void ClearSamples()
    {
        assert (I_MaxSamples && "Profiler::ClearSamples() : Profiler hasn't been created");
        for (uint i = 0;
             i < I_MaxSamples && (History[i].FL_valid || Samples[i].FL_valid);
             ++i)
        {
            Samples[i].FL_valid = false;
            History[i].FL_valid = false;
        }
    }
    void FrameEnd();

    friend struct ProfileInstance;

    const Vec_string &GetLines()
    { return lines; }

private:
    void ProfileBegin (const char* name);
    void ProfileEnd (const char* name);

    uint GetHistory(const char* name)
    {
        uint iSample = 0;

        while ( iSample < I_MaxSamples && History[iSample].FL_valid ) {
            if ( strcmp( name, History[iSample].Name ) == 0 )
                return iSample;
            ++iSample;
        }

        if ( iSample >= I_MaxSamples ) {
            assert( !"Profiler::ProfileBegin : Number of samples exceeded I_MaxSamples" );
            return I_MaxSamples;
        }

        History[iSample].Name = name;
        return iSample;
    }
};

struct ProfileInstance
{
    const char* Name;

    ProfileInstance(const char *name)
    { Name = name; if (Profiler::GetSingletonPtr()) g_Profiler.ProfileBegin(Name); }

    ~ProfileInstance()
    { if (Profiler::GetSingletonPtr()) g_Profiler.ProfileEnd(Name); }
};

#define Profile(text)   ProfileInstance profileInstance(text);

#endif
