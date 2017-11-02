#include "Map.h"
#include <string>
#include <fstream>

#include "../Utils/Filesystem.h"
#include "../Utils/Utils.h"

using namespace World;

bool Map:: GenerateMap       = true;
uint Map:: GenerateMapCols   = 50;
uint Map:: GenerateMapRows   = 50;
uint Map:: InitialPopulation = 1000;
bool Map:: Night             = false;
uint Map:: DistrictSize      = 10;

/////////////////////////////////////

struct RowLoader
{
    const char *Pos;
    const char *End;

    void Init(const char* text)
    {
        Pos = text;
        End = text + strlen(text);
        while (*Pos == ';' || *Pos == ',' || *Pos == ' ' || *Pos == '\t') ++Pos;
    }

    float GetNextFloat()
    {
        char *stop;
        float value = strtod(Pos, &stop);
        Pos = stop;
        while (*Pos == ';' || *Pos == ',' || *Pos == ' ' || *Pos == '\t') ++Pos;
        return value;
    }
};

/////////////////////////////////////

void Map:: Clear()
{
    Grid = 0;
    Rows = Cols = 0;
    MinX = MinY = MaxX = MaxY = 0.f;
    Birds.clear();
    Hawks.clear();
    OnHawkReplace = MapEvent(*this);
    OnBirdRemove  = MapEvent(*this);
}

void Map:: Create()
{
    if (GenerateMap)
        GenerateToFile("Data/map.txt");

    LoadFromFile("Data/map.txt");
    Populate();
}

void Map:: Destroy()
{
    if (Grid) delete[] Grid;

    LstBirdP::iterator BRD_curr = Birds.begin(),
                       BRD_last = Birds.end();
    for(; BRD_curr != BRD_last; ++BRD_curr)
        delete *BRD_curr;

    LstHawkP::iterator HWK_curr = Hawks.begin(),
                       HWK_last = Hawks.end();
    for(; HWK_curr != HWK_last; ++HWK_curr)
        delete *HWK_curr;

    Clear();
}

/////////////////////////////////////

// Generate simple city with big main roads and main square
void Map:: GenerateToFile(const char *fileName)
{
    std::ofstream out(Filesystem::GetFullPath(fileName).c_str());
    if (out.is_open())
    {
        int CenterX = GenerateMapCols >> 1;
        int CenterY = GenerateMapRows >> 1;

        for (int rows = GenerateMapRows; rows; --rows)
        {
            if (rows % 5 == 0)
                for (int cols = GenerateMapCols; cols; --cols)
                    out << "0\t";
            else
                for (int cols = GenerateMapCols; cols; --cols)
                    if (cols % 5 == 0 || (
                        (cols > CenterX - 3 && cols < CenterX + 3) &&
                        (rows > CenterY - 3 && rows < CenterY + 3) ) )
                        out << "0\t";
                    else
                    {
                        float Dst = 1.f - 0.5f * max(abs(CenterX - cols)/(float)CenterX, abs(CenterY - rows)/(float)CenterY);
                        int   Max = (int) (Dst * Quarter::MaxHeight * 0.5f);
                        out << 2.f * (rand() % Max) << "\t"; // randf() * Quarter::MaxHeight << "\t"; // Make height a multiplicity of the texture height
                    }
            out << std::endl;
        }
        out.close();
    }
}

void Map:: LoadFromFile(const char *fileName)
{
    Clear();
    Quarter::MaxHeight = 0; // Will be filled with max value in  the file

    std::ifstream in(Filesystem::GetFullPath(fileName).c_str());
    if (in.is_open())
    {
        std::string buffer;
        RowLoader   rowLoader;

        // Temporary data storage (we don't know size of the map)
        typedef std::vector<Quarter>    VecQuarter;
        typedef std::vector<VecQuarter> VecGrid;
        VecGrid     grid;
        VecQuarter *row;

        // First line - unknown length
        if (in.good())
        {
            grid.resize(++Rows);
            row = & grid.back();

            getline(in, buffer);
            rowLoader.Init(buffer.c_str());
            while (rowLoader.Pos != rowLoader.End)
            {
                row->resize(++Cols);

                row->back().Height = rowLoader.GetNextFloat();
                row->back().Color.init(128 + rand() % 128, 128 + rand() % 128, 128 + rand() % 128);

                if (Quarter::MaxHeight < row->back().Height)
                    Quarter::MaxHeight = row->back().Height;
            }
        }

        // Other lines - known length, unknown number
        while (in.good())
        {
            grid.resize(++Rows);
            row = & grid.back();
            row->resize(Cols);

            getline(in, buffer);
            rowLoader.Init(buffer.c_str());

            VecQuarter::iterator QT_curr = row->begin(),
                                 QT_last = row->end();
            for (; QT_curr != QT_last; ++QT_curr)
            {
                if(rowLoader.Pos == rowLoader.End) // Not enough cols in one of the rows, skip the row
                { row->resize(0); break; }

                QT_curr->Height = rowLoader.GetNextFloat();
                QT_curr->Color.init(128 + rand() % 128, 128 + rand() % 128, 128 + rand() % 128);

                if (Quarter::MaxHeight < QT_curr->Height)
                    Quarter::MaxHeight = QT_curr->Height;
            }

            if (!row->size()) grid.resize(--Rows);
        }

        // copy quarters to final map
        Quarter *GR_curr = Grid = new Quarter[Cols*Rows];

        VecGrid::iterator RW_curr = grid.begin(),
                          RW_last = grid.end();
        for(; RW_curr != RW_last; ++RW_curr)
        {
            VecQuarter::iterator QT_curr = RW_curr->begin(),
                                 QT_last = RW_curr->end();
            for (; QT_curr != QT_last; ++QT_curr, ++GR_curr)
                *GR_curr = *QT_curr;
        }
        in.close();
    }

    float xSpan = Cols * Quarter::SquareWidth * 0.5f;
    MinX = -xSpan; MaxX = xSpan;
    float ySpan = Rows * Quarter::SquareWidth * 0.5f;
    MinY = -ySpan; MaxY = ySpan;
}

/////////////////////////////////////

void Map:: Populate()
{
    for (unsigned int i = 0; i < InitialPopulation; ++i)
    {
        int col = 1 + rand() % (Cols-1);
        int row = 1 + rand() % (Rows-1);

        // Get random position near selected row and col
        xPoint3 P_center; //P_center.zero();
        P_center.x = MinX + col * Quarter::SquareWidth + Quarter::MarginWidth * (1.f - 2.f * randf());
        P_center.y = MinY + row * Quarter::SquareWidth + Quarter::MarginWidth * (1.f - 2.f * randf());
        P_center.z = randf() * Quarter::MaxHeight;

        // Get random velocity vector
        xVector3 V_velocity;
        V_velocity.init(Bird::MaxSpeed * ( 1.0f - 2.0f * randf()),
                        Bird::MaxSpeed * ( 1.0f - 2.0f * randf()),
                        Bird::MaxSpeed * ( 0.1f - 0.2f * randf()));
        float W_velocity = V_velocity.length();
        // Scale velocity to DesiredSpeed
        if (W_velocity > 0.f)
            V_velocity *= Bird::DesiredSpeed / W_velocity;

        // Create a bird
        Bird *bird = new Bird();
        bird->Create(*this, P_center, V_velocity);
    }
}

/////////////////////////////////////

bool Map:: GetNearestCollision(xPlane &PN_collision,
                               float Size,
                               World::Quarter &Quarter,
                               xPoint3        &P_quarter_pos,
                               xPoint3        &P_quarter_pos_Old)
{
    xVector3 V_ray = P_quarter_pos - P_quarter_pos_Old;

    if (Quarter.Height == 0.f)
    {
        // Check collision with the Ground
        if (P_quarter_pos_Old.z >= Size &&
            P_quarter_pos.z     <  Size)
        {
            float t = (Quarter.Height + Size - P_quarter_pos_Old.z) / V_ray.z;
            P_quarter_pos_Old = P_quarter_pos_Old + t * V_ray;

            PN_collision.vector3.init(0.f,0.f,1.f);
            PN_collision.w = Size;
            return true;
        }
        return false;
    }

    const float minMargin = Quarter::MarginWidth - Size;
    const float maxMargin = Quarter::SquareWidth - minMargin;

    xPoint3 P_quarter_pos_Old_New = P_quarter_pos_Old;
    float   time = 100.f;

    // Check collision with the Roof
    if (P_quarter_pos_Old.z >= Quarter.Height + Size &&
        P_quarter_pos.z     <  Quarter.Height + Size)
    {
        float t = (Quarter.Height + Size - P_quarter_pos_Old.z) / V_ray.z;
        xPoint3 P_ray = P_quarter_pos_Old + t * V_ray;

        if (P_ray.x > minMargin && P_ray.x < maxMargin &&
            P_ray.y > minMargin && P_ray.y < maxMargin)
        {
            time = t;
            PN_collision.vector3.init(0.f,0.f,1.f);
            PN_collision.w        = Quarter.Height + Size;
            P_quarter_pos_Old_New = P_ray;
        }
    }
    // Check collision with the Ground (if no collision with the roof)
    if (time == 100.f &&
        P_quarter_pos_Old.z >= Size &&
        P_quarter_pos.z     < Size)
    {
        time = (Size - P_quarter_pos_Old.z) / V_ray.z;
        PN_collision.vector3.init(0.f,0.f,1.f);
        PN_collision.w = Size;
    }

    // Check collision with the West wall
    if (P_quarter_pos_Old.x <= minMargin &&
        P_quarter_pos.x     >  minMargin)
    {
        float t = (minMargin - P_quarter_pos_Old.x) / V_ray.x;
        xPoint3 P_ray = P_quarter_pos_Old + t * V_ray;

        if (t < time && P_ray.z < Quarter.Height && P_ray.y > minMargin && P_ray.y < maxMargin)
        {
            time = t;
            PN_collision.vector3.init(-1.f,0.f,0.f);
            PN_collision.w        = minMargin;
            P_quarter_pos_Old_New = P_ray;
        }
    }
    else
    // Check collision with the East wall
    if (P_quarter_pos_Old.x >= maxMargin &&
        P_quarter_pos.x     <  maxMargin)
    {
        float t = (maxMargin - P_quarter_pos_Old.x) / V_ray.x;
        xPoint3 P_ray = P_quarter_pos_Old + t * V_ray;

        if (t < time && P_ray.z < Quarter.Height && P_ray.y > minMargin && P_ray.y < maxMargin)
        {
            time = t;
            PN_collision.vector3.init(1.f,0.f,0.f);
            PN_collision.w        = maxMargin;
            P_quarter_pos_Old_New = P_ray;
        }
    }

    // Check collision with the South wall
    if (P_quarter_pos_Old.y <= minMargin &&
        P_quarter_pos.y     >  minMargin)
    {
        float t = (minMargin - P_quarter_pos_Old.y) / V_ray.y;
        xPoint3 P_ray = P_quarter_pos_Old + t * V_ray;

        if (t < time && P_ray.z < Quarter.Height && P_ray.x > minMargin && P_ray.x < maxMargin)
        {
            time = t;
            PN_collision.vector3.init(0.f,-1.f,0.f);
            PN_collision.w        = minMargin;
            P_quarter_pos_Old_New = P_ray;
        }
    }
    else
    // Check collision with the North wall
    if (P_quarter_pos_Old.y >= maxMargin &&
        P_quarter_pos.y     <  maxMargin)
    {
        float t = (maxMargin - P_quarter_pos_Old.y) / V_ray.y;
        xPoint3 P_ray = P_quarter_pos_Old + t * V_ray;

        if (t < time && P_ray.z < Quarter.Height && P_ray.x > minMargin && P_ray.x < maxMargin)
        {
            time = t;
            PN_collision.vector3.init(0.f,1.f,0.f);
            PN_collision.w        = maxMargin;
            P_quarter_pos_Old_New = P_ray;
        }
    }

    if (time < 100.f)
    {
        P_quarter_pos_Old = P_quarter_pos_Old_New;
        return true;
    }
    return false;
}

void Map:: Update(float T_delta)
{
    LstBirdP::iterator BRD_curr, BRD_last = Birds.end();
    LstHawkP::iterator HWK_curr, HWK_last = Hawks.end();

    // Update birds and hawks
    for(BRD_curr = Birds.begin(); BRD_curr != BRD_last; ++BRD_curr)
        (*BRD_curr)->Update(T_delta);

    for(HWK_curr = Hawks.begin(); HWK_curr != HWK_last; ++HWK_curr)
        (*HWK_curr)->Update(T_delta);

    // Apply AI
    for(BRD_curr = Birds.begin(); BRD_curr != BRD_last; ++BRD_curr)
        (*BRD_curr)->PostUpdate(T_delta);

    for(HWK_curr = Hawks.begin(); HWK_curr != HWK_last; ++HWK_curr)
        (*HWK_curr)->PostUpdate(T_delta);

    // Convert weak hawks to birds
    for(HWK_curr = Hawks.begin(); HWK_curr != HWK_last;)
    {
        if ((*HWK_curr)->Energy < 0.f)
        {
            Hawk *hawk = *HWK_curr;

            // Create a bird
            Bird *bird = new Bird();
            bird->Create(*this, hawk->P_center_Trfm, hawk->V_velocity);

            OnHawkReplace(hawk, bird);

            hawk->Destroy(false);
            delete hawk;
            HWK_curr = Hawks.erase(HWK_curr);
            HWK_last = Hawks.end();
        }
        else
            ++HWK_curr;
    }
}

/////////////////////////////////////

void Map:: LoadConfigLine(const char *buffer)
{
    if (StartsWith(buffer, "Generate"))
    {
        int level;
        sscanf(buffer+9, "%d", &level);
        Map::GenerateMap = level;
        return;
    }
    if (StartsWith(buffer, "Cols"))
    {
        int val;
        sscanf(buffer+5, "%d", &val);
        Map::GenerateMapCols = (val < 1) ? 1 : (uint)val;
        return;
    }
    if (StartsWith(buffer, "Rows"))
    {
        int val;
        sscanf(buffer+5, "%d", &val);
        Map::GenerateMapRows = (val < 1) ? 1 : (uint)val;
        return;
    }
    if (StartsWith(buffer, "Birds"))
    {
        int val;
        sscanf(buffer+6, "%d", &val);
        Map::InitialPopulation = (val < 0) ? 0 : (uint)val;
        return;
    }
    if (StartsWith(buffer, "Night"))
    {
        int val;
        sscanf(buffer+6, "%d", &val);
        Map::Night = val;
        return;
    }
    if (StartsWith(buffer, "DistrictSize"))
    {
        int val;
        sscanf(buffer+13, "%d", &val);
        Map::DistrictSize = (val <= 0) ? 1 : (uint)val;
        return;
    }
}
