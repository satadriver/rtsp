

#include "fileUtils.h"
#include <stdio.h>




using namespace std;


int GetPathFromFullName(char* strFullName, char* strDst)
{
	strcpy(strDst, strFullName);

	for (int i = strlen(strDst) - 1; i >= 0; i--)
	{
		if (strDst[i] == 0x5c)
		{
			strDst[i + 1] = 0;
			return TRUE;
		}
	}

	return FALSE;
}




int FWriter(string filename, const char* lpdate, int datesize) {
	int ret = 0;

	FILE* fp = fopen(filename.c_str(), "ab+");
	if (fp == 0)
	{
		printf("FWriter fopen file:%s error\r\n", filename.c_str());
		return FALSE;
	}
	ret = fseek(fp, 0, SEEK_END);

	ret = fwrite(lpdate, 1, datesize, fp);
	fclose(fp);
	if (ret == FALSE)
	{
		return FALSE;
	}

	return datesize;
}


int FWriter(string filename, const char* lpdate, int datesize, int cover) {
	int ret = 0;

	FILE* fp = 0;
	if (cover) {
		fp = fopen(filename.c_str(), "wb");
	}
	else {
		fp = fopen(filename.c_str(), "ab+");
	}

	if (fp == 0)
	{
		printf("FWriter fopen file:%s error\r\n", filename.c_str());
		return FALSE;
	}

	ret = fseek(fp, 0, SEEK_END);

	ret = fwrite(lpdate, 1, datesize, fp);
	fclose(fp);
	if (ret == FALSE)
	{
		return FALSE;
	}

	return datesize;
}



int FReader(string filename, char** lpbuf, int* bufsize) {
	if (lpbuf == 0 ) {
		return 0;
	}

	int ret = 0;

	FILE* fp = fopen(filename.c_str(), "rb");
	if (fp == 0)
	{
		printf("FReader fopen file:%s error\r\n", filename.c_str());
		return FALSE;
	}

	ret = fseek(fp, 0, SEEK_END);

	int filesize = ftell(fp);
	if (filesize > 0) {
		ret = fseek(fp, 0, SEEK_SET);

		*bufsize = filesize;
		if (*lpbuf == 0) {
			*lpbuf = new char[filesize + 16];
		}
		
		ret = fread(*lpbuf, 1, filesize, fp);
	}

	fclose(fp);
	if (ret == FALSE)
	{
		delete* lpbuf;
		return FALSE;
	}

	*(*lpbuf + filesize) = 0;
	return filesize;
}



