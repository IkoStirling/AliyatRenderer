#pragma once
#include "AYPath.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
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
	void set(const std::string& path, const T& value);
	template <typename T>
	void set(const std::string& path, const std::vector<T>& value);
	template <typename K, typename V>
	void set(const std::string& path, const std::map<K, V>& value);

	template <typename T>
	T get(const std::string& path);
	template <typename T>
	T get(const std::string& path, const T& default_value);



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
inline void AYConfigWrapper::set(const std::string& path, const T& value)
{
	_pt.put<T>(path, value);
}

template<typename T>
inline void AYConfigWrapper::set(const std::string& path, const std::vector<T>& value)
{
	boost::property_tree::ptree array_node;
	for (const T& val : value) {
		boost::property_tree::ptree item;
		item.put("", val);  // 空 key 表示数组元素
		array_node.push_back(std::make_pair("", item));
	}
	_pt.put_child(path, array_node);  // 存入 ptree
}

template <typename K, typename V>
inline void AYConfigWrapper::set(const std::string& path, const std::map<K, V>& value)
{
	boost::property_tree::ptree map_node;
	for (const auto& [key, val] : value) {
		// 将 key 转换为字符串（兼容 ptree 的节点命名规则）
		std::string key_str = std::to_string(key);  // 若 K 是数值类型
		if constexpr (std::is_same_v<K, std::string>) {
			key_str = key;  // 若 K 已是字符串
		}
		map_node.push_back(std::make_pair(key, val));
	}
	_pt.put_child(path, map_node);  // 存入 ptree
}

template<typename T>
inline T AYConfigWrapper::get(const std::string& path)
{
	return _pt.get<T>(path);
}

template<typename T>
inline T AYConfigWrapper::get(const std::string& path, const T& default_value)
{
	return _pt.get<T>(path,default_value);
}



template<typename T>
inline std::vector<T> AYConfigWrapper::getVector(const std::string& path)
{
	std::vector<T> result;
	if (_type == ConfigType::INI)
	{
		std::cout << "INI not support vector\n";
	}
	try {
		// 检查路径是否存在
		if (!_pt.get_child_optional(path)) {
			return result;  // 返回空 vector
		}
		// 遍历子节点
		for (const auto& item : _pt.get_child(path)) {
			result.push_back(item.second.get_value<T>());
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error parsing vector: " << e.what() << std::endl;
	}
	return result;
}

template<typename K, typename V>
inline std::map<K, V> AYConfigWrapper::getMap(const std::string& path)
{
	std::map<K, V> result;
	if (_type == ConfigType::INI)
	{
		std::cout << "INI not support map\n";
	}
	for (const auto& item : _pt.get_child(path)) {
		result[item.first] = item.second.get_value<V>();
	}
	return result;
}