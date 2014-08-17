#pragma once
#ifdef DEBUG
	#include <iostream>
	#include <boost/format.hpp>
	#include <boost/preprocessor/variadic/size.hpp>
	#include <boost/preprocessor/if.hpp>
	#include <boost/preprocessor/comparison/equal.hpp>
	template <class... Ts>
	std::string ConcatMessage(boost::format&& fmt, Ts&&... /*t*/) { return fmt.str(); }
	template <class T, class... Ts>
	std::string ConcatMessage(boost::format&& fmt, T&& t, Ts&&... ts) {
		fmt % std::forward<T>(t);
		return ConcatMessage(std::move(fmt), std::forward<Ts>(ts)...);
	}
	#define DebugMsg0(msg) {std::cout << msg;}
	#define DebugMsgA(msg, ...) {std::cout << ConcatMessage(boost::format(msg), __VA_ARGS__);}
	#define DebugMsg_(...) \
		BOOST_PP_IF( \
			BOOST_PP_EQUAL( \
				BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), \
				1 \
			), \
			DebugMsg0, \
			DebugMsgA \
		)(__VA_ARGS__)
	#define DebugMsg(...) DebugMsg_(__VA_ARGS__)
	#define DebugMsgLn(...) {DebugMsg_(__VA_ARGS__) std::cout << std::endl;}
#else
	#define DebugMsg(...)
	#define DebugMsgLn(...)
#endif

