#ifndef __incl_Physics_CollisionInfo_h
#define __incl_Physics_CollisionInfo_h

#include "../../Math/xMath.h"

namespace Physics { namespace Colliders {

    struct CollisionInfo {
        bool     FL_collided;

        xVector3 NW_fix_1;
        xPoint3  P_collision_1;

        //xVector3 NW_fix_2; // = - NW_fix_1
        xPoint3  P_collision_2;

        CollisionInfo () : FL_collided(false) {}

        CollisionInfo (bool fl_collided) : FL_collided(fl_collided) {}

        CollisionInfo (xPoint3 p_collision_1,
                       xPoint3 p_collision_2)
              : FL_collided   (true)
              , NW_fix_1      (p_collision_1-p_collision_2)
              , P_collision_1 (p_collision_1)
              , P_collision_2 (p_collision_2)
        {}

        CollisionInfo (xVector3 nw_fix_1,
                       xPoint3 p_collision_1,
                       xPoint3 p_collision_2)
              : FL_collided   (true)
              , NW_fix_1      (nw_fix_1)
              , P_collision_1 (p_collision_1)
              , P_collision_2 (p_collision_2)
        {}

        CollisionInfo &invert()
        {
            NW_fix_1.invert();
            xPoint3 swp = P_collision_1; P_collision_1 = P_collision_2; P_collision_2 = swp;
            return *this;
        }
    };

} } // namespace Physics.Colliders

#endif