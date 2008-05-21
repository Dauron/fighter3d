#include "GLExtensions.h"

bool GL_init_ARB_multisample(void);

#ifndef GL_ARB_multisample
#define GL_ARB_multisample              1

//Accepted by the <cap> parameter of Enable, Disable, and IsEnabled,
//and by the <pname> parameter of GetBooleanv, GetIntegerv, GetFloatv, and GetDoublev:
#define GL_MULTISAMPLE_ARB                         0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB            0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB                 0x809F
#define GL_SAMPLE_COVERAGE_ARB                     0x80A0

// Accepted by the <mask> parameter of PushAttrib:
#define GL_MULTISAMPLE_BIT_ARB                     0x20000000

// Accepted by the <pname> parameter of GetBooleanv, GetDoublev, GetIntegerv, and GetFloatv:
#define GL_SAMPLE_BUFFERS_ARB                      0x80A8
#define GL_SAMPLES_ARB                             0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB               0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB              0x80AB

#endif
