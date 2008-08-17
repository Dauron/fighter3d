#ifndef __incl_Physics_PhysicalWorld_h
#define __incl_Physics_PhysicalWorld_h

#include "IPhysicalBody.h"
#include "Colliders/BVHCollider.h"

namespace Physics {
    using namespace Physics::Colliders;

    class PhysicalWorld
    {
    public:
        typedef std::vector<IPhysicalBody*> ObjectVector;

        virtual const xVector3 &Gravity_Get(const IPhysicalBody &object) const
        {
            static xVector3 NW_gravity = xVector3::Create(0.f,0.f, -10.f);
            return NW_gravity;
        }

        virtual const xVector3 &Gravity_GetNormal(const IPhysicalBody &object) const
        {
            static xVector3 NW_gravity = xVector3::Create(0.f,0.f, -1.f);
            return NW_gravity;
        }

        void Interact(xFLOAT T_time, ObjectVector &L_objects);
    };

} // namespace Physics

#endif
