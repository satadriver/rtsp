#pragma once


#include <winsock.h>
#include <iostream>

#pragma pack(1)

typedef struct  {
	char strip[16];
	int port;
}SocketAddr;


typedef struct {
	int cmd;
	char buf[256];
}NetworkCommand;

#pragma pack()




class RtspClient {
public:

	RtspClient(int type,const char* ip, int port);
	~RtspClient();

	int __stdcall UdpClient(char* data, int size,char * recvBuf,int recvLen);

	int __stdcall TcpClient(char * data,int size,char * recvBuf,int recvLen);

	int Authority(char* sendBuf, int sendLen, char* recvBuf, int recvLen,int seq);

	int GetReplyInfo(const char* recvBuf, int size, int seq);

	int Client(const char * fn);

	int m_port;
	std::string m_ip;
	int m_type;
	std::string m_session;
	std::string m_ssrc;
	SOCKET m_sock;

	//__int64 m_time;
	std::string m_auth;
};