#include "AYConfigWrapper.h"
#include <filesystem>
#include <iostream>

void AYConfigWrapper::loadFromFile(const std::string& path, ConfigType type)
{
	std::string rpath = AYPath::resolve(path);
	_pt.clear();
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
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::write_ini(rpath, _currentPt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(rpath, _currentPt);
			break;
		case AYConfigWrapper::ConfigType::XML:
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
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::read_ini(rpath, _currentPt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::read_json(rpath, _currentPt);
			break;
		case AYConfigWrapper::ConfigType::XML:
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

bool AYConfigWrapper::saveConfig(const std::string& path)
{
	std::string rpath = AYPath::resolve(path);
	return _internalSave(_currentPt, rpath);
}

bool AYConfigWrapper::saveConfig()
{
	if (_path.empty()) return false;
	return saveConfig(_path);
}

bool AYConfigWrapper::remove(const std::string& path)
{
	try {
		// 分割路径
		std::vector<std::string> parts;
		boost::split(parts, path, boost::is_any_of("."));

		if (parts.empty()) {
			std::cerr << "[AYConfigWrapper] Empty path provided for removal\n";
			return false;
		}

		// 存储访问路径用于后续清理
		std::vector<std::pair<std::string, boost::property_tree::ptree*>> nodePath;
		nodePath.reserve(parts.size());

		// 查找目标节点并记录路径
		auto* current = &_currentPt;
		nodePath.emplace_back("", current); // 根节点

		for (size_t i = 0; i < parts.size() - 1; ++i) {
			if (!current->get_child_optional(parts[i])) {
				std::cerr << "[AYConfigWrapper] Parent node not found: " << parts[i] << "\n";
				return false;
			}
			current = &current->get_child(parts[i]);
			nodePath.emplace_back(parts[i], current);
		}

		// 删除目标节点
		const std::string& lastPart = parts.back();
		size_t erased = current->erase(lastPart);
		if (erased == 0) {
			std::cerr << "[AYConfigWrapper] Key not found: " << lastPart << "\n";
			return false;
		}

		_modifiedKeys.insert(path);

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
	catch (const std::exception& e) {
		std::cerr << "[AYConfigWrapper] Error removing config key: " << e.what() << "\n";
		return false;
	}
}

bool AYConfigWrapper::saveIncremental(const std::string& path)
{
	if (!hasChanges()) return true;

	try {
		for (const auto& key : _modifiedKeys) {
			try {
				// 检查键是否存在于当前ptree中
				if (_currentPt.get_child_optional(key)) {
					// 键存在 - 更新值
					_originalPt.put_child(key, _currentPt.get_child(key));
				}
				else {
					// 键不存在 - 删除(对于删除操作)
					std::vector<std::string> parts;
					boost::split(parts, key, boost::is_any_of("."));

					if (!parts.empty()) {
						auto* parent = &_originalPt;
						for (size_t i = 0; i < parts.size() - 1; ++i) {
							parent = &parent->get_child(parts[i]);
						}
						parent->erase(parts.back());
					}
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error applying change for key " << key << ": " << e.what() << std::endl;
				continue;
			}
		}

		bool success = _internalSave(_originalPt, path);
		return success;
	}
	catch (...) {
		return false;
	}
}

bool AYConfigWrapper::saveIncremental()
{
	if (_path.empty()) {
		std::cerr << "No file path specified for saving" << std::endl;
		return false;
	}
	return saveIncremental(_path);
}


bool AYConfigWrapper::_internalSave(const boost::property_tree::ptree& pt, const std::string& rpath)
{
	try {
		switch (_type)
		{
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::write_ini(rpath, pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(rpath, pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
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