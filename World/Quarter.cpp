#include "Quarter.h"
#include "Bird.h"
#include "../Utils/Utils.h"

using namespace World;

float Quarter:: SquareWidth     = 75.f;
float Quarter:: SquareWidth_Inv = 1.f / Quarter::SquareWidth;
float Quarter:: WallWidth       = 25.f;
float Quarter:: MarginWidth     = (Quarter:: SquareWidth - Quarter:: WallWidth) * 0.5f;
float Quarter:: MaxHeight       = 100.f;

void Quarter:: LoadConfigLine(const char *buffer)
{
    if (StartsWith(buffer, "SquareWidth"))
    {
        float val;
        sscanf(buffer+12, "%f", &val);
        Quarter::SquareWidth = val;
        if (Quarter::SquareWidth < EPSILON)
            Quarter::SquareWidth = 75.f;
        Quarter::SquareWidth_Inv = 1.f / Quarter::SquareWidth;

        if (Quarter::SquareWidth <= Quarter::WallWidth)
            Quarter::WallWidth = Quarter::SquareWidth * 0.5f;

        Quarter::MarginWidth = (Quarter:: SquareWidth - Quarter:: WallWidth) * 0.5f;
        return;
    }
    if (StartsWith(buffer, "WallWidth"))
    {
        float val;
        sscanf(buffer+10, "%f", &val);
        Quarter::WallWidth = val;
        if (Quarter::WallWidth < EPSILON || Quarter::WallWidth >= Quarter::SquareWidth)
            Quarter::WallWidth = Quarter::SquareWidth * 0.5f;

        Quarter::MarginWidth = (Quarter:: SquareWidth - Quarter:: WallWidth) * 0.5f;
        return;
    }
    if (StartsWith(buffer, "MaxHeight"))
    {
        float val;
        sscanf(buffer+10, "%f", &val);
        Quarter::MaxHeight = val;
        if (Quarter::MaxHeight < 2.f)
            Quarter::MaxHeight = 2.f;
        return;
    }
}
