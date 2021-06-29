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

	explicit operator bool() const { return this->m_success; };
	friend bool operator== (const Version& v1, const Version& v2);
	friend bool operator!= (const Version& v1, const Version& v2);
	friend bool operator> (const Version& v1, const Version& v2);
	friend bool operator< (const Version& v1, const Version& v2);
	[[nodiscard]] const std::vector<int>& GetVersionInfo() const { return this->m_versionInfo; }
private:
	void Parse(const std::string& versionString);
	std::vector<int> m_versionInfo;
	bool m_success = true;
};

}  // namespace PotatoAlert
