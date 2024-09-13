// Copyright 2022 <github.com/razaqq>
#pragma once

#define PA_NOARG

#define PA_EXPAND(...) __VA_ARGS__

#define PA_CAT_1(a, b) a##b
#define PA_CAT(a, b) PA_CAT_1(a, b)

#define PA_STR_1(...) #__VA_ARGS__
#define PA_STR(...) PA_STR_1(__VA_ARGS__)

#define PA_COUNTER __COUNTER__

#define PA_EMPTY()
#define PA_DEFER_ID(Id) Id PA_EMPTY()
#define PA_COMMA() ,

#define PA_CHAIN_COMMA(chain) PA_CAT(PA_CHAIN_COMMA_0 chain, _END)
#define PA_CHAIN_COMMA_0(X) X PA_CHAIN_COMMA_1
#define PA_CHAIN_COMMA_1(X) PA_DEFER_ID(PA_COMMA)() X PA_CHAIN_COMMA_2
#define PA_CHAIN_COMMA_2(X) PA_DEFER_ID(PA_COMMA)() X PA_CHAIN_COMMA_1
#define PA_CHAIN_COMMA_0_END
#define PA_CHAIN_COMMA_1_END
#define PA_CHAIN_COMMA_2_END

#define PA_ANONYMOUS(name) PA_CAT(name, PA_COUNTER)

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define PA_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define PA_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
	#define PA_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define PA_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define PA_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define PA_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define PA_FUNC_SIG __func__
#else
	#define PA_FUNC_SIG "PA_FUNC_SIG unknown!"
#endif

#define PA_PRAGMA(X) _Pragma(#X)

#define PA_SUPPRESS_WARN_BEGIN                       \
	PA_PRAGMA(warning(push, 0))                      \
	PA_PRAGMA(GCC diagnostic push)                   \
	PA_PRAGMA(GCC diagnostic ignored "-Wall")        \
	PA_PRAGMA(clang diagnostic push)                 \
	PA_PRAGMA(clang diagnostic ignored "-Weverything")

#define PA_SUPPRESS_WARN_END        \
	PA_PRAGMA(GCC diagnostic pop)   \
	PA_PRAGMA(clang diagnostic pop) \
	PA_PRAGMA(warning(pop))
