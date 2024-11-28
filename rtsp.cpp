
#include <iostream>

#include "rtsp.h"
#include "packet.h"

#include "http.h"
#include "fileUtils.h"

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
			perror("send\r\n");
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