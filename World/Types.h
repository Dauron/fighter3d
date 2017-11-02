#ifndef __incl_World_Types_h
#define __incl_World_Types_h

#include <vector>
#include <list>

namespace World {
    struct Map;
    struct Quarter;

    struct Bird;
    typedef std::vector<Bird *> VecBirdP;
    typedef std::list<Bird *>   LstBirdP;

    struct Hawk;
    typedef std::vector<Hawk *> VecHawkP;
    typedef std::list<Hawk *>   LstHawkP;

#define OBJECT_BIRD 1
#define OBJECT_HAWK 2
}

#endif
