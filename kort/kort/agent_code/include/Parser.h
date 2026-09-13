#ifndef PARSER_H
#define PARSER_H

#include <windows.h>
#include "lazy_importer.hpp"

typedef struct {
    PBYTE  original;
    PBYTE  buffer;
    SIZE_T length;
    SIZE_T originalLength;
} Parser, *PParser;

PParser newParser(PBYTE data, SIZE_T size);
BYTE    getByte(PParser parser);
UINT32  getInt32(PParser parser);
UINT64  getInt64(PParser parser);
PCHAR   getString(PParser parser, PSIZE_T size);
PWCHAR  getWString(PParser parser, PSIZE_T size);
PBYTE   getBytes(PParser parser, PSIZE_T size);
VOID    freeParser(PParser parser);

#endif