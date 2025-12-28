#pragma once
#include "AYPath.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>

namespace ayt::engine::config
{
	enum class ConfigType : INT
	{
		INI,
		JSON,
		XML,
		NONE
	};

	class Configer
	{
	public:
		Configer();

		// Load from file (thread-safe)
		void loadFromFile(const std::string& path, ConfigType type);

		// Save full config (thread-safe)
		bool saveConfig(const std::string& path);
		bool saveConfig();

		// Save only modified keys (thread-safe)
		bool saveIncremental(const std::string& path);
		bool saveIncremental();

		// Remove a key (thread-safe)
		bool remove(const std::string& path);

		// Set value (thread-safe)
		template <typename T>
		void set(const std::string& path, const T& value);

		template <typename T>
		void set(const std::string& path, const std::vector<T>& value);

		template <typename K, typename V>
		void set(const std::string& path, const std::map<K, V>& value);

		// Get value (thread-safe)
		template <typename T>
		T get(const std::string& path);

		template <typename T>
		T get(const std::string& path, const T& default_value);

		template <typename T>
		std::vector<T> getVector(const std::string& path);

		template <typename K, typename V>
		std::map<K, V> getMap(const std::string& path);

		// Utility
		bool hasChanges() const;
		const std::unordered_set<std::string>& getModifiedKeys() const;
		void clearChanges();

		// Debug access (still thread-safe, but caller must not hold reference across threads)
		boost::property_tree::ptree getPt();
		const boost::property_tree::ptree& getOriginalPt() const;

	private:
		// Internal save (does file IO, should NOT be called with lock held if it might cause re-entrancy)
		bool _internalSave(const boost::property_tree::ptree& pt, const std::string& path);

		bool _removeEmptyParents(boost::property_tree::ptree& pt, const std::string& path);

		// Data
		boost::property_tree::ptree _currentPt;
		boost::property_tree::ptree _originalPt;
		boost::property_tree::ptree _pt; // (似乎未使用，可删除)
		std::unordered_set<std::string> _modifiedKeys;
		std::string _path;
		ConfigType _type;

		// Mutex for thread safety
		mutable std::mutex _mutex;
		mutable std::mutex _ptreeMutex;
	};

	template<typename T>
	inline void Configer::set(const std::string& path, const T& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto setValue = [this, &path, &value]() {
			_currentPt.put<T>(path, value);
			_modifiedKeys.insert(path);
			};
		try {
			// 获取当前值(如果存在)
			T currentValue;
			bool hasValue = true;
			try {
				currentValue = _currentPt.get<T>(path);
			}
			catch (...) {
				hasValue = false;
			}

			// 只有当值不存在或值不同时才更新
			if (!hasValue || currentValue != value) {
				setValue();
			}
		}
		catch (...) {
			// 如果比较失败，仍然更新值
			setValue();
		}
	}

	template<typename T>
	inline void Configer::set(const std::string& path, const std::vector<T>& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto setValue = [this, &path, &value]() {
			boost::property_tree::ptree array_node;
			for (const T& val : value) {
				boost::property_tree::ptree item;
				item.put("", val);
				array_node.push_back(std::make_pair("", item));
			}
			_currentPt.put_child(path, array_node);
			_modifiedKeys.insert(path);
			};
		try {
			// 检查当前值是否相同
			auto current = getVector<T>(path);
			if (current != value) {
				setValue();
			}
		}
		catch (...) {
			// 如果比较失败，仍然更新值
			setValue();
		}
	}

	template <typename K, typename V>
	inline void Configer::set(const std::string& path, const std::map<K, V>& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto setValue = [this, &path, &value]() {
			boost::property_tree::ptree map_node;
			for (const auto& [key, val] : value) {
				std::string key_str = std::to_string(key);
				if constexpr (std::is_same_v<K, std::string>) {
					key_str = key;
				}
				map_node.push_back(std::make_pair(key_str, val));
			}
			_currentPt.put_child(path, map_node);
			_modifiedKeys.insert(path);
			};
		try {
			// 检查当前值是否相同
			auto current = getMap<K, V>(path);
			if (current != value) {
				setValue();
			}
		}
		catch (...) {
			// 如果比较失败，仍然更新值
			setValue();
		}
	}

	template<typename T>
	inline T Configer::get(const std::string& path)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _currentPt.get<T>(path);
	}

	template<typename T>
	inline T Configer::get(const std::string& path, const T& default_value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _currentPt.get<T>(path, default_value);
	}


	template<typename T>
	inline std::vector<T> Configer::getVector(const std::string& path)
	{
		std::vector<T> result;
		if (_type == ConfigType::INI)
		{
			std::cout << "INI not support vector\n";
		}
		try {
			std::lock_guard<std::mutex> lock(_mutex);

			// 检查路径是否存在
			if (!_currentPt.get_child_optional(path)) {
				return result;  // 返回空 vector
			}
			// 遍历子节点
			for (const auto& item : _currentPt.get_child(path)) {
				result.push_back(item.second.get_value<T>());
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error parsing vector: " << e.what() << std::endl;
		}
		return result;
	}

	template<typename K, typename V>
	inline std::map<K, V> Configer::getMap(const std::string& path)
	{
		std::map<K, V> result;
		if (_type == ConfigType::INI)
		{
			std::cout << "INI not support map\n";
		}
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (const auto& item : _currentPt.get_child(path)) {
				result[item.first] = item.second.get_value<V>();
			}
		}
		return result;
	}
}
