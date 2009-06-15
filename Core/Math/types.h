#ifndef __Core_Math_types_h
#define __Core_Math_types_h

#include "math.h"

namespace Math
{

#define Core_Math_Types

#include "vector2.h"
typedef Vector2 Point2;

#include "vector3.h"
typedef Vector3 Point3;

#include "vector4.h"
typedef Vector4 Point4;

#include "plane.h"

#include "quaternion.h"

#include "matrix.h"

#undef Core_Math_Types

}

#endif

