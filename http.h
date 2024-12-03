#pragma once

#include <iostream>
#include <vector>

int Contain(std::string hdr, std::string v);

int GetUrlCmd(const char* url);

std::string GetUri(std::string url);

std::string GetUrl(std::string url);

std::string GetHttpHeader(const char* data, int len, char** lphttpdata);

std::vector<std::string> GetUrlParams(std::string url);

std::string GetHeaderValue(const char* lphttphdr, std::string  searchkey);

std::string GetIPFromUrl(std::string url);