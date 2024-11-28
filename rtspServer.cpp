
#include <winsock.h>
#include <iostream>
#include "rtspserver.h"
#include "packet.h"
#include "fileUtils.h"
#include "http.h"
#include "myJson.h"
#include "utils.h"
#include "md5.h"
#include "rtsp.h"


using namespace std;





#pragma comment(lib,"ws2_32.lib")



unsigned int g_servUdpPort1 = 50000;

unsigned int g_servUdpPort2 = 50001;

//unsigned int g_ssrc = 0x12345678;


RtspServer::RtspServer(const char* ip, int port,const char * fn ) {
	WSADATA wsa = { 0 };
	int ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		perror("WSAStartup error\r\n");
	}
	m_ip = ip;
	m_port = port;

	m_type = -1;
	if (fn) {
		m_fn = fn;
	}
	
	if (m_fn == "") {
		m_fn = PACKET_RSTP_FILENAME;
	}

	char* file = 0;
	int fs = 0;

	ret = FReader(m_fn, &file, &fs);
	if (ret <= 0) {
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
		return;
	}

	int size = ntohs(rtsp->length);

	RtpHeader* rtp = (RtpHeader*)(data + sizeof(RtspHeader));

	char strSsrc[16];
	sprintf(strSsrc, "%u", ntohl(rtp->SyncSrcId));
	m_ssrc = strSsrc;

	char* rtpdata = (char*)data + sizeof(RtpHeader);
	int rtpds = size - sizeof(RtpHeader);
}


RtspServer::~RtspServer() {
	WSACleanup();
	if (m_file) {
		delete m_file;
		m_file = 0;
	}
}



int __stdcall RtspServer::RtpServer(RtspServer * instance) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = ntohs(g_servUdpPort1);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr));
	if (ret) {
		closesocket(s);
		perror("bind\r\n");
		return -1;
	}

	while (1) {
		char recvBuf[0x1000];
		int recvLimit = sizeof(recvBuf);
		int addrsize = sizeof(sockaddr_in);
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



int __stdcall RtspServer::RtcpServer(RtspServer* instance) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = ntohs(g_servUdpPort2);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr));
	if (ret) {
		closesocket(s);
		perror("bind\r\n");
		return -1;
	}

	while (1) {
		char recvBuf[0x1000];
		int recvLimit = sizeof(recvBuf)-1;
		int addrsize = sizeof(sockaddr_in);
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









string RtspServer::GetTransport(int type) {
	char str[1024];
	const char* formatT = "RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=%s;mode=\"PLAY\"";
	const char* formatU = "RTP/AVP;unicast;client_port=%u-%u;server_port=%u-%u;ssrc=%s;mode=\"PLAY\"";
	if (type == 0) {
		sprintf(str, formatT,m_ssrc.c_str());

	}
	else {
		sprintf(str, formatU,m_udpPort1, m_udpPort2, g_servUdpPort1,g_servUdpPort2, m_ssrc.c_str());
	}
	return str;
}


string RtspServer::GetRTPInfo(string url) {
	const char* format = "url=%s;seq=%s;rtptime=%s";
	char str[1024] = { 0 };
	sprintf(str, format, url.c_str(), m_seq.c_str(), m_rtpTime.c_str());
	return str;
}


string GetCSeq(char* recvBuf, int recvLen) {
	string cseq = GetHeaderValue(recvBuf, "CSeq");
	return cseq;
}

int RtspServer::ProcessRtsp(int sc) {
	char sendBuf[0x1000];
	char recvBuf[0x1000];
	int ret = 0;
	int sendLen = 0;
	int recvLen = 0;
	int recvLimit = sizeof(recvBuf);
	int tcpPlay = 0;
	while (1) {
		int recvLen = recv(sc, recvBuf, recvLimit - 1, 0);
		if (recvLen > 0 && recvLen < recvLimit) {
			recvBuf[recvLen] = 0;

			int sendLen = 0;
			string cseq = GetCSeq(recvBuf, recvLimit);

			if (memcmp(recvBuf, "OPTIONS ", 8) == 0) {
				sendLen = sprintf(sendBuf, g_strOptionsResponse, cseq.c_str());
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
					m_udpPort1 = atoi(port1.c_str());
					m_udpPort2 = atoi(port2.c_str());
					tranport = GetTransport(1);
					m_type = 1;

					HANDLE ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RtcpServer, this, 0, 0);
					if (ht) {
						CloseHandle(ht);
					}

					ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RtpServer, this, 0, 0);
					if (ht) {
						CloseHandle(ht);
					}
				}
				else {
					tranport = GetTransport(0);
					m_type = 0;
				}

				sendLen = sprintf(sendBuf, g_strSetupResponse, cseq.c_str(), tranport.c_str(), g_strSession);
			}
			else if (memcmp(recvBuf, "PLAY ", 5) == 0) {
				string url = GetUrl(recvBuf);
				string info = GetRTPInfo(url);
				sendLen = sprintf(sendBuf, g_strPlayResponse, cseq.c_str(), info.c_str(), g_strSession);

				char* buf = new char[m_fs + 0x10000];
				memcpy(buf, sendBuf, sendLen);
				memcpy(buf + sendLen, m_data, m_dataSize);

				char* ptr = buf + sendLen;
				RtspHeader* rtsp = (RtspHeader*)ptr;
				if (rtsp->magic != 0x24) {
					break;
				}

				int size = ntohs(rtsp->length);

				int sendlen = send(sc, (char*)buf, sizeof(RtspHeader) + size + sendLen, 0);
				if (sendLen <= 0) {
					perror("send\r\n");
					break;
				}
				ptr = ptr + sizeof(RtspHeader) + size;

				while (ptr - buf < m_dataSize + sendLen)
				{
					RtspHeader* rtsp = (RtspHeader*)ptr;
					if (rtsp->magic != 0x24) {
						break;
					}

					int size = ntohs(rtsp->length);

					RtpHeader* rtp = (RtpHeader*)(ptr + sizeof(RtspHeader));

					char* rtpdata = (char*)rtp + sizeof(RtpHeader);
					int rtpds = size - sizeof(RtpHeader);

					int sendLen = send(sc, (char*)rtsp, sizeof(RtspHeader) + size, 0);
					if (sendLen <= 0) {
						perror("send\r\n");
						break;
					}

					//int recvLen = recv(sock, (char*)rtsp, 0x1000, 0);

					ptr = ptr + sizeof(RtspHeader) + size;

					//Sleep(100);
				}		
				
				//int sendlen = send(sc, buf, sendLen + m_dataSize, 0);
				delete []buf;
				break;
				//tcpPlay = 1;
			}
			else if (memcmp(recvBuf, "DESCRIBE ", 9) == 0) {

				string url = GetUrl(recvBuf);

				string ip = GetIPFromUrl(recvBuf);
				char data[0x1000];

				int descLen = sprintf(data, g_strDescribeDataF, ip.c_str(), url.c_str(), url.c_str());
				sendLen = sprintf(sendBuf, g_strDescribeResponse, cseq.c_str(),url.c_str(), descLen, data);
			}
			else if (memcmp(recvBuf, "TEARDOWN ", 9) == 0) {
				sendLen = sprintf(sendBuf, g_strTeardownResponse, cseq.c_str(), g_strSession);
			}
			else {
				break;
			}

			int sendlen = send(sc, sendBuf, sendLen, 0);
			if (sendlen <= 0) {
				perror("send\r\n");
				break;
			}
			else {
			}
			if (tcpPlay) {
				//ret = SendRtpStream(PACKET_RSTP_FILENAME, sc);
				tcpPlay = 0;
			}
		}
		else {
			break;
		}
	}
	closesocket(sc);
	return 0;
}



int RtspServer::Server(const char* fn) {

	int ret = 0;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		perror("socket\r\n");
		return -1;
	}
	m_sock = s;

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = ntohs(m_port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		closesocket(s);
		perror("bind\r\n");
		return -1;
	}

	ret = listen(m_sock, 4);
	if (ret) {
		perror("listen\r\n");
		closesocket(s);
		return -1;
	}

	while (1) {
		sockaddr_in sac;
		int saSize = sizeof(sockaddr_in);
		int sc = accept(s, (sockaddr*)&sac, &saSize);
		if (sc != -1) {
			
			ret = ProcessRtsp(sc);
		}
		else {
			closesocket(s);
			break;
		}
	}

	closesocket(s);

	return 0;

}