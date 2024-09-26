// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/Log.hpp"
#include "Core/StandardPaths.hpp"

#include <GameFileUnpack/GameFileUnpack.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <filesystem>

#include <QDir>
#include <QStandardPaths>


namespace fs = std::filesystem;
using PotatoAlert::Core::Byte;
using PotatoAlert::Core::File;
using namespace PotatoAlert::GameFileUnpack;
typedef DirectoryTree::TreeNode TreeNode;
using PotatoAlert::GameFileUnpack::Unpacker;

namespace {

static fs::path GetGameFileRootPath()
{
	const auto rootPath = PotatoAlert::Core::GetModuleRootPath();
	if (!rootPath.has_value())
	{
		std::exit(1);
	}
	return fs::path(rootPath.value()).remove_filename() / "GameFiles";
}

static fs::path GetTempDirectory()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const fs::path configPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation).append("/PotatoAlert")).filesystemAbsolutePath();
#else
	const fs::path configPath = PotatoAlert::Core::ToFilesystemAbsolutePath(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation).append("/PotatoAlert")));
#endif

	if (!fs::exists(configPath))
	{
		fs::create_directories(configPath);
	}

	return configPath;
}

static fs::path GetGameFilePath(std::string_view fileName)
{
	return GetGameFileRootPath() / fileName;
}

}

class TestRunListener : public Catch::EventListenerBase
{
public:
	using Catch::EventListenerBase::EventListenerBase;

	void testRunStarting(Catch::TestRunInfo const&) override
	{
		PotatoAlert::Core::Log::Init(PotatoAlert::Core::AppDataPath("PotatoAlert") / "GameFileUnpackTest.log");
	}
};
CATCH_REGISTER_LISTENER(TestRunListener)

TEST_CASE("GameFileUnpackTest_DirectoryTreeTest")
{
	DirectoryTree tree;

	tree.Insert(FileRecord{ "", "content/testFile.txt" });
	std::optional<TreeNode> record1 = tree.Find("content/testFile.txt");
	REQUIRE(record1);
	REQUIRE(record1->File);
	REQUIRE(record1->File->Path == "content/testFile.txt");

	tree.Insert(FileRecord{ "", "content/testFile2.txt" });
	std::optional<TreeNode> record2 = tree.Find("content/");
	REQUIRE(record2);
	REQUIRE_FALSE(record2->File);
	REQUIRE(record2->Nodes.contains("testFile.txt"));
	REQUIRE(record2->Nodes.contains("testFile2.txt"));
	REQUIRE_FALSE(record2->File.has_value());
}

TEST_CASE("GameFileUnpackTest_IdxFileTest")
{
	fs::path idxFilePath = GetGameFilePath("vehicles_level6_usa.idx");
	fs::path pkgFilePath = GetGameFilePath("vehicles_level6_usa_0001.pkg");

	File file = File::Open(idxFilePath, File::Flags::Open | File::Flags::Read);
	REQUIRE(file);
	std::vector<Byte> data;
	REQUIRE(file.ReadAll(data));
	UnpackResult<IdxFile> idxFileRes = IdxFile::Parse(data);
	REQUIRE(idxFileRes);
	IdxFile& idxFile = *idxFileRes;
	REQUIRE(idxFile.Files.size() == 39);
	REQUIRE(idxFile.Nodes.size() == 54);

	REQUIRE(idxFile.Files[4].Size == 1799);
	REQUIRE(idxFile.Files[4].UncompressedSize == 2872);
	REQUIRE(idxFile.Files[4].NodeId == 9050552029570354906);
	REQUIRE(idxFile.Files[4].Offset == 0x6226F);
	REQUIRE(idxFile.Files[4].Path ==
			"content/gameplay/usa/gun/secondary/textures/AGS206_3in50_MK21_Sub_ao.dds");
	REQUIRE(idxFile.Nodes[1704543301444328984].Name == "AGS206_3in50_MK21_Sub_mg.dds");
	REQUIRE(idxFile.PkgName == "vehicles_level6_usa_0001.pkg");
}

TEST_CASE("GameFileUnpackTest_UnpackerTest")
{
	Unpacker unpacker(GetGameFileRootPath(), GetGameFileRootPath());
	REQUIRE(unpacker.Parse());
	REQUIRE(unpacker.Extract(
			R"(content/gameplay/usa/gun/secondary/textures/AGS206_3in50_MK21_Sub_ao.dds)",
			GetTempDirectory())
	);
}
