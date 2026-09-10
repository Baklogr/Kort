#include "Parser.h"

static UINT32 readInt32LE(PBYTE buf) {
    return (UINT32)buf[0] | ((UINT32)buf[1] << 8) |
           ((UINT32)buf[2] << 16) | ((UINT32)buf[3] << 24);
}

PParser newParser(PBYTE data, SIZE_T size) {
    PParser p = (PParser)LI_FN(LocalAlloc)(LPTR, sizeof(Parser));
    if (!p) return NULL;
    p->original = data;
    p->buffer = data;
    p->length = size;
    p->originalLength = size;
    return p;
}

BYTE getByte(PParser parser) {
    if (!parser || parser->length < 1) return 0;
    BYTE b = parser->buffer[0];
    parser->buffer += 1;
    parser->length -= 1;
    return b;
}

UINT32 getInt32(PParser parser) {
    if (!parser || parser->length < 4) return 0;
    UINT32 v = readInt32LE(parser->buffer);
    parser->buffer += 4;
    parser->length -= 4;
    return v;
}

UINT64 getInt64(PParser parser) {
    UINT64 low = getInt32(parser);
    UINT64 high = getInt32(parser);
    return low | (high << 32);
}

PBYTE getBytes(PParser parser, PSIZE_T size) {
    if (!parser || !size) return NULL;
    SIZE_T len = (*size == 0) ? getInt32(parser) : *size;
    *size = len;
    if (parser->length < len) return NULL;
    PBYTE out = (PBYTE)LI_FN(LocalAlloc)(LPTR, len);
    if (!out) return NULL;
    LI_FN(memcpy)(out, parser->buffer, len);
    parser->buffer += len;
    parser->length -= len;
    return out;
}

PCHAR getString(PParser parser, PSIZE_T size) {
    return (PCHAR)getBytes(parser, size);
}

PWCHAR getWString(PParser parser, PSIZE_T size) {
    if (!parser || !size) return NULL;
    SIZE_T byteLen = (*size == 0) ? getInt32(parser) : *size;
    if (parser->length < byteLen) return NULL;
    PWCHAR out = (PWCHAR)LI_FN(LocalAlloc)(LPTR, byteLen + sizeof(WCHAR));
    if (!out) return NULL;
    LI_FN(memcpy)(out, parser->buffer, byteLen);
    *size = byteLen / sizeof(WCHAR);
    parser->buffer += byteLen;
    parser->length -= byteLen;
    return out;
}

VOID freeParser(PParser parser) {
    if (!parser) return;
    LI_FN(LocalFree)(parser);
}