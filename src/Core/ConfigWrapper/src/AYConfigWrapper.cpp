#include "AYConfigWrapper.h"
#include <filesystem>
#include <iostream>

void AYConfigWrapper::loadFromFile(const std::string& path, ConfigType type)
{
	std::string rpath = AYPath::resolve(path);
	_pt.clear();
	_type = type;
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
			boost::property_tree::write_ini(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::write_xml(rpath, _pt);
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
			boost::property_tree::read_ini(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::read_json(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::read_xml(rpath, _pt);
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
	std::string rpath = AYPath::resolve(path);
	try {
		switch (_type)
		{
		case AYConfigWrapper::ConfigType::INI:
			boost::property_tree::write_ini(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::JSON:
			boost::property_tree::write_json(rpath, _pt);
			break;
		case AYConfigWrapper::ConfigType::XML:
			boost::property_tree::write_xml(rpath, _pt);
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