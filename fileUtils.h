#pragma once


#pragma once


#include <windows.h>
#include <iostream>



int GetPathFromFullName(char* strFullName, char* strDst);

int FReader(std::string filename, char** lpbuf, int* bufsize);

int FWriter(std::string filename, const char* lpdate, int datesize, int cover);

int FWriter(std::string filename, const char* lpdate, int datesize);
