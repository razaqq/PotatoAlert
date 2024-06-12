// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <array>
#include <cstdint>
#include <span>


namespace PotatoAlert::Core {

class Blowfish
{
public:
	Blowfish() : m_pArray({}), m_sBoxes({}) {}
	explicit Blowfish(std::span<const Byte> key);
	Blowfish(Blowfish const&) = delete;
	Blowfish(Blowfish&&) = delete;

	Blowfish operator=(Blowfish&) = delete;
	Blowfish operator=(Blowfish&&) = delete;
	
	~Blowfish() = default;

	void InitKey(std::span<const Byte> key);

	bool Decrypt(std::span<const Byte> src, std::span<Byte> dst) const;
	bool Encrypt(std::span<const Byte> src, std::span<Byte> dst) const;

	void EncryptBlock(uint32_t* left, uint32_t* right) const;
	void DecryptBlock(uint32_t* left, uint32_t* right) const;

	static constexpr uint32_t BlockSize() { return 8; }

	static void ReverseByteOrder(uint32_t& x);

public:
	static constexpr int N = 16;

private:
	std::array<uint32_t, N + 2> m_pArray;
	std::array<std::array<uint32_t, 256>, 4> m_sBoxes;
	[[nodiscard]] uint32_t F(uint32_t x) const;
};

}  // namespace PotatoAlert::Core
