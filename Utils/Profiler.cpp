#include "Profiler.h"

void Profiler :: ProfileBegin (const char* name)
{
    uint iSample = 0;
    while ( iSample < I_MaxSamples && Samples[iSample].FL_valid ) {
        if ( strcmp( name, Samples[iSample].Name ) == 0 ) {
            // Sample is already known
            ++Samples[iSample].I_OpenProfiles;
            ++Samples[iSample].I_ProfileInstances;
            Samples[iSample].T_Start = GetTick();

            assert(Samples[iSample].I_OpenProfiles && "Profiler::ProfileBegin : Recurention is not supported");
            return;
        }
        iSample++;
    }

    if ( iSample >= I_MaxSamples ) {
        assert( !"Profiler::ProfileBegin : Number of samples exceeded I_MaxSamples" );
        return;
    }

    ProfileSample &sample = Samples[iSample];
    sample.Name               = name;
    sample.FL_valid           = true;
    sample.I_OpenProfiles     = 1;
    sample.I_ProfileInstances = 1;
    sample.T_Samples          = 0.f;
    sample.T_ChildrenSamples  = 0.f;
    sample.T_Start            = GetTick();
}

void Profiler :: ProfileEnd (const char* name)
{
    uint iSample = 0;
    uint I_CountParents = 0;

    while ( iSample < I_MaxSamples && Samples[iSample].FL_valid ) {
        if ( strcmp( name, Samples[iSample].Name ) == 0 ) {
            // We found sample
            uint  iParent   = 0;
            int   ID_parent = -1; // direct parent
            float T_Stop    = GetTick();

            assert( Samples[iSample].I_OpenProfiles > 0 &&
                    "Profiler::ProfileEnd : Trying to close a sample that hasn't been opened" );
            --Samples[iSample].I_OpenProfiles;

            while ( iParent < I_MaxSamples && Samples[iParent].FL_valid ) {
                if ( Samples[iParent].I_OpenProfiles > 0 )
                {
                    ++I_CountParents;

                    if (ID_parent < 0)
                        ID_parent = iParent;
                    else
                    if (Samples[iParent].T_Start >= Samples[ID_parent].T_Start)
                        ID_parent = iParent;
                }
                ++iParent;
            }

            Samples[iSample].I_NumParents = I_CountParents;

            float T_duration = T_Stop - Samples[iSample].T_Start;

            if (ID_parent >= 0)
                Samples[ID_parent].T_ChildrenSamples += T_duration;

            Samples[iSample].T_Samples += T_duration;
            return;
        }
        ++iSample;
    }
    assert( !"Profiler::ProfileEnd : Sample with given name has not been found" );
}

void Profiler :: FrameEnd()
{
    assert (I_MaxSamples && "Profiler::FrameEnd() : Create() hasn't been called");

    uint iSample = 0;

    T_FrameEnd = GetTick();

    float T_delta     = T_FrameEnd - T_FrameStart;
    if (T_delta < 0.00000001f) return;
    float T_delta_Inv = 1.f / T_delta;
    float W_newRatio  = T_delta * 0.0008f;
    if (W_newRatio > 1.f) W_newRatio = 1.f;
    float W_oldRatio  = 1.f - W_newRatio;

    lines.clear();

    char buff1[256], buff2[256];
    lines.push_back("  Mean :   Min :   Max :   # : Name");

    while ( iSample < I_MaxSamples && Samples[iSample].FL_valid )
    {
        ProfileSample &sample  = Samples[iSample];

        assert ( !sample.I_OpenProfiles &&
                 "Profiler::FrameEnd : Sample hasn't been finished" );

        uint iHistory = GetHistory( sample.Name );
        if (iHistory == I_MaxSamples) continue;

        ProfileSampleHistory &history = History[iHistory];

        float T_Prcnt      = (sample.T_Samples - sample.T_ChildrenSamples) * T_delta_Inv * 100.f;
        if (history.FL_valid)
        {
            history.T_AvgPrcnt = W_oldRatio * history.T_AvgPrcnt
                               + W_newRatio * T_Prcnt;
            if (T_Prcnt < history.T_MinPrcnt)
                history.T_MinPrcnt = T_Prcnt;
            else
                history.T_MinPrcnt = W_oldRatio * history.T_MinPrcnt
                                   + W_newRatio * T_Prcnt;
            if (T_Prcnt > history.T_MaxPrcnt)
                history.T_MaxPrcnt = T_Prcnt;
            else
                history.T_MaxPrcnt = W_oldRatio * history.T_MaxPrcnt
                                   + W_newRatio * T_Prcnt;
        }
        else
            history.T_AvgPrcnt = history.T_MinPrcnt = history.T_MaxPrcnt = T_Prcnt;
        history.FL_valid           = true;

        char *name  = buff1;
        char *nameT = buff2;

        strcpy (name, history.Name);
        for (size_t indent=0; indent < sample.I_NumParents; ++indent)
        {
            sprintf( nameT, "    %s", name );
            char* swp = name; name = nameT; nameT = swp;
        }

        sprintf(nameT, " %5.1f : %5.1f : %5.1f : %3d : %s",
            history.T_AvgPrcnt, history.T_MinPrcnt, history.T_MaxPrcnt,
            sample.I_ProfileInstances, name);
        lines.push_back(nameT);

        sample.FL_valid = false;
        ++iSample;
    }

    T_FrameStart = GetTick();
}
