#ifndef __incl_Physics_Colliders_CollisionInfo_h
#define __incl_Physics_Colliders_CollisionInfo_h

#include <vector>
#include "../../Math/xMath.h"
#include "../../Math/Figures/xIFigure3d.h"

namespace Physics { namespace Colliders {
    using namespace ::Math::Figures;

    struct CollisionInfo {
        const xIFigure3d *Figure1;
        const xIFigure3d *Figure2;

        xPoint3  P_collision_1;
        xPoint3  P_collision_2;

        xDWORD   ID_subobj_1;
        xDWORD   ID_subobj_2;

        xVector3 NW_fix_1;
        //xVector3 NW_fix_2; // = - NW_fix_1


        CollisionInfo () {}

        CollisionInfo (const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : Figure1       (&figure1)
              , Figure2       (&figure2)
              , NW_fix_1      (p_collision_2-p_collision_1)
              , P_collision_1 (p_collision_1)
              , P_collision_2 (p_collision_2)
        {}

        CollisionInfo (const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       const xVector3 &nw_fix_1,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : Figure1       (&figure1)
              , Figure2       (&figure2)
              , NW_fix_1      (nw_fix_1)
              , P_collision_1 (p_collision_1)
              , P_collision_2 (p_collision_2)
        {}

        CollisionInfo (const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       xDWORD id_subobj_1,           xDWORD id_subobj_2,
                       const xVector3 &nw_fix_1,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : Figure1       (&figure1)
              , Figure2       (&figure2)
              , ID_subobj_1   (id_subobj_1)
              , ID_subobj_2   (id_subobj_2)
              , NW_fix_1      (nw_fix_1)
              , P_collision_1 (p_collision_1)
              , P_collision_2 (p_collision_2)
        {}

        CollisionInfo &invert()
        {
            NW_fix_1.invert();
            xPoint3 swp = P_collision_1; P_collision_1 = P_collision_2; P_collision_2 = swp;
            const xIFigure3d *fswp = Figure1; Figure1 = Figure2; Figure2 = fswp;
            xDWORD iswp = ID_subobj_1; ID_subobj_1 = ID_subobj_2; ID_subobj_2 = iswp;
            return *this;
        }
    };

    struct CollisionSet {
        typedef std::vector<CollisionInfo> CollisionVec;

        CollisionVec collisions;

        void Add(const CollisionInfo &ci)
        {
            collisions.push_back(ci);
        }
        void Clear()
        {
            collisions.clear();
        }
        bool IsCollided()
        {
            return collisions.size();
        }
        xDWORD Count()
        {
            return collisions.size();
        }

        void MergeCollisions(xPoint3  &P_collision_1, xPoint3 &P_collision_2, xVector3 &NW_fix_1)
        {
            if (!collisions.size()) return;

            xVector3 NW_Max; NW_Max.init(EPSILON,EPSILON,EPSILON);
            xDWORD   I_MaxX = 0;
            xDWORD   I_MaxY = 0;
            xDWORD   I_MaxZ = 0;
            xVector3 NW_Min; NW_Min.init(-EPSILON,-EPSILON,-EPSILON);
            xDWORD   I_MinX = 0;
            xDWORD   I_MinY = 0;
            xDWORD   I_MinZ = 0;
            xFLOAT   S_minLen = xFLOAT_HUGE_POSITIVE;
            NW_fix_1.zero();

            P_collision_1.zero();
            P_collision_2.zero();

            CollisionVec::iterator iter, end = collisions.end();
            for (iter = collisions.begin(); iter != end; ++iter)
            {
                P_collision_1 += iter->P_collision_1;
                P_collision_2 += iter->P_collision_2;
                NW_fix_1 += iter->NW_fix_1;
                S_minLen = min (S_minLen, iter->NW_fix_1.lengthSqr());

                if (iter->NW_fix_1.x > EPSILON)
                {
                    if (iter->NW_fix_1.x > NW_Max.x) NW_Max.x = iter->NW_fix_1.x;
                    ++I_MaxX;
                }
                else
                {
                    if (iter->NW_fix_1.x < NW_Min.x) NW_Min.x = iter->NW_fix_1.x;
                    ++I_MinX;
                }
                if (iter->NW_fix_1.y > EPSILON)
                {
                    if (iter->NW_fix_1.y > NW_Max.y) NW_Max.y = iter->NW_fix_1.y;
                    ++I_MaxY;
                }
                else
                {
                    if (iter->NW_fix_1.y < NW_Min.y) NW_Min.y = iter->NW_fix_1.y;
                    ++I_MinY;
                }
                if (iter->NW_fix_1.z > EPSILON)
                {
                    if (iter->NW_fix_1.z > NW_Max.z) NW_Max.z = iter->NW_fix_1.z;
                    ++I_MaxZ;
                }
                else
                {
                    if (iter->NW_fix_1.z < NW_Min.z) NW_Min.z = iter->NW_fix_1.z;
                    ++I_MinZ;
                }
            }

            xFLOAT I_count_Inv = 1.f / (xFLOAT)collisions.size();
            P_collision_1 *= I_count_Inv;
            P_collision_2 *= I_count_Inv;
            NW_fix_1      *= I_count_Inv;
            if (NW_fix_1.lengthSqr() >= S_minLen)
                return;

            if (I_MaxX > 0 || I_MinX > 0)
            {
                xFLOAT W_max = I_MaxX / ((xFLOAT) I_MaxX + I_MinX);
                NW_fix_1.x = NW_Max.x * W_max + NW_Min.x * (1.f - W_max);
            }
            else
                NW_fix_1.x = 0.f;
            if (I_MaxY > 0 || I_MinY > 0)
            {
                xFLOAT W_max = I_MaxY / ((xFLOAT) I_MaxY + I_MinY);
                NW_fix_1.y = NW_Max.y * W_max + NW_Min.y * (1.f - W_max);
            }
            else
                NW_fix_1.y = 0.f;
            if (I_MaxZ > 0 || I_MinZ > 0)
            {
                xFLOAT W_max = I_MaxZ / ((xFLOAT) I_MaxZ + I_MinZ);
                NW_fix_1.z = NW_Max.z * W_max + NW_Min.z * (1.f - W_max);
            }
            else
                NW_fix_1.z = 0.f;

            if (NW_fix_1.lengthSqr() < EPSILON) NW_fix_1.init(0.f, 0.f, 0.1f); // add random fix if locked
        }
    };

} } // namespace Physics.Colliders

#endif
