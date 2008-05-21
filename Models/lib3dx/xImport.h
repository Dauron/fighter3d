#ifndef __incl_xImport_h
#define __incl_xImport_h

#include "../lib3ds/file.h"
#include "xModel.h"

xModel *xImportFileFrom3ds(Lib3dsFile *model);
xModel *xLoadFrom3dmFile(const char *fileName);

#endif
