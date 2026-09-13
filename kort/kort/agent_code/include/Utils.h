#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

// Повертає масив IP-адрес у вигляді UINT32 (little-endian), кількість у *pCount.
// Пам'ять виділяється через LocalAlloc — caller має звільнити.
DWORD* getIPAddress(PUINT32 pCount);

// ANSI-рядок з назвою ОС (наприклад, "Windows 10 (Build 19045)")
PCHAR  getOsName(void);

// 1 байт архітектури: 9 = x64, 0 = x86
BYTE   getArch(void);

// ANSI hostname — caller звільняє
PCHAR  getHostname(void);

// ANSI username — caller звільняє
PCHAR  getUserName(void);

// Wide domain — caller звільняє
PWCHAR getDomain(void);

// ANSI повний шлях до поточного процесу — caller звільняє
PCHAR  getCurrentProcName(void);

#endif