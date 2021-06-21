// Copyright 2020 <github.com/razaqq>
#pragma once

#include <vector>
#include <string>


namespace PotatoAlert {

class Version
{
public:
	explicit Version(const std::string& versionString);
	explicit Version(const char* versionString);

	explicit operator bool() const { return this->success; };
	friend bool operator== (const Version& v1, const Version& v2);
	friend bool operator!= (const Version& v1, const Version& v2);
	friend bool operator> (const Version& v1, const Version& v2);
	friend bool operator< (const Version& v1, const Version& v2);
	[[nodiscard]] const std::vector<int>& GetVersionInfo() const { return this->versionInfo; }
private:
	void Parse(const std::string& versionString);
	std::vector<int> versionInfo;
	bool success = true;
};

}  // namespace PotatoAlert
