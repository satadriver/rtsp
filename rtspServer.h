#pragma once
#pragma once


#include <winsock.h>
#include <iostream>

#pragma pack(1)



#pragma pack()




class RtspServer {
public:

	RtspServer( const char* ip, int port,const char * fn=0);
	~RtspServer();

	static int __stdcall RtcpServer(RtspServer*);

	static int __stdcall RtpServer(RtspServer*);

	std::string GetTransport(int type);

	int ProcessRtsp(int sc);

	std::string GetRTPInfo(std::string url);

	int Server(const char* fn);

	std::string m_fn;
	int m_port;
	std::string m_ip;
	int m_type;

	SOCKET m_sock;

	int m_udpPort1;
	int m_udpPort2;

	std::string m_seq;
	std::string m_rtpTime;
	std::string m_ssrc;

	char* m_file;
	char* m_data;
	int m_fs;
	int m_dataSize;
};