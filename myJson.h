
#pragma once


#include <iostream>

#include <vector>



std::string JsonQuotaStringValue(std::string data, std::string key);

std::string JsonStringValue(std::string data, std::string key);

std::string JsonBaseValue(std::string data, std::string key);

std::vector<std::string>* JsonFromArray(std::string data);

std::vector<std::string>* JsonFromRoot(std::string substr);

std::string GetStringValue(std::string data, std::string key);
