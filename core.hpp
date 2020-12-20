#pragma once

//  To use ArknightsScriptParser and it's derived libraries, 
//  the compiler must support C++14. And it's recommended to 
//  use C++17, in order to avoid some unnecessary work.

#include <vector>
#include <string>
#include <utility>

#define ARKSP_EASY_READALL(_Str, _Ifs) \
	_Str.assign(std::istreambuf_iterator<char>( _Ifs ), \
	std::istreambuf_iterator<char>())

namespace arksp {
	enum TokenType
	{
		Func = 0,
		Prop = 1,
		Text = 2
	};
	typedef std::tuple<std::string, std::vector<std::pair<std::string, std::string>>, std::string> token;
}