#ifndef __incl_World_Quarter_h
#define __incl_World_Quarter_h

#include "Hawk.h"

namespace World {
    using namespace World;

    struct Quarter
    {
        static float SquareWidth;     // Width of the quarter square
        static float SquareWidth_Inv; // 1/SquareWidth
        static float WallWidth;       // Width of the buildings
        static float MarginWidth;     // (SquareWidth - WallWidth) * 0.5f
        static float MaxHeight;       // Used for generation of the map, replaced with max value from loaded map

        static void LoadConfigLine(const char *buffer);

    public:
        float    Height; // Height of the building
        xColor3b Color;  // Color of the building
        VecBirdP Birds;  // Birds that are currently flying over the quarter
        VecHawkP Hawks;  // Hawks that are currently flying over the quarter
    };

} // namespace World

#endif
