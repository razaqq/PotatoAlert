// Copyright 2020 <github.com/razaqq>
#pragma once

#include <vector>
#include <string>


namespace PotatoAlert {

    class Version
    {
    public:
        explicit Version(std::string& versionString);
        explicit Version(const char* versionString);

        friend bool operator== (const Version& v1, const Version& v2);
        friend bool operator> (const Version& v1, const Version& v2);
        friend bool operator< (const Version& v1, const Version& v2);
        std::vector<int>& getVersionInfo() { return this->versionInfo; };
    private:
        void parse(std::string& versionString);
        std::vector<int> versionInfo;
        bool success = true;
    };

} // namespace PotatoAlert
