#ifndef __Core_types_h
#define __Core_types_h

#include <string>
#include <vector>

// Explicite types
typedef bool           Bool;

typedef unsigned char  Uint8;
typedef unsigned short Uint16;
typedef unsigned long  Uint32;

typedef signed   char  Int8;
typedef signed   short Int16;
typedef signed   long  Int32;

typedef float          Float32;
typedef double         Float64;

typedef std::string    String;
#define TXT(a)         (a)
#define TDArray        std::vector

// Implicite types
typedef Uint8     Byte;
typedef Uint32    Uint;
typedef Int32     Int;
typedef Float32   Float;

// Ranges
const Uint8       Uint8_MIN   = 0;
const Uint8       Uint8_MAX   = static_cast<Uint8>( 0xFF );
const Uint16      Uint16_MIN  = 0;
const Uint16      Uint16_MAX  = static_cast<Uint16>( 0xFFFF );
const Uint32      Uint32_MIN  = 0;
const Uint32      Uint32_MAX  = static_cast<Uint32>( 0xFFFFFFFF );
const Uint        Uint_MIN    = Uint32_MIN;
const Uint        Uint_MAX    = Uint32_MAX;

const Int8        Int8_MIN   = static_cast<Int8>( 0x80 );
const Int8        Int8_MAX   = static_cast<Int8>( 0x7F );
const Int16       Int16_MIN  = static_cast<Int16>( 0x80FF );
const Int16       Int16_MAX  = static_cast<Int16>( 0x7FFF );
const Int32       Int32_MIN  = static_cast<Int32>( 0x80000000 );
const Int32       Int32_MAX  = static_cast<Int32>( 0x7FFFFFFF );
const Int         Int_MIN    = Int32_MIN;
const Int         Int_MAX    = Int32_MAX;

const Float       Float_EPSILON       =  0.000000001f;
const Float       Float_HUGE_POSITIVE =  1000000000.f;
const Float       Float_HUGE_NEGATIVE = -1000000000.f;

// Basic arrays
typedef Uint8     Uint8_2     [2];
typedef Uint8     Uint8_3     [3];
typedef Uint8     Uint8_4     [4];

typedef Uint16    Uint16_2    [2];
typedef Uint16    Uint16_3    [3];
typedef Uint16    Uint16_4    [4];

typedef Uint32    Uint32_2    [2];
typedef Uint32    Uint32_3    [3];
typedef Uint32    Uint32_4    [4];

typedef Float32   Float32_2   [2];
typedef Float32   Float32_3   [3];
typedef Float32   Float32_4   [4];

typedef Float32   Float32_4x4 [4][4];

#include "tostring.h"

#endif

