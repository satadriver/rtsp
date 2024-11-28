
#include "rtspClient.h"
#include "packet.h"
#include <time.h>
#include "fileUtils.h"
#include "http.h"
#include "myJson.h"
#include "md5.h"



using namespace std;




const char* g_strOptions =
"OPTIONS rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %d\r\n\r\n";

const char* g_strDescribe = 
"DESCRIBE rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %d\r\n"
"Accept: application/sdp\r\n\r\n";

const char* g_strSetup = 
"SETUP rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"	///stream=0
"CSeq: %d\r\n"
//"Blocksize: 64000\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

const char* g_strPlay =
"PLAY rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %d\r\n"
"Range: npt=0.000-\r\n"
"Session: %s\r\n\r\n";

const char* g_strTeardown =
"TEARDOWN rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %d\r\n"
//"Range: npt=0.000-\r\n";
"Session: %s\r\n\r\n";


const char* g_strDescribeAuth =
"%s rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
"Accept: application/sdp\r\n\r\n";

const char* g_strSetupAuth =
"%s rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"	///stream=0
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
//"Blocksize: 64000\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

const char* g_strPlayAuth =
"%s rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
"Range: npt=now-\r\n"
"Session: %s\r\n\r\n";


const char* g_strTeardownAuth =
"%s rtsp://%s/axis-media/media.amp RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
//"Range: npt=0.000-\r\n"
"Session: %s\r\n\r\n";



const char * g_strOptionsResponse = 
"RTSP/1.0 200 OK\r\n"
"CSeq: %s\r\n"
"Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, SET_PARAMETER, TEARDOWN\r\n"
"Server: GStreamer RTSP server\r\n\r\n";

const char* g_strDescribeResponse =
"RTSP/1.0 200 OK\r\n"
"CSeq: %s\r\n"
"Content-Type: application/sdp\r\n"
"Content-Base: %s\r\n"
"Server: GStreamer RTSP server\r\n"
"Content-Length: %d\r\n\r\n%s";

const char* g_strDescribeDataF =
"v=0\r\n"
"o=- 2884328511627416078 1 IN IP4 %s\r\n"
"s=Session streamed with GStreamer\r\n"
"i=rtsp-server\r\n"
"t=0 0\r\n"
"a=tool:GStreamer\r\n"
"a=type:broadcast\r\n"
"a=range:npt=now-\r\n"
"a=control:%s\r\n"
"m=video 0 RTP/AVP 96\r\n"
"c=IN IP4 0.0.0.0\r\n"
"b=AS:51200\r\n"
"a=rtpmap:96 H264/90000\r\n"
"a=fmtp:96 packetization-mode=1;profile-level-id=640029;sprop-parameter-sets=Z2QAKa0AxSAeAIn5ZuAgIDSgAQesADN/mAHiRFQ=,aO48sA==\r\n"
"a=ts-refclk:local\r\n"
"a=mediaclk:sender\r\n"
"a=x-onvif-track:VIDEO000\r\n"
"a=recvonly\r\n"
"a=control:rtsp:%s\r\n"
"a=framerate:25.000000\r\n"
"a=transform:1.000000,0.000000,0.000000;0.000000,1.000000,0.000000;0.000000,0.000000,1.000000\r\n"
"a=x-sensor-transform:1.000000,0.000000,0.000000;0.000000,1.000000,0.000000;0.000000,0.000000,1.000000\r\n";


const char * g_strSetupResponse = 
"RTSP/1.0 200 OK\r\n"
"CSeq: %s\r\n"
"Transport: %s\r\n"
"Server: GStreamer RTSP server\r\n"
"Session: %s;timeout=60\r\n\r\n";


const char* g_strPlayResponse =
"RTSP/1.0 200 OK\r\n"
"CSeq: %s\r\n"
"Server: GStreamer RTSP server\r\n"
"RTP-Info: %s\r\n"
"Session: %s;timeout=60\r\n"
"Range: npt=now-\r\n\r\n";



const char * g_strTeardownResponse = 
"RTSP/1.0 200 OK\r\n"
"CSeq: %s\r\n"
"Server: GStreamer RTSP server\r\n"
"Session: %s;timeout=60\r\n"
"Connection: close\r\n\r\n";



const char* g_strSession = "0123456789abcdef";


const char* g_rtspReq[4] = { g_strOptions ,g_strDescribe ,g_strSetup ,g_strPlay };

