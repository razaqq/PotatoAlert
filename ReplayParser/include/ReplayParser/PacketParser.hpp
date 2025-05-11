// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/ByteReader.hpp"

#include "ReplayParser/Packets.hpp"
#include "ReplayParser/Result.hpp"

#include <cstdint>
#include <span>
#include <type_traits>


namespace PotatoAlert::ReplayParser {

constexpr std::optional<PacketBaseType> GetPacketType(uint32_t id, Core::Version version)
{
	switch (id)
	{
		case 0x00: return PacketBaseType::BasePlayerCreate;
		case 0x01: return PacketBaseType::CellPlayerCreate;
		case 0x02: return PacketBaseType::EntityControl;
		case 0x03: return PacketBaseType::EntityEnter;
		case 0x04: return PacketBaseType::EntityLeave;
		case 0x05: return PacketBaseType::EntityCreate;
		case 0x07: return PacketBaseType::EntityProperty;
		case 0x08: return PacketBaseType::EntityMethod;
		case 0x0A: return PacketBaseType::PlayerPosition;
		case 0x16: return PacketBaseType::Version;
		case 0x20: return PacketBaseType::PlayerEntity;
		case 0x22:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::NestedPropertyUpdate;
			}
			else
			{
				return PacketBaseType::Result;
			}
		}
		case 0x23:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::NestedPropertyUpdate;
			}
			break;
		}
		case 0x24:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::Camera;
			}
			break;
		}
		case 0x25:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::Camera;
			}
			break;
		}
		case 0x26:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::CameraMode;
			}
			break;
		}
		case 0x27:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::Map;
			}
			else
			{
				return PacketBaseType::CameraMode;
			}
		}
		case 0x28:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::Map;
			}
			break;
		}
		case 0x2B:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::PlayerOrientation;
			}
			break;
		}
		case 0x2C:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::PlayerOrientation;
			}
			break;
		}
		case 0x2E:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::CameraFreeLook;
			}
			break;
		}
		case 0x2F:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::CameraFreeLook;
			}
			break;
		}
		case 0x31:
		{
			if (version <= Core::Version(12, 5, 0))
			{
				return PacketBaseType::CruiseState;
			}
			break;
		}
		case 0x32:
		{
			if (version > Core::Version(12, 5, 0))
			{
				return PacketBaseType::CruiseState;
			}
			break;
		}

		default:
			return {};
	}

	return std::nullopt;
}

template<typename Packet, typename Handler>
struct OnType { Handler handler; };

template<typename Packet, typename Handler>
OnType<Packet, Handler&&> On(Handler&& handler)
{
	return { static_cast<Handler&&>(handler) };
}

template<typename... Packet, typename... Handler>
auto MakePacketParser(OnType<Packet, Handler&&>... handlers)
{
	return [... handlers = OnType<Packet, std::decay_t<Handler>>{ static_cast<Handler&&>(handlers.handler) }](
		PacketBaseType type, float clock, std::span<const Byte> data, PacketParseContext& ctx) -> ReplayResult<void>
	{
		ReplayResult<void> res = {};

		auto const invokeHandler = [&]<typename P, typename H>(const OnType<P, H>& h)
		{
			if (type == P::Type)
			{
				res = [&]() -> ReplayResult<void>
				{
					PA_TRY(packet, P::Parse(data, ctx, clock));

					using R = std::invoke_result_t<H, decltype(packet)>;
					if constexpr (std::is_same_v<R, ReplayResult<void>>)
					{
						PA_TRYV(h.handler(std::move(packet)));
					}
					else if constexpr (std::is_same_v<R, void>)
					{
						h.handler(std::move(packet));
					}
					else
					{
						static_assert(false, "Return type of handler has to be ReplayResult<void> or void");
					}

					return {};
				}();
			}
		};

		(invokeHandler(handlers), ...);

		return res;
	};
}

inline ReplayResult<void> ParsePackets(Core::ByteReader<>& reader, PacketParseContext& ctx, auto&& parser)
{
	do {
		uint32_t id;
		float clock;
		std::span<const Byte> data;

		PA_TRYV_OR_ELSE([&]() -> Core::Result<void>
		{
			uint32_t size;
			PA_TRYD(reader.ReadTo(size));
			PA_TRYD(reader.ReadTo(id));
			PA_TRYD(reader.ReadTo(clock));
			PA_TRYA(data, reader.Read(size));
			return {};
		}(),
		{
			return PA_REPLAY_ERROR("Packet had invalid size - {}", error.message());
		});

		const std::optional<PacketBaseType> type = GetPacketType(id, ctx.Version);
		if (type)
		{
			PA_TRYV(parser(*type, clock, data, ctx));
		}
	} while (!reader.Empty());

	return {};
}

}  // namespace PotatoAlert::ReplayParser
