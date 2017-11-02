#ifndef __incl_World_Map_h
#define __incl_World_Map_h

#include "Quarter.h"
#include "../Utils/Delegate.h"

namespace World {
    using namespace World;

    struct Map
    {
        static bool GenerateMap;       // Should the map be generated or just loaded
        static uint GenerateMapCols;   // Size of the generated map
        static uint GenerateMapRows;   // Size of the generated map
        static uint InitialPopulation; // Size of the initial bird population
        static bool Night;             // Evening or Night lighting

        static uint DistrictSize;      // Size ot the district (used for rendering optimizations)

        static void LoadConfigLine(const char *buffer);

    public:
        int   Cols, Rows;              // Number of buildings in the X and Y dimensions

        float MinX, MinY, MaxX, MaxY;  // Dimensions of the map (calculated from Cols, Rows and Quarter::SquareWidth)

        LstBirdP Birds;                // Population of birds
        LstHawkP Hawks;                // Population of hawks
        Quarter *Grid;                 // Array of building quarters

        Map() { Clear(); }

        void Clear();
        void Create();
        void Destroy();

        void Update(float T_delta);

        // Events that notify about Hawk->Bird conversions and Birds eaten by Hawks
        typedef Delegate<Map, Hawk*, Bird*> MapEvent;
        MapEvent OnHawkReplace;
        MapEvent OnBirdRemove;

        // Performs sweeped sphere collision test at given quarter. Returns collision plane
        bool GetNearestCollision(xPlane &PN_collision,
                                 float Size,
                                 World::Quarter &Quarter,
                                 xPoint3        &P_quarter_pos,
                                 xPoint3        &P_quarter_pos_Old);

    private:
        // Generate simple city with big main roads and main square
        static void GenerateToFile(const char *fileName);

        void LoadFromFile(const char *fileName);
        void Populate();
    };

} // namespace World

#endif
