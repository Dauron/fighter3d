#ifndef __incl_Physics_Colliders_CollisionInfo_h
#define __incl_Physics_Colliders_CollisionInfo_h

#include <vector>
#include "../../Math/xMath.h"
#include "../../Math/Figures/xIFigure3d.h"

namespace Physics {

  class IPhysicalBody;

  namespace Colliders {
    using namespace ::Math::Figures;

    struct BoneWeight {
        xBYTE  I_bone;
        xFLOAT W_bone;
    };

    struct CollisionPoint {
        IPhysicalBody    *Offender;

        const xIFigure3d *Figure;
        xDWORD            ID_subobj;

        xPoint3           P_collision;
        xVector3          N_fix;
        xVector3          NW_fix;
        xVector3          V_action;
        xVector3          V_reaction;

        xFLOAT            W_fix;

        BoneWeight        W_Bones[12];
        xBYTE             I_Bones;

        CollisionPoint()
        {}
        CollisionPoint(IPhysicalBody *offender, const xPoint3 &p_collision)
            : Offender(offender), Figure(NULL), P_collision(p_collision), I_Bones(0)
        {}
        CollisionPoint(IPhysicalBody *offender, const xIFigure3d &figure, const xPoint3 &p_collision, xDWORD id_subobj = 0)
            : Offender(offender), Figure(&figure), ID_subobj(id_subobj), P_collision(p_collision)
        { SetBoneWeights(); }
        CollisionPoint(IPhysicalBody *offender, const xIFigure3d &figure, const xPoint3 &p_collision,
                       const xVector3 &nw_fix, xDWORD id_subobj = 0)
            : Offender(offender), Figure(&figure), ID_subobj(id_subobj), P_collision(p_collision), NW_fix(nw_fix)
        { SetBoneWeights(); }

        void SetBoneWeights();
    };

    struct CollisionInfo {
    private:
        CollisionPoint object1;
        CollisionPoint object2;
        bool           inverted;

    public:
        CollisionPoint &CPoint1_Get() { return inverted ? object2 : object1; }
        CollisionPoint &CPoint2_Get() { return inverted ? object1 : object2; }

        CollisionInfo () : inverted(false) {}

        CollisionInfo (const CollisionPoint &cp1, const CollisionPoint &cp2) : object1(cp1), object2(cp2), inverted(false) {}

        CollisionInfo (IPhysicalBody    *body1,      IPhysicalBody    *body2,
                       const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : object1(body2, figure1, p_collision_1, p_collision_2-p_collision_1)
              , object2(body1, figure2, p_collision_2, p_collision_1-p_collision_2)
              , inverted(false)
        {}

        CollisionInfo (IPhysicalBody    *body1,      IPhysicalBody    *body2,
                       const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       const xVector3 &nw_fix_1,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : object1(body2, figure1, p_collision_1,  nw_fix_1)
              , object2(body1, figure2, p_collision_2, -nw_fix_1)
              , inverted(false)
        {}

        CollisionInfo (IPhysicalBody    *body1,      IPhysicalBody    *body2,
                       const xIFigure3d &figure1,    const xIFigure3d &figure2,
                       xDWORD id_subobj_1,           xDWORD id_subobj_2,
                       const xVector3 &nw_fix_1,
                       const xPoint3 &p_collision_1, const xPoint3 &p_collision_2)
              : object1(body2, figure1, p_collision_1,  nw_fix_1, id_subobj_1)
              , object2(body1, figure2, p_collision_2, -nw_fix_1, id_subobj_2)
              , inverted(false)
        {}

        CollisionInfo &invert()
        {
            inverted = !inverted;
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
                CollisionPoint &cp1 = iter->CPoint1_Get();
                P_collision_1 += cp1.P_collision;
                P_collision_2 += iter->CPoint2_Get().P_collision;
                NW_fix_1 += cp1.NW_fix;
                S_minLen = min (S_minLen, cp1.NW_fix.lengthSqr());

                if (cp1.NW_fix.x > EPSILON)
                {
                    if (cp1.NW_fix.x > NW_Max.x) NW_Max.x = cp1.NW_fix.x;
                    ++I_MaxX;
                }
                else
                {
                    if (cp1.NW_fix.x < NW_Min.x) NW_Min.x = cp1.NW_fix.x;
                    ++I_MinX;
                }
                if (cp1.NW_fix.y > EPSILON)
                {
                    if (cp1.NW_fix.y > NW_Max.y) NW_Max.y = cp1.NW_fix.y;
                    ++I_MaxY;
                }
                else
                {
                    if (cp1.NW_fix.y < NW_Min.y) NW_Min.y = cp1.NW_fix.y;
                    ++I_MinY;
                }
                if (cp1.NW_fix.z > EPSILON)
                {
                    if (cp1.NW_fix.z > NW_Max.z) NW_Max.z = cp1.NW_fix.z;
                    ++I_MaxZ;
                }
                else
                {
                    if (cp1.NW_fix.z < NW_Min.z) NW_Min.z = cp1.NW_fix.z;
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
