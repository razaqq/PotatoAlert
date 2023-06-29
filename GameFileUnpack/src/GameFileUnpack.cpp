// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/FileMapping.hpp"
#include "Core/Log.hpp"
#include "Core/String.hpp"
#include "Core/Zlib.hpp"

#include "GameFileUnpack/GameFileUnpack.hpp"

#include <optional>
#include <string>
#include <span>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::File;
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
namespace fs = std::filesystem;

namespace {

static constexpr char g_sep = static_cast<char>(fs::path::preferred_separator);

static bool ReadNullTerminatedString(std::span<Byte> data, int64_t offset, std::string& out)
{
	size_t length = 0;
	for (int64_t i = offset; i < data.size(); i++)
	{
		if (data[i] == Byte{ '\0' })
		{
			length = i - offset;
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

static bool WriteFileData(std::string_view file, std::span<const Byte> data)
{
	// write the data
	if (File outFile = File::Open(file, File::Flags::Open | File::Flags::Write | File::Flags::Create))
	{
		if (!outFile.Write(data))
		{
			LOG_ERROR("Failed to write data to outfile {} - {}", file, File::LastError());
			return false;
		}
		return true;
	}
	else
	{
		LOG_ERROR("Failed to open file for writing {} - {}", file, File::LastError());
		return false;
	}
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
	if (auto pos = fileRecord.Path.rfind('/'); pos != std::string::npos)
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

Unpacker::Unpacker(std::string_view pkgPath, std::string_view idxPath) : m_pkgPath(pkgPath)
{
	if (!fs::exists(idxPath))
	{
		LOG_ERROR("IdxPath does not exist: {}", idxPath);
		return;
	}

	for (const auto& entry : fs::recursive_directory_iterator(idxPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".idx")
		{
			if (File file = File::Open(entry.path().string(), File::Flags::Open | File::Flags::Read))
			{
				if (std::vector<Byte> data; file.ReadAll(data))
				{
					if (std::optional<IdxFile> idxFileRes = IdxFile::Parse(std::span{ data }))
					{
						IdxFile idxFile = idxFileRes.value();
						for (const auto& [path, fileRecord] : idxFile.Files)
						{
							m_directoryTree.Insert(
								FileRecord{ idxFile.PkgName, path, fileRecord.Id, fileRecord.Offset, fileRecord.Size, fileRecord.UncompressedSize }
							);
						}
					}
					else
					{
						LOG_ERROR("Failed to parse idxFile");
					}
				}
				else
				{
					LOG_ERROR("Failed to read idxFile: {}", File::LastError());
				}
			}
			else
			{
				LOG_ERROR("Failed to open idxFile for reading: {}", File::LastError());
			}
		}
	}
}

bool Unpacker::Extract(std::string_view nodeName, std::string_view dst) const
{
	std::optional<DirectoryTree::TreeNode> nodeResult = m_directoryTree.Find(nodeName);
	if (!nodeResult)
	{
		LOG_ERROR("There exists no node with name {} in directory tree", nodeName);
		return false;
	}
	TreeNode rootNode = nodeResult.value();

	std::vector<TreeNode*> stack = { &rootNode };

	while (!stack.empty())
	{
		TreeNode* node = stack.back();
		stack.pop_back();
		for (auto& [_, child] : node->Nodes)
		{
			stack.push_back(&child);
		}

		if (node->File)
		{
			if (!ExtractFile(node->File.value(), dst))
			{
				LOG_ERROR("Failed to extract file: {}", node->File.value().Path);
				return false;
			}
		}
	}

	return true;
}

bool Unpacker::ExtractFile(const FileRecord& fileRecord, std::string_view dst) const
{
	if (File inFile = File::Open((fs::path(m_pkgPath) / fileRecord.PkgName).string(), File::Flags::Open | File::Flags::Read))
	{
		uint64_t fileSize = inFile.Size();
		if (FileMapping mapping = FileMapping::Open(inFile, FileMapping::Flags::Read, fileSize))
		{
			if (void* dataPtr = mapping.Map(FileMapping::Flags::Read, 0, fileSize))
			{
				if (fileRecord.Offset + fileRecord.Size > fileSize)
				{
					LOG_ERROR("Got offset ({} - {}) out of size bounds ({})",
							  fileRecord.Offset, fileRecord.Offset + fileRecord.Size, fileSize);
					return false;
				}

				// create output directories if they don't exist yet
				fs::path outDir = (fs::path(dst) / fileRecord.Path).remove_filename();
				if (!fs::exists(outDir))
				{
					std::error_code ec;
					fs::create_directories(outDir, ec);
					if (ec)
					{
						LOG_ERROR("Failed to create game file scripts directory: {}", ec.message());
						return false;
					}
				}

				const std::string filePath = (fs::path(dst) / fileRecord.Path).string();

				// check if data is compressed and inflate
				std::span data{ static_cast<const Byte*>(dataPtr), fileSize };
				if (fileRecord.Size != fileRecord.UncompressedSize)
				{
					std::vector<Byte> inflated = Core::Zlib::Inflate(
						data.subspan(fileRecord.Offset, fileRecord.Size), false
					);
					return WriteFileData(filePath, std::span{ inflated });
				}

				return WriteFileData(filePath, data.subspan(fileRecord.Offset, fileRecord.Size));
			}
			else
			{
				LOG_ERROR("Failed to map PkgFile into memory: {}", FileMapping::LastError());
				return false;
			}
		}
		else
		{
			LOG_ERROR("Failed to create file mapping: {}", FileMapping::LastError());
			return false;
		}
	}
	else
	{
		LOG_ERROR("Failed to open pkg file for reading: {}", File::LastError());
		return false;
	}
}

std::optional<IdxHeader> IdxHeader::Parse(std::span<Byte> data)
{
	if (data.size() != HeaderSize)
	{
		LOG_ERROR("Invalid IdxHeader Length");
		return {};
	}

	IdxHeader header;

	Byte signature[4];
	if (!TakeInto(data, signature) || std::memcmp(signature, g_IdxSignature.data(), 4) != 0)
	{
		LOG_ERROR("Invalid Idx Header");
		return {};
	}

	if (!TakeInto(data, header.FirstBlock))
		return {};

	if (!TakeInto(data, header.Nodes))
		return {};

	if (!TakeInto(data, header.Files))
		return {};

	if (!TakeInto(data, header.Unknown1))
		return {};

	if (!TakeInto(data, header.Unknown2))
		return {};

	if (!TakeInto(data, header.ThirdOffset))
		return {};

	if (!TakeInto(data, header.TrailerOffset))
		return {};

	return header;
}

std::optional<Node> Node::Parse(std::span<Byte> data, std::span<Byte> fullData)
{
	if (data.size() != NodeSize)
	{
		LOG_ERROR("Invalid Node size {}", data.size());
		return {};
	}

	Node node;

	if (!TakeInto(data, node.Unknown))
		return {};

	int64_t ptr;
	if (!TakeInto(data, ptr))
		return {};

	if (ptr >= fullData.size())
	{
		LOG_ERROR("String pointer {} outside data range {}", ptr, fullData.size());
		return {};
	}

	if (!ReadNullTerminatedString(fullData, ptr, node.Name))
	{
		LOG_ERROR("Failed to get node name");
		return {};
	}

	if (!TakeInto(data, node.Id))
		return {};

	if (!TakeInto(data, node.Parent))
		return {};

	return node;
}

std::optional<FileRecord> FileRecord::Parse(std::span<Byte> data, const std::unordered_map<uint64_t, Node>& nodes)
{
	if (data.size() != FileRecordSize)
	{
		LOG_ERROR("Invalid RawFileRecord size {}", data.size());
		return {};
	}

	FileRecord fileRecord;

	if (!TakeInto(data, fileRecord.Id))
		return {};

	Take(data, 8);

	if (!TakeInto(data, fileRecord.Offset))
		return {};

	Take(data, 8);

	if (!TakeInto(data, fileRecord.Size))
		return {};

	Take(data, 4);

	if (!TakeInto(data, fileRecord.UncompressedSize))
		return {};

	std::vector<std::string_view> paths;
	uint64_t current = fileRecord.Id;

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

std::optional<IdxFile> IdxFile::Parse(std::span<Byte> data)
{
	const std::span originalData = data;

	if (data.size() < HeaderSize)
	{
		LOG_ERROR("Invalid IdxFile size {}", data.size());
		return {};
	}

	IdxFile file;

	// parse headers
	const std::optional<IdxHeader> headerResult = IdxHeader::Parse(Take(data, HeaderSize));
	if (!headerResult)
	{
		LOG_ERROR("Failed to parse IdxHeader");
		return {};
	}
	IdxHeader header = headerResult.value();

	// parse nodes
	if (data.size() < header.Nodes * NodeSize)
	{
		LOG_ERROR("Data too small for {} Nodes, expected at least {} bytes but only got {}",
				  header.Nodes, header.Nodes * NodeSize, data.size());
		return {};
	}

	for (int32_t i = 0; i < header.Nodes; i++)
	{
		if (std::optional<Node> nodeResult = Node::Parse(Take(data, NodeSize), data))
		{
			file.Nodes[nodeResult.value().Id] = nodeResult.value();
		}
		else
		{
			LOG_ERROR("Failed to parse Node");
			return {};
		}
	}

	// parse file records
	if (originalData.size() < header.ThirdOffset + 0x10)
	{
		LOG_ERROR("File record data ({} bytes) smaller than offset ({})", originalData.size(), header.ThirdOffset + 0x10);
		return {};
	}
	std::span fileRecordData = originalData.subspan(header.ThirdOffset + 0x10);

	if (fileRecordData.size() < header.Files * FileRecordSize)
	{
		LOG_ERROR("File record too small for {} RawFileRecords, expected at least {} bytes but only got {}",
				  header.Files, header.Files * FileRecordSize, data.size());
		return {};
	}
	for (int32_t i = 0; i < header.Files; i++)
	{
		if (std::optional<FileRecord> fileRecordResult = FileRecord::Parse(Take(fileRecordData, FileRecordSize), file.Nodes))
		{
			file.Files[fileRecordResult.value().Path] = fileRecordResult.value();
		}
		else
		{
			LOG_ERROR("Failed to parse RawFileRecord");
			return {};
		}
	}

	// parse trailer
	if (originalData.size() < header.TrailerOffset + 0x10)
	{
		LOG_ERROR("Trailer data ({} bytes) smaller than offset ({})", originalData.size(), header.ThirdOffset + 0x10);
		return {};
	}
	std::span trailerData = originalData.subspan(header.TrailerOffset + 0x10);

	struct
	{
		int64_t unknown1;
		int64_t unknown2;
		uint64_t unknown3;
	} unknown;
	if (!TakeInto(trailerData, unknown))
	{
		LOG_ERROR("Failed to take unknown from trailer data");
		return {};
	}

	if (!ReadNullTerminatedString(trailerData, 0, file.PkgName))
	{
		LOG_ERROR("Failed to get file pkg name");
		return {};
	}

	return file;
}
