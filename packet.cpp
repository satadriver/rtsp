
#include "rtspClient.h"
#include "packet.h"
#include <time.h>
#include "fileUtils.h"
#include "http.h"
#include "myJson.h"
#include "md5.h"



using namespace std;

#ifdef _WIN32
const char* g_strOptionsUrl =
"rtsp://%s/axis-media/media.amp?camera=1&overview=0&resolution=1280x720&videoframeskipmode=empty&videozprofile=classic&fps=30&timestamp=%I64u&Axis-Orig-Sw=true";
#else
const char* g_strOptionsUrl =
"rtsp://%s/axis-media/media.amp?camera=1&overview=0&resolution=1280x720&videoframeskipmode=empty&videozprofile=classic&fps=30&timestamp=%llu&Axis-Orig-Sw=true";
#endif


const char* g_strOptions =
"OPTIONS %s RTSP/1.0\r\n"
"CSeq: %u\r\n\r\n";

const char* g_strDescribe = 
"DESCRIBE %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Accept: application/sdp\r\n\r\n";

const char* g_strSetup = 
"SETUP %s RTSP/1.0\r\n"	///stream=0
"CSeq: %u\r\n"
"Blocksize: 64000\r\n"
"%s\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

const char* g_strPlay =
"PLAY %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"%s\r\n"
"Range: npt=0.000-\r\n"
"Session: %s\r\n\r\n";

const char* g_strTeardown =
"TEARDOWN %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"%s\r\n"
"Range: npt=0.000-\r\n"
"Session: %s\r\n\r\n";


const char* g_strDescribeAuth =
"%s %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
"Accept: application/sdp\r\n\r\n";

const char* g_strSetupAuth =
"%s %s RTSP/1.0\r\n"	///stream=0
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
"Blocksize: 64000\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

const char* g_strPlayAuth =
"%s %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n"
"Range: npt=0.000-\r\n"
"Session: %s\r\n\r\n";


const char* g_strTeardownAuth =
"%s %s RTSP/1.0\r\n"
"CSeq: %u\r\n"
"%s\r\n"
"Range: npt=0.000-\r\n"
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



const char* g_strDescPtzResponse =
"v=0\r\n"
"o=- 10412905542010325405 1 IN IP4 %s\r\n"
"s=Session streamed with GStreamer\r\n"
"i=rtsp-server\r\n"
"t=0 0\r\n"
"a=tool:GStreamer\r\n"
"a=type:broadcast\r\n"
"a=range:npt=now-\r\n"
"a=control:%s\r\n"
"m=application 0 RTP/AVP 98\r\n"
"c=IN IP4 0.0.0.0\r\n"
"a=rtpmap:98 vnd.onvif.metadata/90000\r\n"
"a=ts-refclk:local\r\n"
"a=mediaclk:sender\r\n"
"a=x-onvif-track:APPLICATION000\r\n"
"a=recvonly\r\n"
"a=control:%s\r\n";

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
"a=fmtp:96 packetization-mode=1;profile-level-id=640029;sprop-parameter-sets=Z2QAKa0AxSAUAW6bgICA0oAEHrAAzf5gB4kRUA==,aO48sA==\r\n"
"a=ts-refclk:local\r\n"
"a=mediaclk:sender\r\n"
"a=x-onvif-track:VIDEO000\r\n"
"a=recvonly\r\n"
"a=control:%s\r\n"
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




const char * g_strXml1=
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<tt:MetadataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\">\r\n"
"</tt:MetadataStream>\r\n";


const char* g_strXml2 =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<tt:MetadataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\">\r\n"
"<tt:PTZ>\r\n"
"<tt:PTZStatus>\r\n"
"<tt:Position> \r\n"
"<tt:PanTilt x=\"-0.552896\" y=\"-0.441377\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace\"/>\r\n"
"<tt:Zoom x=\"0.000000\" space=\"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace\"/>\r\n"
"</tt:Position>\r\n"
"<tt:MoveStatus>\r\n"
"<tt:PanTilt>IDLE</tt:PanTilt>\r\n"
"<tt:Zoom>IDLE</tt:Zoom>\r\n"
"</tt:MoveStatus>\r\n"
"<tt:UtcTime>2028-11-21T18:46:05.743443Z</tt:UtcTime>\r\n"
"</tt:PTZStatus> \r\n"
"</tt:PTZ> \r\n"
"</tt:MetadataStream>\r\n";

const char* g_strXml3 =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<tt:MetadataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\">\r\n"
"<tt:Event><wsnt:NotificationMessage xmlns:tns1=\"http://www.onvif.org/ver10/topics\"" 
"xmlns:tnsaxis=\"http://www.axis.com/2009/event/topics\" xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\""
"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\">"
"<wsnt:Topic Dialect=\"http://docs.oasis-open.org/wsn/t-1/TopicExpression/Simple\">"
"tns1:VideoSource/GlobalSceneChange/ImagingService</wsnt:Topic><wsnt:ProducerReference>"
"<wsa5:Address>uri://25ec1bd5-605e-45aa-abe9-a800a6080a55/ProducerReference</wsa5:Address>"
"</wsnt:ProducerReference><wsnt:Message><tt:Message UtcTime=\"2024-11-21T13:18:41.920399Z\" "
"PropertyOperation=\"Initialized\"><tt:Source><tt:SimpleItem Name=\"Source\" Value=\"0\"/>"
"</tt:Source><tt:Key></tt:Key><tt:Data><tt:SimpleItem Name=\"State\" Value=\"0\"/></tt:Data>"
"</tt:Message></wsnt:Message></wsnt:NotificationMessage></tt:Event></tt:MetadataStream>\r\n";
