#ifndef __incl_World_Hawk_h
#define __incl_World_Hawk_h

#include "Bird.h"

namespace World {
    using namespace World;

    struct Hawk : public Bird
    {
        static float EatEnergy;             // Energy gained when a bird is eaten
        static float InitialEnergy;         // Initial energy
        static float InitialSpeed;          // Initial speed
        static float InitialSpeed_Sqr_Inv;  // 1 / ( InitialSpeed * InitialSpeed )
        static float InitialSpeedDump;      // Dumping of the initial velocity
        static float SpeedDump;             // Dumping of the bird tracking velocity
        static float EyeDistance;           // Hawk's view distance
        static float EyeDistance_Sqr;       // EyeDistance*EyeDistance
        static float MaxSpeed;              // Hawk's maximal speed
        static float MaxSpeed_Sqr;          // MaxSpeed*MaxSpeed
        static float MaxAcceleration;       // Hawk's maximal acceleration
        static float MaxAcceleration_Sqr;   // MaxAcceleration*MaxAcceleration

        static float FlockCenterAcceleration;   // "Steer towards closest bird" rule acceleration
        static float FlockNearestAcceleration;  // "Match closest bird heading" rule acceleration

        static void LoadConfigLine(const char *buffer);

    protected:
        virtual void SetCurrentQuarter(World::Quarter &quarter); // Updates Quarter field

        virtual void DetermineVisibleBirds();

        virtual void ProcessCollisions();

    public:

        xVector3 V_initial_speed; // Initial speed
        float    Energy;          // Hawk's energy
        float    Gravity;         // Gravity accumulator (we want to process gravity separately from V_initial_speed and V_velocity)
        uint     BirdsEaten;      // Eaten birds counter

        virtual void Clear()
        {
            Bird::Clear();
            Energy     = 0.f;
            Gravity    = 0.f;
            BirdsEaten = 0;
            V_initial_speed.zero();
        }

        virtual void Create(World::Map &map, const xPoint3 &P_center, const xVector3 &V_velocity);
        virtual void Destroy(bool FL_withRemove = true);

        virtual void Update(float T_delta);
        virtual void PostUpdate(float T_delta);
    };

} // namespace World

#endif
