#pragma once

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#ifndef _MSC_VER
#include <locale>
#endif // _MSC_VER    clang thought that there is no std::tolower(char)

#include "core.hpp"

namespace arksp {
	class Lexer {
	public:
		~Lexer() = default;

		static std::vector<arksp::token> lexer(const std::string& text) {
			if (text.empty()) {
				throw std::string("Error: Empty Text");
				return std::vector<arksp::token>();
			}

			std::vector<std::string> vecText;
			boost::split(vecText, text, boost::is_any_of("\n"), boost::algorithm::token_compress_on);
			if (vecText.empty()) {
				throw std::string("Error: Empty Script");
				return std::vector<arksp::token>();
			}

			std::vector<arksp::token> ret;
			int iCount = 0;
			enum Statement
			{
				EMPTY,
				FUNC,
				PROP,
				VALUE,
				TEXT
			};
			for (auto s : vecText) {
				++iCount;
				if (s == "" || s[0] == '{' || s[0] == '}' || s[0] == '/') {  //  filter useless statements
					continue;
				}

				if (s[0] != '[') {  //  PlainText
					ret.push_back(make_token("PlainText", {}, clean_space(s)));
					continue;
				}

				//  a simple state machine				
				Statement state = Statement::EMPTY;
				std::string term(""), strFunc("");
				std::vector<std::pair<std::string, std::string>> vecProp;
				for (auto c : s) {
					if (state == Statement::TEXT) {
						goto textappend;
						//  sorry for using goto, but it is 1:02 am now, and i want to sleep
					}
					if (c == '[') {
						state = Statement::FUNC;
						term = "";
						continue;
					}
					if (c == '(') {
						if (state != Statement::FUNC) {
							throw std::string("Line " + std::to_string(iCount) + " Syntax error: No Func\n" + s);
							return std::vector<arksp::token>();
						}
						else {
							state = Statement::PROP;
							strFunc = term;
							term = "";
							continue;
						}
					}
					if (c == ')') {
						if (state != Statement::PROP) {
							throw std::string("Line " + std::to_string(iCount) + " Syntax error: Brackets not matched \"(\" missing\n" + s);
							return std::vector<arksp::token>();
						}
						std::vector<std::string> tempVec;
						boost::split(tempVec, term, boost::is_any_of(",="), boost::algorithm::token_compress_off);
						/*
						if (tempVec.size() % 2 != 0) {
							throw std::string("Line " + std::to_string(iCount) + " Syntax error: Prop and value not matched\n" + s);
							return std::vector<arksp::token>();
						}*/
						for (auto& i : tempVec) {
							if (i == "") {
								i = "0";
								continue;
							}
							for (auto& ch : i) {
								if (ch == '\"') {
									ch = 0;
								}
							}
							i = clean_space(i);
						}
						for (auto ite = tempVec.begin(); ite < tempVec.end(); ite += 2) {
							vecProp.push_back({ *ite,*(ite + 1) });
						}
						term = "";
						state = Statement::EMPTY;
						continue;
					}
					if (c == ']') {
						if (state == Statement::FUNC) {
							strFunc = term;
							if (strstr(strFunc.c_str(), "name=") != NULL) {
								vecProp.push_back({ "name",std::string(strFunc.begin() + 6,strFunc.end() - 1) });
								strFunc = "name";
							}
							state = Statement::TEXT;
							term = "";
							continue;
						}
						if (state == Statement::PROP) {
							throw std::string("Line " + std::to_string(iCount) + " Syntax error: Brackets not matched \")\" missing\n" + s);
							return std::vector<arksp::token>();
						}
						state = Statement::TEXT;
						term = "";
						continue;
					}
					textappend:
					term += c;
				}
				if (state != Statement::TEXT) {
					throw std::string("Line " + std::to_string(iCount) + " Syntax error: Brackets not matched \"]\" missing\n" + s);
					return std::vector<arksp::token>();
				}
				ret.push_back(make_token(strFunc, vecProp, clean_space(term)));
				term = "";
			}

#ifdef _MSC_VER
			for (auto& s : ret) {
				for (auto& i : std::get<arksp::TokenType::Func>(s)) {
					i = std::tolower(i);
				}
			}
#else
			std::locale locale;
			for (auto& s : ret) {
				for (auto& i : std::get<arksp::TokenType::Func>(s)) {
					i = std::tolower(i, locale);
				}
			}
#endif
			return ret;
		}

	private:
		static inline arksp::token make_token(const std::string& func,
			const std::vector<std::pair<std::string, std::string>>& prop,
			const std::string& text) {
			return { func,prop,text };
		}
		static inline std::string clean_space(const std::string& str) {
			std::string ret;
			for (auto& c : str) {
				if (c != ' ' && c != '\0') {
					ret += c;
				}
			}
			return ret;
		}

		Lexer() {};  //  S H E N  B I
		Lexer(const Lexer&) {}
		Lexer& operator=(const Lexer&) {}
	};
}