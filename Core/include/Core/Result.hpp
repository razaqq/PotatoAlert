// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Preprocessor.hpp"

#include <expected>
#include <system_error>
#include <type_traits>


namespace PotatoAlert::Core {

template<typename T, typename E = std::error_code>
using Result = std::expected<T, E>;

template<class E>
inline std::unexpected<std::decay_t<E>> MakeUnexpected(E&& e)
{
	return std::unexpected<std::decay_t<E>>(std::forward<E>(e));
}

#define PA_ERROR(...) (::PotatoAlert::Core::MakeUnexpected(__VA_ARGS__))

inline constexpr auto& ResultValue = std::in_place;
inline constexpr auto& ResultError = std::unexpect;

inline Result<void> AsResult(const std::error_code error)
{
	return error ? Result<void>{ ResultError, error } : Result<void>{};
}

inline std::error_code AsErrorCode(const Result<void>& result)
{
	return result ? std::error_code() : result.error();
}

#define PA_DETAIL_TRY_VALUE_T(result) \
	typename ::std::remove_cvref_t<decltype(result)>::value_type

#define PA_DETAIL_TRY_IS_VOID(result) \
	(::std::is_void_v<PA_DETAIL_TRY_VALUE_T(result)>)

#define PA_DETAIL_TRY_S0(_0, ...) \
	_0

#define PA_DETAIL_TRY_S2(_0, _1, _2, ...) \
	_2

#define PA_DETAIL_TRY_H(spec) \
	PA_EXPAND(PA_DETAIL_TRY_S0 PA_EXPAND(PA_DETAIL_TRY_S2 PA_EXPAND((PA_EXPAND spec, spec, (auto)))))

#define PA_DETAIL_TRY_V(spec) \
	PA_EXPAND(PA_DETAIL_TRY_S2 PA_EXPAND((, PA_EXPAND spec, spec)))

#define PA_DETAIL_TRY_R \
	PA_CAT(PA_DETAIL_TRY_r, PA_COUNTER)

#define PA_DETAIL_TRY_INTO(return, hspec, result, ...)                    \
	hspec result = (__VA_ARGS__);                                         \
	if (!result)                                                          \
	{                                                                     \
		return PA_ERROR(static_cast<decltype(result)&&>(result).error()); \
	}

#define PA_DETAIL_TRY_2(return, hspec, vspec, result, ...)                                                                                   \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__) static_assert(!PA_DETAIL_TRY_IS_VOID(result), "TRY requires a non-void result."); \
	auto&& vspec = *static_cast<decltype(result)&&>(result)

#define PA_DETAIL_TRY_1(return, spec, ...) \
	PA_DETAIL_TRY_2(return, PA_DETAIL_TRY_H(spec), PA_DETAIL_TRY_V(spec), PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRY(spec, ...) \
	PA_DETAIL_TRY_1(return, spec, __VA_ARGS__)

#define PA_CO_TRY(spec, ...) \
	PA_DETAIL_TRY_1(co_return, spec, __VA_ARGS__)


#define PA_DETAIL_TRYA_2(return, hspec, left, result, ...)                                                                                    \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__) static_assert(!PA_DETAIL_TRY_IS_VOID(result), "TRYA requires a non-void result."); \
	((void)(left = *static_cast<decltype(result)&&>(result)))

#define PA_DETAIL_TRYA_1(return, left, ...) \
	PA_DETAIL_TRYA_2(return, PA_DETAIL_TRY_H(left), PA_DETAIL_TRY_V(left), PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRYA(left, ...) \
	PA_DETAIL_TRYA_1(return, left, __VA_ARGS__)

#define PA_CO_TRYA(left, ...) \
	PA_DETAIL_TRYA_1(co_return, left, __VA_ARGS__)


#define PA_DETAIL_TRYS_2(return, hspec, vspec, result, ...)                                                                                   \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__) static_assert(!PA_DETAIL_TRY_IS_VOID(result), "TRYS requires a non-void result."); \
	auto&& [PA_EXPAND vspec] = *static_cast<decltype(result)&&>(result)

#define PA_DETAIL_TRYS_1(return, spec, ...) \
	PA_DETAIL_TRYS_2(return, auto, spec, PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRYS(spec, ...) \
	PA_DETAIL_TRYS_1(return, spec, __VA_ARGS__)

#define PA_CO_TRYS(spec, ...) \
	PA_DETAIL_TRYS_1(co_return, spec, __VA_ARGS__)


#define PA_DETAIL_TRYV_2(return, hspec, result, ...)                                                                                     \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__) static_assert(PA_DETAIL_TRY_IS_VOID(result), "TRYV requires a void result."); \
	((void)result)

#define PA_DETAIL_TRYV_1(return, ...) \
	PA_DETAIL_TRYV_2(return, auto, PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRYV(...) \
	PA_DETAIL_TRYV_1(return, __VA_ARGS__)

#define PA_CO_TRYV(...) \
	PA_DETAIL_TRYV_1(co_return, __VA_ARGS__)


#define PA_DETAIL_TRYD_2(return, hspec, result, ...) \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__)((void)result)

#define PA_DETAIL_TRYD_1(return, ...) \
	PA_DETAIL_TRYD_2(return, auto, PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRYD(...) \
	PA_DETAIL_TRYD_1(return, __VA_ARGS__)

#define PA_CO_TRYD(...) \
	PA_DETAIL_TRYD_1(co_return, __VA_ARGS__)

#define PA_DETAIL_TRY_OR_ELSE(hspec, vspec, result, func, ...)                           \
	hspec result = (func);                                                               \
	if (!result)                                                                         \
	{                                                                                    \
		[[maybe_unused]] auto&& error = static_cast<decltype(result)&&>(result).error(); \
		__VA_ARGS__                                                                      \
	}                                                                                    \
	auto&& vspec = *static_cast<decltype(result)&&>(result)

#define PA_TRY_OR_ELSE(spec, func, ...)                                             \
	static_assert(!PA_DETAIL_TRY_IS_VOID(func), "TRY requires a non-void result."); \
	PA_DETAIL_TRY_OR_ELSE(PA_DETAIL_TRY_H(spec), PA_DETAIL_TRY_V(spec), PA_DETAIL_TRY_R, func, __VA_ARGS__)

#define PA_DETAIL_TRYV_OR_ELSE(hspec, result, func, ...)                                 \
	hspec result = (func);                                                               \
	if (!result)                                                                         \
	{                                                                                    \
		[[maybe_unused]] auto&& error = static_cast<decltype(result)&&>(result).error(); \
		__VA_ARGS__                                                                      \
	}                                                                                    \
	((void)result)

#define PA_TRYV_OR_ELSE(func, ...)                                              \
	static_assert(PA_DETAIL_TRY_IS_VOID(func), "TRYV requires a void result."); \
	PA_DETAIL_TRYV_OR_ELSE(auto, PA_DETAIL_TRY_R, func, __VA_ARGS__)

#define PA_DETAIL_TRY_IS_PTR(result) \
	(::std::indirectly_readable<PA_DETAIL_TRY_VALUE_T(result)>)

#define PA_DETAIL_TRY_PTR_2(return, hspec, vspec, result, ...)                                                                                               \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__)                                           \
	static_assert(PA_DETAIL_TRY_IS_PTR(result), "TRY_PTR requires an indirectly readable result."); \
	auto&& vspec = **(result);                                                                                                                                \
	((void)0)

#define PA_DETAIL_TRY_PTR_1(return, spec, ...) \
	PA_DETAIL_TRY_PTR_2(return, PA_DETAIL_TRY_H(spec), PA_DETAIL_TRY_V(spec), PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRY_PTR(spec, ...) \
	PA_DETAIL_TRY_PTR_1(return, spec, __VA_ARGS__)


#define PA_DETAIL_TRY_PTRA_2(return, hspec, left, result, ...)                             \
	PA_DETAIL_TRY_INTO(return, hspec, result, __VA_ARGS__)                             \
	static_assert(PA_DETAIL_TRY_IS_PTR(result), "TRY_PTRA requires an indirectly readable result."); \
	((void)(left = static_cast<std::remove_pointer_t<PA_DETAIL_TRY_VALUE_T(result)>&&>(**(result))))
	//((void)(left = **(result)

#define PA_DETAIL_TRY_PTRA_1(return, left, ...) \
	PA_DETAIL_TRY_PTRA_2(return, PA_DETAIL_TRY_H(left), PA_DETAIL_TRY_V(left), PA_DETAIL_TRY_R, __VA_ARGS__)

#define PA_TRY_PTRA(left, ...) \
	PA_DETAIL_TRY_PTRA_1(return, left, __VA_ARGS__)

}  // namespace PotatoAlert::Core
