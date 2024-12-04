

#include "http.h"
#include <iostream>
#include <vector>
#include "public.h"

#include <cstring>

using namespace std;



int Contain(string hdr,string v) {
	string url = GetUrl(hdr);
	vector <string> vs = GetUrlParams(url);
	for (unsigned int i = 0; i < vs.size(); i++) {
		if (vs[i].find(v) != -1) {
			return TRUE;
		}
	}
	return FALSE;
}


vector<string> GetUrlParams(string url) {
	vector<string> ret;
	int pos = url.find("?");
	if (pos != -1)
	{
		string substr = url.substr(pos + 1);

		while (TRUE)
		{
			pos = substr.find("&");
			if (pos != -1)
			{
				ret.push_back(substr.substr(0, pos));
				substr = substr.substr(pos + 1);
			}
			else {
				ret.push_back(substr);
				break;
			}
		}
	}
	return ret;
}






string GetHeaderValue(const char* lphttphdr, string  searchkey) {

	string key = "\r\n" + searchkey + ": ";
	char* phdr = strstr((char*)lphttphdr, key.c_str());
	if (phdr)
	{
		phdr += key.length();
		char* pend = strstr(phdr, "\r\n");
		int len = pend - phdr;
		if (pend && len > 0 && len < 256)
		{
			string value = string(phdr, len);
			return value;
		}
	}

	return "";
}


string GetHttpHeader(const char* data, int len, char** lphttpdata) {

	char* lphdr = strstr((char*)data, "\r\n\r\n");
	if (lphdr == FALSE)
	{
		*lphttpdata = 0;
		return string(data);
	}

	lphdr += 4;
	string httphdr = string(data, lphdr - data);
	*lphttpdata = lphdr;
	return httphdr;
}




int GetUrlCmd(const char* url) {
	if (memcmp(url, "DESCRIBE ", 9) == 0) {
		return 9;
	}
	else if (memcmp(url, "ANNOUNCE ", 9) == 0) {
		return 9;
	}
	else if (memcmp(url, "OPTIONS ", 8) == 0) {
		return 8;
	}
	else if (memcmp(url, "PLAY ", 5) == 0) {
		return 5;
	}
	else if (memcmp(url, "RECORD ", 7) == 0) {
		return 7;
	}
	else if (memcmp(url, "SETUP ", 6) == 0) {
		return 6;
	}
	else if (memcmp(url, "TEARDOWN ", 9) == 0) {
		return 9;
	}
	else if (memcmp(url, "SET_PARAMETER ", 14) == 0) {
		return 14;
	}
	else if (memcmp(url, "GET_PARAMETER ", 14) == 0) {
		return 14;
	}
	return 0;
}


string GetUri(string url) {
	int start = GetUrlCmd(url.c_str());
	if (start <= 0)
	{
		start = 0;
	}

	int pos = url.find("?");
	if (pos != -1)
	{
		url = url.substr(start, pos);
	}
	else {
		url = url.substr(start);
	}

	return url;
}


string GetUrl_old(string url) {
	int start = GetUrlCmd(url.c_str());
	if (start <= 0)
	{
		return url;
	}
	url = url.substr(start);

	int pos = url.find(" ");
	if (pos != -1)
	{
		url = url.substr(0, pos);
	}

	return url;
}



string GetUrl(string url) {
	int start = url.find(" ");
	if (start <= 0)
	{
		return url;
	}
	url = url.substr(start + 1);

	int pos = url.find(" ");
	if (pos != -1)
	{
		url = url.substr(0, pos);
	}

	return url;
}




string GetIPFromUrl(string url) {
	int start = url.find("rtsp://");
	if (start <= 0)
	{
		return url;
	}
	url = url.substr(start + strlen("rtsp://"));

	int pos = url.find("/");
	if (pos != -1)
	{
		url = url.substr(0, pos);
	}

	pos = url.find(":");
	if (pos != -1)
	{
		url = url.substr(0, pos);
	}

	return url;
}
