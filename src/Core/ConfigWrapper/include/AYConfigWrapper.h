#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>
#include <map>

class AYConfigWrapper
{
public:
	
	enum class ConfigType {
		INI,
		JSON,
		XML,
		//YAML
	};
	
	void loadFromFile(const std::string& path, ConfigType type);
	void saveConfig(const std::string& path);

	template <typename T>
	T get(const std::string& path, const std::string& default_value = "");

	template <typename T>
	void set(const std::string& path, const T& value);

	template <typename T>
	std::vector<T> getVector(const std::string& path);

	template <typename K, typename V>
	std::map<K, V> getMap(const std::string& path);


private:
	boost::property_tree::ptree _pt;
	std::string _path;
	ConfigType _type;
};

template<typename T>
inline T AYConfigWrapper::get(const std::string& path, const std::string& default_value)
{
	return _pt.get<T>(path,default_value);
}

template<typename T>
inline void AYConfigWrapper::set(const std::string& path, const T& value)
{
	_pt.put<T>(path, value);
}

template<typename T>
inline std::vector<T> AYConfigWrapper::getVector(const std::string& path)
{
	std::vector<T> result;
	for (const auto& item : _pt.get_child(path)) {
		result.push_back(item.second.get_value<T>());
	}
	return result;
}

template<typename K, typename V>
inline std::map<K, V> AYConfigWrapper::getMap(const std::string& path)
{
	std::map<K, V> result;
	for (const auto& item : _pt.get_child(path)) {
		result[item.first] = item.second.get_value<V>();
	}
	return result;
}