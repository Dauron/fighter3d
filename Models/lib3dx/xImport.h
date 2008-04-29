#ifndef __incl_xImport_h
#define __incl_xImport_h

#include "../lib3ds/file.h"
#include "xModel.h"

xFile *xImportFileFrom3ds(Lib3dsFile *model);
xFile *xLoadFrom3dmFile(const char *fileName);

#endif
