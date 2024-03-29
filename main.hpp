#include "callstack_spoof.hpp"

// 调用方式: SPOOFF(函数名, 参数...) // 重载函数调用方式: static_cast<返回值类型 (*)(参数)>(重载函数名)
#define SPOOFF(funcname, ...) (CallSpoofer::SpoofCall<decltype(funcname(__VA_ARGS__)), decltype(funcname)>(funcname))(__VA_ARGS__)


// 调用方式: SPOOFF2(函数名)(参数...) // 重载函数调用方式: static_cast<返回值类型 (*)(参数)>(重载函数名)
#define SPOOFF2(funcPtr) CallSpoofer::spooff<decltype(funcPtr), funcPtr>
template <typename funcPtr, funcPtr f, typename... Args> inline auto spooff(Args&&... args) {
	using ReturnType = ::std::invoke_result_t<funcPtr, decltype(args)...>;
	auto CallSpooferInstance = (CallSpoofer::SpoofCall<ReturnType, funcPtr>(f));
	if constexpr (::std::is_same<ReturnType, void>::value) {
		CallSpooferInstance(::std::forward<Args>(args)...);
		return;
	}
	else {
		return CallSpooferInstance(::std::forward<Args>(args)...);
	}
}


// 使用匿名函数+tuple实现
// 只支持没有引用类型参数的函数的重载, 因为::std::make_tuple(__VA_ARGS__)创建的tuple没有存储原始类型信息(引用类型都变成了其原始类型存储)
#include <tuple>
#define SPOOFOF(funcname, ...)                                                                                                                    \
        ::std::apply([](auto... args) /* 不能使用auto&&... args */ -> auto {                                                                       \
            using OverloadedFuncRetType = decltype(funcname(__VA_ARGS__)); /* 根据提供的参数让编译器决议出应该调用的重载函数然后获取其返回值的类型 */       \
            using OverloadedFuncType = OverloadedFuncRetType (*)(decltype(args)...); /* 根据参数类型和返回值类型拼装出确定的函数的声明 */                \
            auto specific_func = static_cast<OverloadedFuncType>(funcname); /* 用之前拼装出的函数声明来明确告诉编译器去使用众多重载函数中的哪一个 */        \
            auto csf = (CallSpoofer::SpoofCall<OverloadedFuncRetType, decltype(specific_func)>(specific_func));                                   \
	        if constexpr (::std::is_same<OverloadedFuncRetType, void>::value) { /* 判断函数返回类型是否为void */                                     \
                csf(/*::std::forward<decltype(args)>(args)*/args...);                                                                             \
                return;                                                                                                                           \
            }                                                                                                                                     \
            else {                                                                                                                                \
                return csf(/*::std::forward<decltype(args)>(args)*/args...);                                                                      \
            }                                                                                                                                     \
        }, ::std::make_tuple(__VA_ARGS__))


// 匿名函数直接传参实现
// 如果匿名函数形参为auto&&... args则不支持基础数据类型如int, 但是支持引用类型参数
// 如果匿名函数形参为auto... args则支持基础数据类型如int, 但是不支持引用类型参数
#include <tuple>
#define SPOOFOF(funcname, ...)                                                                                                                    \
        ([](auto&&... args) -> auto {                                                                       \
            using OverloadedFuncRetType = decltype(funcname(__VA_ARGS__)); /* 根据提供的参数让编译器决议出应该调用的重载函数然后获取其返回值的类型 */       \
            using OverloadedFuncType = OverloadedFuncRetType (*)(decltype(args)...); /* 根据参数类型和返回值类型拼装出确定的函数的声明 */                \
            auto specific_func = static_cast<OverloadedFuncType>(funcname); /* 用之前拼装出的函数声明来明确告诉编译器去使用众多重载函数中的哪一个 */        \
            auto csf = (CallSpoofer::SpoofCall<OverloadedFuncRetType, decltype(specific_func)>(specific_func));                                   \
	        if constexpr (::std::is_same<OverloadedFuncRetType, void>::value) { /* 判断函数返回类型是否为void */                                     \
                csf(::std::forward<decltype(args)>(args)...);                                                                             \
                return;                                                                                                                           \
            }                                                                                                                                     \
            else {                                                                                                                                \
                return csf(::std::forward<decltype(args)>(args)...);                                                                      \
            }                                                                                                                                     \
        })(__VA_ARGS__)


template <typename _Tp>
struct remove_rvreference
{
    typedef _Tp type;
};
template <typename _Tp>
struct remove_rvreference<_Tp&&>
{
    typedef _Tp type;
};

// 匿名函数直接传参实现改进
// 将参数包中的右值引用转换为其原始类型, 左值引用不变: typename remove_rvreference<decltype(args)>::type 这样可以实现支持含有基础类型参数的重载函数
#include <tuple>
#define SPOOFOF(funcname, ...)                                                                                                                                         \
        ([](auto&&... args) -> auto {                                                                                                                                   \
            using OverloadedFuncRetType = decltype(funcname(__VA_ARGS__)); /* 根据提供的参数让编译器决议出应该调用的重载函数然后获取其返回值的类型 */                             \
            using OverloadedFuncType = OverloadedFuncRetType (*)(typename remove_rvreference<decltype(args)>::type...); /* 根据参数类型和返回值类型拼装出确定的函数的声明 */   \
            auto specific_func = static_cast<OverloadedFuncType>(funcname); /* 用之前拼装出的函数声明来明确告诉编译器去使用众多重载函数中的哪一个 */                              \
            auto csf = (CallSpoofer::SpoofCall<OverloadedFuncRetType, decltype(specific_func)>(specific_func));                                                         \
	        if constexpr (::std::is_same<OverloadedFuncRetType, void>::value) { /* 判断函数返回类型是否为void */                                                           \
                csf(::std::forward<decltype(args)>(args)...);                                                                                                           \
                return;                                                                                                                                                 \
            }                                                                                                                                                           \
            else {                                                                                                                                                      \
                return csf(::std::forward<decltype(args)>(args)...);                                                                                                    \
            }                                                                                                                                                           \
        })(__VA_ARGS__)


// 匿名函数+宏实现
#define ExpandArgs_0pram()
#define ExpandArgs_1pram(a1) decltype(a1)
#define ExpandArgs_2pram(a1,a2) decltype(a1),decltype(a2)
#define ExpandArgs_3pram(a1,a2,a3) decltype(a1),decltype(a2),decltype(a3)
#define ExpandArgs_4pram(a1,a2,a3,a4) decltype(a1),decltype(a2),decltype(a3),decltype(a4)
#define ExpandArgs_5pram(a1,a2,a3,a4,a5) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5)
#define ExpandArgs_6pram(a1,a2,a3,a4,a5,a6) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5),decltype(a6)
#define ExpandArgs_7pram(a1,a2,a3,a4,a5,a6,a7) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5),decltype(a6),decltype(a7)
#define ExpandArgs_8pram(a1,a2,a3,a4,a5,a6,a7,a8) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5),decltype(a6),decltype(a7),decltype(a8)
#define ExpandArgs_9pram(a1,a2,a3,a4,a5,a6,a7,a8,a9) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5),decltype(a6),decltype(a7),decltype(a8),decltype(a9)
#define ExpandArgs_10pram(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) decltype(a1),decltype(a2),decltype(a3),decltype(a4),decltype(a5),decltype(a6),decltype(a7),decltype(a8),decltype(a9),decltype(a10)

#define ExpandArgs_GetMacro(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, NAME, ...) NAME

#define eatComma(...) ,##__VA_ARGS__
#define leftBracket (

#define ExpandArgs(...) ExpandArgs_GetMacro leftBracket eatComma(__VA_ARGS__), ExpandArgs_10pram, ExpandArgs_9pram, ExpandArgs_8pram, ExpandArgs_7pram, ExpandArgs_6pram, ExpandArgs_5pram, ExpandArgs_4pram, ExpandArgs_3pram, ExpandArgs_2pram, ExpandArgs_1pram, ExpandArgs_0pram)(__VA_ARGS__)
#define Expand_LeftBracket(x) x // 辅助延迟展开, 使leftBracket被替换为"("并且让eatComma(__VA_ARGS__)返回的",##__VA_ARGS__"不再被视为整体

// 目前最大支持10个参数, 可以继续增加
#define SPOOFOF6(funcname, ...)                                                                                                                                 \
        ([&]() -> auto {                                                                                                                                        \
            using OverloadedFuncRetType = decltype(funcname(__VA_ARGS__)); /* 根据提供的参数让编译器决议出应该调用的重载函数然后获取其返回值的类型 */                     \
            using OverloadedFuncType = OverloadedFuncRetType (*)(Expand_LeftBracket(ExpandArgs(__VA_ARGS__))); /* 根据参数类型和返回值类型拼装出确定的函数的声明 */    \
            auto specific_func = static_cast<OverloadedFuncType>(funcname); /* 用之前拼装出的函数声明来明确告诉编译器去使用众多重载函数中的哪一个 */                      \
            auto csf = (CallSpoofer::SpoofCall<OverloadedFuncRetType, decltype(specific_func)>(specific_func));                                                 \
	        if constexpr (::std::is_same<OverloadedFuncRetType, void>::value) { /* 判断函数返回类型是否为void */                                                   \
                csf(__VA_ARGS__);                                                                                                                               \
                return;                                                                                                                                         \
            }                                                                                                                                                   \
            else {                                                                                                                                              \
                return csf(__VA_ARGS__);                                                                                                                        \
            }                                                                                                                                                   \
        })()