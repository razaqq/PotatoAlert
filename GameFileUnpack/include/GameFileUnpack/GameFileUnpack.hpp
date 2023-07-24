// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>


namespace PotatoAlert::GameFileUnpack {

static constexpr std::array g_IdxSignature = {
	std::byte{ 0x49 }, std::byte{ 0x53 },
	std::byte{ 0x46 }, std::byte{ 0x50 }
};

static constexpr uint32_t HeaderSize = 56;
struct IdxHeader
{
	int32_t Nodes;
	int32_t Files;
	int64_t ThirdOffset;
	int64_t TrailerOffset;
	std::array<std::byte, 12> FirstBlock;
	int64_t Unknown1;
	int64_t Unknown2;

	static std::optional<IdxHeader> Parse(std::span<Core::Byte> data);
};

static constexpr uint32_t NodeSize = 32;
struct Node
{
	std::string Name;
	uint64_t Id;
	uint64_t Parent;
	std::array<Core::Byte, 8> Unknown;

	static std::optional<Node> Parse(std::span<Core::Byte> data, std::span<Core::Byte> fullData);
};

static constexpr uint32_t FileRecordSize = 48;
struct FileRecord
{
	std::string PkgName;
	std::string Path;
	uint64_t Id;
	int64_t Offset;
	int32_t Size;
	int64_t UncompressedSize;

	static std::optional<FileRecord> Parse(std::span<Core::Byte> data, const std::unordered_map<uint64_t, Node>& nodes);
};


struct IdxFile
{
	std::string PkgName;
	std::unordered_map<uint64_t, Node> Nodes;
	std::unordered_map<std::string, FileRecord> Files;

	static std::optional<IdxFile> Parse(std::span<Core::Byte> data);
};

class DirectoryTree
{
public:
	struct TreeNode
	{
		std::unordered_map<std::string, TreeNode> Nodes;
		std::optional<FileRecord> File;
	};

	void Insert(const FileRecord& fileRecord);
	[[nodiscard]] std::optional<TreeNode> Find(std::string_view path) const;

private:
	TreeNode m_root;

	TreeNode& CreatePath(std::string_view path);
};

class Unpacker
{
public:
	Unpacker(const std::filesystem::path& pkgPath, const std::filesystem::path& idxPath);
	Unpacker(std::string_view pkgPath, std::string_view idxPath);
	bool Extract(std::string_view node, const std::filesystem::path& dst) const;
	bool Extract(std::string_view node, std::string_view dst) const;

private:
	DirectoryTree m_directoryTree;
	std::filesystem::path m_pkgPath;

	bool ExtractFile(const FileRecord& fileRecord, const std::filesystem::path& dst) const;
};

}  // namespace PotatoAlert::GameFileUnpack
