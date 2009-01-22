#ifndef __incl_xImport_h
#define __incl_xImport_h

#ifdef WIN32
#
#   include "../lib3ds/file.h"
#   pragma comment(lib, "lib3ds.lib")
#
#else
#
#    include <lib3ds/file.h>
#
#endif

#include "xModel.h"

xModel *xImportFileFrom3ds(Lib3dsFile *model);

#endif
