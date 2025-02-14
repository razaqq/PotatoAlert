// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Preprocessor.hpp"
#include "Core/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>


namespace PotatoAlert::GameFileUnpack {

static constexpr uint32_t HeaderSize = 0x38;
static constexpr uint32_t HeaderDataOffset = 0x10;  // size until version
struct IdxHeader
{
	uint32_t Endianness;
	uint32_t MurmurHash;
	uint32_t Version;

	uint32_t NodeCount;
	uint32_t FileCount;
	uint32_t VolumeCount;  // Number of pkg files

	uint32_t Unknown;

	// offsets from base of this struct
	uint64_t NodeTablePtr;
	uint64_t FileRecordTablePtr;
	uint64_t VolumeTablePtr;

	static Core::Result<IdxHeader> Parse(std::span<const Core::Byte> data);
};
static_assert(sizeof(IdxHeader) == HeaderSize);

static constexpr uint32_t NodeSize = 0x20;
struct PA_API Node
{
	std::string Name;
	uint64_t Id;
	uint64_t Parent;

	static Core::Result<Node> Parse(std::span<const Core::Byte> data, uint64_t offset, std::span<const Core::Byte> fullData);
};

static constexpr uint32_t FileRecordSize = 0x30;
struct PA_API FileRecord
{
	std::string PkgName;
	std::string Path;
	uint64_t NodeId;
	uint64_t VolumeId;
	uint64_t Offset;
	uint64_t CompressionInfo;
	uint32_t Size;
	uint32_t Crc32;
	uint64_t UncompressedSize;
	uint32_t Padding;

	static Core::Result<FileRecord> Parse(std::span<const Core::Byte> data, const std::unordered_map<uint64_t, Node>& nodes);
};

static constexpr uint32_t VolumeSize = 0x18;
struct PA_API Volume
{
	std::string Name;
	uint64_t Id;

	static Core::Result<Volume> Parse(std::span<const Core::Byte> data, uint64_t offset, std::span<const Core::Byte> fullData);
};

struct PA_API IdxFile
{
	std::string_view PkgName;
	std::unordered_map<uint64_t, Node> Nodes;
	std::vector<FileRecord> Files;
	std::vector<Volume> Volumes;

	static Core::Result<IdxFile> Parse(std::span<const Core::Byte> data);
};

class PA_API DirectoryTree
{
public:
	struct TreeNode
	{
		std::string Path;
		std::unordered_map<std::string, TreeNode> Nodes;
		std::optional<FileRecord> File;
	};

	void Insert(const FileRecord& fileRecord);
	[[nodiscard]] std::optional<TreeNode> Find(std::string_view path) const;
	[[nodiscard]] const TreeNode& GetRoot() const;

private:
	TreeNode m_root;

	TreeNode& CreatePath(std::string_view path);
};

class PA_API Unpacker
{
public:
	explicit Unpacker(std::filesystem::path pkgPath, std::filesystem::path idxPath);
	[[nodiscard]] Core::Result<void> Parse();
	[[nodiscard]] Core::Result<void> Extract(std::string_view nodeName, const std::filesystem::path& dst, bool preservePath = true) const;
	[[nodiscard]] Core::Result<void> Extract(std::string_view nodeName, std::string_view pattern, const std::filesystem::path& dst, bool preservePath = true) const;
	[[nodiscard]] const DirectoryTree& GetDirectoryTree() const;

private:
	DirectoryTree m_directoryTree;
	std::filesystem::path m_pkgPath;
	std::filesystem::path m_idxPath;

	[[nodiscard]] Core::Result<void> ExtractFile(const FileRecord& fileRecord, const std::filesystem::path& dst) const;
};

}  // namespace PotatoAlert::GameFileUnpack
