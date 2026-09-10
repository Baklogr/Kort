#ifndef PACKAGE_H
#define PACKAGE_H

#include <windows.h>
#include "lazy_importer.hpp"

typedef struct {
    PVOID  buffer;
    SIZE_T length;
} Package, *PPackage;

PPackage newPackage(BYTE cmd, BOOL bAddCmd);
BOOL addByte(PPackage pkg, BYTE value);
BOOL addInt32(PPackage pkg, UINT32 value);
BOOL addInt64(PPackage pkg, UINT64 value);
BOOL addString(PPackage pkg, PCHAR str, BOOL bWithSize);
BOOL addWString(PPackage pkg, PWCHAR str, BOOL bWithSize);
BOOL addBytes(PPackage pkg, PBYTE data, SIZE_T size, BOOL bWithSize);
VOID freePackage(PPackage pkg);

#endif