#include "Bird.h"

#include "Map.h"
#include "Quarter.h"

#include "../Utils/Profiler.h"
#include "../Utils/Utils.h"

using namespace World;

float Bird:: EyeDistance          = 50.f;
float Bird:: EyeDistance_Sqr      = Bird::EyeDistance * Bird::EyeDistance;
float Bird:: EyeDistance_Inv      = 1.f / Bird::EyeDistance;
float Bird:: KeepAwayDistance     = 5.f;
float Bird:: KeepAwayDistance_Sqr = Bird::KeepAwayDistance * Bird::KeepAwayDistance;
float Bird:: KeepAwayDistance_Inv = 1.f / Bird::KeepAwayDistance;
float Bird:: DesiredSpeed         = 40.f;
float Bird:: MaxSpeed             = 60.f;
float Bird:: MaxSpeed_Sqr         = Bird::MaxSpeed * Bird::MaxSpeed;
float Bird:: MaxSpeed_Inv         = 1.f / Bird::MaxSpeed;
float Bird:: MaxAcceleration      = 2.5f;
float Bird:: MaxAcceleration_Sqr  = Bird::MaxAcceleration * Bird::MaxAcceleration;
float Bird:: MinAltitude          = 5.f;
float Bird:: MaxAltitude          = 80.f;
float Bird:: Size                 = 1.f;
float Bird:: Size_Sqr             = Bird::Size * Bird::Size;

float Bird:: CruiseAcceleration        = 0.5f;
float Bird:: MinAltitudeAcceleration   = 0.8f;
float Bird:: MaxAltitudeAcceleration   = 0.5f;
float Bird:: FlockCenterAcceleration   = 0.1f;
float Bird:: FlockNearestAcceleration  = 0.5f;
float Bird:: FlockKeepAwayAcceleration = 0.5f;
float Bird:: AvoidEnemyAcceleration    = 0.3f;
float Bird:: BounceSpeedRetention      = 0.4f;

VecBirdP Bird:: VisibleBirds(20);
VecHawkP Bird:: VisibleHawks(5);

/////////////////////////////////////

void Bird:: Create(World::Map &map, const xPoint3 &P_center, const xVector3 &V_velocity)
{
    this->Type     = OBJECT_BIRD;

    this->S_radius = Size;
    this->P_center.zero();

    this->P_center_Trfm = P_center;
    this->V_velocity    = V_velocity;

    pMap     = &map;
    pQuarter = NULL;
    map.Birds.push_back(this);

    DetermineCurrentPosition();
}

void Bird:: Destroy(bool FL_withRemove)
{
    if (pQuarter)
    {
        pQuarter->Birds[Index] = pQuarter->Birds.back();
        pQuarter->Birds[Index]->Index = Index;
        pQuarter->Birds.pop_back();
    }
    if (FL_withRemove)
        pMap->Birds.remove(this);
}

/////////////////////////////////////

void Bird:: SetCurrentQuarter(World::Quarter &quarter)
{
    if (pQuarter != &quarter)
    {
        // Remove from the old Quarter
        if (pQuarter)
        {
            pQuarter->Birds[Index] = pQuarter->Birds.back();
            pQuarter->Birds[Index]->Index = Index;
            pQuarter->Birds.pop_back();
        }
        // Insert into the new Quarter
        pQuarter = &quarter;
        Index = pQuarter->Birds.size();
        pQuarter->Birds.push_back(this);
    }
}

void Bird:: DetermineCurrentPosition()
{
    // If the bird flies away from map, teleport it to the oposite side of the city
    if (P_center_Trfm.x < pMap->MinX) P_center_Trfm.x = pMap->MaxX;
    if (P_center_Trfm.y < pMap->MinY) P_center_Trfm.y = pMap->MaxY;
    if (P_center_Trfm.x > pMap->MaxX) P_center_Trfm.x = pMap->MinX;
    if (P_center_Trfm.y > pMap->MaxY) P_center_Trfm.y = pMap->MinY;

    // Get position over the city
    float x = P_center_Trfm.x - pMap->MinX;
    float y = P_center_Trfm.y - pMap->MinY;

    // Get current Quarter's col and row
    Col = (int)(x * Quarter::SquareWidth_Inv);
    if (Col >= pMap->Cols) Col = pMap->Cols-1;
    Row = (int)(y * Quarter::SquareWidth_Inv);
    if (Row >= pMap->Rows) Row = pMap->Rows-1;

    // Get position over the current Quarter
    P_quarter_pos.x = x - Col * Quarter::SquareWidth;
    P_quarter_pos.y = y - Row * Quarter::SquareWidth;
    P_quarter_pos.z = P_center_Trfm.z;
    P_quarter_pos_Old = P_quarter_pos;

    Row = pMap->Rows - Row - 1;
    SetCurrentQuarter(pMap->Grid[Row * pMap->Cols + Col]);
}

void Bird:: UpdateCurrentPosition()
{
    bool FL_changed = false;

    while (P_quarter_pos.x < 0)
    {
        if (Col > 0)
        {
            --Col;
            P_quarter_pos.x     += Quarter::SquareWidth;
            P_quarter_pos_Old.x += Quarter::SquareWidth;
        }
        else
        {
            Col = pMap->Cols - 1;
            P_center_Trfm.x     += pMap->MaxX - pMap->MinX;
            P_quarter_pos.x     += Quarter::SquareWidth;
            P_quarter_pos_Old.x += Quarter::SquareWidth;
        }
        FL_changed = true;
    }
    while (P_quarter_pos.x > Quarter::SquareWidth)
    {
        if (Col < pMap->Cols - 1)
        {
            ++Col;
            P_quarter_pos.x     -= Quarter::SquareWidth;
            P_quarter_pos_Old.x -= Quarter::SquareWidth;
        }
        else
        {
            Col = 0;
            P_center_Trfm.x     -= pMap->MaxX - pMap->MinX;
            P_quarter_pos.x     -= Quarter::SquareWidth;
            P_quarter_pos_Old.x -= Quarter::SquareWidth;
        }
        FL_changed = true;
    }
    while (P_quarter_pos.y < 0)
    {
        if (Row < pMap->Rows-1)
        {
            ++Row;
            P_quarter_pos.y     += Quarter::SquareWidth;
            P_quarter_pos_Old.y += Quarter::SquareWidth;
        }
        else
        {
            Row = 0;
            P_center_Trfm.y     += pMap->MaxX - pMap->MinX;
            P_quarter_pos.y     += Quarter::SquareWidth;
            P_quarter_pos_Old.y += Quarter::SquareWidth;
        }
        FL_changed = true;
    }
    while (P_quarter_pos.y > Quarter::SquareWidth)
    {
        if (Row > 0)
        {
            --Row;
            P_quarter_pos.y     -= Quarter::SquareWidth;
            P_quarter_pos_Old.y -= Quarter::SquareWidth;
        }
        else
        {
            Row = pMap->Rows - 1;
            P_center_Trfm.y     -= pMap->MaxX - pMap->MinX;
            P_quarter_pos.y     -= Quarter::SquareWidth;
            P_quarter_pos_Old.y -= Quarter::SquareWidth;
        }
        FL_changed = true;
    }

    if (FL_changed)
        SetCurrentQuarter(pMap->Grid[Row * pMap->Cols + Col]);
}

/////////////////////////////////////

void Bird:: DetermineVisibleBirdsOnNearbyQuarter(const World::Quarter &quarter,
                                                 float EyeDistance_Sqr,
                                                 int &I_minDist, float &S_minDist_Sqr)
{
    VecBirdP::const_iterator BRD_curr = quarter.Birds.begin(),
                             BRD_last = quarter.Birds.end();
    for(; BRD_curr != BRD_last; ++BRD_curr)
    {
        float S_distance_Sqr = ((**BRD_curr).P_center_Trfm - P_center_Trfm).lengthSqr();
        if (S_distance_Sqr < EyeDistance_Sqr)
        {
            if (S_minDist_Sqr > S_distance_Sqr)
            {
                S_minDist_Sqr = S_distance_Sqr;
                I_minDist     = VisibleBirds.size();
            }
            VisibleBirds.push_back(*BRD_curr);
        }
    }
}

void Bird:: DetermineVisibleBirds()
{
    VisibleBirds.clear();

    float S_minDist_Sqr = xFLOAT_HUGE_POSITIVE;
    int   I_minDist     = 0;

    // Check current Quarter
    VecBirdP::const_iterator BRD_curr = pQuarter->Birds.begin(),
                             BRD_last = pQuarter->Birds.end();
    for(; BRD_curr != BRD_last; ++BRD_curr)
        if (*BRD_curr != this)
        {
            Bird &bird = **BRD_curr;
            float S_distance_Sqr = (bird.P_center_Trfm - P_center_Trfm).lengthSqr();

            if (S_distance_Sqr < Bird::EyeDistance_Sqr)
            {
                if (S_minDist_Sqr > S_distance_Sqr)
                {
                    S_minDist_Sqr = S_distance_Sqr;
                    I_minDist     = VisibleBirds.size();
                }
                VisibleBirds.push_back(*BRD_curr);
            }
        }

    // Check nearby Quarters
    // Check to the W
    if (Col > 0)
    {
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NW
        if (Row > 0)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SW
        if (Row < pMap->Rows-1)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the E
    if (Col < pMap->Cols-1)
    {
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NE
        if (Row > 0)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SE
        if (Row < pMap->Rows-1)
            DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the N
    if (Row > 0)
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter-pMap->Cols), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    // Check to the S
    if (Row < pMap->Rows-1)
        DetermineVisibleBirdsOnNearbyQuarter(*(pQuarter+pMap->Cols), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);

    // Move closest bird to the first position
    if (I_minDist > 0)
    {
        Bird *swap              = VisibleBirds.front();
        VisibleBirds.front()    = VisibleBirds[I_minDist];
        VisibleBirds[I_minDist] = swap;
    }
}

void Bird:: DetermineVisibleHawksOnNearbyQuarter(const World::Quarter &quarter,
                                                 float EyeDistance_Sqr,
                                                 int &I_minDist, float &S_minDist_Sqr)
{
    VecHawkP::const_iterator BRD_curr = quarter.Hawks.begin(),
                             BRD_last = quarter.Hawks.end();
    for(; BRD_curr != BRD_last; ++BRD_curr)
    {
        float S_distance_Sqr = ((**BRD_curr).P_center_Trfm - P_center_Trfm).lengthSqr();
        if (S_distance_Sqr < EyeDistance_Sqr)
        {
            if (S_minDist_Sqr > S_distance_Sqr)
            {
                S_minDist_Sqr = S_distance_Sqr;
                I_minDist     = VisibleHawks.size();
            }
            VisibleHawks.push_back(*BRD_curr);
        }
    }
}

void Bird:: DetermineVisibleHawks()
{
    VisibleHawks.clear();

    float S_minDist_Sqr = xFLOAT_HUGE_POSITIVE;
    int   I_minDist     = 0;

    // Check current Quarter
    DetermineVisibleHawksOnNearbyQuarter(*pQuarter, Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);

    // Check nearby Quarters
    // Check to the W
    if (Col > 0)
    {
        DetermineVisibleHawksOnNearbyQuarter(*(pQuarter-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NW
        if (Row > 0)
            DetermineVisibleHawksOnNearbyQuarter(*(pQuarter-pMap->Cols-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SW
        if (Row < pMap->Rows-1)
            DetermineVisibleHawksOnNearbyQuarter(*(pQuarter+pMap->Cols-1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the E
    if (Col < pMap->Cols-1)
    {
        DetermineVisibleHawksOnNearbyQuarter(*(pQuarter+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the NE
        if (Row > 0)
            DetermineVisibleHawksOnNearbyQuarter(*(pQuarter-pMap->Cols+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
        // Check to the SE
        if (Row < pMap->Rows-1)
            DetermineVisibleHawksOnNearbyQuarter(*(pQuarter+pMap->Cols+1), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    }
    // Check to the N
    if (Row > 0)
        DetermineVisibleHawksOnNearbyQuarter(*(pQuarter-pMap->Cols), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);
    // Check to the S
    if (Row < pMap->Rows-1)
        DetermineVisibleHawksOnNearbyQuarter(*(pQuarter+pMap->Cols), Bird::EyeDistance_Sqr, I_minDist, S_minDist_Sqr);

    // Move closest bird to the first position
    if (I_minDist > 0)
    {
        Hawk *swap              = VisibleHawks.front();
        VisibleHawks.front()    = VisibleHawks[I_minDist];
        VisibleHawks[I_minDist] = swap;
    }
}

/////////////////////////////////////

bool Bird:: GetNearestCollision(xPlane &PN_collision)
{
    return pMap->GetNearestCollision(PN_collision, Bird::Size, *pQuarter, P_quarter_pos, P_quarter_pos_Old);
}

void Bird:: ProcessCollisions()
{
    Profile("Collision detection & responce");

    xPlane PN_collision;

    while ( GetNearestCollision(PN_collision) )
    {
        if (PN_collision.z != 0.f)  // ground/roof bounce
        {
            P_quarter_pos.z   = 2.f * PN_collision.w - P_quarter_pos.z;
            V_velocity.z      = -V_velocity.z;
        }
        else
        if (PN_collision.x != 0.f) // west/east wall bounce
        {
            P_quarter_pos.x   = 2.f * PN_collision.w - P_quarter_pos.x;
            V_velocity.x      = -V_velocity.x;
        }
        else
        if (PN_collision.y != 0.f) // north/south wall bounce
        {
            P_quarter_pos.y   = 2.f * PN_collision.w - P_quarter_pos.y;
            V_velocity.y      = -V_velocity.y;
        }
        V_velocity      *= Bird::BounceSpeedRetention;

        UpdateCurrentPosition();
    }
    P_center_Trfm.x = Col * Quarter::SquareWidth + P_quarter_pos.x + pMap->MinX;
    P_center_Trfm.y = (pMap->Rows-1-Row) * Quarter::SquareWidth + P_quarter_pos.y + pMap->MinY;
    P_center_Trfm.z = P_quarter_pos.z;
}

/////////////////////////////////////

void Bird:: Update(float T_delta)
{
    Profile("Update");

    // Upadate position
    P_quarter_pos_Old  = P_quarter_pos;
    xVector3 V_shift   = V_velocity * T_delta;
    P_center_Trfm     += V_shift;
    P_quarter_pos     += V_shift;

    // Update current Quarter
    UpdateCurrentPosition();
    ProcessCollisions();

    // No rotations - just tranlation is enough
    //MX_LocalToWorld_Set() *= xMatrixTranslateT(P_center_Trfm.xyz);
    //MX_LocalToWorld_Set() = xMatrixTranslateT(P_center_Trfm.xyz);
}

void Bird:: PostUpdate(float T_delta)
{
    Profile("PostUpdate");

    xVector3 A_change; A_change.zero();

    ////////////////////////// Maintaint cruising speed
    {
        float    W_speed  = V_velocity.length();
        xVector3 V_cruise = V_velocity;
        float    W_diff   = (W_speed - Bird::DesiredSpeed) * Bird::MaxSpeed_Inv;
        float    W_urgency = fabs(W_diff) * Bird::MaxAcceleration;
        if (W_urgency < Bird::CruiseAcceleration) W_urgency = Bird::CruiseAcceleration;

        int jitter = rand() % 100;
        if (jitter < 40)
            if (V_cruise.x * Sign(W_diff) > 0)
                V_cruise.x -= Bird::CruiseAcceleration;
            else
                V_cruise.x += Bird::CruiseAcceleration;
        else
        if (jitter < 80)
            if (V_cruise.y * Sign(W_diff) > 0)
                V_cruise.y -= Bird::CruiseAcceleration;
            else
                V_cruise.y += Bird::CruiseAcceleration;
        else
           if (V_cruise.z * Sign(W_diff) > 0)
                V_cruise.z -= Bird::CruiseAcceleration;
            else
                V_cruise.z += Bird::CruiseAcceleration;

        A_change += V_cruise * ( W_urgency * (W_diff > 0 ? -1 : 1) / W_speed ) ;
    }

    ////////////////////////// Keep distance to buildings
    int   xCell, yCell;
    float xDist, yDist;

    {
        // Determine distance to the nearest building walls, cells are marked as:
        // -1 -1 | 0 -1 | 1 -1
        // -------------------
        // -1  0 | 0  0 | 1  0  // 0 0 = building
        // -------------------
        // -1  1 | 0  1 | 1  1
        // X
        if (P_quarter_pos.x < Quarter::MarginWidth)
        { xCell = -1; xDist = Quarter::MarginWidth - P_quarter_pos.x; }
        else
        if (P_quarter_pos.x > Quarter::MarginWidth + Quarter::WallWidth)
        { xCell = 1; xDist = P_quarter_pos.x - Quarter::MarginWidth - Quarter::WallWidth; }
        else
        { xCell = 0; xDist = 0; }
        // Y
        if (P_quarter_pos.y < Quarter::MarginWidth)
        { yCell = -1; yDist = Quarter::MarginWidth - P_quarter_pos.y; }
        else
        if (P_quarter_pos.y > Quarter::MarginWidth + Quarter::WallWidth)
        { yCell = 1; yDist = P_quarter_pos.y - Quarter::MarginWidth - Quarter::WallWidth; }
        else
        { yCell = 0; yDist = 0; }

        // If the bird flies below the top of the building, it must avoid collision with walls
        if (P_quarter_pos.z < pQuarter->Height)
        {
            if (xCell == 0 && yCell == 0)
            {} // The bird broke a window ;P
            else
            if (xCell == 0 && yDist < Bird::KeepAwayDistance)
                A_change.y += yCell * Bird::MaxAcceleration * (1 - yDist * Bird::KeepAwayDistance_Inv);
            else
            if (yCell == 0 && xDist < Bird::KeepAwayDistance)
                A_change.x += xCell * Bird::MaxAcceleration * (1 - xDist * Bird::KeepAwayDistance_Inv);
            else
            if (xDist * xDist + yDist * yDist < Bird::KeepAwayDistance_Sqr)
            {
                if (fabs(V_velocity.x) > fabs(V_velocity.y))
                    A_change.y += yCell * Bird::MaxAcceleration * (1 - yDist * Bird::KeepAwayDistance_Inv);
                else
                    A_change.x += xCell * Bird::MaxAcceleration * (1 - xDist * Bird::KeepAwayDistance_Inv);
            }
        }
    }

    ////////////////////////// Keep distance to the ground
    {
        float S_toLowAlt = P_center_Trfm.z - Bird::MinAltitude;
        // If over the building, keep distance to the roof
        if (xCell == 0 && yCell == 0)
            S_toLowAlt -= pQuarter->Height;
        if (S_toLowAlt < 0)
            A_change.z += Bird::MinAltitudeAcceleration;
        else
        if (S_toLowAlt < Bird::KeepAwayDistance && (V_velocity.z < 0.f/* || A_change.z < 0.f*/))
            A_change.z += Bird::MinAltitudeAcceleration * (1 - S_toLowAlt * Bird::KeepAwayDistance_Inv);
    }
    ////////////////////////// Keep distance to the sky
    {
        float S_toTopAlt = Bird::MaxAltitude - P_center_Trfm.z;
        if (S_toTopAlt < 0)
            A_change.z -= Bird::MaxAltitudeAcceleration;
        else
        if (S_toTopAlt < Bird::KeepAwayDistance && (V_velocity.z > 0.f/* || A_change.z > 0.f*/))
            A_change.z -= Bird::MaxAltitudeAcceleration * (1 - S_toTopAlt * Bird::KeepAwayDistance_Inv);
    }

    // Flock behaviour
    {
        Profile("Flock behaviour");
        DetermineVisibleBirds();
        if (VisibleBirds.size())
        {
            ////////////////////////// Try to move towards the center of the flock.
            VecBirdP::iterator BRD_curr = VisibleBirds.begin(),
                               BRD_last = VisibleBirds.end();

            xPoint3 P_mass_center; P_mass_center.zero();
            for(; BRD_curr != BRD_last; ++BRD_curr)
                P_mass_center += (**BRD_curr).P_center_Trfm;
            P_mass_center /= (float)VisibleBirds.size();
            A_change += Bird::FlockCenterAcceleration * (P_mass_center - P_center_Trfm).normalize();

            ////////////////////////// Try to move the same way our nearest flockmate does.
            A_change += Bird::FlockNearestAcceleration * xVector3::Normalize(VisibleBirds.front()->V_velocity);

            ////////////////////////// Try to maintain our desired separation distance from our nearest flockmate.
            xVector3 V_keepAway     = P_center_Trfm - VisibleBirds.front()->P_center_Trfm;
            float    W_keepAway_Sqr = V_keepAway.lengthSqr();
            if (W_keepAway_Sqr < Bird::KeepAwayDistance_Sqr)
            {
                float W_keepAway = sqrt(W_keepAway_Sqr);
                float W_scale = Bird::MaxAcceleration * (1.f - W_keepAway * Bird::KeepAwayDistance_Inv);
                if (W_scale < Bird::FlockKeepAwayAcceleration) W_scale = Bird::FlockKeepAwayAcceleration;
                A_change += V_keepAway  * (W_scale * 2.f / W_keepAway);
            }
        }
    }
    {
        Profile("Hawk avoidance");
        DetermineVisibleHawks();
        if (VisibleHawks.size())
        {
            ////////////////////////// Try to avoid enemies
            xVector3 V_dir   = P_center_Trfm - VisibleHawks.front()->P_center_Trfm;
            float    S_dir   = V_dir.length();
            float    W_scale = Bird::AvoidEnemyAcceleration * (1.f - S_dir * Bird::EyeDistance_Inv);
            A_change += W_scale * V_dir;
        }
    }

    ////////////////////////// Ensure max acceleration
    float W_change_Sqr = A_change.lengthSqr();
    if (W_change_Sqr > Bird::MaxAcceleration_Sqr)
        A_change *= Bird::MaxAcceleration / sqrt(W_change_Sqr);

    if (T_delta > 0.f)
        V_velocity += A_change * T_delta * 800.f;
    else
        V_velocity += A_change;

    ////////////////////////// Ensure max velocity
    W_change_Sqr = V_velocity.lengthSqr();
    if (W_change_Sqr > Bird::MaxSpeed_Sqr)
        V_velocity *= Bird::MaxSpeed / sqrt(W_change_Sqr);
}

/////////////////////////////////////

void Bird:: LoadConfigLine(const char *buffer)
{
    if (StartsWith(buffer, "CruiseAcceleration"))
    {
        float val;
        sscanf(buffer+19, "%f", &val);
        Bird::CruiseAcceleration = val;
        if (Bird::CruiseAcceleration < 0.f)
            Bird::CruiseAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "MinAltitudeAcceleration"))
    {
        float val;
        sscanf(buffer+24, "%f", &val);
        Bird::MinAltitudeAcceleration = val;
        if (Bird::MinAltitudeAcceleration < 0.f)
            Bird::MinAltitudeAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "MaxAltitudeAcceleration"))
    {
        float val;
        sscanf(buffer+24, "%f", &val);
        Bird::MaxAltitudeAcceleration = val;
        if (Bird::MaxAltitudeAcceleration < 0.f)
            Bird::MaxAltitudeAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "FlockCenterAcceleration"))
    {
        float val;
        sscanf(buffer+24, "%f", &val);
        Bird::FlockCenterAcceleration = val;
        if (Bird::FlockCenterAcceleration < 0.f)
            Bird::FlockCenterAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "FlockNearestAcceleration"))
    {
        float val;
        sscanf(buffer+25, "%f", &val);
        Bird::FlockNearestAcceleration = val;
        if (Bird::FlockNearestAcceleration < 0.f)
            Bird::FlockNearestAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "FlockKeepAwayAcceleration"))
    {
        float val;
        sscanf(buffer+26, "%f", &val);
        Bird::FlockKeepAwayAcceleration = val;
        if (Bird::FlockKeepAwayAcceleration < 0.f)
            Bird::FlockKeepAwayAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "AvoidEnemyAcceleration"))
    {
        float val;
        sscanf(buffer+23, "%f", &val);
        Bird::AvoidEnemyAcceleration = val;
        if (Bird::AvoidEnemyAcceleration < 0.f)
            Bird::AvoidEnemyAcceleration = 0.f;
        return;
    }
    if (StartsWith(buffer, "BounceSpeedRetention"))
    {
        float val;
        sscanf(buffer+21, "%f", &val);
        Bird::BounceSpeedRetention = val;
        if (Bird::BounceSpeedRetention < 0.f)
            Bird::BounceSpeedRetention = 0.f;
        else
        if (Bird::BounceSpeedRetention > 1.f)
            Bird::BounceSpeedRetention = 1.f;
        return;
    }

    if (StartsWith(buffer, "EyeDistance"))
    {
        float val;
        sscanf(buffer+12, "%f", &val);
        Bird::EyeDistance = val;
        if (Bird::EyeDistance < EPSILON)
            Bird::EyeDistance = 50.f;

        Bird::EyeDistance_Sqr = Bird::EyeDistance * Bird::EyeDistance;
        Bird::EyeDistance_Inv = 1.f / Bird::EyeDistance;
        return;
    }
    if (StartsWith(buffer, "KeepAwayDistance"))
    {
        float val;
        sscanf(buffer+17, "%f", &val);
        Bird::KeepAwayDistance = val;
        if (Bird::KeepAwayDistance < EPSILON)
            Bird::KeepAwayDistance = 5.f;

        Bird::KeepAwayDistance_Sqr = Bird::KeepAwayDistance * Bird::KeepAwayDistance;
        Bird::KeepAwayDistance_Inv = 1.f / Bird::KeepAwayDistance;
        return;
    }
    if (StartsWith(buffer, "MaxSpeed"))
    {
        float val;
        sscanf(buffer+9, "%f", &val);
        Bird::MaxSpeed = val;
        if (Bird::MaxSpeed < EPSILON)
            Bird::MaxSpeed = 60.f;

        if (Bird::MaxSpeed < Bird::DesiredSpeed)
            Bird::DesiredSpeed = Bird::MaxSpeed * 0.6f;

        Bird::MaxSpeed_Sqr = Bird::MaxSpeed * Bird::MaxSpeed;
        Bird::MaxSpeed_Inv = 1.f / Bird::MaxSpeed;
        return;
    }
    if (StartsWith(buffer, "DesiredSpeed"))
    {
        float val;
        sscanf(buffer+13, "%f", &val);
        Bird::DesiredSpeed = val;
        if (Bird::DesiredSpeed < EPSILON)
            Bird::DesiredSpeed = 40.f;

        if (Bird::MaxSpeed < Bird::DesiredSpeed)
        {
            Bird::MaxSpeed = Bird::DesiredSpeed * 1.5f;
            Bird::MaxSpeed_Sqr = Bird::MaxSpeed * Bird::MaxSpeed;
            Bird::MaxSpeed_Inv = 1.f / Bird::MaxSpeed;
        }
        return;
    }
    if (StartsWith(buffer, "MaxAcceleration"))
    {
        float val;
        sscanf(buffer+16, "%f", &val);
        Bird::MaxAcceleration = val;
        if (Bird::MaxAcceleration < EPSILON)
            Bird::MaxAcceleration = 2.5f;

        Bird::MaxAcceleration_Sqr = Bird::MaxAcceleration * Bird::MaxAcceleration;
        return;
    }
    if (StartsWith(buffer, "MinAltitude"))
    {
        float val;
        sscanf(buffer+12, "%f", &val);
        Bird::MinAltitude = val;
        if (Bird::MinAltitude < 0.f)
            Bird::MinAltitude = 0.f;
        return;
    }
    if (StartsWith(buffer, "MaxAltitude"))
    {
        float val;
        sscanf(buffer+12, "%f", &val);
        Bird::MaxAltitude = val;
        if (Bird::MaxAltitude < 0.f)
            Bird::MaxAltitude = 0.f;
        return;
    }
    if (StartsWith(buffer, "Size"))
    {
        float val;
        sscanf(buffer+5, "%f", &val);
        Bird::Size = val;
        if (Bird::Size < EPSILON)
            Bird::Size = 1.f;

        Bird::Size_Sqr = Bird::Size * Bird::Size;
        return;
    }
}
