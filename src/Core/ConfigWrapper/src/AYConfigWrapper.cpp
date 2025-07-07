#include "AYConfigWrapper.h"
#include <filesystem>
#include <iostream>

void AYConfigWrapper::loadFromFile(const std::string& path, ConfigType type)
{
	_pt.clear();
	_type = type;
    std::filesystem::path p(path);
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

	if (!std::filesystem::exists(path))
	{
		switch (type)
		{
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::write_ini(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::write_xml(path, _pt);
			break;
		default:
			if (path.ends_with(".ini")) {
				std::ofstream(path) << "[default]\n";
			}
			else if (path.ends_with(".json")) {
				std::ofstream(path) << "{}\n";
			}
			else if (path.ends_with(".xml")) {
				std::ofstream(path) << "<root/>\n";
			}
			break;
		}
	}

	try {
		switch (type)
		{
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::read_ini(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::read_json(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::read_xml(path, _pt);
			break;
		default:
			break;
		}
	}
	catch (boost::property_tree::ptree_error& e)
	{
		std::cerr << "Read config file error: " << e.what() << std::endl;
	}
}

void AYConfigWrapper::saveConfig(const std::string& path)
{
	try {
		switch (_type)
		{
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::write_ini(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(path, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::write_xml(path, _pt);
			break;
		default:
			break;
		}
	}
	catch (boost::property_tree::ptree_error& e)
	{
		std::cerr << "Write config file error: " << e.what() << std::endl;
	}
}