// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>


namespace PotatoAlert::GameFileUnpack {

using UnpackError = std::string;
template<typename T>
using UnpackResult = Core::Result<T, UnpackError>;

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

	static UnpackResult<IdxHeader> Parse(std::span<const Core::Byte> data);
};
static_assert(sizeof(IdxHeader) == HeaderSize);

static constexpr uint32_t NodeSize = 0x20;
struct Node
{
	std::string Name;
	uint64_t Id;
	uint64_t Parent;

	static UnpackResult<Node> Parse(std::span<const Core::Byte> data, uint64_t offset, std::span<const Core::Byte> fullData);
};

static constexpr uint32_t FileRecordSize = 0x30;
struct FileRecord
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

	static UnpackResult<FileRecord> Parse(std::span<const Core::Byte> data, const std::unordered_map<uint64_t, Node>& nodes);
};

static constexpr uint32_t VolumeSize = 0x18;
struct Volume
{
	std::string Name;
	uint64_t Id;

	static UnpackResult<Volume> Parse(std::span<const Core::Byte> data, uint64_t offset, std::span<const Core::Byte> fullData);
};

struct IdxFile
{
	std::string_view PkgName;
	std::unordered_map<uint64_t, Node> Nodes;
	std::vector<FileRecord> Files;
	std::vector<Volume> Volumes;

	static UnpackResult<IdxFile> Parse(std::span<const Core::Byte> data);
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
	explicit Unpacker(std::filesystem::path pkgPath, std::filesystem::path idxPath);
	UnpackResult<void> Parse();
	UnpackResult<void> Extract(std::string_view node, const std::filesystem::path& dst, bool preservePath = true) const;

private:
	DirectoryTree m_directoryTree;
	std::filesystem::path m_pkgPath;
	std::filesystem::path m_idxPath;

	UnpackResult<void> ExtractFile(const FileRecord& fileRecord, const std::filesystem::path& dst) const;
};

}  // namespace PotatoAlert::GameFileUnpack
