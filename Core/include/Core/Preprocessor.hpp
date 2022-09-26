// Copyright 2022 <github.com/razaqq>
#pragma once

#define PA_EXPAND(...) __VA_ARGS__

#define PA_CAT_1(a, b) a##b
#define PA_CAT(a, b) PA_CAT_1(a, b)

#define PA_STR_1(...) #__VA_ARGS__
#define PA_STR(...) PA_STR_1(__VA_ARGS__)

#define PA_COUNTER __COUNTER__

#define PA_EMPTY()
#define PA_DEFER(Id) Id PA_EMPTY()
#define PA_COMMA() ,

#define PA_CHAIN_COMMA(chain) PA_CAT(PA_CHAIN_COMMA_0 chain, _END)
#define PA_CHAIN_COMMA_0(X) X PA_CHAIN_COMMA_1
#define PA_CHAIN_COMMA_1(X) PA_DEFER(PA_COMMA)() X PA_CHAIN_COMMA_2
#define PA_CHAIN_COMMA_2(X) PA_DEFER(PA_COMMA)() X PA_CHAIN_COMMA_1
#define PA_CHAIN_COMMA_0_END
#define PA_CHAIN_COMMA_1_END
#define PA_CHAIN_COMMA_2_END
