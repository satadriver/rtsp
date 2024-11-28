

#include <string>

#include "utils.h"

using namespace std;

string getstring(const char* start, const  char* end, const  char* lpdata) {
	char* lphdr = 0;
	if (start == 0 || *start == 0) {
		lphdr = (char*)lpdata;
	}
	else {
		lphdr = strstr((char*)lpdata, start);
		if (lphdr) {
			lphdr += strlen(start);
		}
	}

	if (lphdr)
	{
		char* lpend = 0;
		if (end == 0 || *end == 0) {
			lpend = lphdr;
			return lphdr;
		}
		else {
			lpend = strstr(lphdr, end);
			if (lpend) {
				int len = lpend - lphdr;
				return string(lphdr, len);
			}

		}
	}
	return "";
}