#ifndef KORT_H
#define KORT_H

#include <windows.h>
#include "lazy_importer.hpp"

typedef struct {
    PCHAR  agentID;
    PWCHAR hostName;
    DWORD  httpPort;
    PWCHAR endPoint;
    PWCHAR userAgent;
    PWCHAR httpMethod;
    BOOL   isSSL;
    BOOL   isProxyEnabled;
    PWCHAR proxyURL;
    UINT32 sleeptime;
} CONFIG_KORT, *PCONFIG_KORT;

extern PCONFIG_KORT kortConfig;

BOOL InitConfig(void);

#endif