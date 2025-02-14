// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/FileMagic.hpp"
#include "Core/FileMapping.hpp"
#include "Core/Format.hpp"
#include "Core/Log.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"
#include "Core/Zlib.hpp"

#include "GameFileUnpack/GameFileUnpack.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <regex>
#include <string>
#include <span>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::File;
using PotatoAlert::Core::FileMagic;
using PotatoAlert::Core::FileMapping;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::Take;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::Core::TakeString;
using PotatoAlert::GameFileUnpack::DirectoryTree;
using TreeNode = DirectoryTree::TreeNode;
using PotatoAlert::GameFileUnpack::IdxFile;
using PotatoAlert::GameFileUnpack::IdxHeader;
using PotatoAlert::GameFileUnpack::Node;
using PotatoAlert::GameFileUnpack::FileRecord;
using PotatoAlert::GameFileUnpack::Unpacker;
using PotatoAlert::GameFileUnpack::Volume;

namespace fs = std::filesystem;

namespace {

enum class UnpackError
{
	InvalidIdxFileSize,
	InvalidIdxHeaderSize,
	InvalidIdxHeaderMagic,

	InvalidHeaderVersion,
	InvalidHeaderEndianness,
	InvalidDataSize,
	InvalidNodeSize,
	InvalidVolumeCount,
	IdxPathDoesNotExist,
	FailedToIterateIdxPath,
	FailedToReadIdxFile,
	FailedToOpenIdxFile,

	FailedToReadPkgFile,
	FailedToOpenPkgFile,
	FailedToMapPkgFile,

	FailedToReadNodeNameLength,
	FailedToReadNodeNamePtr,
	InvalidNodeNamePtr,
	FailedToReadNodeName,
	InvalidNodeNameLength,
	FailedToReadNodeId,
	FailedToReadNodeParent,

	InvalidFileRecordSize,
	InvalidFileRecordNodeId,

	InvalidVolumeSize,
	FailedToReadVolumeNameLength,
	FailedToReadVolumeNamePtr,
	InvalidVolumeNamePtr,
	FailedToReadVolumeName,
	InvalidVolumeNameLength,

	InvalidPkgDecompression,
	NoMatchingNodeInDirectoryTree,
	FileRecordOffsetOutOfBounds,
	FailedToWriteData,
};

struct UnpackErrorCategory : std::error_category
{
	const char* name() const noexcept override
	{
		return "GameFileUnpack";
	}

	std::string message(int e) const override;
};

const UnpackErrorCategory g_unpackErrorCategory{};

static std::error_code MakeError(UnpackError err)
{
	return { static_cast<std::underlying_type_t<UnpackError>>(err), g_unpackErrorCategory };
}

static bool ReadNullTerminatedString(std::span<const Byte> data, uint64_t offset, std::string& out)
{
	size_t length = 0;
	for (uint64_t i = offset; i < data.size(); i++)
	{
		if (data[i] == Byte{ '\0' })
		{
			length = i - offset + 1;
			break;
		}
	}
	if (length == 0)
	{
		LOG_ERROR("Invalid String Length");
		return false;
	}

	std::span stringData = data.subspan(offset, length);
	if (!TakeString(stringData, out, length))
	{
		LOG_ERROR("Failed to read string");
		return false;
	}

	return true;
}

static Result<void> WriteFileData(const fs::path& file, std::span<const Byte> data)
{
	// write the data
	if (const File outFile = File::Open(file, File::Flags::Open | File::Flags::Write | File::Flags::Create))
	{
		if (outFile.Write(data))
		{
			return {};
		}
	}
	return PA_ERROR(MakeError(UnpackError::FailedToWriteData));
}

}

std::string UnpackErrorCategory::message(int e) const
{
	switch (static_cast<UnpackError>(e))
	{
		case UnpackError::InvalidIdxFileSize:
			return "IdxFile has invalid size";
		case UnpackError::InvalidIdxHeaderMagic:
			return "IdxFile has invalid magic";
		case UnpackError::InvalidIdxHeaderSize:
			return "IdxFile has invalid header size";
		case UnpackError::InvalidHeaderVersion:
			return "Header Version is not 0x40";
		case UnpackError::InvalidHeaderEndianness:
			return "Header Endianness is not 0x20000000";
		case UnpackError::InvalidDataSize:
			return "IdxFile has invalid data size";
		case UnpackError::InvalidVolumeCount:
			return "IdxFile volume count doesn't match header";
		case UnpackError::FailedToWriteData:
			return "Failed to write data to outfile";
		case UnpackError::IdxPathDoesNotExist:
			return "IdxPath does not exist";
		case UnpackError::FailedToIterateIdxPath:
			return "Failed to iterate idx path";
		case UnpackError::FailedToReadIdxFile:
			return "Failed to read idx file";
		case UnpackError::FailedToOpenIdxFile:
			return "Failed to open idx file";
		case UnpackError::FailedToReadPkgFile:
			return "Failed to read pkg file";
		case UnpackError::FailedToOpenPkgFile:
			return "Failed to open pkg file";
		case UnpackError::FailedToMapPkgFile:
			return "Failed to map pkg file";
		case UnpackError::InvalidPkgDecompression:
			return "Decompressed file size does not match file record";
		case UnpackError::NoMatchingNodeInDirectoryTree:
			return "No matching node in directory tree";
		case UnpackError::FileRecordOffsetOutOfBounds:
			return "FileRecord offset is out of bounds";
		case UnpackError::InvalidNodeSize:
			return "Invalid node size";
		case UnpackError::FailedToReadNodeNameLength:
			return "Failed to read node name length";
		case UnpackError::FailedToReadNodeNamePtr:
			return "Failed to read node name ptr";
		case UnpackError::InvalidNodeNamePtr:
			return "Invalid node name ptr";
		case UnpackError::FailedToReadNodeName:
			return "Failed to read node name";
		case UnpackError::InvalidNodeNameLength:
			return "Node name length does not match expected value";
		case UnpackError::FailedToReadNodeId:
			return "Failed to read node id";
		case UnpackError::FailedToReadNodeParent:
			return "Failed to read node parent";
		case UnpackError::InvalidFileRecordSize:
			return "Invalid FileRecord size";
		case UnpackError::InvalidFileRecordNodeId:
			return "FileRecord references non-existent node id";
		case UnpackError::InvalidVolumeSize:
			return "Invalid volume size";
		case UnpackError::FailedToReadVolumeNameLength:
			return "Failed to read volume name length";
		case UnpackError::FailedToReadVolumeNamePtr:
			return "Failed to read volume name ptr";
		case UnpackError::InvalidVolumeNamePtr:
			return "Invalid volume name ptr";
		case UnpackError::FailedToReadVolumeName:
			return "Failed to read volume name";
		case UnpackError::InvalidVolumeNameLength:
			return "Invalid volume name length";
	}
	return "Unknown error";
}


std::optional<DirectoryTree::TreeNode> DirectoryTree::Find(std::string_view path) const
{
	const TreeNode* current = &m_root;
	for (std::string_view part : Core::String::Split(path, "/"))
	{
		if (part.empty()) continue;
		if (auto it = current->Nodes.find(part.data()); it != current->Nodes.end())
		{
			current = &it->second;
		}
		else
		{
			return {};
		}
	}

	return *current;
}

const TreeNode& DirectoryTree::GetRoot() const
{
	return m_root;
}

void DirectoryTree::Insert(const FileRecord& fileRecord)
{
	if (const size_t pos = fileRecord.Path.rfind('/'); pos != std::string::npos)
	{
		TreeNode& node = CreatePath(fileRecord.Path);
		node.Path = fileRecord.Path;
		node.File = fileRecord;
	}
	else
	{
		m_root.Nodes[fileRecord.Path] = TreeNode{ .File = fileRecord };
	}
}

DirectoryTree::TreeNode& DirectoryTree::CreatePath(std::string_view path)
{
	TreeNode* current = &m_root;
	const std::vector<std::string> pathSplit = Core::String::Split(path, "/");

	for (size_t i = 0; i < pathSplit.size(); i++)
	{
		std::string_view part = pathSplit[i];

		if (part.empty()) continue;
		if (auto it = current->Nodes.find(part.data()); it != current->Nodes.end())
		{
			current = &it->second;
		}
		else
		{
			// can ignore the 2nd value since we already checked if it exists
			auto [node, _] = current->Nodes.emplace(part, TreeNode{});
			node->second.Path = std::span(pathSplit).subspan(0, i + 1) | std::views::join_with('/') | std::ranges::to<std::string>();
			current = &node->second;
 		}
	}
	return *current;
}

Unpacker::Unpacker(fs::path pkgPath, fs::path idxPath) : m_pkgPath(std::move(pkgPath)), m_idxPath(std::move(idxPath))
{
}

Result<void> Unpacker::Parse()
{
	if (!fs::exists(m_idxPath))
	{
		return PA_ERROR(MakeError(UnpackError::IdxPathDoesNotExist));
	}

	std::error_code ec;
	auto it = fs::recursive_directory_iterator(m_idxPath, ec);
	if (ec)
	{
		return PA_ERROR(MakeError(UnpackError::FailedToIterateIdxPath));
	}

	for (const fs::directory_entry& entry : it)
	{
		if (entry.is_regular_file() && entry.path().extension() == ".idx")
		{
			if (File file = File::Open(entry.path(), File::Flags::Open | File::Flags::Read))
			{
				if (std::vector<Byte> data; file.ReadAll(data))
				{
					PA_TRY(idxFile, IdxFile::Parse(data));
					for (FileRecord& fileRecord : idxFile.Files)
					{
						fileRecord.PkgName = idxFile.PkgName;
						m_directoryTree.Insert(fileRecord);
					}
				}
				else
				{
					return PA_ERROR(MakeError(UnpackError::FailedToReadIdxFile));
				}
			}
			else
			{
				return PA_ERROR(MakeError(UnpackError::FailedToOpenIdxFile));
			}
		}
	}

	return {};
}

Result<void> Unpacker::Extract(std::string_view nodeName, const fs::path& dst, bool preservePath) const
{
	const std::optional<DirectoryTree::TreeNode> nodeResult = m_directoryTree.Find(nodeName);
	if (!nodeResult)
	{
		return PA_ERROR(MakeError(UnpackError::NoMatchingNodeInDirectoryTree));
	}
	TreeNode rootNode = nodeResult.value();

	std::vector<TreeNode*> stack = { &rootNode };

	while (!stack.empty())
	{
		TreeNode* node = stack.back();
		stack.pop_back();
		for (TreeNode& child : node->Nodes | std::views::values)
		{
			stack.push_back(&child);
		}

		if (node->File)
		{
			fs::path filePath;
			if (!preservePath)
			{
				const fs::path rel = fs::relative(node->File->Path, nodeName);
				if (rel == fs::path("."))
					filePath = dst / fs::path(nodeName).filename();
				else
					filePath = dst / rel;
			}
			else
			{
				filePath = dst / node->File->Path;
			}

			// create output directories if they don't exist yet
			const fs::path outDir = fs::path(filePath).remove_filename();
			PA_TRY(exists, Core::PathExists(outDir));
			if (!exists)
			{
				PA_TRYV(Core::CreatePath(outDir));
			}

			PA_TRYV(ExtractFile(node->File.value(), filePath));
		}
	}

	return {};
}

Result<void> Unpacker::Extract(std::string_view nodeName, std::string_view pattern, const std::filesystem::path& dst, bool preservePath) const
{
	const std::optional<DirectoryTree::TreeNode> nodeResult = m_directoryTree.Find(nodeName);
	if (!nodeResult)
	{
		return PA_ERROR(MakeError(UnpackError::NoMatchingNodeInDirectoryTree));
	}
	TreeNode rootNode = nodeResult.value();

	std::vector<TreeNode*> stack = { &rootNode };

	while (!stack.empty())
	{
		TreeNode* node = stack.back();
		stack.pop_back();
		for (TreeNode& child : node->Nodes | std::views::values)
		{
			stack.push_back(&child);
		}

		if (node->File)
		{
			fs::path filePath;
			if (!preservePath)
			{
				const fs::path rel = fs::relative(node->File->Path, nodeName);
				if (rel == fs::path("."))
					filePath = dst / fs::path(nodeName).filename();
				else
					filePath = dst / rel;
			}
			else
			{
				filePath = dst / node->File->Path;
			}

			std::regex re(pattern.data());
			if (!std::regex_search(node->File->Path, re))
			{
				continue;
			}

			// create output directories if they don't exist yet
			const fs::path outDir = fs::path(filePath).remove_filename();
			PA_TRY(exists, Core::PathExists(outDir));
			if (!exists)
			{
				PA_TRYV(Core::CreatePath(outDir));
			}

			PA_TRYV(ExtractFile(node->File.value(), filePath));
		}
	}

	return {};
}

const DirectoryTree& Unpacker::GetDirectoryTree() const
{
	return m_directoryTree;
}

Result<void> Unpacker::ExtractFile(const FileRecord& fileRecord, const fs::path& dst) const
{
	if (const File inFile = File::Open(m_pkgPath / fileRecord.PkgName, File::Flags::Open | File::Flags::Read))
	{
		uint64_t fileSize = inFile.Size();
		if (FileMapping mapping = FileMapping::Open(inFile, FileMapping::Flags::Read, fileSize))
		{
			if (const void* dataPtr = mapping.Map(FileMapping::Flags::Read, 0, fileSize))
			{
				PA_DEFER
				{
					mapping.Unmap(dataPtr, fileSize);
					mapping.Close();
				};

				if (fileRecord.Offset + fileRecord.Size > fileSize)
				{
					return PA_ERROR(MakeError(UnpackError::FileRecordOffsetOutOfBounds));
				}

				// check if data is compressed and inflate
				const std::span data{ static_cast<const Byte*>(dataPtr), fileSize };
				if (fileRecord.Size != fileRecord.UncompressedSize)
				{
					std::vector<Byte> inflated = Core::Zlib::Inflate(
						data.subspan(fileRecord.Offset, fileRecord.Size), false
					);
					if (inflated.size() != fileRecord.UncompressedSize)
					{
						return PA_ERROR(MakeError(UnpackError::InvalidPkgDecompression));
					}
					return WriteFileData(dst, std::span{ inflated });
				}
				return WriteFileData(dst, data.subspan(fileRecord.Offset, fileRecord.Size));
			}
			return PA_ERROR(MakeError(UnpackError::FailedToMapPkgFile));
		}
		return PA_ERROR(MakeError(UnpackError::FailedToMapPkgFile));
	}
	return PA_ERROR(MakeError(UnpackError::FailedToOpenPkgFile));
}

Result<IdxHeader> IdxHeader::Parse(std::span<const Byte> data)
{
	if (data.size() != HeaderSize)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidIdxHeaderSize));
	}

	IdxHeader header;

	if (!FileMagic<'I', 'S', 'F', 'P'>(data))
	{
		return PA_ERROR(MakeError(UnpackError::InvalidIdxHeaderMagic));
	}

	TakeInto(data, header.Endianness);
	TakeInto(data, header.MurmurHash);
	TakeInto(data, header.Version);

	TakeInto(data, header.NodeCount);
	TakeInto(data, header.FileCount);
	TakeInto(data, header.VolumeCount);

	TakeInto(data, header.Unknown);

	TakeInto(data, header.NodeTablePtr);
	TakeInto(data, header.FileRecordTablePtr);
	TakeInto(data, header.VolumeTablePtr);

	return header;
}

Result<Node> Node::Parse(std::span<const Byte> data, uint64_t offset, std::span<const Byte> fullData)
{
	if (data.size() != NodeSize)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidNodeSize));
	}

	Node node;

	uint64_t nameLength;
	if (!TakeInto(data, nameLength))
		return PA_ERROR(MakeError(UnpackError::FailedToReadNodeNameLength));

	uint64_t namePtr;
	if (!TakeInto(data, namePtr))
		return PA_ERROR(MakeError(UnpackError::FailedToReadNodeNamePtr));

	namePtr += offset;

	if (namePtr >= fullData.size())
	{
		return PA_ERROR(MakeError(UnpackError::InvalidNodeNamePtr));
	}

	if (!ReadNullTerminatedString(fullData, namePtr, node.Name))
	{
		return PA_ERROR(MakeError(UnpackError::FailedToReadNodeName));
	}

	if (nameLength != node.Name.size())
	{
		return PA_ERROR(MakeError(UnpackError::InvalidNodeNameLength));
	}

	node.Name.pop_back();  // remove the double \0, otherwise we get issues down the line

	if (!TakeInto(data, node.Id))
		return PA_ERROR(MakeError(UnpackError::FailedToReadNodeId));

	if (!TakeInto(data, node.Parent))
		return PA_ERROR(MakeError(UnpackError::FailedToReadNodeParent));

	return node;
}

Result<FileRecord> FileRecord::Parse(std::span<const Byte> data, const std::unordered_map<uint64_t, Node>& nodes)
{
	if (data.size() != FileRecordSize)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidFileRecordSize));
	}

	FileRecord fileRecord;

	TakeInto(data, fileRecord.NodeId);
	TakeInto(data, fileRecord.VolumeId);
	TakeInto(data, fileRecord.Offset);
	TakeInto(data, fileRecord.CompressionInfo);
	TakeInto(data, fileRecord.Size);
	TakeInto(data, fileRecord.Crc32);
	TakeInto(data, fileRecord.UncompressedSize);
	TakeInto(data, fileRecord.Padding);

	if (!nodes.contains(fileRecord.NodeId))
	{
		return PA_ERROR(MakeError(UnpackError::InvalidFileRecordNodeId));
	}

	std::vector<std::string_view> paths;
	uint64_t current = fileRecord.NodeId;

	while (nodes.contains(current))
	{
		const Node& node = nodes.at(current);
		current = node.Parent;
		paths.emplace_back(node.Name);
	}

	std::ranges::reverse(paths);
	fileRecord.Path = Core::String::Join(paths, "/");

	return fileRecord;
}

Result<Volume> Volume::Parse(std::span<const Byte> data, uint64_t offset, std::span<const Byte> fullData)
{
	if (data.size() < VolumeSize)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidVolumeSize));
	}

	Volume volume;

	uint64_t nameLength;
	if (!TakeInto(data, nameLength))
		return PA_ERROR(MakeError(UnpackError::FailedToReadVolumeNameLength));

	uint64_t namePtr;
	if (!TakeInto(data, namePtr))
		return PA_ERROR(MakeError(UnpackError::FailedToReadVolumeNamePtr));

	namePtr += offset;

	if (namePtr >= fullData.size())
	{
		return PA_ERROR(MakeError(UnpackError::InvalidVolumeNamePtr));
	}

	if (!ReadNullTerminatedString(fullData, namePtr, volume.Name))
	{
		return PA_ERROR(MakeError(UnpackError::FailedToReadVolumeName));
	}

	if (nameLength != volume.Name.size())
	{
		return PA_ERROR(MakeError(UnpackError::InvalidVolumeNameLength));
	}

	volume.Name.pop_back();  // remove the double \0, otherwise we get issues down the line

	TakeInto(data, volume.Id);

	return volume;
}

Result<IdxFile> IdxFile::Parse(std::span<const Byte> data)
{
	const std::span originalData = data;

	if (data.size() < HeaderSize)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidIdxFileSize));
	}

	IdxFile file;

	// parse headers
	PA_TRY(header, IdxHeader::Parse(Take(data, HeaderSize)));

	if (header.Endianness != 0x2000000)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidHeaderEndianness));
	}

	if (header.Version != 0x40)
	{
		return PA_ERROR(MakeError(UnpackError::InvalidHeaderVersion));
	}

	auto getData = [originalData](uint64_t offset, uint64_t size) -> Result<std::span<const Byte>>
	{
		if (offset + size > originalData.size())
		{
			return PA_ERROR(MakeError(UnpackError::InvalidDataSize));
		}
		return originalData.subspan(offset, size);
	};

	PA_TRY(nodeData, getData(header.NodeTablePtr + HeaderDataOffset, header.NodeCount * NodeSize));
	PA_TRY(fileRecordData, getData(header.FileRecordTablePtr + HeaderDataOffset, header.FileCount * FileRecordSize));
	PA_TRY(volumeData, getData(header.VolumeTablePtr + HeaderDataOffset, header.VolumeCount * VolumeSize));

	// parse nodes
	for (uint32_t i = 0; i < header.NodeCount; i++)
	{
		const uint64_t offset = header.NodeTablePtr + HeaderDataOffset + i * NodeSize;
		PA_TRY(node, Node::Parse(Take(nodeData, NodeSize), offset, originalData));
		file.Nodes[node.Id] = node;
	}

	// parse file records
	for (uint32_t i = 0; i < header.FileCount; i++)
	{
		PA_TRY(fileRecord, FileRecord::Parse(Take(fileRecordData, FileRecordSize), file.Nodes));
		file.Files.emplace_back(std::move(fileRecord));
	}

	// parse volumes
	for (uint32_t i = 0; i < header.VolumeCount; i++)
	{
		const uint64_t offset = header.VolumeTablePtr + HeaderDataOffset + i * VolumeSize;
		PA_TRY(volume, Volume::Parse(Take(volumeData, VolumeSize), offset, originalData));
		file.Volumes.emplace_back(std::move(volume));
	}

	if (file.Volumes.size() != 1)
		return PA_ERROR(MakeError(UnpackError::InvalidVolumeCount));
	file.PkgName = file.Volumes[0].Name;

	return file;
}
