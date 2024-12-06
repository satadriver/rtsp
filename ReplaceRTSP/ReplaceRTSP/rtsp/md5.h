
#pragma once
/* MD5.H - header file for MD5.C
 */

 /* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 rights reserved.

 License to copy and use this software is granted provided that it
 is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 Algorithm" in all material mentioning or referencing this software
 or this function.

 License is also granted to make and use derivative works provided
 that such works are identified as "derived from the RSA Data
 Security, Inc. MD5 Message-Digest Algorithm" in all material
 mentioning or referencing the derived work.

 RSA Data Security, Inc. makes no representations concerning either
 the merchantability of this software or the suitability of this
 software for any particular purpose. It is provided "as is"
 without express or implied warranty of any kind.

 These notices must be retained in any copies of any part of this
 documentation and/or software.
  */

#define MD5_VALUE_LEN      16      //MD5��ֵ�ĳ���
#define MD5_STRING_LEN     (32+1)  //MD5�ַ����ĳ���

typedef unsigned char MD5[MD5_VALUE_LEN];   //MD5����ֵ����
typedef char MD5_STR[MD5_STRING_LEN];       //MD5���ַ�������

/*  �����ļ���MD5ֵ
* INPUT:
*   filename:	Ҫ������ļ�
*   digest:		���ڴ洢������
* RETURN:
*   0:	��������
*   -1:	�޷����ļ�
*/
int MD5File(const char* filename, MD5 digest);

/*  �����ڴ���MD5ֵ
* INPUT:
*   buffer:	�ڴ�����ʼ��ַ
*   size:	�ڴ��ĳ���
*   digest:	���ڴ洢������
*/
void MD5Buffer(void* buffer, size_t size, MD5 digest);

/*  ����MD5������ȷ��
* RETURN:
*   0:	MD5��������
*   -1:	MD5�޷���������
*/
int MD5Test(void);

/*  ��MD5ֵ��ӡ����Ļ
* INPUT:
*   digest:	Ҫ��ӡ��MD5ֵ
*/
void MD5Print(MD5 digest);

/*  ��MD5ֵת��ΪMD5�ַ���
* INPUT:
*   digest:	����ת����MD5ֵ
*   str:	���ڴ��ת�����
*/
void MD5String(MD5 digest, MD5_STR str);


int GetMD5(unsigned char* data, int len, unsigned char* dst, int strFormat);