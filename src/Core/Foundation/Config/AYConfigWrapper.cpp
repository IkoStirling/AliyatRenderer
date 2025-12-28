#include "AYConfigWrapper.h"
#include <filesystem>
#include <iostream>

namespace ayt::engine::config
{
	using ayt::engine::path::Path;

	Configer::Configer()
		: _type(ConfigType::NONE)
	{
	}

	void Configer::loadFromFile(const std::string& path, ConfigType type)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		std::string rpath = Path::resolve(path);
		_type = type;
		_path = rpath;

		std::filesystem::path p(rpath);
		if (!std::filesystem::exists(p.parent_path()))
		{
			try
			{
				std::filesystem::create_directories(p.parent_path());
			}
			catch (const std::exception& e)
			{
				std::cerr << "Config does not exist and create directories failed: " << e.what() << std::endl;
			}
		}

		if (!std::filesystem::exists(rpath))
		{
			switch (type)
			{
			case ConfigType::INI:
				boost::property_tree::write_ini(rpath, _currentPt);
				break;
			case ConfigType::JSON:
				boost::property_tree::write_json(rpath, _currentPt);
				break;
			case ConfigType::XML:
				boost::property_tree::write_xml(rpath, _currentPt);
				break;
			default:
				if (rpath.ends_with(".ini")) {
					std::ofstream(rpath) << "[default]\n";
				}
				else if (rpath.ends_with(".json")) {
					std::ofstream(rpath) << "{}\n";
				}
				else if (rpath.ends_with(".xml")) {
					std::ofstream(rpath) << "<root/>\n";
				}
				break;
			}
		}

		try {
			switch (type)
			{
			case ConfigType::INI:
				boost::property_tree::read_ini(rpath, _currentPt);
				break;
			case ConfigType::JSON:
				boost::property_tree::read_json(rpath, _currentPt);
				break;
			case ConfigType::XML:
				boost::property_tree::read_xml(rpath, _currentPt);
				break;
			default:
				break;
			}
			_modifiedKeys.clear();
			_originalPt = _currentPt;
		}
		catch (boost::property_tree::ptree_error& e)
		{
			std::cerr << "Read config file error: " << e.what() << std::endl;
		}
	}

	bool Configer::saveConfig(const std::string& path)
	{
		std::string rpath = Path::resolve(path);

		std::lock_guard<std::mutex> lock(_mutex);
		return _internalSave(_currentPt, rpath);
	}

	bool Configer::saveConfig()
	{
		if (_path.empty()) return false;
		return _internalSave(_currentPt, _path);
	}

	bool Configer::remove(const std::string& path)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		try {
			bool success = _removeEmptyParents(_currentPt, path);
			if (success)
				_modifiedKeys.insert(path);
			return success;
		}
		catch (const std::exception& e) {
			std::cerr << "[Configer] Error removing config key: " << e.what() << "\n";
			return false;
		}
	}

	bool Configer::saveIncremental(const std::string& path)
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (!hasChanges()) return true;

			try {
				for (const auto& key : _modifiedKeys) {
					try {
						if (_currentPt.get_child_optional(key)) {
							_originalPt.put_child(key, _currentPt.get_child(key));
						}
						else {
							_removeEmptyParents(_originalPt, key);
						}
					}
					catch (...) {
						continue;
					}
				}
			}
			catch (...) {
				return false;
			}
		}

		bool success = false;
		try {
			success = _internalSave(_originalPt, path);
		}
		catch (const std::exception& e) {
			std::cerr << "Error in _internalSave: " << e.what() << std::endl;
			success = false;
		}
		return success;
	}

	bool Configer::saveIncremental()
	{
		if (_path.empty()) {
			std::cerr << "No file path specified for saving" << std::endl;
			return false;
		}
		return saveIncremental(_path);
	}

	bool Configer::hasChanges() const
	{
		return !_modifiedKeys.empty();
	}

	const std::unordered_set<std::string>& Configer::getModifiedKeys() const
	{
		//std::lock_guard<std::mutex> lock(_mutex);
		return _modifiedKeys;
	}

	void Configer::clearChanges()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_modifiedKeys.clear();
		_originalPt = _currentPt;
	}

	boost::property_tree::ptree Configer::getPt()
	{
		std::lock_guard<std::mutex> lock(_ptreeMutex);
		return _currentPt;
	}

	const boost::property_tree::ptree& Configer::getOriginalPt() const
	{
		std::lock_guard<std::mutex> lock(_ptreeMutex);
		return _originalPt;
	}

	bool Configer::_internalSave(const boost::property_tree::ptree& pt, const std::string& rpath)
	{
		try {
			switch (_type)
			{
			case ConfigType::INI:
				boost::property_tree::write_ini(rpath, pt);
				break;
			case ConfigType::JSON:
				boost::property_tree::write_json(rpath, pt);
				break;
			case ConfigType::XML:
				boost::property_tree::write_xml(rpath, pt);
				break;
			default:
				return false;
			}
			_originalPt = _currentPt;  // 更新原始状态
			_modifiedKeys.clear();     // 清空修改记录
			return true;
		}
		catch (boost::property_tree::ptree_error& e)
		{
			std::cerr << "Write config file error: " << e.what() << std::endl;
			return false;
		}
	}

	bool Configer::_removeEmptyParents(boost::property_tree::ptree& pt, const std::string& path)
	{
		// 分割路径
		std::vector<std::string> parts;
		boost::split(parts, path, boost::is_any_of("."));

		if (parts.empty()) {
			std::cerr << "[Configer] Empty path provided for removal\n";
			return false;
		}

		// 存储访问路径用于后续清理
		std::vector<std::pair<std::string, boost::property_tree::ptree*>> nodePath;
		nodePath.reserve(parts.size());

		// 查找目标节点并记录路径
		auto* current = &pt;
		nodePath.emplace_back("", current); // 根节点

		for (size_t i = 0; i < parts.size() - 1; ++i) {
			if (!current->get_child_optional(parts[i])) {
				std::cerr << "[Configer] Parent node not found: " << parts[i] << "\n";
				return false;
			}
			current = &current->get_child(parts[i]);
			nodePath.emplace_back(parts[i], current);
		}

		// 删除目标节点
		const std::string& lastPart = parts.back();
		size_t erased = current->erase(lastPart);
		if (erased == 0) {
			std::cerr << "[Configer] Key not found: " << lastPart << "\n";
			return false;
		}


		// 自底向上清理空父节点
		for (int i = static_cast<int>(nodePath.size()) - 1; i > 0; --i) {
			auto& [name, node] = nodePath[i];
			if (node->empty()) {
				auto& parent = *nodePath[i - 1].second;
				parent.erase(name);
			}
			else {
				break; // 遇到非空节点停止清理
			}
		}
		return true;
	}
}
