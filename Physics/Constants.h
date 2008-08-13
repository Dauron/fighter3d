#ifndef __incl_Physics_Constants_h
#define __incl_Physics_Constants_h

#define AIR_DENSITY         1.226f       // g [kg/m3]
#define DRAG_COEFFICIENT    0.2f         // Cx - should be specific for every object
#define REFERENCE_AREA(R)   (PI*(R)*(R)) // A  - front area, should be more specific for every object (not circular)
#define air_drag(R, V_Sqr)  (0.5f * AIR_DENSITY * REFERENCE_AREA(R) * DRAG_COEFFICIENT * (V_Sqr * 0.07716049382f)) // V [m/s] -> [km/h]

#endif
