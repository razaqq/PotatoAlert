// Copyright 2022 <github.com/razaqq>
#pragma once

#include <memory>
#include <string>


namespace PotatoAlert::Core {

class ApplicationGuard
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	explicit ApplicationGuard(std::string name);
	~ApplicationGuard();
	bool Reset();
	bool OtherInstance() const;

private:
	std::string m_name;
	Handle m_handle;
};

}  // namespace PotatoAlert::Core
