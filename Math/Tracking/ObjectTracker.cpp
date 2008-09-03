#include "ObjectTracker.h"

using namespace Math::Tracking;

void ObjectTracker :: UpdateDestination()
{
    if (Mode == TRACK_NOTHING)
        return;

    if (Targets == NULL || Targets->L_objects.size() == 0)
        return;

    if (Mode == TRACK_OBJECT)
    {
        const TrackedObject &to = *Targets->L_objects[ID_object];
        //P_destination = to.MX_LocalToWorld_Get().
        //    preTransformP(to.P_center + NW_destination_shift);
        P_destination = to.P_center_Trfm + to.MX_LocalToWorld_Get().
            preTransformV(NW_destination_shift);
        return;
    }
    if (Mode == TRACK_SUBOBJECT)
    {
        TrackedObject &to = *Targets->L_objects[ID_object];
        const TrackedObject &ts = to.GetSubObject(ID_subobject);
        P_destination = to.MX_LocalToWorld_Get().
            preTransformP(ts.MX_LocalToWorld_Get().
            preTransformP(ts.P_center + NW_destination_shift) );
        return;
    }
    if (Mode == TRACK_ALL_CENTER)
    {
        xPoint3 P_min, P_max, P_min_t, P_max_t;
        Vec_TrackedObject::iterator TO_curr = Targets->L_objects.begin(),
                                    TO_last = Targets->L_objects.end();

        xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min, P_max);
        for(++TO_curr; TO_curr != TO_last; ++TO_curr)
        {
            xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_t, P_max_t);
            if (P_min_t.x < P_min.x) { P_min.x = P_min_t.x; }
            if (P_max_t.x > P_max.x) { P_max.x = P_max_t.x; }
            if (P_min_t.y < P_min.y) { P_min.y = P_min_t.y; }
            if (P_max_t.y > P_max.y) { P_max.y = P_max_t.y; }
            if (P_min_t.z < P_min.z) { P_min.z = P_min_t.z; }
            if (P_max_t.z > P_max.z) { P_max.z = P_max_t.z; }
        }

        P_destination = (P_min + P_max) * 0.5f + NW_destination_shift;
        return;
    }
    if (Mode == TRACK_CUSTOM_SCRIPT && ScriptData && Script)
    {
        Script(*this, ScriptData);
        return;
    }
}

void ObjectTracker :: InterpolatePosition(xPoint3 &P_current,
                                          const xPoint3 &P_center,
                                          xFLOAT W_progress)
{
    if (Mode == TRACK_NOTHING || P_current == P_destination) return;

    xPoint3     P_current_Loc = P_current - P_center;
    xQuaternion QT_rotation   =
                    xQuaternion::GetRotation(P_current_Loc, P_destination-P_center);
    xPoint3     P_current_Rot = QT_rotation.rotate(P_current_Loc) + P_center;
    xVector3    NW_shift      = P_destination - P_current_Rot;

    if (QT_rotation.w <= 0.999f)
    {
        P_current = xQuaternion::Interpolate(QT_rotation, W_progress).rotate(P_current_Loc) + P_center;
        QT_rotation.interpolate(1.f - W_progress).vector3.invert();
        QT_rotation.rotate(NW_shift);
        P_current += NW_shift * W_progress;
    }
    else
        P_current += (P_destination - P_current) * W_progress;
}
