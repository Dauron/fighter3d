#include "Hawk.h"

#include "Map.h"
#include "Quarter.h"

#include "../Utils/Profiler.h"
#include "../Utils/Utils.h"

using namespace World;

float Hawk:: EatEnergy            = 1000.f;
float Hawk:: InitialEnergy        = 20000.f;
float Hawk:: InitialSpeed         = 400.f;
float Hawk:: InitialSpeed_Sqr_Inv = 1.f / (Hawk::InitialSpeed * Hawk::InitialSpeed);
float Hawk:: InitialSpeedDump     = 0.2f;
float Hawk:: SpeedDump            = 0.2f;
float Hawk:: EyeDistance          = 200.f;
float Hawk:: EyeDistance_Sqr      = Hawk::EyeDistance * Hawk::EyeDistance;
float Hawk:: MaxSpeed             = Bird::MaxSpeed * 1.5f;
float Hawk:: MaxSpeed_Sqr         = Hawk::MaxSpeed * Hawk::MaxSpeed;
float Hawk:: MaxAcceleration      = 2.5f;
float Hawk:: MaxAcceleration_Sqr  = Hawk::MaxAcceleration * Hawk::MaxAcceleration;

float Hawk:: FlockCenterAcceleration   = 1.1f;
float Hawk:: FlockNearestAcceleration  = 0.9f;

/////////////////////////////////////

void Hawk:: Create(World::Map &map, const xPoint3 &P_center, const xVector3 &V_velocity)
{
    this->Type     = OBJECT_HAWK;

    this->S_radius = Size;
    this->P_center.zero();

    this->P_center_Trfm   = P_center;
    this->V_velocity.zero();
    this->V_initial_speed = xVector3::Normalize(V_velocity) * Hawk::InitialSpeed;
    this->Energy          = Hawk::InitialEnergy;
    this->Gravity         = 0.f;
    this->BirdsEaten      = 0;

    pMap     = &map;
    pQuarter = NULL;
    map.Hawks.push_back(this);

    DetermineCurrentPosition();
}

void Hawk:: Destroy(bool FL_withRemove)
{
    if (pQuarter)
    {
        pQuarter->Hawks[Index] = pQuarter->Hawks.back();
        pQuarter->Hawks[Index]->Index = Index;
        pQuarter->Hawks.pop_back();
    }
    if (FL_withRemove)
        pMap->Hawks.remove(this);
}

/////////////////////////////////////

void Hawk:: SetCurrentQuarter(World::Quarter &quarter)
{
    if (pQuarter != &quarter)
    {
        // Remove from the old Quarter
        if (pQuarter)
        {
            pQuarter->Hawks[Index] = pQuarter->Hawks.back();
            pQuarter->Hawks[Index]->Index = Index;
            pQuarter->Hawks.pop_back();
        }
        // Insert into the new Quarter
        pQuarter = &quarter;
        Index = pQuarter->Hawks.size();
        pQuarter->Hawks.push_back(this);
    }
}

/////////////////////////////////////

void Hawk:: DetermineVisibleBirds()
{
    VisibleBirds.clear();

    float S_minDist_Sqr = xFLOAT_HUGE_POSITIVE;
    int   I_minDist     = 0;

    // Check current Quarter
    DetermineVisibleBirdsOnNearbyQuarter(*pQuarter, Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);

    // Check nearby Quarters
    // Check to the W
    if (Col > 0)
    {
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NW
        if (Row > 0)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols-1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SW
        if (Row < pMap->Rows-1)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols-1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the E
    if (Col < pMap->Cols-1)
    {
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NE
        if (Row > 0)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols+1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SE
        if (Row < pMap->Rows-1)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols+1), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the N
    if (Row > 0)
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    // Check to the S
    if (Row < pMap->Rows-1)
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols), Hawk::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);

    // Move closest bird to the first position
    if (I_minDist > 0)
    {
        Bird *swap              = VisibleBirds.front();
        VisibleBirds.front()    = VisibleBirds[I_minDist];
        VisibleBirds[I_minDist] = swap;
    }
}

/////////////////////////////////////

void Hawk:: ProcessCollisions()
{
    Profile("Collision detection & responce");

    xPlane PN_collision;

    while ( GetNearestCollision(PN_collision) )
    {
        if (PN_collision.z != 0.f)  // ground/roof bounce
        {
            P_quarter_pos.z   = 2.f * PN_collision.w - P_quarter_pos.z;
            V_velocity.z      = -V_velocity.z;
            V_initial_speed.z = -V_initial_speed.z;
            Gravity           = -Gravity;
        }
        else
        if (PN_collision.x != 0.f) // west/east wall bounce
        {
            P_quarter_pos.x   = 2.f * PN_collision.w - P_quarter_pos.x;
            V_velocity.x      = -V_velocity.x;
            V_initial_speed.x = -V_initial_speed.x;
        }
        else
        if (PN_collision.y != 0.f) // north/south wall bounce
        {
            P_quarter_pos.y   = 2.f * PN_collision.w - P_quarter_pos.y;
            V_velocity.y      = -V_velocity.y;
            V_initial_speed.y = -V_initial_speed.y;
        }
        V_velocity      *= Bird::BounceSpeedRetention;
        V_initial_speed *= Bird::BounceSpeedRetention;
        Gravity         *= Bird::BounceSpeedRetention;

        UpdateCurrentPosition();
    }
    P_center_Trfm.x = Col * Quarter::SquareWidth + P_quarter_pos.x + pMap->MinX;
    P_center_Trfm.y = (pMap->Rows-1-Row) * Quarter::SquareWidth + P_quarter_pos.y + pMap->MinY;
    P_center_Trfm.z = P_quarter_pos.z;
}

/////////////////////////////////////

void Hawk:: Update(float T_delta)
{
    Profile("Update");

    // Upadate position
    P_quarter_pos_Old  = P_quarter_pos;
    xVector3 V_shift   = (V_initial_speed + V_velocity) * T_delta;
    V_shift.z         += Gravity * T_delta;
    P_center_Trfm     += V_shift;
    P_quarter_pos     += V_shift;

    // Update current Quarter
    UpdateCurrentPosition();
    ProcessCollisions();

    // No rotations - just tranlation is enough
    //MX_LocalToWorld_Set() *= xMatrixTranslateT(P_center_Trfm.xyz);
    //MX_LocalToWorld_Set() = xMatrixTranslateT(P_center_Trfm.xyz);
}

void Hawk:: PostUpdate(float T_delta)
{
    Profile("PostUpdate");

    xVector3 A_change; A_change.zero();

    ////////////////////////// Follow birds
    {
        Profile("Hunt birds");
        DetermineVisibleBirds();
        if (VisibleBirds.size())
        {
            Bird    *bird  = VisibleBirds.front();
            xVector3 V_dir = bird->P_center_Trfm - P_center_Trfm;
            float    S_dir = V_dir.lengthSqr();

            ////////////////////////// Eat bird
            if (S_dir < Bird::Size_Sqr*4.f)
            {
                Hawk *hawk = this;
                pMap->OnBirdRemove(hawk, bird);
                ++BirdsEaten;

                Energy += EatEnergy;
                bird->Destroy(true);
                delete bird;
            }
            else
            {
                ////////////////////////// Try to move towards the nearest bird
                V_dir /= sqrt(S_dir);
                //if (xVector3::DotProduct(V_velocity, V_dir) < 0.8f * W_speed)
                    A_change += Hawk::FlockCenterAcceleration * V_dir;

                ////////////////////////// Try to move the same way the nearest bird does.
                A_change += Hawk::FlockNearestAcceleration * xVector3::Normalize(bird->V_velocity);
            }
        }
    }

    ////////////////////////// Ensure max acceleration
    float W_change_Sqr = A_change.lengthSqr();
    if (W_change_Sqr > Hawk::MaxAcceleration_Sqr)
        A_change *= Hawk::MaxAcceleration / sqrt(W_change_Sqr);

    V_velocity -= V_velocity * Hawk::SpeedDump * T_delta;
    xVector3 V_old = V_velocity;
    if (T_delta > 0.f)
    {
        float len   = V_initial_speed.lengthSqr();
        float scale = T_delta * 800.f * (1.f - 0.8f * len * Hawk::InitialSpeed_Sqr_Inv);
        //Energy -= A_change.length() * scale;
        V_velocity += A_change * scale;
        // Gravity
        if (A_change.z <= 0.f)
        {
            Gravity -= 10.f * T_delta;
            if (Gravity < -1000.f)
                Gravity = -1000.f;
        }
    }
    else
    {
        float len   = V_initial_speed.lengthSqr();
        float scale = T_delta * 800.f * (1.f - 0.8f * len * Hawk::InitialSpeed_Sqr_Inv);
        //Energy -= A_change.length() * scale;
        V_velocity += A_change * scale;
    }
    if (A_change.z > 0.f && Gravity < 0)
        Gravity = 0.f;

    ////////////////////////// Ensure max velocity
    W_change_Sqr = V_velocity.lengthSqr();
    if (W_change_Sqr > Hawk::MaxSpeed_Sqr)
        V_velocity *= Hawk::MaxSpeed / sqrt(W_change_Sqr);
    Energy -= (V_velocity - V_old).length();

    if (T_delta < 10.f)
        V_initial_speed -= V_initial_speed * Hawk::InitialSpeedDump * T_delta;
}

/////////////////////////////////////

void Hawk:: LoadConfigLine(const char *buffer)
{
    if (StartsWith(buffer, "FlockCenterAcceleration"))
    {
        float val;
        sscanf(buffer+24, "%f", &val);
        Hawk::FlockCenterAcceleration = val;
        if (Hawk::FlockCenterAcceleration < 0.f)
            Hawk::FlockCenterAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "FlockNearestAcceleration"))
    {
        float val;
        sscanf(buffer+25, "%f", &val);
        Hawk::FlockNearestAcceleration = val;
        if (Hawk::FlockNearestAcceleration < 0.f)
            Hawk::FlockNearestAcceleration = 0.f;
        return;
    }

    if (StartsWith(buffer, "EatEnergy"))
    {
        float val;
        sscanf(buffer+10, "%f", &val);
        Hawk::EatEnergy = val;
        if (Hawk::EatEnergy < EPSILON)
            Hawk::EatEnergy = 1000.f;
        return;
    }
    if (StartsWith(buffer, "InitialEnergy"))
    {
        float val;
        sscanf(buffer+14, "%f", &val);
        Hawk::InitialEnergy = val;
        if (Hawk::InitialEnergy < EPSILON)
            Hawk::InitialEnergy = 20000.f;
        return;
    }
    if (StartsWith(buffer, "InitialSpeedDump"))
    {
        float val;
        sscanf(buffer+17, "%f", &val);
        Hawk::InitialSpeedDump = val;
        if (Hawk::InitialSpeedDump < 0.f)
            Hawk::InitialSpeedDump = 0.f;
        else
        if (Hawk::InitialSpeedDump > 1.f)
            Hawk::InitialSpeedDump = 1.f;
        return;
    }
    if (StartsWith(buffer, "InitialSpeed"))
    {
        float val;
        sscanf(buffer+13, "%f", &val);
        Hawk::InitialSpeed = val;
        if (Hawk::InitialSpeed < EPSILON)
            Hawk::InitialSpeed = 400.f;

        Hawk::InitialSpeed_Sqr_Inv = 1.f / (Hawk::InitialSpeed * Hawk::InitialSpeed);
        return;
    }
    if (StartsWith(buffer, "SpeedDump"))
    {
        float val;
        sscanf(buffer+10, "%f", &val);
        Hawk::SpeedDump = val;
        if (Hawk::SpeedDump < 0.f)
            Hawk::SpeedDump = 0.f;
        else
        if (Hawk::SpeedDump > 1.f)
            Hawk::SpeedDump = 1.f;
        return;
    }

    if (StartsWith(buffer, "EyeDistance"))
    {
        float val;
        sscanf(buffer+12, "%f", &val);
        Hawk::EyeDistance = val;
        if (Hawk::EyeDistance < EPSILON)
            Hawk::EyeDistance = 50.f;

        Hawk::EyeDistance_Sqr = Hawk::EyeDistance * Hawk::EyeDistance;
        return;
    }
    if (StartsWith(buffer, "MaxSpeed"))
    {
        float val;
        sscanf(buffer+9, "%f", &val);
        Hawk::MaxSpeed = val;
        if (Hawk::MaxSpeed < EPSILON)
            Hawk::MaxSpeed = 80.f;

        Hawk::MaxSpeed_Sqr = Hawk::MaxSpeed * Hawk::MaxSpeed;
        return;
    }
    if (StartsWith(buffer, "MaxAcceleration"))
    {
        float val;
        sscanf(buffer+16, "%f", &val);
        Hawk::MaxAcceleration = val;
        if (Hawk::MaxAcceleration < EPSILON)
            Hawk::MaxAcceleration = 2.5f;

        Hawk::MaxAcceleration_Sqr = Hawk::MaxAcceleration * Hawk::MaxAcceleration;
        return;
    }
}
