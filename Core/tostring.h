#ifndef __Core_tostring_h
#define __Core_tostring_h

#include "types.h"
#include <sstream>

template< typename T >
inline String ToString( T val ) { return String(); }

template<>
inline String ToString( Bool val )
{
    std::stringstream ss;
    std::string out;
    ss << val;
    ss >> out;
    return out;
}

template<>
inline String ToString( Int32 val )
{
    std::stringstream ss;
    std::string out;
    ss << val;
    ss >> out;
    return out;
}

template<>
inline String ToString( Int16 val ) { return ToString<Int32>( val ); }
template<>
inline String ToString( Int8 val ) { return ToString<Int32>( val ); }

template<>
inline String ToString( Uint32 val )
{
    std::stringstream ss;
    std::string out;
    ss << val;
    ss >> out;
    return out;
}

template<>
inline String ToString( Uint16 val ) { return ToString<Uint32>( val ); }
template<>
inline String ToString( Uint8 val ) { return ToString<Uint32>( val ); }

template<>
inline String ToString( Float64 val )
{
    std::stringstream ss;
    std::string out;
    ss << val;
    ss >> out;
    return out;
}

template<>
inline String ToString( Float32 val ) { return ToString<Float64>( val ); }

#endif

