#include "kort.h"
#include "config.h"

PCONFIG_KORT kortConfig = NULL;

static PVOID li_alloc(SIZE_T size) {
    return LI_FN(LocalAlloc)(LPTR, size);
}

static PCHAR dup_str(PCSTR src) {
    if (!src) return NULL;
    SIZE_T len = 0; while (src[len]) len++;
    PCHAR out = (PCHAR)li_alloc(len + 1);
    if (!out) return NULL;
    LI_FN(memcpy)(out, src, len + 1);
    return out;
}

static PWCHAR dup_wstr(PCWSTR src) {
    if (!src) return NULL;
    SIZE_T len = 0; while (src[len]) len++;
    PWCHAR out = (PWCHAR)li_alloc((len + 1) * sizeof(WCHAR));
    if (!out) return NULL;
    LI_FN(memcpy)(out, src, (len + 1) * sizeof(WCHAR));
    return out;
}

BOOL InitConfig(void) {
    kortConfig = (PCONFIG_KORT)li_alloc(sizeof(CONFIG_KORT));
    if (!kortConfig) return FALSE;

    kortConfig->agentID        = dup_str(CONFIG_UUID);
    kortConfig->hostName       = dup_wstr(CONFIG_HOST);
    kortConfig->httpPort       = CONFIG_PORT;
    kortConfig->endPoint       = dup_wstr(CONFIG_POST_URI);
    kortConfig->userAgent      = dup_wstr(CONFIG_UA);
    kortConfig->httpMethod     = dup_wstr(L"POST");
    kortConfig->isSSL          = FALSE;
    kortConfig->isProxyEnabled = FALSE;
    kortConfig->proxyURL       = NULL;
    kortConfig->sleeptime      = CONFIG_SLEEP_JITTER ? CONFIG_SLEEP_JITTER : 5000;

    return TRUE;
}