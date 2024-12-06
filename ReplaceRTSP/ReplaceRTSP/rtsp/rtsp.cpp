
#include <iostream>


#include "packet.h"

#include "http.h"
#include "fileUtils.h"

#ifdef _WIN32
#else
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <arpa/inet.h>
#endif

#include "rtsp.h"


using namespace std;


int ParseRtpStream(const char * sfn, const char * dfn) {

	FWriter(dfn, "", 0, 1);

	char* file = 0;
	int fs = 0;

	int ret = FReader(sfn, &file, &fs);
	if (ret <= 0) {
		return 0;
	}

	char* data = 0;
	string hdr = GetHttpHeader(file, fs, &data);

	char* ptr = data;
	while (ptr - file < fs)
	{
		RtspHeader* rtsp = (RtspHeader*)ptr;
		if (rtsp->magic != 0x24) {
			break;
		}

		int size = ntohs(rtsp->length);

		RtpHeader* rtp = (RtpHeader*)(ptr + sizeof(RtspHeader));

		char* rtpdata = (char*)rtp + sizeof(RtpHeader);
		int rtpds = size - sizeof(RtpHeader) ;

		//FWriter(dfn, "\x00\x00\x00\x01", 4, 0);

		FWriter(dfn, rtpdata, rtpds, 0);

		ptr = ptr + sizeof(RtspHeader) + size;
	}

	delete[] file;
	return 0;
}





int SendRtpStream(const char* sfn, int sock) {

	char* file = 0;
	int fs = 0;

	int ret = FReader(sfn, &file, &fs);
	if (ret <= 0) {
		return 0;
	}

	char* data = 0;
	string hdr = GetHttpHeader(file, fs, &data);

	char* ptr = data;
	while (ptr - file < fs)
	{
		RtspHeader* rtsp = (RtspHeader*)ptr;
		if (rtsp->magic != 0x24) {
			break;
		}

		int size = ntohs(rtsp->length);

		RtpHeader* rtp = (RtpHeader*)(ptr + sizeof(RtspHeader));

		char* rtpdata = (char*)rtp + sizeof(RtpHeader);
		int rtpds = size - sizeof(RtpHeader);

		//int total = fs - (ptr - file);
		//int sendLen = send(sock, (char*)rtsp, total, 0);
		int sendLen = send(sock, (char*)rtsp, sizeof(RtspHeader) + size, 0);
		if (sendLen <= 0) {
			//perror("send\r\n");
			break;
		}
		//break;

		//int recvLen = recv(sock, (char*)rtsp, 0x1000, 0);

		ptr = ptr + sizeof(RtspHeader) + size;

		//Sleep(100);
	}

	delete[] file;
	return 0;
}






int LzyFormat(const char* sfn, const char* dfn,unsigned long ts) {

	FWriter(dfn, "", 0, 1);

	char* file = 0;
	int fs = 0;

	int ret = FReader(sfn, &file, &fs);
	if (ret <= 0) {
		return 0;
	}

	char* data = 0;
	string hdr = GetHttpHeader(file, fs, &data);

	char* ptr = data;
	int dfnSize = 0;

	unsigned int tag = 0;

	int not0 = 0;
	int equal0 = 0;
	int cnt = 0;

	while (ptr - file < fs)
	{
		RtspHeader* rtsp = (RtspHeader*)ptr;
		if (rtsp->magic != 0x24) {
			break;
		}

		int size = ntohs(rtsp->length);

		RtpHeader* rtp = (RtpHeader*)(ptr + sizeof(RtspHeader));

		char* rtpdata = (char*)rtp + sizeof(RtpHeader);
		int rtpds = size - sizeof(RtpHeader);

		if (ts == 0 && tag == 0) {
			ts = ntohl(rtp->ts);
			tag = 1;
		}

		unsigned int offset = ntohl(rtp->ts) - ts;
		if (offset != 0) {
			not0++;
			// printf("frame:%d, not 0:%d, ts:%x\r\n",cnt,not0,ntohl(rtp->ts));
		}
		else {
			equal0++;
			// printf("frame:%d,equal 0:%d, ts:%x\r\n", cnt,equal0, ntohl(rtp->ts));
		}
		cnt++;

		ts = ntohl(rtp->ts);

		LzyHeader lzy;
		lzy.size = ntohl(rtpds);
		lzy.tsOffset = ntohl(offset );

		FWriter(dfn, (char*)&lzy, sizeof(LzyHeader), 0);

		FWriter(dfn, (char*)rtpdata, rtpds, 0);

		ptr = ptr + sizeof(RtspHeader) + size;
	}

	delete[] file;

	// printf("frame:%d, not 0:%d, equal 0:%d\r\n", cnt, not0, equal0);
	return 0;
}