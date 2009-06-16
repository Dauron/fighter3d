#ifndef __Core_fromstring_h
#define __Core_fromstring_h

#include "types.h"
#include <cstdlib>

inline void FromString( const Char *buffer, Bool &res, const Char *&bufEnd ) { char *end; res = (strtol( buffer, &end, 10 ) != 0); bufEnd = end; }

inline void FromString( const Char *buffer, Int32 &res, const Char *&bufEnd ) { char *end; res = (strtol( buffer, &end, 10 )); bufEnd = end; }
inline void FromString( const Char *buffer, Int16 &res, const Char *&bufEnd ) { char *end; res = (strtol( buffer, &end, 10 )); bufEnd = end; }
inline void FromString( const Char *buffer, Int8  &res, const Char *&bufEnd ) { char *end; res = (strtol( buffer, &end, 10 )); bufEnd = end; }

inline void FromString( const Char *buffer, Uint32 &res, const Char *&bufEnd ) { char *end; res = (strtoul( buffer, &end, 10 )); bufEnd = end; }
inline void FromString( const Char *buffer, Uint16 &res, const Char *&bufEnd ) { char *end; res = (strtoul( buffer, &end, 10 )); bufEnd = end; }
inline void FromString( const Char *buffer, Uint8  &res, const Char *&bufEnd ) { char *end; res = (strtoul( buffer, &end, 10 )); bufEnd = end; }

inline void FromString( const Char *buffer, Float64 &res, const Char *&bufEnd ) { char *end; res = (strtod( buffer, &end )); bufEnd = end; }
inline void FromString( const Char *buffer, Float32 &res, const Char *&bufEnd ) { char *end; res = (strtod( buffer, &end )); bufEnd = end; }

#endif

