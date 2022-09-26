// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Core/Log.hpp"
#include "Core/Process.hpp"

#include <compare>
#include <map>


namespace PotatoAlert::Client {

class TypeId;

template<typename T>
inline constexpr TypeId TypeOf();

class TypeId
{
private:
	const void* m_ptr;

public:
	auto operator<=>(const TypeId&) const = default;

private:
	explicit constexpr TypeId(const void* const ptr) : m_ptr(ptr)
	{
	}

	template<typename T>
	friend constexpr TypeId TypeOf();
};

template<typename T>
inline constexpr char TypeIdObject = {};

template<typename T>
inline constexpr TypeId TypeOf()
{
	return TypeId(&TypeIdObject<T>);
}

class ServiceProvider
{
private:
	std::map<TypeId, void*> m_services;

public:
	template<typename Service>
	bool Add(Service& service)
	{
		return m_services.try_emplace(TypeOf<Service>(), &service).second;
	}
	
	template<typename Service>
	Service& Get() const
	{
		if (const auto it = m_services.find(TypeOf<Service>()); it != m_services.end())
		{
			return *static_cast<Service*>(it->second);
		}
		LOG_ERROR("Service '{}' didnt exist", typeid(Service).name());
		Core::ExitCurrentProcessWithError(1);
	}
};

}  // namespace PotatoAlert::Client
