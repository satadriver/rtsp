#pragma once


#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#endif

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

	RtspClient(int type,const char* ip, int port,int seconds = 0);
	~RtspClient();

#ifdef _WIN32
	int __stdcall UdpClient(char* data, int size,char * recvBuf,int recvLen);
	int __stdcall TcpClient(char * data,int size,char * recvBuf,int recvLen);
#else
	int __attribute__((__stdcall__)) UdpClient(char* data, int size,char * recvBuf,int recvLen);
	int __attribute__((__stdcall__)) TcpClient(char * data,int size,char * recvBuf,int recvLen);
#endif

	int Authority(char* sendBuf, int sendLen, char* recvBuf, int recvLen,int seq);

	int GetReplyInfo(const char* recvBuf, int size, int seq);

	int Client(const char * fn);

	int m_type = 0;

	int m_port=0;

	std::string m_ip="";
	
	std::string m_session="";

	std::string m_ssrc="";

	int m_sock = 0;

	int m_seconds = 0;

	std::string m_auth="";

	std::string m_seq = "";
	std::string m_ts = "";
};
