#ifndef __Core_math_h
#define __Core_math_h

#include "../types.h"
#include <cmath>
#include <cstring>
#include <cassert>

namespace Math
{
    const Float Pi        = 3.1415926535f;
    const Float Pi_Inv    = 1.f / Pi;

    const Float64 Pi_Long     = 3.1415926535897932384626433832795;
    const Float64 Pi_Inv_Long = 1.0 / Pi_Long;

    template <typename T> static inline
    T DegToRad( T num ) { return num * 0.01745329252f; }

    template <typename T> static inline
    T RadToDeg( T num ) { return num * 57.29577951f; }

    #define DEGTORAD(num)       ((num)*0.01745329252f)
    #define RADTODEG(num)       ((num)*57.29577951f)

    const Float  Float_EPSILON_RADS = 0.000001f;

    template <typename T> static inline
    T Abs( T num )          { return num < 0 ? -num : num; }

    template <typename T> static inline
    T Sign( T num )         { return num < 0 ? -1
                                   : num > 0 ?  1
                                   : 0; }



    template <typename T> static inline
    T Max( T num1, T num2 ) { return num1 >= num2 ? num1 : num2; }

    template <typename T> static inline
    T Min( T num1, T num2 ) { return num1 <= num2 ? num1 : num2; }


    template <typename T> static inline
    void MinMax( T num1, T num2, T &min, T &max )
    {
        if ( num1 >= num2 ) { max = num1; min = num2; }
        else                { max = num2; min = num1; }
    }

    template <typename T> static inline
    void MinMax( T num1, T num2, T num3, T &min, T &max )
    {
        if ( num1 >= num2 )
        {
            if ( num1 >= num3 )
            { max = num1; min = Min( num2, num3 ); }
            else
            { max = num3; min = num2; }
        }
        else
        {
            if ( num2 >= num3 )
            { max = num2; min = Min( num1, num3 ); }
            else
            { max = num3; min = num1; }
        }
    }

    template <typename T> static inline
    void MinMax( T num1, T num2, T num3, T &min, T &med, T &max )
    {
        if ( num1 >= num2 )
        {
            if ( num1 >= num3 )
            { max = num1; MinMax( num2, num3, min, med ); }
            else
            { max = num3; med = num1; min = num2; }
        }
        else
        {
            if ( num2 >= num3 )
            { max = num2; MinMax( num1, num3, min, med ); }
            else
            { max = num3; med = num2; min = num1; }
        }
    }

    static inline
    Bool IsZero   ( Float num )              { return Abs( num ) < Float_EPSILON; }
    static inline
    Bool Equals   ( Float num1, Float num2 ) { return  IsZero( num1 - num2 ); }
    static inline
    Bool NotEquals( Float num1, Float num2 ) { return !IsZero( num1 - num2 ); }
}

#endif

