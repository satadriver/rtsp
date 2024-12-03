
#pragma once

#include "rtspClient.h"




#pragma pack(1)



#define PACKET_CMD_FILENAME "authority.txt"

#define PACKET_RSTP_FILENAME "rstp.txt"


typedef struct  {
	unsigned short tag;
	unsigned short size;

}RTSPHEADER;




#pragma pack()

extern const char* g_strOptionsUrl;

extern const char* g_strOptions;
extern const char* g_strDescribe;
extern const char* g_strSetup;
extern const char* g_strPlay;
extern const char* g_strTeardown;

extern const char* g_strDescribeAuth;
extern const char* g_strSetupAuth;
extern const char* g_strPlayAuth;
extern const char* g_strTeardownAuth;

extern const char* g_rtspReq[4];

extern const char* g_strSession;

extern const char* g_strOptionsResponse;
extern const char* g_strDescribeResponse;
extern const char* g_strDescribeDataF;
extern const char* g_strSetupResponse;
extern const char* g_strPlayResponse;
extern const char* g_strTeardownResponse;

extern const char* g_strDescPtzResponse;