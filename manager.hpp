#pragma once

//  When having problem using boost::bind or bind dosn't make you happy, 
//  define ARKSP_QT to use the signal&slot of Qt. 
//  Be noticed that the design of emitSignal() is different in ARKSP_QT.
//  If you want to use QML, define ARKSP_INVO to enable Q_INVOKABLE of some functions.
//  In principle, if ARKSP_INVO is defined, ARKSP_QT also should be defined, 
//  but if you don't follow that, you will only get a warning.

//  I don't know why in Qt Creator the macro defined in main.cpp don't take effect,
//  even using the same compiler as VS2019.
//  If you use Qt Creator, you might need to define macros in this hpp.

#if defined(ARKSP_INVO) && !defined(ARKSP_QT)
#pragma message("arksp: ARKSP_INVO defined but ARKSP_QT not defined")
#define ARKSP_QT
#endif

#ifndef ARKSP_QT
#define ARKSP_SIGNAL_TYPE(_Text,_Prop) std::string _Text, std::vector<std::pair<std::string, std::string>> _Prop
#define ARKSP_SIGNAL_GLOBAL(_Func_name, _Text, _Prop) std::string _Func_name, std::string _Text, \
	std::vector<std::pair<std::string, std::string>> _Prop
#include <boost/signals2.hpp>
#include <functional>
#else
#include <qobject.h>
#include <qstring.h>
#include <qvariant.h>
#include <qmap.h>
#endif

#include <utility>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <regex>

#ifdef ARKSP_INVO
#include "lexer.hpp"
#include <qfile.h>
#endif

#include "core.hpp"

namespace arksp {
#ifdef ARKSP_QT
	class Manager : public QObject {
		Q_OBJECT
#else
	class Manager {
#endif
	public:
		Manager() {}

		using sizeType = std::vector<arksp::token>::size_type;

		bool init(const std::vector<arksp::token>& vecToken) {
			m_vecToken.clear();
			m_vecToken.shrink_to_fit();
			m_iteToken = std::vector<arksp::token>::iterator();
#ifndef ARKSP_QT
			m_vecSig.clear();
			m_vecSig.shrink_to_fit();
#endif

			if (vecToken.empty()) {
				throw std::string("Error: Empty vecToken");
				return false;
			}
			m_vecToken = vecToken;
			m_iteToken = m_vecToken.begin();

			return true;
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool init(QString path) {
			m_vecToken.clear();
			m_vecToken.shrink_to_fit();
			m_iteToken = std::vector<arksp::token>::iterator();

			QFile file(path);
			if (!file.exists()) {
				emit signalException("Error: File " + path + " not exists");
				return false;
			}
			file.open(QIODevice::ReadOnly);

			try {
				m_vecToken = arksp::Lexer::lexer(file.readAll().toStdString());
				m_iteToken = m_vecToken.begin();
			}
			catch (std::exception& e) {
				emit signalException(QString(e.what()));
				return false;
			}
			catch (std::string& e) {
				emit signalException(QString::fromStdString(e));
				return false;
			}
			file.close();
			return true;
		}
#endif

#ifndef ARKSP_QT
		bool connect(const std::string& func_name,
			const std::function<void(ARKSP_SIGNAL_TYPE())>& slot,
			const int& group = 0) {
			at<std::string, SignalType>(func_name, m_vecSig).connect(group, slot);
			return true;
		}
		bool connect(const std::string& func_name,
			std::function<void(ARKSP_SIGNAL_TYPE())>&& slot,
			const int& group = 0) {
			at<std::string, SignalType>(func_name, m_vecSig).connect(group, slot);
			return true;
		}
		bool connect(const std::function<void(ARKSP_SIGNAL_GLOBAL())>& slot,
			const int& group = 0) {
			m_globalSig.connect(group, slot);
			return true;
		}
		bool connect(std::function<void(ARKSP_SIGNAL_GLOBAL())>&& slot,
			const int& group = 0) {
			m_globalSig.connect(group, slot);
			return true;
		}
		bool disconnect(const int& group) {
			m_globalSig.disconnect(group);
			return true;
		}
		bool disconnect(const std::string& func_name, const int& group) {
			for (auto ite = m_vecSig.begin(); ite < m_vecSig.end(); ++ite) {
				if (ite->first == func_name) {
					ite->second.disconnect(group);
				}
			}
			return true;
		}
#endif

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrForward(const unsigned int& step = 1) {
#else
		bool ptrForward(const unsigned int& step = 1) {
#endif
			if (m_iteToken + step >= m_vecToken.end()) {  //  no need for exception
				return false;
			}
			m_iteToken += step;
			return true;
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrBackward(const unsigned int& step = 1) {
#else
		bool ptrBackward(const unsigned int& step = 1) {
#endif		
			if (m_iteToken - step < m_vecToken.begin()) {  //  no need for exception
				return false;
			}
			m_iteToken -= step;
			return true;
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrMoveToPoint(const QString & qchoice) {
			std::string choice = qchoice.toStdString();
#else
		bool ptrMoveToPoint(const std::string & choice) {
#endif		
			for (arksp::Manager::sizeType s = m_iteToken - m_vecToken.begin(); s < m_vecToken.size(); ++s) {
				if (std::get<arksp::Func>(m_vecToken[s]) == "predicate" &&
					getValueByPropName("references", std::get<arksp::Prop>(m_vecToken[s])) == choice) {
					m_iteToken = m_vecToken.begin() + s;
					return true;
				}
			}
			return false;
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrGoto(const unsigned int& point) {
#else
		bool ptrGoto(const unsigned int& point) {
#endif
			if (point < m_vecToken.size() || point >= 0) {
				m_iteToken = m_vecToken.begin() + point;
				return true;
			}
			else {
#ifdef ARKSP_INVO
				emit signalException("Error: Out of index");
#else
				throw std::string("Error: Out of index");
#endif
				return false;
			}
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrRewind() {
#else
		bool ptrRewind() {
#endif
			m_iteToken = m_vecToken.begin();
			return true;
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool ptrFastForward() {
#else
		bool ptrFastForward() {
#endif
			if (m_vecToken.empty()) {
#ifdef ARKSP_INVO
				emit signalException("Error: Empty vecToken");
#else
				throw std::string("Error: Empty vecToken");
#endif
				return false;
			}
			m_iteToken = m_vecToken.end() - 1;
			return true;
		}

		arksp::token getToken(void) {
			return *m_iteToken;
		}
#ifdef ARKSP_INVO
		Q_INVOKABLE unsigned int getIndex(void) {
#else
		std::vector<arksp::token>::size_type getIndex(void) {
#endif
			return (m_iteToken - m_vecToken.begin());
		}


#ifdef ARKSP_INVO
		Q_INVOKABLE unsigned int getSize() {
#else
		auto getSize() {
#endif		
			return m_vecToken.size();
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE QString getFuncName() {
			return QString::fromStdString(std::get<arksp::Func>(*m_iteToken));
		}
		Q_INVOKABLE QVariantMap getPropMap() {
			QVariantMap qvm;
			for (auto s : std::get<arksp::Prop>(*m_iteToken)) {
				qvm[QString::fromStdString(s.first)] = QString::fromStdString(s.second);
			}
			return qvm;
		}
		Q_INVOKABLE QString getText() {
			return QString::fromStdString(std::get<arksp::Text>(*m_iteToken));
		}
#endif

#ifdef ARKSP_INVO
		Q_INVOKABLE bool replace(const QString & json_) {
			std::string json = json_.toStdString();
#else
		bool replace(const std::string & json) {
#endif
			int index = 0;
			try {
				boost::property_tree::ptree root;
				std::stringstream ss(json);
				boost::property_tree::read_json(ss, root);

				for (auto& s : m_vecToken) {
					++index;
					for (auto& i : std::get<arksp::Prop>(s)) {
						if (i.second != "" && i.second[0] == '$') {
							auto item = root.get_child(std::string(i.second.begin() + 1, i.second.end()));
							i.second = item.data();
						}
					}
				}

				return true;
			}
			catch (std::exception& e) {
#ifdef ARKSP_INVO
				emit signalException("Line " + QString::number(index) + " Syntax error: Value invalid\nBoost: " + QString(e.what()));
#else
				throw std::string("Line " + std::to_string(index) + " Syntax error: Value invalid\nBoost: " + e.what());
#endif
				return false;
			}
		}

#ifdef ARKSP_INVO
		Q_INVOKABLE bool setNickname(const QString& nickname_) {
			std::string nickname = nickname_.toStdString();
#else
		bool setNickname(const std::string & nickname) {
#endif
			std::regex reg("\\{@nickname\\}");			
			for (auto& s : m_vecToken) {
				std::get<arksp::Text>(s) = std::regex_replace(std::get<arksp::Text>(s), reg, nickname);
			}
			return true;
		}

		arksp::token operator[](const std::vector<arksp::token>::size_type& index) {
			auto ite = m_vecToken.begin() + index;
			if (ite >= m_vecToken.end() || ite < m_vecToken.begin()) {
				throw std::string("Error: Pointer of vecToken out of index");
				return arksp::token();
			}
			return *ite;
		}

		static inline std::string getValueByPropName(const std::string & name, const std::vector<std::pair<std::string, std::string>>&prop) {
			for (auto ite = prop.begin(); ite < prop.end(); ++ite) {
				if (ite->first == name) {
					return ite->second;
				}
			}
			return std::string();
		}

#ifndef ARKSP_QT
		void emitSignal() {
			if (m_vecToken.empty()) {
				throw std::string("Error: Empty vecToken");
				return;
			}
			auto strFunc = std::get<arksp::Func>(*m_iteToken), strText = std::get<arksp::Text>(*m_iteToken);
			auto vecProp = std::get<arksp::Prop>(*m_iteToken);
			at<std::string, SignalType>(strFunc, m_vecSig)(strText, vecProp);
			m_globalSig(strFunc, strText, vecProp);
			return;
		}
#else
#ifdef ARKSP_INVO
		Q_INVOKABLE void emitSignal() {
#else
		void emitSignal() {
#endif
			if (m_vecToken.empty()) {
#ifdef ARKSP_INVO
				emit signalException("Error: Empty vecToken");
#else
				throw std::string("Error: Empty vecToken");
#endif
				return;
			}

			QVariantMap qvm;
			for (auto s : std::get<arksp::Prop>(*m_iteToken)) {
				qvm[QString::fromStdString(s.first)] = QString::fromStdString(s.second);
			}
			emit signalToken(QString::fromStdString(std::get<arksp::Func>(*m_iteToken)),
				qvm,
				QString::fromStdString(std::get<arksp::Text>(*m_iteToken)));
		}
#endif

#ifdef ARKSP_QT
	signals:
		void signalToken(QString func_name,
			QVariantMap prop_map,
			QString text);
#endif
#ifdef ARKSP_INVO
		void signalException(QString exception);
#endif

		template<typename A, typename B>
		static inline B& at(const A& key, std::vector<std::pair<A, B>>& vec) {
#ifndef _MSC_VER
			int i = 0;
#else
			typename std::vector<std::pair<A, B>>::size_type i = 0;
#endif
			for (; i < vec.size(); ++i) {
				if (vec[i].first == key) {
					break;
				}
			}
			if (i == vec.size()) {
				vec.push_back({ key,B() });
			}
			return ((vec.begin() + i)->second);
		}

	private:
#ifndef ARKSP_QT
		typedef boost::signals2::signal<void(std::string, std::vector<std::pair<std::string, std::string>>)> SignalType;
		std::vector<std::pair<std::string, SignalType>> m_vecSig;
		boost::signals2::signal<void(std::string, std::string, std::vector<std::pair<std::string, std::string>>)> m_globalSig;
#endif
		std::vector<arksp::token> m_vecToken;
		std::vector<arksp::token>::iterator m_iteToken;		
	};
}