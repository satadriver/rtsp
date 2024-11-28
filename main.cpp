

#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
#include <time.h>

#include "packet.h"
#include "md5.h"
#include "fileUtils.h"
#include "rtspClient.h"
#include "h264.h"
#include "rtspServer.h"

int main(int argc, char** argv) {

	int ret = 0;

	FWriter(PACKET_CMD_FILENAME, "\r\n", 2, 1);

	if (argc < 3) {
		printf("example:%s 192.168.1.101 554",argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "--rtsp2h264") == 0) {
		char rtspPath[104];
		sprintf(rtspPath, "rtsp://root:pass@%s/axis-media/media.amp", argv[2]);
		rtsp2h264(rtspPath,15);
		//rtsp2h264((char*)"rtsp://127.0.0.1:554/live");
		return 0;
	}
	else if (strcmp(argv[1], "--server") == 0) {
		int port = atoi(argv[2]);
		if (port == 0) {
			port = 554;
		}
		RtspServer* server = new RtspServer(argv[1], port);
		server->Server((char*)"rtsp.h264");
	}
	else if (strcmp(argv[1], "--client") == 0) {
		int port = atoi(argv[3]);
		RtspClient* network = new RtspClient(0, argv[2], port);
		network->Client((char*)"rtsp.h264");
	}

	return 0;
}