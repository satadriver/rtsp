

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>
//#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#define closesocket close
#endif

#include <iostream>
#include "rtspServer.h"
#include "packet.h"
#include "fileUtils.h"
#include "http.h"
#include "myJson.h"
#include "utils.h"
#include "md5.h"
#include "rtsp.h"
#include "public.h"


using namespace std;





#pragma comment(lib,"ws2_32.lib")



unsigned int g_servUdpPort1 = 50000;

unsigned int g_servUdpPort2 = 50001;




RtspServer::RtspServer(const char* port,const char * fn ) {
	int ret = 0;

#ifdef _WIN32
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		perror("WSAStartup error\r\n");
	}
#endif

	if (port == 0) {
		m_port = 554;
	}
	else {
		m_port = atoi(port);
	}

	m_type = -1;

	if (fn == 0) {
		m_fn = PACKET_RSTP_FILENAME;
	}
	else {
		m_fn = fn;
	}

	char* file = 0;
	int fs = 0;
	ret = FReader(m_fn, &file, &fs);
	if (ret <= 0) {
		printf("%s read file:%s error\r\n", __FUNCTION__, m_fn.c_str());
		return ;
	}
	m_fs = fs;
	m_file = file;

	char* data = 0;
	string hdr = GetHttpHeader(file, fs, &data);
	string rtpInfo = GetHeaderValue(hdr.c_str(), "RTP-Info");
	string seq = getstring((char*)"seq=", (char*)";", (char*)rtpInfo.c_str());
	if (seq == "") {
		seq = getstring((char*)"seq=", (char*)"", (char*)rtpInfo.c_str());
	}
	string rtptime = getstring((char*)"rtptime=", (char*)";", (char*)rtpInfo.c_str());
	if (rtptime == "") {
		rtptime = getstring((char*)"rtptime=", (char*)"", (char*)rtpInfo.c_str());
	}
	m_rtpTime = rtptime;
	m_seq = seq;

	m_data = data;

	m_dataSize = m_fs - (m_data - m_file);

	RtspHeader* rtsp = (RtspHeader*)data;
	if (rtsp->magic != 0x24) {
		printf("%s file:%s format error\r\n", __FUNCTION__, m_fn.c_str());
		return;
	}

	int size = ntohs(rtsp->length);
	RtpHeader* rtp = (RtpHeader*)(data + sizeof(RtspHeader));
	char strSsrc[16];
	sprintf(strSsrc, "%u", ntohl(rtp->SyncSrcId));
	m_ssrc = strSsrc;

	printf("%s file:%s ssrc:%s, seq:%s, rtptime:%s\r\n", 
		__FUNCTION__, m_fn.c_str(), m_ssrc.c_str(),seq.c_str(),rtptime.c_str());

	char* rtpdata = (char*)data + sizeof(RtpHeader);
	int rtpds = size - sizeof(RtpHeader);

	m_playOffset = m_data;
}


RtspServer::~RtspServer() {
#ifdef _WIN32
	WSACleanup();
#endif
	if (m_file) {
		delete m_file;
		m_file = 0;
	}
}




#ifdef _WIN32
void * __stdcall RtspServer::RtpServer(void * param) 
#else
void * __attribute__((__stdcall__))  RtspServer::RtpServer(void*param)
#endif
{
	RtspServer * instance =(RtspServer *) param;
	int ret = 0;

	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return 0;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	#else
	sa.sin_addr.s_addr = INADDR_ANY;
	#endif
	sa.sin_port = ntohs(g_servUdpPort1);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr));
	if (ret) {
		closesocket(s);
		perror("bind\r\n");
		return 0;
	}

	while (1) {
		char recvBuf[0x1000];
		int recvLimit = sizeof(recvBuf);
		socklen_t addrsize = sizeof(sockaddr_in);
		sockaddr_in saClient = { 0 };
		int recvlen = recvfrom(s, recvBuf, recvLimit-1, 0, (sockaddr*)&saClient, &addrsize);
		if (recvlen > 0 && recvlen < recvLimit) {
			recvBuf[recvlen] = 0;
			if (memcmp(recvBuf, "\xce\xfa\xed\xfe", 4) == 0) {

			}
			else {

			}
		}
		else {
			perror("recvfrom error\r\n");
		}
	}

	closesocket(s);

	return 0;
}


#ifdef _WIN32
void *  __stdcall RtspServer::RtcpServer(void* param) 
#else
void *  __attribute__((__stdcall__)) RtspServer::RtcpServer(void* param) 
#endif
{
RtspServer * instance = (RtspServer *)param;
	int ret = 0;

	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return 0;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	#else
	sa.sin_addr.s_addr = INADDR_ANY;
	#endif
	sa.sin_port = ntohs(g_servUdpPort2);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr));
	if (ret) {
		closesocket(s);
		perror("bind\r\n");
		return 0;
	}

	while (1) {
		char recvBuf[0x1000];
		int recvLimit = sizeof(recvBuf)-1;
		socklen_t addrsize = sizeof(sockaddr_in);
		sockaddr_in saClient = { 0 };
		int recvlen = recvfrom(s, recvBuf, recvLimit-1, 0, (sockaddr*)&saClient, &addrsize);
		if (recvlen > 0 && recvlen < recvLimit) {
			recvBuf[recvlen] = 0;
			if (memcmp(recvBuf, "\xce\xfa\xed\xfe", 4) == 0) {

			}
			else {

			}
		}
		else {
			perror("recvfrom error\r\n");
		}
	}

	closesocket(s);

	return 0;
}




string RtspServer::GetTransport(int type,int udpPort1,int udpPort2) {
	char str[1024];
	const char* formatT = "RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=%s;mode=\"PLAY\"";
	const char* formatU = "RTP/AVP;unicast;client_port=%u-%u;server_port=%u-%u;ssrc=%s;mode=\"PLAY\"";
	if (type == 0) {
		sprintf(str, formatT,m_ssrc.c_str());
	}
	else {
		sprintf(str, formatU,udpPort1, udpPort2, g_servUdpPort1,g_servUdpPort2, m_ssrc.c_str());
	}
	return str;
}


string RtspServer::GetRTPInfo(string url) {
	const char* format = "url=%s;seq=%s;rtptime=%s";
	char str[0x1000] = { 0 };
	sprintf(str, format, url.c_str(), m_seq.c_str(), m_rtpTime.c_str());
	return str;
}


string GetCSeq(char* recvBuf, int recvLen) {
	string cseq = GetHeaderValue(recvBuf, "CSeq");
	return cseq;
}

#ifdef _WIN32
void* __stdcall RtspServer::ProcessRtsp(void* param)
#else
void* __attribute__((__stdcall__)) RtspServer::ProcessRtsp(void* param)
#endif
{
	RtspThreadParam* rtspParm = (RtspThreadParam*)param;
	int sc = (int)rtspParm->sc;
	RtspServer* server = rtspParm->ptr;

	int ret = 0;
	char sendBuf[0x1000];
	char recvBuf[0x1000];
	
	int sendLen = 0;
	int recvLen = 0;
	int recvLimit = sizeof(recvBuf);

	int sl = 0;

	int udpPort1 = 0;
	int udpPort2 = 0;

	while (1) {
		int recvLen = recv(sc, recvBuf, recvLimit - 1, 0);
		if (recvLen > 0 && recvLen < recvLimit) {
			recvBuf[recvLen] = 0;

			int sendLen = 0;
			string cseq = GetCSeq(recvBuf, recvLimit);

			if (memcmp(recvBuf, "OPTIONS ", 8) == 0) {
				sendLen = sprintf(sendBuf, g_strOptionsResponse, cseq.c_str());
				sl = send(sc, sendBuf, sendLen, 0);
			}
			else if (memcmp(recvBuf, "SETUP ", 6) == 0) {
				string trans = GetHeaderValue(recvBuf, "Transport");

				string tranport = "";
				int p = trans.find("RTP/AVP/TCP;");
				if (p == -1) {
					p = trans.find("client_port=");
					string ports = trans.substr(p + strlen("client_port="));
					p = ports.find("-");
					string port1 = ports.substr(0, p);
					string port2 = ports.substr(p + 1);
					udpPort1 = atoi(port1.c_str());
					udpPort2 = atoi(port2.c_str());
					tranport = server->GetTransport(1,0,0);
					server->m_type = 1;
#ifdef _WIN32
					HANDLE ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RtcpServer, server, 0, 0);
					if (ht) {
						CloseHandle(ht);
						ht = 0;
					}
					ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RtpServer, server, 0, 0);
					if (ht) {
						CloseHandle(ht);
					}
#else
					pthread_t pid = 0;
					pthread_create(&pid,0,RtcpServer,server);
					pthread_create(&pid,0,RtpServer,server);
#endif
				}
				else {
					tranport = server->GetTransport(0,udpPort1,udpPort2);
					server->m_type = 0;
				}

				sendLen = sprintf(sendBuf, g_strSetupResponse, cseq.c_str(), tranport.c_str(), g_strSession);
				sl = send(sc, sendBuf, sendLen, 0);
			}
			else if (memcmp(recvBuf, "PLAY ", 5) == 0) {

				string url = GetUrl(recvBuf);
				string info = server->GetRTPInfo(url);
				sendLen = sprintf(sendBuf, g_strPlayResponse, cseq.c_str(), info.c_str(), g_strSession);
				sl = send(sc, (char*)sendBuf, sendLen, 0);

				printf("send PLAY response:%s\r\n", sendBuf);

				int type = Contain(recvBuf, "ptz=all");
				if (type) {
					char* playResp = 0;
					int playRespSize = 0;
					ret = FReader("playResponse.dat", &playResp, &playRespSize);
					if (ret) {
						sl = send(sc, playResp, playRespSize, 0);
					}
				}
				else
				{
						//while (1) 
						{
							if (0) {
								sl = send(sc, server->m_data, server->m_dataSize, 0);
								//sl = recv(sc, recvBuf, recvLimit, 0);
							}
							else {
								int cnt = 0;
								char* ptr = server->m_data;

								while (ptr - server->m_data < server->m_dataSize)
								{
									RtspHeader* rtsp = (RtspHeader*)ptr;
									if (rtsp->magic != 0x24) {
										break;
									}

									int size = ntohs(rtsp->length);

									RtpHeader* rtp = (RtpHeader*)(ptr + sizeof(RtspHeader));

									char* rtpdata = (char*)rtp + sizeof(RtpHeader);
									int rtpds = size - sizeof(RtpHeader);

									sl = send(sc, (char*)rtsp, sizeof(RtspHeader) + size, 0);
									if (sl <= 0) {
										perror("send\r\n");
										break;
									}

									ptr = ptr + sizeof(RtspHeader) + size;

									cnt++;
									if (cnt >= 10000 / 400) {
										server->m_playOffset = ptr;
										//break;
									}
#ifdef _WIN32
									//Sleep(400);
#else
									//usleep(400000);
#endif
									//break;
								}
							}
						}

#ifdef _WIN32
						//Sleep(3000);
#else
						//sleep(3);
#endif

						break;
					
				}
			}
			else if (memcmp(recvBuf, "DESCRIBE ", 9) == 0) {
				char data[0x1000];
				string url = GetUrl(recvBuf);

				string ip = GetIPFromUrl(recvBuf);
				int descLen = 0;
				int type = Contain(recvBuf, "ptz=all");
				if (type) {
					descLen = sprintf(data, g_strDescPtzResponse, ip.c_str(), url.c_str(), url.c_str());
				}
				else {
					descLen = sprintf(data, g_strDescribeDataF, ip.c_str(), url.c_str(), url.c_str());	
				}
				sendLen = sprintf(sendBuf, g_strDescribeResponse, cseq.c_str(), url.c_str(), descLen, data);
				sl = send(sc, sendBuf, sendLen, 0);
			}
			else if (memcmp(recvBuf, "TEARDOWN ", 9) == 0) {
				sendLen = sprintf(sendBuf, g_strTeardownResponse, cseq.c_str(), g_strSession);
				sl = send(sc, sendBuf, sendLen, 0);
			}
			else {
				printf("unrecognized packet:%s\r\n", recvBuf);
				break;
			}
		
			if (sl <= 0) {
				printf("%s send error\r\n", __FUNCTION__);
				perror("send\r\n");
				break;
			}
			else {
				printf("send response:%s\r\n", sendBuf);
			}
		}
		else {
			printf("%s recv error\r\n", __FUNCTION__);
			perror("recv");
			break;
		}
	}
	closesocket(sc);
	delete param;

	printf("Send rtsp steam ok\r\n");

	return 0;
}



int RtspServer::Server(const char* fn) {

	int ret = 0;

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		perror("socket\r\n");
		return -1;
	}
	m_sock = s;

	int opt = 1;
	ret = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR,( char*)&opt, sizeof(opt));

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	#ifdef _WIN32
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	#else
	sa.sin_addr.s_addr = INADDR_ANY;
	#endif
	sa.sin_port = ntohs(m_port);

	do{
		ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
		if (ret) {
			perror("bind\r\n");	
			
#ifdef _WIN32
			closesocket(s);
			return -1;
#else
			sleep(1);
			KillMonolith();		
#endif
		}
		else{
			printf("bind ok\r\n");
			break;
		}
	}while(ret);

	ret = listen(m_sock, 4);
	if (ret) {
		perror("listen\r\n");
		closesocket(s);
		return -1;
	}

	while (1) {
		sockaddr_in sac;
		socklen_t saSize = sizeof(sockaddr_in);
		int sc = accept(s, (sockaddr*)&sac, &saSize);
		if (sc != -1) {
			if (1) 
			{
				RtspThreadParam* param = new RtspThreadParam;
				param->sc = sc;
				param->ptr = this;
#ifdef _WIN32
				HANDLE ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ProcessRtsp, param, 0, 0);
				if (ht) {
					CloseHandle(ht);
				}
#else
				pthread_t pid = 0;
				pthread_create(&pid, 0, ProcessRtsp, param);
#endif
			}
			else {
				closesocket(sc);
			}
		}
		else {
			printf("%s accept error\r\n",__FUNCTION__);
			perror("accept");

			break;
		}
	}

	closesocket(s);

	return 0;

}
