// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <fstream>


namespace PotatoAlert {

class Logger
{
public:
	Logger();
	~Logger();
	void Debug(const char* text);
	void Info(const char* text);
	void Warn(const char* text);
	void Error(const char* text);
private:
	std::ofstream _ofs;
	std::string getTimeString() const;
	Logger(const Logger&);
	Logger& operator=(const Logger&);
};

};  // namespace PotatoAlert
