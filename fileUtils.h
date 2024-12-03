
#pragma once


#ifdef _WIN32
#else
#include <cstring>
#endif
#include <iostream>

#include "public.h"



#pragma pack(1)





#pragma pack()


int GetPathFromFullName(char* strFullName, char* strDst);

int FReader(std::string filename, char** lpbuf, int* bufsize);

int FWriter(std::string filename, const char* lpdate, int datesize, int cover);

int FWriter(std::string filename, const char* lpdate, int datesize);
