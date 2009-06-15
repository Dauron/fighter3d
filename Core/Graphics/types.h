#ifndef __Core_Graphics_types_h
#define __Core_Graphics_types_h

#include "../types.h"
#include "../Math/types.h"

namespace Graphics
{

#define Core_Graphics_Types

#include "color3b.h"
#include "color4b.h"

typedef Math::Vector2 TextUV;

typedef Math::Vector3 Color3f;
typedef Math::Vector3 TextUVW;

typedef Math::Vector4 Color4f;

#undef Core_Graphics_Types

}

#endif

