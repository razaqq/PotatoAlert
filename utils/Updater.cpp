// Copyright 2020 <github.com/razaqq>

#include "Updater.h"
#include <windows.h>
#include <string>
#include <filesystem>


using PotatoAlert::Updater;
namespace fs = std::filesystem;

const char* updateURL = "";
const char* versionURL = "";

bool Updater::updateAvailable()
{
	return false;  // TODO
}

void Updater::update()
{
    auto root = fs::absolute(fs::current_path());

    auto temp = (root / "PotatoAlert_temp.exe").string();
    remove(temp.c_str()); // ignore return code

    auto src = (root / "PotatoAlert_new.exe").string();
    auto dst = (root / "PotatoAlert.exe").string();

    rename(dst.c_str(), temp.c_str());
    CopyFile(src.c_str(), dst.c_str(), false);
    static char buffer[512];
    strcpy(buffer, dst.c_str());  // DEPRECATED

    /* CreateProcess API initialization */
    STARTUPINFO siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);

    ::CreateProcess(buffer, // application name/path
                    nullptr, // command line (optional)
                    nullptr, // no process attributes (default)
                    nullptr, // default security attributes
                    false,
                    CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE,
                    nullptr, // default env
                    nullptr, // default working dir
                    &siStartupInfo,
                    &piProcessInfo);


    ::TerminateProcess( GetCurrentProcess(),0);
    ::ExitProcess(0); // exit this process

    // this does not return.
}

void Updater::download()
{

}
