#pragma once
#include <string>

class UFileHandler
{
public:
	UFileHandler(const std::string& path) {};
	std::string read() {};
	void wtite(const std::string& content) {};
	void append(const std::string& content) {};


};