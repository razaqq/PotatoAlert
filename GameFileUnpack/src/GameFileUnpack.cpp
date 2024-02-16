// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/FileMagic.hpp"
#include "Core/FileMapping.hpp"
#include "Core/Format.hpp"
#include "Core/Log.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"
#include "Core/Zlib.hpp"

#include "GameFileUnpack/GameFileUnpack.hpp"

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <span>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::File;
using PotatoAlert::Core::FileMagic;
using PotatoAlert::Core::FileMapping;
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
using PotatoAlert::GameFileUnpack::UnpackResult;
using PotatoAlert::GameFileUnpack::Volume;

namespace fs = std::filesystem;

#define PA_UNPACK_ERROR(...) (::std::unexpected(::PotatoAlert::GameFileUnpack::UnpackError(fmt::format(__VA_ARGS__))))

namespace {

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

static UnpackResult<void> WriteFileData(const fs::path& file, std::span<const Byte> data)
{
	// write the data
	if (const File outFile = File::Open(file, File::Flags::Open | File::Flags::Write | File::Flags::Create))
	{
		if (outFile.Write(data))
		{
			return {};
		}
	}
	return PA_UNPACK_ERROR("Failed to write data to outfile {} - {}", file, File::LastError());
}

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

void DirectoryTree::Insert(const FileRecord& fileRecord)
{
	if (const size_t pos = fileRecord.Path.rfind('/'); pos != std::string::npos)
	{
		TreeNode& node = CreatePath(fileRecord.Path);
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
	for (std::string_view part : Core::String::Split(path, "/"))
	{
		if (part.empty()) continue;
		if (auto it = current->Nodes.find(part.data()); it != current->Nodes.end())
		{
			current = &it->second;
		}
		else
		{
			// can ignore the 2nd value since we already checked if it exists
			auto [node, _] = current->Nodes.emplace(part, TreeNode{});
			current = &node->second;
 		}
	}
	return *current;
}

Unpacker::Unpacker(fs::path pkgPath, fs::path idxPath) : m_pkgPath(std::move(pkgPath)), m_idxPath(std::move(idxPath))
{
}

UnpackResult<void> Unpacker::Parse()
{
	if (!fs::exists(m_idxPath))
	{
		return PA_UNPACK_ERROR("IdxPath does not exist: {}", m_idxPath);
	}

	for (const auto& entry : fs::recursive_directory_iterator(m_idxPath))
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
					return PA_UNPACK_ERROR("Failed to read idxFile: {}", File::LastError());
				}
			}
			else
			{
				return PA_UNPACK_ERROR("Failed to open idxFile for reading: {}", File::LastError());
			}
		}
	}

	return {};
}

UnpackResult<void> Unpacker::Extract(std::string_view nodeName, const fs::path& dst, bool preservePath) const
{
	const std::optional<DirectoryTree::TreeNode> nodeResult = m_directoryTree.Find(nodeName);
	if (!nodeResult)
	{
		return PA_UNPACK_ERROR("There exists no node with name {} in directory tree", nodeName);
	}
	TreeNode rootNode = nodeResult.value();

	std::vector<TreeNode*> stack = { &rootNode };

	while (!stack.empty())
	{
		TreeNode* node = stack.back();
		stack.pop_back();
		for (auto& child : node->Nodes | std::views::values)
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
			fs::path outDir = filePath;
			outDir.remove_filename();
			if (!fs::exists(outDir))
			{
				std::error_code ec;
				fs::create_directories(outDir, ec);
				if (ec)
				{
					return PA_UNPACK_ERROR("Failed to create game file scripts directory: {}", ec);
				}
			}

			PA_TRYV(ExtractFile(node->File.value(), filePath));
		}
	}

	return {};
}

UnpackResult<void> Unpacker::ExtractFile(const FileRecord& fileRecord, const fs::path& dst) const
{
	if (const File inFile = File::Open(m_pkgPath / fileRecord.PkgName, File::Flags::Open | File::Flags::Read))
	{
		uint64_t fileSize = inFile.Size();
		if (FileMapping mapping = FileMapping::Open(inFile, FileMapping::Flags::Read, fileSize))
		{
			if (const void* dataPtr = mapping.Map(FileMapping::Flags::Read, 0, fileSize))
			{
				if (fileRecord.Offset + fileRecord.Size > fileSize)
				{
					return PA_UNPACK_ERROR("Got offset ({} - {}) out of size bounds ({})",
						fileRecord.Offset, fileRecord.Offset + fileRecord.Size, fileSize);
				}

				// check if data is compressed and inflate
				const std::span data{ static_cast<const Byte*>(dataPtr), fileSize };
				if (fileRecord.Size != fileRecord.UncompressedSize)
				{
					std::vector<Byte> inflated = Core::Zlib::Inflate(
						data.subspan(fileRecord.Offset, fileRecord.Size), false
					);
					return WriteFileData(dst, std::span{ inflated });
				}

				return WriteFileData(dst, data.subspan(fileRecord.Offset, fileRecord.Size));
			}
			return PA_UNPACK_ERROR("Failed to map PkgFile into memory: {}", FileMapping::LastError());
		}
		return PA_UNPACK_ERROR("Failed to create file mapping: {}", FileMapping::LastError());
	}
	return PA_UNPACK_ERROR("Failed to open pkg file for reading: {}", File::LastError());
}

UnpackResult<IdxHeader> IdxHeader::Parse(std::span<const Byte> data)
{
	if (data.size() != HeaderSize)
	{
		return PA_UNPACK_ERROR("Invalid IdxHeader Length");
	}

	IdxHeader header;

	if (!FileMagic<'I', 'S', 'F', 'P'>(data))
	{
		return PA_UNPACK_ERROR("Invalid Idx Header");
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

UnpackResult<Node> Node::Parse(std::span<const Byte> data, uint64_t offset, std::span<const Byte> fullData)
{
	if (data.size() != NodeSize)
	{
		return PA_UNPACK_ERROR("Invalid Node size {}", data.size());
	}

	Node node;

	uint64_t nameLength;
	if (!TakeInto(data, nameLength))
		return PA_UNPACK_ERROR("Failed read Node name length");

	uint64_t namePtr;
	if (!TakeInto(data, namePtr))
		return PA_UNPACK_ERROR("Failed read Node namePtr");

	namePtr += offset;

	if (namePtr >= fullData.size())
	{
		return PA_UNPACK_ERROR("Node name pointer {} outside data range {}", namePtr, fullData.size());
	}

	if (!ReadNullTerminatedString(fullData, namePtr, node.Name))
	{
		return PA_UNPACK_ERROR("Failed to get node name");
	}

	if (nameLength != node.Name.size())
	{
		return PA_UNPACK_ERROR("Node has invalid name length {} != {}", nameLength, node.Name.size());
	}

	node.Name.pop_back();  // remove the double \0, otherwise we get issues down the line

	if (!TakeInto(data, node.Id))
		return PA_UNPACK_ERROR("Failed read node.Id");

	if (!TakeInto(data, node.Parent))
		return PA_UNPACK_ERROR("Failed read node.Parent");

	return node;
}

UnpackResult<FileRecord> FileRecord::Parse(std::span<const Byte> data, const std::unordered_map<uint64_t, Node>& nodes)
{
	if (data.size() != FileRecordSize)
	{
		return PA_UNPACK_ERROR("Invalid RawFileRecord size {}", data.size());
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
		return PA_UNPACK_ERROR("FileRecord references node with id {}, but that doesnt exist", fileRecord.NodeId);
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

UnpackResult<Volume> Volume::Parse(std::span<const Byte> data, uint64_t offset, std::span<const Byte> fullData)
{
	if (data.size() < VolumeSize)
	{
		return PA_UNPACK_ERROR("Invalid IdxFile size {}", data.size());
	}

	Volume volume;

	uint64_t nameLength;
	if (!TakeInto(data, nameLength))
		return PA_UNPACK_ERROR("Failed read Volume name length");

	uint64_t namePtr;
	if (!TakeInto(data, namePtr))
		return PA_UNPACK_ERROR("Failed read Volume namePtr");

	namePtr += offset;

	if (namePtr >= fullData.size())
	{
		return PA_UNPACK_ERROR("Volume name pointer {} outside data range {}", namePtr, fullData.size());
	}

	if (!ReadNullTerminatedString(fullData, namePtr, volume.Name))
	{
		return PA_UNPACK_ERROR("Failed to get Volume name");
	}

	if (nameLength != volume.Name.size())
	{
		return PA_UNPACK_ERROR("Volume has invalid name length {} != {}", nameLength, volume.Name.size());
	}

	volume.Name.pop_back();  // remove the double \0, otherwise we get issues down the line

	TakeInto(data, volume.Id);

	return volume;
}

UnpackResult<IdxFile> IdxFile::Parse(std::span<const Byte> data)
{
	const std::span originalData = data;

	if (data.size() < HeaderSize)
	{
		return PA_UNPACK_ERROR("Invalid IdxFile size {}", data.size());
	}

	IdxFile file;

	// parse headers
	PA_TRY(header, IdxHeader::Parse(Take(data, HeaderSize)));

	if (header.Endianness != 0x2000000)
	{
		return PA_UNPACK_ERROR("Endianness is not 0x20000000");
	}

	if (header.Version != 0x40)
	{
		return PA_UNPACK_ERROR("Endianness is not 0x40");
	}

	auto getData = [originalData](uint64_t offset, uint64_t size) -> UnpackResult<std::span<const Byte>>
	{
		if (offset + size > originalData.size())
		{
			return PA_UNPACK_ERROR("Data too small: offset {} + size {} > {}", offset, size, originalData.size());
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
		return PA_UNPACK_ERROR("IdxFile had volume count {} != 1", file.Volumes.size());
	file.PkgName = file.Volumes[0].Name;

	return file;
}
