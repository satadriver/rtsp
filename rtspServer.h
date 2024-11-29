
#pragma once


#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#endif

#include <iostream>

#pragma pack(1)



#pragma pack()




class RtspServer {
public:

	RtspServer( const char * port=0,const char * fn=0);
	~RtspServer();

#ifdef _WIN32
	static void * __stdcall RtcpServer(void*);

	static void* __stdcall RtpServer(void*);
#else
	static void * __attribute__((__stdcall__))  RtcpServer(void*);

	static void * __attribute__((__stdcall__))  RtpServer(void*);
#endif

	std::string GetTransport(int type);

	int ProcessRtsp(int sc);

	std::string GetRTPInfo(std::string url);

	int Server(const char* fn);

	std::string m_fn;
	int m_port=0;
	int m_type=0;

	int m_sock=0;

	int m_udpPort1=0;
	int m_udpPort2=0;

	std::string m_seq="";
	std::string m_rtpTime="";
	std::string m_ssrc="";

	char* m_file=0;
	char* m_data=0;
	int m_fs=0;
	int m_dataSize=0;
};
