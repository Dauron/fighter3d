#ifndef __incl_World_ObjectTypes_h
#define __incl_World_ObjectTypes_h

namespace AI {

    struct ObjectType
    {
        enum eObjectType
        {
            Unknown   = 0,
            Structure = 1,
            Physical  = 2,
            Human     = 3
        };
    };

} // namespace AI

#endif
