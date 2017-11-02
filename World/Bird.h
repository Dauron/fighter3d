#ifndef __incl_World_Bird_h
#define __incl_World_Bird_h

#include "../Math/xMath.h"
#include "../Math/Tracking/TrackedObject.h"

#include "Types.h"

namespace World {
    using namespace World;

    struct Bird : public Math::Tracking::TrackedObject
    {
        static float EyeDistance;          // Bird's view distance
        static float EyeDistance_Sqr;      // EyeDistance*EyeDistance
        static float EyeDistance_Inv;      // 1 / EyeDistance
        static float KeepAwayDistance;     // Obstacle avoidance distance
        static float KeepAwayDistance_Sqr; // KeepAwayDistance*KeepAwayDistance
        static float KeepAwayDistance_Inv; // 1 / KeepAwayDistance
        static float DesiredSpeed;         // How fast the bird would like to fly
        static float MaxSpeed;             // Bird's maximal speed
        static float MaxSpeed_Sqr;         // MaxSpeed*MaxSpeed
        static float MaxSpeed_Inv;         // 1 / MaxSpeed
        static float MaxAcceleration;      // Bird's maximal acceleration
        static float MaxAcceleration_Sqr;  // MaxAcceleration*MaxAcceleration
        static float MinAltitude;          // Minimal altitude above ground/obstacles
        static float MaxAltitude;          // Maximal altitude above ground
        static float Size;                 // Size of the bird (radius)
        static float Size_Sqr;             // Size*Size

        static float CruiseAcceleration;        // Cruising mode acceleration
        static float MinAltitudeAcceleration;   // MinAltitude avoidance acceleration
        static float MaxAltitudeAcceleration;   // MaxAltitude avoidance acceleration
        static float FlockCenterAcceleration;   // "Steer towards flock center" rule acceleration
        static float FlockNearestAcceleration;  // "Match neighbour heading" rule acceleration
        static float FlockKeepAwayAcceleration; // "Keep distance from neighbours" rule acceleration
        static float AvoidEnemyAcceleration;    // "Avoid enemy" rule acceleration
        static float BounceSpeedRetention;      // How much speed will be retained after collision

        static void LoadConfigLine(const char *buffer);

    protected:
        Map     *pMap;        // Map
        Quarter *pQuarter;    // Quarter over which the bird flies
        int      Index;       // Index in the current Quarter's array of birds
        int      Col, Row;    // Current quarter row and col

        static VecBirdP VisibleBirds; // Common memory for visible birds data
        static VecHawkP VisibleHawks; // Common memory for visible hawks data


        virtual void SetCurrentQuarter(World::Quarter &quarter); // Updates Quarter field
        void DetermineCurrentPosition();                         // Determines current Quarter based on P_center_Trfm (used initially)
        void UpdateCurrentPosition();                            // Determines current Quarter based on P_quarter_pos (used during updates)

        // Find nearby birds
        virtual void DetermineVisibleBirds();
        void DetermineVisibleBirdsOnNearbyQuarter(const World::Quarter &quarter,
                                                  float EyeDistance_Sqr,
                                                  int &I_minDist, float &S_minDist_Sqr);
        // Find nearby hawks
        void DetermineVisibleHawks();
        void DetermineVisibleHawksOnNearbyQuarter(const World::Quarter &quarter,
                                                  float EyeDistance_Sqr,
                                                  int &I_minDist, float &S_minDist_Sqr);

        // Find collisions and respond to them
        bool GetNearestCollision(xPlane &PN_collision);
        virtual void ProcessCollisions();

    public:
        xPoint3  P_quarter_pos;     // Position above the current quarter
        xPoint3  P_quarter_pos_Old; // Old position above the current quarter
        //       P_center_Trfm      // Position on the map (defined in TrackedObject class)
        xVector3 V_velocity;        // Current velocity vector

        virtual void Clear()
        {
            MX_LocalToWorld_Set().identity();
            P_center.zero();
            P_center_Trfm.zero();
            P_quarter_pos.zero();
            P_quarter_pos_Old.zero();
            V_velocity.zero();

            pMap        = NULL;
            pQuarter    = NULL;
            Index       = 0;
            Col = Row   = 0;

            S_radius = 0;
        }

        virtual void Create(World::Map &map, const xPoint3 &P_center, const xVector3 &V_velocity);
        virtual void Destroy(bool FL_withRemove = true);

        virtual void Update(float T_delta);
        virtual void PostUpdate(float T_delta);
    };

} // namespace World

#endif
