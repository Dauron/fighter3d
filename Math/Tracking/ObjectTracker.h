#ifndef __incl_Math_Tracking_ObjectTracker_h
#define __incl_Math_Tracking_ObjectTracker_h

#include "TrackedObject.h"
#include <vector>
#include <string>

namespace Math { namespace Tracking {
    using namespace ::Math::Tracking;

    typedef std::vector<TrackedObject*> Vec_TrackedObject;

    struct TrackingSet
    {
        Vec_TrackedObject L_objects;
    };

    struct ObjectTracker;

    typedef void (*TrackingScript)(ObjectTracker &tracker, xBYTE *data);

    struct ObjectTracker
    {
        enum TrackingMode {
            TRACK_NOTHING,
            TRACK_OBJECT,
            TRACK_SUBOBJECT,
            TRACK_ALL_CENTER,
            TRACK_CUSTOM_SCRIPT
        };

        TrackingMode   Mode;
        std::string    ScriptName;
        TrackingScript Script;
        xBYTE         *ScriptData;

        TrackingSet  *Targets;
        xBYTE         ID_object;
        xBYTE         ID_subobject;
        xVector3      NW_destination_shift;

        xPoint3       P_destination;

        void Init(bool FL_init_empty = true)
        {
            Targets = (FL_init_empty) ? NULL : new TrackingSet();
            Mode    = TRACK_NOTHING;
            ScriptName.clear();
            Script     = NULL;
            ScriptData = NULL;
            P_destination.zero();
            NW_destination_shift.zero();
            ID_object = ID_subobject = 0;
        }

        void UpdateDestination();
        void InterpolatePosition(xPoint3 &P_current,
                                 const xPoint3 &P_center,
                                 xFLOAT W_progress);

        void Free()
        {
            if (Targets) { delete Targets; Targets = NULL; }
        }
    };

} } // namespace Math.Tracking

#endif
