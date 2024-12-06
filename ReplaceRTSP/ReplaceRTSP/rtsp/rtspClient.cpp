
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define closesocket close
#endif

#include "public.h"
#include <iostream>
#include "rtspClient.h"
#include "packet.h"
#include "fileUtils.h"
#include "http.h"
#include "myJson.h"
#include "utils.h"
#include "md5.h"
#include "rtsp.h"


using namespace std;


#pragma comment(lib,"ws2_32.lib")



RtspClient::RtspClient(int type,const char * ip,int port,int seconds) {
	int ret = 0;
	#ifdef _WIN32
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		//perror("WSAStartup error\r\n");
	}
	#endif
	m_ip = ip;
	m_port = port;
	m_type = type;
	m_session = ""; 
	m_ssrc = "";
	m_seconds = seconds;
}


RtspClient::~RtspClient() {
#ifdef _WIN32
	WSACleanup();
#endif
}



int RtspClient::GetReplyInfo(const char* recvBuf, int size,int seq) {
	if (seq == 1) {
		char* http = 0;
		string hdr = GetHttpHeader(recvBuf, size, &http);

		//m_resolution = getstring((char*)"&resolution=", "&", http);
		//m_fps = getstring((char*)"a=framerate:", ".", http);
		//m_fps = "30";
	}
	else if (seq == 2) {
		string strSession = GetHeaderValue(recvBuf, "Session");
		if (strSession != "") {
			string session = getstring((char*)"", (char*)";", (char*)strSession.c_str());
			m_session = session;
		}

		string transport = GetHeaderValue(recvBuf, "Transport");
		string ssrc = getstring((char*)"ssrc=", (char*)";", (char*)transport.c_str());
		if (ssrc != "") {
			m_ssrc = ssrc;
		}
	}
	else if (seq == 3) {
		string rtp = GetHeaderValue(recvBuf, "RTP-Info");
		string seq = getstring((char*)"seq=", (char*)";", (char*)rtp.c_str());
		if (seq == "") {
			seq = getstring((char*)"seq=", (char*)"", (char*)rtp.c_str());
		}
		if (seq != "") {
			m_seq = seq;
		}
		string rtptime = getstring((char*)"rtptime=", (char*)";", (char*)rtp.c_str());
		if (rtptime == "") {
			rtptime = getstring((char*)"rtptime=", (char*)"", (char*)rtp.c_str());
			if (rtptime != "") {
				m_ts = rtptime;
			}
		}
	}
	return 0;
}


string GetMethod(const char * sendBuf) {
	int methodLen = GetUrlCmd(sendBuf);
	string method = string(sendBuf, methodLen - 1);
	return method;
}


int RtspClient::Authority(char* sendBuf, int sendLen, char* recvBuf, int recvLimit,int seq) {

	string user = "";

	char* recvData = 0;
	string recvHdr = GetHttpHeader(recvBuf, recvLimit, &recvData);

	string auth = GetHeaderValue(recvBuf, "WWW-Authenticate");
	string realm = GetStringValue(auth, "realm");
	string nonce = GetStringValue(auth, "nonce");
	string uri = GetStringValue(auth, "uri");

	string cseq = GetHeaderValue(sendBuf, "CSeq");

	string method = GetMethod(sendBuf);

	const char* pass = OBJECT_PASSWORD;
	const char* username = OBJECT_USERNAME;
	if (user == "") {
		user = username;
	}
	if (uri == "") {
		uri = GetUrl(sendBuf);
	}

	char strUrp[1024];
	int urplen = sprintf(strUrp, "%s:%s:%s", user.c_str(), realm.c_str(), pass);
	char urpMd5[64];
	GetMD5((unsigned char*)strUrp, urplen, (unsigned char*)urpMd5, 1);

	char strPu[0x1000];
	int pulen = sprintf(strPu, "%s:%s", method.c_str(), uri.c_str());
	char puMd5[64];
	GetMD5((unsigned char*)strPu, pulen, (unsigned char*)puMd5, 1);

	char code[1024];
	int codelen = sprintf(code, "%s:%s:%s", urpMd5, nonce.c_str(), puMd5);
	char codeMd5[64];
	GetMD5((unsigned char*)code, codelen, (unsigned char*)codeMd5, 1);

	if (m_auth == "") {
		const char* g_authForamt =
			"Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"";
		char strMyAuth[0x1000];
		sprintf(strMyAuth, g_authForamt, user.c_str(), realm.c_str(), nonce.c_str(), uri.c_str(), codeMd5);
		m_auth = strMyAuth;
	}

	char strAuth[0x1000];
	int authLen = 0;
	if (seq == 1) {
		authLen = sprintf(strAuth, g_strDescribeAuth, method.c_str(), uri.c_str(), atoi(cseq.c_str()),
			user.c_str(), realm.c_str(), nonce.c_str(), uri.c_str(), codeMd5);
	}
	else if (seq == 2) {
		authLen = sprintf(strAuth, g_strSetupAuth, 
			method.c_str(), uri.c_str(), atoi(cseq.c_str()),
			user.c_str(), realm.c_str(), nonce.c_str(), uri.c_str(), codeMd5);
	}
	else if (seq == 3) {
		authLen = sprintf(strAuth, g_strPlayAuth, 
			method.c_str(), uri.c_str(), atoi(cseq.c_str()),
			user.c_str(), realm.c_str(), nonce.c_str(), uri.c_str(), codeMd5, m_session.c_str());
	}


	int sendAuthLen = send(m_sock, strAuth, authLen, 0);
	if (sendAuthLen <= 0) {
		// printf("%s send method:%s length:%d error\r\n", __FUNCTION__, method.c_str(),sendAuthLen);
		return 0;
	}
	FWriter(PACKET_CMD_FILENAME, strAuth, authLen, 0);

	int recvLen = recv(m_sock, recvBuf, recvLimit, 0);
	if (recvLen > 0) {
		FWriter(PACKET_CMD_FILENAME, recvBuf, recvLen, 0);
		recvBuf[recvLen] = 0;
		const char* successResponse = "RTSP/1.0 200 OK\r\n";
		if ( memcmp(recvBuf, successResponse, strlen(successResponse))!= 0 ) {
			// printf("%s recv method:%s error\r\n",__FUNCTION__, method.c_str());
			return 0;
		}
		GetReplyInfo(recvBuf, recvLen, seq);

		return recvLen;
	}
	else {
		// printf("%s recv method:%s length:%d error\r\n", __FUNCTION__,method.c_str(), recvLen);
	}

	return 0;	
}

void test() {
	const char* strtest = "OPTIONS rtsp://192.168.1.101/axis-media/media.amp?camera=1&overview=0&resolution=1280x720&videoframeskipmode=empty&videozprofile=classic&fps=30&timestamp=1732171464026&Axis-Orig-Sw=true RTSP/1.0";
	int test = Contain(strtest, "camera=1");
}

int RtspClient::Client(const char * fn) {
	int ret = 0;

#ifdef _DEBUG
	test();
#endif

	srand((unsigned int)time(0));

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		//perror("socket\r\n");
		return -1;
	}
	m_sock = s;

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	
	#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = inet_addr(m_ip.c_str());
	#else
	sa.sin_addr.s_addr = inet_addr(m_ip.c_str());
	#endif

	sa.sin_port = ntohs(m_port);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		//perror("connect\r\n");
		return -1;
	}

	char sendBuf[0x1000];
	char recvBuf[0x1000];

	int sendLen = 0;
	int recvLen = 0;
	int recvLimit = 0;

	//SYSTEMTIME systime;
	//GetLocalTime(&systime);
	//time_t t = time(0) * 1000 + systime.wMilliseconds;
	time_t t = time(0) * 1000;

	char initUrl[0x1000];

	int cseq = rand();

	int seq = 0;
	int n = sizeof(g_rtspReq) / sizeof(char*);
	for (seq = 0; seq < n; seq++) {

		const char* format = g_rtspReq[seq]; 
		
		sendLen = 0;
		if (seq == 3) {
			sendLen = sprintf(sendBuf, format, initUrl, cseq ++,m_auth.c_str(), (char*) m_session.c_str());
		}
		else if (seq == 2) {
			sendLen = sprintf(sendBuf, format, initUrl, cseq++, m_auth.c_str());
		}	
		else if (seq == 1) {
			sendLen = sprintf(sendBuf, format, initUrl, cseq++);
		}
		else if (seq == 0) {		
			sprintf(initUrl, g_strOptionsUrl, m_ip.c_str(),t);
			sendLen = sprintf(sendBuf, format, initUrl,cseq ++);
		}
		else {
			//perror("error sequence number!\r\n");
			break;
		}

		int sendlen = send(s,sendBuf, sendLen, 0);
		if (sendLen <= 0) {
			//perror("send\r\n");
			break;
		}
		FWriter(PACKET_CMD_FILENAME, sendBuf, sendLen, 0);

		recvLimit = sizeof(recvBuf);
		recvLen = recv(s, recvBuf, recvLimit, 0);
		if (recvLen > 0 && recvLen < recvLimit) {
			recvBuf[recvLen] = 0;
		}
		else {
			//perror("recv\r\n");
			break;
		}
		FWriter(PACKET_CMD_FILENAME, recvBuf, recvLen, 0);

		const char* unAuthority = "RTSP/1.0 401 Unauthorized\r\n";

		const char* badRequest = "RTSP/1.0 400 Bad Request\r\n";

		const char* successResponse = "RTSP/1.0 200 OK\r\n";

		if (memcmp(recvBuf, unAuthority, strlen(unAuthority)) == 0) {
			recvLen = Authority(sendBuf, sendLen, recvBuf, recvLimit,seq);
			if (recvLen <= 0) {
				// printf("%s Authority error\r\n", __FUNCTION__);
				break;
			}
		}
		else if (memcmp(recvBuf, badRequest, strlen(badRequest)) == 0) {
			// printf("%s Authority error:%s\r\n", __FUNCTION__, badRequest);
			break;
		}
		else if (memcmp(recvBuf, successResponse, strlen(successResponse))==0) {
			GetReplyInfo(recvBuf, recvLen, seq);
		}
		else if (memcmp(recvBuf, successResponse, strlen(successResponse)) != 0) {
			// printf("%s Authority not success\r\n", __FUNCTION__);
			break;
		}
	}

	time_t timeNow = time(0);
	if (seq == 4) {

		FWriter(PACKET_RSTP_FILENAME, "", 0, 1);

		FWriter(PACKET_RSTP_FILENAME, recvBuf, recvLen, 0);
		int videoLimit = 0x100000;
		char* videoBuf = new char[videoLimit];
		int cnt = 0;
		while (time(0) - timeNow <= m_seconds) 
		{
			int videoLen = recv(s, videoBuf, videoLimit - 1, 0);
			if (videoLen > 0 && videoLen < videoLimit) {
				videoBuf[videoLen] = 0;
				FWriter(PACKET_RSTP_FILENAME, videoBuf, videoLen, 0);
			}
			else {
				//perror("recv\r\n");
				break;
			}
			cnt++;
			// printf("Get rtsp frame:%d,size:%d\r\n", cnt, videoLen);
		}

		sendLen = sprintf(sendBuf, g_strTeardownAuth,"TEARDOWN", initUrl, cseq++, m_auth.c_str(),
			(char*)m_session.c_str());
		int sendlen = send(s, sendBuf, sendLen, 0);

		delete[]videoBuf;

		//ret = ParseRtpStream(PACKET_RSTP_FILENAME, fn);
		 
		char* nextptr = 0;
		unsigned int ts =(unsigned int) strtoll(m_ts.c_str(), &nextptr,10);

		// ret = LzyFormat(PACKET_RSTP_FILENAME, fn, ts);
	}

	closesocket(s);

	return 0;
}


	
#ifdef _WIN32
int __stdcall RtspClient::TcpClient(char * sendBuf,int sendLen,char * recvBuf,int recvLen) 
#else
int __attribute__((__stdcall__)) RtspClient::TcpClient(char * sendBuf,int sendLen,char * recvBuf,int recvLen)
#endif
{
	int ret = 0;

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		//perror("socket\r\n");
		return -1;
	}
	m_sock = s;

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	
	#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = inet_addr(m_ip.c_str());
	#else
	sa.sin_addr.s_addr = inet_addr(m_ip.c_str());
	#endif

	sa.sin_port = ntohs(m_port);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		//perror("connect\r\n");
		return -1;
	}

	ret = send(s, (char*)sendBuf, sendLen, 0);
	if (ret > 0) {

	}
	else {
		//perror("send\r\n");
	}

	int len = recv(s, recvBuf, recvLen, 0);
	if (len > 0) {
		recvBuf[len] = 0;
	}
	else {
		//perror("recv\r\n");
	}

	closesocket(s);

	return len;
}

#ifdef _WIN32
int __stdcall RtspClient::UdpClient(char* sendBuf, int sendLen, char* recvBuf, int recvLen)
#else
int __attribute__((__stdcall__)) RtspClient::UdpClient(char* sendBuf, int sendLen, char* recvBuf, int recvLen)
#endif
{
	int ret = 0;

	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		//perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = inet_addr(m_ip.c_str());
#else
	sa.sin_addr.s_addr = inet_addr(m_ip.c_str());
#endif

	sa.sin_port = ntohs(m_port);

	ret = sendto(s, (char*)sendBuf, sendLen, 0, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret > 0) {
		socklen_t addrsize = sizeof(sockaddr_in);
		sockaddr_in saClient = { 0 };
		int recvlen = recvfrom(s, recvBuf, recvLen, 0, (sockaddr*)&saClient, &addrsize);
		if (recvlen > 0) {
			recvBuf[recvlen] = 0;
			closesocket(s);
			return recvlen;
		}
		else {
			//perror("recvfrom error\r\n");
		}
	}
	else {
		//perror("sendto error\r\n");
	}

	closesocket(s);

	return 0;
}