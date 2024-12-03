
#pragma once

#ifndef _WIN32
#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#endif





#pragma pack(1)

typedef struct RtspHeader {
    uint8_t  magic;        // 0x24
    uint8_t  channel;
    uint16_t length;
    uint8_t  payload[0];   // >> RtpHeader
} ;


typedef struct RtpHeader {
#if 0
    uint8_t  version : 2;
    uint8_t  padding : 1;
    uint8_t  externsion : 1;
    uint8_t  CSrcId : 4;
    uint8_t  marker : 1;
    uint8_t  pt : 7;          // payload type
#else
    uint8_t  exts;
    uint8_t  type;
#endif
    uint16_t seqNo;         // Sequence number
    uint32_t ts;            // Timestamp
    uint32_t SyncSrcId;     // Synchronization source (SSRC) identifier
                            // Contributing source (SSRC_n) identifier
    uint8_t payload[0];     // Frame data
} ;



//FU Indicator定义实际就是H264的NAL Header，H264
typedef struct _tagFUIndicator
{
    uint8_t F : 1;       /*1 bit，forbidden_zero_bit（禁止位），一般为0*/
    uint8_t NRI : 2;     /*2 bit，nal_ref_idc（nal参考级别），代表这一个NAL的重要级别，比如sps、pps、idr该值一般为11（二进制），由*/
    uint8_t TYPE : 5;    /* set to 28 or 29 */
} FUIndicator;




//FU Header定义，H264
typedef struct _tagH264FUHeader
{
    uint8_t S : 1;       /*1 bit，Start位，当设置为1时，Start位表示一个分片NAL单元的开始。当FU有效载荷不是分片NAL单元有效载荷起始时，该值被设置为0*/
    uint8_t E : 1;       /*1 bit，End位，当设置为1时，表示分片NAL单元的结束，即当该NAL单元分片最后一个分片时，该值为1，否则End位被设置为0*/
    uint8_t R : 1;       /*1 bit，预留位，一般为0*/
    uint8_t Type : 5;    /*5 bit，nalu_unit_type， set to nal type */
} H264FUHeader;



typedef struct tagH265NalHeader
{
    uint8_t F : 1;       /* start flag */
    uint8_t Type : 6;       /* end flag */
    uint8_t LayerId : 6;       /* end flag */
    uint8_t Tid : 3;       /* set to nal type */
} H265NalHeader;



typedef struct {
    unsigned int size;
    unsigned int tsOffset;
    char data[0];
}LzyHeader;

#pragma pack()


int ParseRtpStream(const char* sfn, const char* dfn);


int SendRtpStream(const char* sfn, int sock);

int LzyFormat(const char* sfn, const char* dfn,unsigned long long ts);
