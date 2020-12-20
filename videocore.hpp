#pragma once

#ifdef ARKSP_CONTEXT
#include <opencv2/core.hpp>
#define ARKSP_COMP_PROC_TYPE(_List, _Prop) \
	std::list<std::shared_ptr<cv::Mat>>* _List, \
	const std::vector<std::pair<std::string, std::string>>& _Prop
#endif

#include <vector>
#include <list>
#include <map>
#include <utility>
#include <functional>

#include "core.hpp"
#include "manager.hpp"

namespace arksp {
#ifdef ARKSP_CONTEXT
	class Component {
	public:
		Component() {}

		void upFit(const unsigned long& maxFrame) {
			if (getFrameNumber() >= maxFrame) {
				return;
			}
			else if (getFrameNumber() == 0) {
				m_listMat.resize(maxFrame);
				return;
			}
			else {
				iterator ite = m_listMat.end();
				--ite;
				m_listMat.push_back(*ite);
				upFit(maxFrame);
			}
		}

		void process(const std::function<void(ARKSP_COMP_PROC_TYPE())>& ProcessFunction,
			const std::vector<std::pair<std::string, std::string>>& prop) {
			ProcessFunction(&m_listMat, prop);
		}
		void process(std::function<void(ARKSP_COMP_PROC_TYPE())>&& ProcessFunction,
			const std::vector<std::pair<std::string, std::string>>& prop) {
			ProcessFunction(&m_listMat, prop);
		}

		typedef std::list<std::shared_ptr<cv::Mat>>::iterator iterator;
		unsigned long getFrameNumber() {
			return m_listMat.size();
		}
		auto empty() {
			return m_listMat.empty();
		}
		iterator begin() {
			return m_listMat.begin();
		}
		iterator end() {
			return m_listMat.end();
		}

	private:
		std::list<std::shared_ptr<cv::Mat>> m_listMat;
	};

	class Context {
	public:
		typedef std::pair<std::string, std::vector<std::pair<std::string, std::string>>> FuncType;
		
		Context(std::vector<std::vector<std::pair<std::string, std::string>>>* env) {
			m_env = env;
		}

		void setFunc(const FuncType& func) {
			m_vecFunc.push_back(func);
		}

		static Context create(std::vector<std::vector<std::pair<std::string,std::string>>>* env) {
			return Context(env);
		}

	private:
		Context() {}
		std::vector<FuncType> m_vecFunc;
		std::vector<std::vector<std::pair<std::string, std::string>>>* m_env = nullptr;

		static inline auto getProp(const std::string& func,
			std::vector<FuncType>& vecFunc) {
			auto ite = std::find_if(vecFunc.begin(), vecFunc.end(), [&](const auto& p) {
				return p.first == func;
				});
			if (ite != vecFunc.end()) {
				return ite->second;
			}
			else {
				return std::vector<std::pair<std::string, std::string>>();
			}
		}
	};
#endif

	class Environment {
	public:
		typedef std::vector<std::pair<std::string, std::string>> EnvState;
		typedef std::vector<std::pair<std::string, std::string>> PropType;

		Environment() {
			m_env.push_back(current);
#ifdef ARKSP_CONTEXT
			m_ctx.push_back(Context::create(&m_env));
#endif
		}

		void slotRead(ARKSP_SIGNAL_GLOBAL(func, text, prop)) {
			auto _ite2 = m_env.end() - 1;
			if (func == "background") {
				setEnv(func, arksp::Manager::getValueByPropName("image", prop), current);
				if (arksp::Manager::getValueByPropName("xscale", prop) != "") {
					setEnv("bg_xscale", arksp::Manager::getValueByPropName("xscale", prop), current);
				}
				if (arksp::Manager::getValueByPropName("yscale", prop) != "") {
					setEnv("bg_yscale", arksp::Manager::getValueByPropName("yscale", prop), current);
				}
				if (arksp::Manager::getValueByPropName("y", prop) != "") {
					setEnv("bg_y", arksp::Manager::getValueByPropName("y", prop), current);
				}
				if (arksp::Manager::getValueByPropName("x", prop) != "") {
					setEnv("bg_x", arksp::Manager::getValueByPropName("x", prop), current);
				}
			}
			else if (func == "backgroundtween") {
				auto _yTo = arksp::Manager::getValueByPropName("yTo", prop);
				auto _xTo = arksp::Manager::getValueByPropName("xTo", prop);
				if (_yTo != "") {
					auto fin = std::to_string(std::stof(_yTo) + std::stof(getValueOfEnv("bg_y", current)));
					setEnv("bg_y", fin, current);
				}
				if (_xTo != "") {
					auto fin = std::to_string(std::stof(_xTo) + std::stof(getValueOfEnv("bg_x", current)));
					setEnv("bg_x", fin, current);
				}
			}
			else if (func == "character") {
				//  character() means clear
				if (arksp::Manager::getValueByPropName("name", prop) == "") {
					setEnv("middle", "", current);
					setEnv("left", "", current);
					setEnv("right", "", current);
					setEnv("middle_xpos", "0", current);
					setEnv("middle_ypos", "0", current);
					setEnv("first_xpos", "0", current);
					setEnv("first_ypos", "0", current);
					setEnv("second_xpos", "0", current);
					setEnv("second_ypos", "0", current);
				}
				else if (arksp::Manager::getValueByPropName("name2", prop) == "") {
					setEnv("middle", arksp::Manager::getValueByPropName("name", prop), current);
					//  it's clearly that if B only has name, A should be cleaned
					setEnv("first_xpos", "0", current);
					setEnv("first_ypos", "0", current);
					setEnv("second_xpos", "0", current);
					setEnv("second_ypos", "0", current);
					setEnv("left", "", current);
					setEnv("right", "", current);
				}
				else {
					//  if B has name and name2, but A only has name, then clean the state
					if (!getValueOfEnv("middle", *_ite2).empty()) {
						setEnv("middle_xpos", "0", current);
						setEnv("middle_ypos", "0", current);
						setEnv("first_xpos", "0", current);
						setEnv("first_ypos", "0", current);
						setEnv("second_xpos", "0", current);
						setEnv("second_ypos", "0", current);
					}

					setEnv("middle", "", current);
					setEnv("left", arksp::Manager::getValueByPropName("name", prop), current);
					setEnv("right", arksp::Manager::getValueByPropName("name2", prop), current);
				}
			}
			else if (func == "characteraction") {
				auto type = arksp::Manager::getValueByPropName("type", prop);
				auto xpos = arksp::Manager::getValueByPropName("xpos", prop);
				auto ypos = arksp::Manager::getValueByPropName("ypos", prop);
				if (arksp::Manager::getValueByPropName("name", prop) == "left") {
					if (type == "exit") {
						if (arksp::Manager::getValueByPropName("direction", prop) == "left") {
							setEnv("first_xpos", "-4000", current);
						}
						else {
							setEnv("first_xpos", "4000", current);
						}
					}
					else {
						if (!xpos.empty()) {
							auto f = std::stof(xpos) + std::stof(getValueOfEnv("first_xpos", *_ite2));
							setEnv("first_xpos", std::to_string(f), current);
						}
						if (!ypos.empty()) {
							auto f = std::stof(ypos) + std::stof(getValueOfEnv("first_ypos", *_ite2));
							setEnv("first_ypos", std::to_string(f), current);
						}
					}
				}
				else {
					if (type == "exit") {
						if (arksp::Manager::getValueByPropName("direction", prop) == "left") {
							setEnv("second_xpos", "-2000", current);
						}
						else {
							setEnv("second_xpos", "2000", current);
						}
					}
					else {
						if (!xpos.empty()) {
							auto f = std::stof(xpos) + std::stof(getValueOfEnv("second_xpos", *_ite2));
							setEnv("second_xpos", std::to_string(f), current);
						}
						if (!ypos.empty()) {
							auto f = std::stof(ypos) + std::stof(getValueOfEnv("second_ypos", *_ite2));
							setEnv("second_ypos", std::to_string(f), current);
						}
					}
				}
			}
			else if (func == "image") {
				setEnv(func, arksp::Manager::getValueByPropName("image", prop), current);
				if (arksp::Manager::getValueByPropName("xscale", prop) != "") {
					setEnv("image_xScale", arksp::Manager::getValueByPropName("xScale", prop), current);
				}
				if (arksp::Manager::getValueByPropName("yscale", prop) != "") {
					setEnv("image_yScale", arksp::Manager::getValueByPropName("yScale", prop), current);
				}
				if (arksp::Manager::getValueByPropName("y", prop) != "") {
					setEnv("image_y", arksp::Manager::getValueByPropName("y", prop), current);
				}
				if (arksp::Manager::getValueByPropName("x", prop) != "") {
					setEnv("image_x", arksp::Manager::getValueByPropName("x", prop), current);
				}
			}
			else if (func == "imagetween") {
				auto _yTo = arksp::Manager::getValueByPropName("yTo", prop);
				auto _xTo = arksp::Manager::getValueByPropName("xTo", prop);
				auto _xScale = arksp::Manager::getValueByPropName("xScaleTo", prop);
				auto _yScale = arksp::Manager::getValueByPropName("yScaleTo", prop);
				if (_yTo != "") {
					auto fin = std::to_string(std::stof(_yTo) + std::stof(getValueOfEnv("bg_y", current)));
					setEnv("image_y", fin, current);
				}
				if (_xTo != "") {
					auto fin = std::to_string(std::stof(_xTo) + std::stof(getValueOfEnv("bg_x", current)));
					setEnv("image_x", fin, current);
				}
				if (_xScale != "") {
					setEnv("image_xScale", _xScale, current);
				}
				if (_yScale != "") {
					setEnv("image_yScale", _yScale, current);
				}
			}

#ifdef ARKSP_CONTEXT
			auto _ite = m_ctx.end() - 1;
			_ite->setFunc({ func,prop });
#endif
			if (func == "delay" || arksp::Manager::getValueByPropName("block", prop) == "true") {
#ifdef ARKSP_CONTEXT
				m_ctx.push_back(Context::create(&m_env));
#endif
				m_env.push_back(current);
			}
		}

		EnvState* getContext(std::vector<EnvState>::size_type index) {
			auto ite = m_env.begin() + index;
			if (ite >= m_env.end() || ite < m_env.begin()) {
				throw std::string("Error: out of index in Env list");
				return nullptr;
			}
			return &(*ite);
		}

		static inline std::string getValueOfEnv(const std::string& key, const EnvState& env) {
			auto _ite = std::find_if(env.begin(), env.end(), [&](const auto& p) {
				return p.first == key;
				});
			if (_ite != env.end()) {
				return _ite->second;
			}
			else {
				return std::string();
			}
		}
		static inline void setEnv(const std::string& key,
			const std::string& value,
			EnvState& env) {
			auto _ite = std::find_if(env.begin(), env.end(), [&](const auto& p) {
				return p.first == key;
				});
			if (_ite != env.end()) {
				_ite->second = value;
			}
			else {
				env.push_back({ key,value });
			}
		}

#ifdef ARKSP_DEBUG
		std::vector<EnvState> m_env;
#ifdef ARKSP_CONTEXT
		std::vector<Context> m_ctx;
#endif
#endif

	private:
#ifndef ARKSP_DEBUG
		std::vector<EnvState> m_env;		
#ifdef ARKSP_CONTEXT
		std::vector<Context> m_ctx;
#endif
#endif
		EnvState current{ {"middle",""},
				{"left",""},
				{"right",""},
				{"background","default"},
				{"bg_xscale","1"},
				{"bg_yscale","1"},
				{"bg_x","0"},
				{"bg_y","0"},
				{"image",""},
				{"image_xScale","1"},
				{"image_yScale","1"},
				{"image_x","0"},
				{"image_y","0"},
				{"first_xpos","0"},
				{"first_ypos","0"},
				{"second_xpos","0"},
				{"second_ypos","0"},
				{"middle_xpos","0"},
				{"middle_ypos","0"} };
	};
}