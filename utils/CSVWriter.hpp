// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <fstream>
#include <initializer_list>


namespace PotatoAlert {

class CSVWriter
{
public:
	CSVWriter();
	~CSVWriter();
	void saveMatch(const std::string& jsonObj);
private:
	std::ofstream file;
	static std::string getFilePath();
};

}  // namespace PotatoAlert
