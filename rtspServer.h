
#pragma once


#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#endif

#include <iostream>

#include "rtsp.h"




class RtspServer {
public:

	RtspServer( const char * port=0,const char * fn=0);
	~RtspServer();

#ifdef _WIN32
	static void * __stdcall RtcpServer(void*);

	static void* __stdcall RtpServer(void*);
	static void* __stdcall ProcessRtsp(void * param);
#else
	static void * __attribute__((__stdcall__))  RtcpServer(void*);

	static void * __attribute__((__stdcall__))  RtpServer(void*);
	static void * __attribute__((__stdcall__)) ProcessRtsp(void* );
#endif

	int TrimFrame();

	int GetFrameBorder();

	int PlayFrame(int sc,char* recvBuf, int recvLen, char* sendBuf, int sendLen);

	std::string GetTransport(int type,int ,int);

	std::string GetRTPInfo(std::string url);

	int Server(const char* fn);

	std::string m_fn="";
	int m_port=0;
	int m_type=0;

	int m_sock=0;

	std::string m_seq="";
	//std::string m_rtpTime="";
	std::string m_ssrc="";

	char* m_file=0;
	char* m_data=0;
	int m_fs=0;
	int m_dataSize=0;

	unsigned int m_rtpTm = 0;

	int m_frameTotal = 0;

	unsigned int m_playDelay = 0;

	RtpHeader* m_firstRtp = 0;
	RtpHeader* m_lastRtp = 0;
};



#pragma pack(1)

typedef struct {
	int sc;
	RtspServer* ptr;
	unsigned long pid;
}RtspThreadParam;

#pragma pack()