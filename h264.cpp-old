


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma comment(lib,"lib/avcodec.lib")
#pragma comment(lib,"lib/avdevice.lib")
#pragma comment(lib,"lib/avfilter.lib")

#pragma comment(lib,"lib/avformat.lib")
#pragma comment(lib,"lib/avutil.lib")
#pragma comment(lib,"lib/postproc.lib")
#pragma comment(lib,"lib/swscale.lib")

#ifdef __cplusplus 
extern "C"
{
#endif
#include <libavformat/avformat.h> 
#include <libavcodec/avcodec.h> 
#include <libswscale\\swscale.h>

#ifdef __cplusplus
}
#endif



#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus 
extern "C"
{
#endif
	/*Include ffmpeg header file*/
#include <libavformat/avformat.h> 
#ifdef __cplusplus
}
#endif





int rtsp2h264_old(char * filepath,int seconds)
{

	AVFormatContext* pFormatCtx;
	int				i, videoindex;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	AVFrame* pFrame, * pFrameYUV;
	uint8_t* out_buffer;
	AVPacket* packet;
	int ret, got_picture;

	struct SwsContext* img_convert_ctx;
	//下面是RTSP地址,按照使用的网络摄像机的URL格式即可
	//char filepath[] = "rtsp://user:passport@192.168.x.xxx:xxx/h264/ch1/main/av_stream";

	av_register_all();    //初始化所有组件，只有调用了该函数，才能使用复用器和编解码器,在所有FFmpeg程序中第一个被调用
	avformat_network_init();   //加载socket库以及网络加密协议相关的库，为后续使用网络相关提供支持
	pFormatCtx = avformat_alloc_context();    //用来申请AVFormatContext类变量并初始化默认参数。申请的空间通过void avformat_free_context(AVFormatContext *s)函数释放。

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)//打开网络流或文件流
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)    //读取一部分视音频数据并且获得一些相关的信息
		{
			printf("Couldn't find stream information.\n");
			return -1;
		}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)     //在多个数据流中找到视频流 video stream(类型为AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);    //查找video stream 相对应的解码器
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)  //打开解码器
	{
		printf("Could not open codec.\n");
		return -1;
	}
	pFrame = av_frame_alloc();    //为解码帧分配内存
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture*)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

	//Output Info---输出一些文件（RTSP）信息
	printf("---------------- File Information ---------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, 4, NULL, NULL, NULL);


	packet = (AVPacket*)av_malloc(sizeof(AVPacket));

	FILE* fpSave;
	if ((fpSave = fopen("test.h264", "wb")) == NULL) //h264保存的文件名
		return 0;
	for (int i = 0;i < 200;i ++)
	{
		//------------------------------
	if (av_read_frame(pFormatCtx, packet) >= 0)  //从流中读取读取数据到Packet中
	{
		if (packet->stream_index == videoindex)
		{
			fwrite(packet->data, 1, packet->size, fpSave);//写数据到文件中
		}
		av_free_packet(packet);
	}
	}


	//--------------
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);     //需要关闭avformat_open_input打开的输入流，avcodec_open2打开的CODEC
	avformat_close_input(&pFormatCtx);

	return 0;
}









#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>

#define ADTS_HEADER_LEN  7;

const int sampling_frequencies[] = {
    96000,  // 0x0
    88200,  // 0x1
    64000,  // 0x2
    48000,  // 0x3
    44100,  // 0x4
    32000,  // 0x5
    24000,  // 0x6
    22050,  // 0x7
    16000,  // 0x8
    12000,  // 0x9
    11025,  // 0xa
    8000   // 0xb
    // 0xc d e f是保留的
};

int adts_header(char* const p_adts_header, const int data_length,
    const int profile, const int samplerate,
    const int channels)
{

    int sampling_frequency_index = 3; // 默认使用48000hz
    int adtsLen = data_length + 7;

    int frequencies_size = sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0]);
    int i = 0;
    for (i = 0; i < frequencies_size; i++)
    {
        if (sampling_frequencies[i] == samplerate)
        {
            sampling_frequency_index = i;
            break;
        }
    }
    if (i >= frequencies_size)
    {
        printf("unsupport samplerate:%d\n", samplerate);
        return -1;
    }

    p_adts_header[0] = 0xff;         //syncword:0xfff                          高8bits
    p_adts_header[1] = 0xf0;         //syncword:0xfff                          低4bits
    p_adts_header[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    p_adts_header[1] |= (0 << 1);    //Layer:0                                 2bits
    p_adts_header[1] |= 1;           //protection absent:1                     1bit

    p_adts_header[2] = (profile) << 6;            //profile:profile               2bits
    p_adts_header[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits
    p_adts_header[2] |= (0 << 1);             //private bit:0                   1bit
    p_adts_header[2] |= (channels & 0x04) >> 2; //channel configuration:channels  高1bit

    p_adts_header[3] = (channels & 0x03) << 6; //channel configuration:channels 低2bits
    p_adts_header[3] |= (0 << 5);               //original：0                1bit
    p_adts_header[3] |= (0 << 4);               //home：0                    1bit
    p_adts_header[3] |= (0 << 3);               //copyright id bit：0        1bit
    p_adts_header[3] |= (0 << 2);               //copyright id start：0      1bit
    p_adts_header[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    p_adts_header[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    p_adts_header[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    p_adts_header[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    p_adts_header[6] = 0xfc;      //‭11111100‬       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

    return 0;
}

int rtsp2h264( char * in_filename,int seconds)
{
    int ret = -1;
    char errors[1024];


    char* aac_filename = NULL;

    FILE* aac_fd = NULL;

    int audio_index = -1;
    int len = 0;


    AVFormatContext* ifmt_ctx = NULL;
    AVPacket pkt;

    // 设置打印级别
    av_log_set_level(AV_LOG_DEBUG);

    aac_filename = (char*)"test.h264";

    if (in_filename == NULL || aac_filename == NULL)
    {
        av_log(NULL, AV_LOG_DEBUG, "src or dts file is null, plz check them!\n");
        return -1;
    }

    aac_fd = fopen(aac_filename, "wb");
    if (!aac_fd)
    {
        av_log(NULL, AV_LOG_DEBUG, "Could not open destination file %s\n", aac_filename);
        return -1;
    }

    // 打开输入文件
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL)) < 0)
    {
        av_strerror(ret, errors, 1024);
        av_log(NULL, AV_LOG_DEBUG, "Could not open source file: %s, %d(%s)\n",
            in_filename,
            ret,
            errors);
        return -1;
    }

    // 获取解码器信息
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
    {
        av_strerror(ret, errors, 1024);
        av_log(NULL, AV_LOG_DEBUG, "failed to find stream information: %s, %d(%s)\n",
            in_filename,
            ret,
            errors);
        return -1;
    }

    // dump媒体信息
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // 初始化packet
    av_init_packet(&pkt);

    // 查找audio对应的steam index
    audio_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (audio_index < 0)
    {
        av_log(NULL, AV_LOG_DEBUG, "Could not find %s stream in input file %s\n",
            av_get_media_type_string(AVMEDIA_TYPE_AUDIO),
            in_filename);
        //return AVERROR(EINVAL);
    }

    // 打印AAC级别
    printf("audio profile:%d, FF_PROFILE_AAC_LOW:%d\n",
        ifmt_ctx->streams[audio_index]->codecpar->profile,
        FF_PROFILE_AAC_LOW);

    if (ifmt_ctx->streams[audio_index]->codecpar->codec_id != AVMEDIA_TYPE_VIDEO)
    {
        printf("the media file no contain AAC stream, it's codec_id is %d\n",
            ifmt_ctx->streams[audio_index]->codecpar->codec_id);
        //goto failed;
    }
    // 读取媒体文件，并把aac数据帧写入到本地文件
    while (av_read_frame(ifmt_ctx, &pkt) >= 0)
    {
        if (pkt.stream_index == audio_index)
        {
            char adts_header_buf[7] = { 0 };
            adts_header(adts_header_buf, pkt.size,
                ifmt_ctx->streams[audio_index]->codecpar->profile,
                ifmt_ctx->streams[audio_index]->codecpar->sample_rate,
                ifmt_ctx->streams[audio_index]->codecpar->channels);
            fwrite(adts_header_buf, 1, 7, aac_fd);  // 写adts header , ts流不适用，ts流分离出来的packet带了adts header
            len = fwrite(pkt.data, 1, pkt.size, aac_fd);   // 写adts data
            if (len != pkt.size)
            {
                av_log(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal pkt.size(%d, %d)\n",
                    len,
                    pkt.size);
            }
        }
        av_packet_unref(&pkt);
    }

failed:
    // 关闭输入文件
    if (ifmt_ctx)
    {
        avformat_close_input(&ifmt_ctx);
    }
    if (aac_fd)
    {
        fclose(aac_fd);
    }

    return 0;
}
