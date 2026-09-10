#include "Package.h"

static VOID writeInt32LE(PUCHAR buf, UINT32 v) {
    buf[0] = (UINT8)(v & 0xFF);
    buf[1] = (UINT8)((v >> 8) & 0xFF);
    buf[2] = (UINT8)((v >> 16) & 0xFF);
    buf[3] = (UINT8)((v >> 24) & 0xFF);
}

static PVOID li_alloc(SIZE_T size) {
    return LI_FN(LocalAlloc)(LPTR, size);
}

static PVOID li_realloc(PVOID ptr, SIZE_T size) {
    return LI_FN(LocalReAlloc)(ptr, size, LMEM_MOVEABLE | LMEM_ZEROINIT);
}

static VOID li_memcpy(PVOID dst, const VOID* src, SIZE_T size) {
    LI_FN(memcpy)(dst, src, size);
}

static SIZE_T li_strlen(PCHAR s) {
    SIZE_T n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

static SIZE_T li_wcslen(PWCHAR s) {
    SIZE_T n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

PPackage newPackage(BYTE cmd, BOOL bAddCmd) {
    PPackage pkg = (PPackage)li_alloc(sizeof(Package));
    if (!pkg) return NULL;
    pkg->buffer = NULL;
    pkg->length = 0;
    if (bAddCmd) addByte(pkg, cmd);
    return pkg;
}

BOOL addByte(PPackage pkg, BYTE value) {
    if (!pkg) return FALSE;
    pkg->buffer = li_realloc(pkg->buffer, pkg->length + 1);
    if (!pkg->buffer) return FALSE;
    ((PUCHAR)pkg->buffer)[pkg->length] = value;
    pkg->length += 1;
    return TRUE;
}

BOOL addInt32(PPackage pkg, UINT32 value) {
    if (!pkg) return FALSE;
    pkg->buffer = li_realloc(pkg->buffer, pkg->length + 4);
    if (!pkg->buffer) return FALSE;
    writeInt32LE((PUCHAR)pkg->buffer + pkg->length, value);
    pkg->length += 4;
    return TRUE;
}

BOOL addInt64(PPackage pkg, UINT64 value) {
    addInt32(pkg, (UINT32)(value & 0xFFFFFFFF));
    addInt32(pkg, (UINT32)(value >> 32));
    return TRUE;
}

BOOL addString(PPackage pkg, PCHAR str, BOOL bWithSize) {
    if (!pkg || !str) return FALSE;
    SIZE_T len = li_strlen(str);
    if (bWithSize) addInt32(pkg, (UINT32)len);
    pkg->buffer = li_realloc(pkg->buffer, pkg->length + len);
    if (!pkg->buffer) return FALSE;
    li_memcpy((PUCHAR)pkg->buffer + pkg->length, str, len);
    pkg->length += len;
    return TRUE;
}

BOOL addWString(PPackage pkg, PWCHAR str, BOOL bWithSize) {
    if (!pkg || !str) return FALSE;
    SIZE_T len = li_wcslen(str);
    SIZE_T byteLen = len * sizeof(WCHAR);
    if (bWithSize) addInt32(pkg, (UINT32)byteLen);
    pkg->buffer = li_realloc(pkg->buffer, pkg->length + byteLen);
    if (!pkg->buffer) return FALSE;
    li_memcpy((PUCHAR)pkg->buffer + pkg->length, str, byteLen);
    pkg->length += byteLen;
    return TRUE;
}

BOOL addBytes(PPackage pkg, PBYTE data, SIZE_T size, BOOL bWithSize) {
    if (!pkg || !data) return FALSE;
    if (bWithSize) addInt32(pkg, (UINT32)size);
    pkg->buffer = li_realloc(pkg->buffer, pkg->length + size);
    if (!pkg->buffer) return FALSE;
    li_memcpy((PUCHAR)pkg->buffer + pkg->length, data, size);
    pkg->length += size;
    return TRUE;
}

VOID freePackage(PPackage pkg) {
    if (!pkg) return;
    if (pkg->buffer) LI_FN(LocalFree)(pkg->buffer);
    LI_FN(LocalFree)(pkg);
}