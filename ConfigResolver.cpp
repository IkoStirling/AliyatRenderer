#include <httplib.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <regex>
#include "AYPath.h"
#include "AYConfigWrapper.h"

namespace fs = std::filesystem;

class TemplateEngine {
public:
    static std::string Render(const std::string& templateName,
        const std::map<std::string, std::string>& params)
    {
        std::string templatePath = AYPath::resolve(templateName);
        std::ifstream file(templatePath, std::ios::binary);
        if (!file.is_open()) {
            return "Template not found: " + templateName;
        }

        // 读取文件内容到字符串
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        // 去除UTF-8 BOM (EF BB BF)
        if (content.size() >= 3 &&
            static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            content.erase(0, 3);
        }


        for (const auto& [key, value] : params) {
            replaceAll(content, "{{ " + key + " }}", value);
        }

        return content;
    }

private:
    static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
};



struct ConfigFile {
    std::string path;
    std::string displayName;
    AYConfigWrapper::ConfigType type;
};

ConfigFile currentEditingFile;
AYConfigWrapper currentConfig;
AYConfigWrapper::ConfigType currentConfigType;

std::vector<ConfigFile> scanConfigDirectory(const std::string& dirPath) {
    std::vector<ConfigFile> files;

    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            AYConfigWrapper::ConfigType type = AYConfigWrapper::ConfigType::NONE;

            if (ext == ".ini") type = AYConfigWrapper::ConfigType::INI;
            else if (ext == ".json") type = AYConfigWrapper::ConfigType::JSON;
            else if (ext == ".xml") type = AYConfigWrapper::ConfigType::XML;
            else continue;

            ConfigFile file;
            file.path = entry.path().generic_string();
            file.type = type;
            std::string relativePath = fs::relative(entry.path(), dirPath).generic_string();
            file.displayName = relativePath;

            files.push_back(file);
        }
    }
    return files;
}

std::string formatDouble(double value, int precision = 6) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;

    // 去除末尾多余的0和小数点
    std::string result = oss.str();
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) {
        // 从末尾开始删除多余的0
        result.erase(result.find_last_not_of('0') + 1);
        // 如果小数点后没有数字，删除小数点
        if (result.back() == '.') {
            result.pop_back();
        }
    }
    return result;
}


std::string generateFileList(const std::vector<ConfigFile>& files) {
    std::string items;
    for (const auto& file : files) {
        std::map<std::string, std::string> params = {
            {"encoded_path", httplib::detail::encode_url(file.path)},
            {"display_name", file.displayName},
            {"active_class", (file.path == currentEditingFile.path) ? "active-file" : ""}
        };
        items += TemplateEngine::Render("@web/ConfigResolver/templates/file-item.html", params);
    }

    std::map<std::string, std::string> params = { {"files", items} };
    return TemplateEngine::Render("@web/ConfigResolver/templates/file-list.html", params);
}

std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (char c : input) {
        switch (c) {
        case '&':  output += "&amp;";  break;
        case '\"': output += "&quot;"; break;
        case '\'': output += "&apos;"; break;
        case '<':  output += "&lt;";   break;
        case '>':  output += "&gt;";   break;
        default:   output += c;        break;
        }
    }
    return output;
}

void generateConfigItems(const boost::property_tree::ptree& pt, std::ostringstream& oss, const std::string& path = "") {
    if (pt.empty()) {
        std::string value;
        try {
            value = pt.data();
        }
        catch (const std::exception& e) {
            value = "[Error reading value]";
        }

        // 生成键单元格
        oss << "<tr>"
            << TemplateEngine::Render("@web/ConfigResolver/templates/delete-item.html",
                {
                    {"file_path", httplib::detail::encode_url(currentEditingFile.path)},
                    {"key", path}
                });

        // 生成值单元格
        oss << "<td class='value-cell'>";
        // 根据值类型生成不同的输入控件
        if (value == "true" || value == "false") {
            oss << "<select name='" << path << "'>"
                << "<option value='true'" << (value == "true" ? " selected" : "") << ">true</option>"
                << "<option value='false'" << (value == "false" ? " selected" : "") << ">false</option>"
                << "</select>";
        }
        else if (!value.empty() && std::regex_match(value, std::regex("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$"))) {
            float numValue = std::stof(value);
            std::string formatted = formatDouble(numValue, 5);
            oss << "<input type='number' step='any' name='" << path
                << "' value='" << escapeHtml(formatted) << "'>";
        }
        else {
            oss << "<input type='text' name='" << path
                << "' value='" << value << "'>";
        }

        oss << "</td>"
            << "</tr>";

        return;
    }

    for (const auto& child : pt) {
        std::string childPath = path.empty() ? child.first : path + "." + child.first;
        generateConfigItems(child.second, oss, childPath);
    }
}

std::string generateConfigEditor() {
    if (currentEditingFile.path.empty()) {
        return "<p>Please select a configuration file to edit.</p>";
    }

    // 生成配置项HTML
    std::ostringstream configItemsStream;
    generateConfigItems(currentConfig.getPt(), configItemsStream);
    std::string configItems = configItemsStream.str();

    // 准备模板参数
    std::map<std::string, std::string> params = {
        {"display_name", currentEditingFile.displayName},
        {"file_path", currentEditingFile.path},
        {"config_items", configItems}
    };

    return TemplateEngine::Render("@web/ConfigResolver/templates/config_editor.html", params);
}

// 获取配置文件类型
AYConfigWrapper::ConfigType getConfigType(const std::string& filename) {
    std::string ext = fs::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".json") return AYConfigWrapper::ConfigType::JSON;
    if (ext == ".xml") return AYConfigWrapper::ConfigType::XML;
    if (ext == ".ini") return AYConfigWrapper::ConfigType::INI;
    return AYConfigWrapper::ConfigType::INI; // 默认
}

void updateConfigFromPost(const httplib::Request& req) {
    for (const auto& field : req.params) {
        const std::string& key = field.first;
        const std::string& value = field.second;

        if (key == "file") continue;
        if (field.first == "key" || field.first.find("delete") != std::string::npos) {
            continue;
        }

        try {
            std::string trimmedValue = value;
            boost::algorithm::trim(trimmedValue);

            // 获取当前值
            std::string currentValue;
            try {
                currentValue = currentConfig.get<std::string>(key);
                boost::algorithm::trim(currentValue);
            }
            catch (...) {
                // 键不存在，需要设置
                currentValue = "";
            }


            if (currentValue != trimmedValue) {
                if (trimmedValue == "true" || trimmedValue == "false") {
                    currentConfig.set(key, trimmedValue == "true");
                }
                else {
                    // 尝试解析为数字
                    try {
                        if (trimmedValue.find('.') != std::string::npos ||
                            trimmedValue.find('e') != std::string::npos ||
                            trimmedValue.find('E') != std::string::npos) {
                            currentConfig.set(key, std::stod(trimmedValue));
                        }
                        else {
                            currentConfig.set(key, std::stoi(trimmedValue));
                        }
                    }
                    catch (...) {
                        // 默认为字符串
                        currentConfig.set(key, trimmedValue);
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error setting value for key " << key << ": " << e.what() << std::endl;
        }
    }
}


int main() {
    std::vector<ConfigFile> configFiles;
    const std::string configDir = AYPath::resolve("@config");
    configFiles = std::move(scanConfigDirectory(configDir));

    std::filesystem::file_time_type lastConfigUpdateTime;
    lastConfigUpdateTime = std::filesystem::last_write_time(configDir);

    httplib::Server svr;

    std::string staticPath = AYPath::resolve("@web/ConfigResolver/static");
    svr.set_mount_point("/static", staticPath.c_str());

    //svr.Get("/static/.*", [](const httplib::Request& req, httplib::Response& res) {
    //    res.set_header("Cache-Control", "public, max-age=86400"); // 1天缓存
    //    });

    // 添加配置项
    svr.Post("/add", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("file") && req.has_param("new_key") && req.has_param("new_value") && req.has_param("new_type")) {
            std::string filePath = req.get_param_value("file");
            std::string newKey = req.get_param_value("new_key");
            std::string newValue = req.get_param_value("new_value");
            std::string newType = req.get_param_value("new_type");

            if (filePath == currentEditingFile.path) {
                try {
                    // 根据类型设置值
                    if (newType == "boolean") {
                        currentConfig.set(newKey, newValue == "true");
                    }
                    else if (newType == "number") {
                        if (newValue.find('.') != std::string::npos) {
                            currentConfig.set(newKey, std::stof(newValue));
                        }
                        else {
                            currentConfig.set(newKey, std::stoi(newValue));
                        }
                    }
                    else {
                        currentConfig.set(newKey, newValue);
                    }

                    // 保存文件
                    if (!currentConfig.saveIncremental()) {
                        res.status = 500;
                        res.set_content("Failed to save changes", "text/plain");
                        return;
                    }
                }
                catch (const std::exception& e) {
                    res.status = 500;
                    res.set_content("Error adding new configuration", "text/plain");
                    return;
                }
            }
        }
        res.set_redirect("/");
        });

    // 删除配置项
    svr.Get("/delete", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("file") || !req.has_param("key")) {
            res.status = 400;
            res.set_content("Missing parameters", "text/plain");
            return;
        }

        std::string filePath = req.get_param_value("file");
        std::string key = req.get_param_value("key");

        if (filePath != currentEditingFile.path) {
            res.set_redirect("/");
            return;
        }

        if (!currentConfig.remove(key)) {
            std::cout << "[main delete] remove key error\n";
            res.status = 500;
            res.set_content("Failed to remove key", "text/plain");
            return;
        }

        // 使用增量保存
        if (!currentConfig.saveIncremental()) {
            std::cout << "[main delete] saveIncremental error\n";
            res.status = 500;
            res.set_content("Failed to save changes", "text/plain");
            return;
        }

        res.set_redirect("/");
        });

    // 主页 - 显示文件列表
    svr.Get("/", [&configFiles](const httplib::Request&, httplib::Response& res) {
        // 生成文件列表HTML
        std::string fileListHtml = generateFileList(configFiles);

        // 生成编辑器HTML（如果有选中的文件）
        std::string editorHtml = currentEditingFile.path.empty()
            ? "<div class='no-selection'>Please select a configuration file</div>"
            : generateConfigEditor();

        // 准备模板参数
        std::map<std::string, std::string> params = {
            {"content", fileListHtml + editorHtml}
        };

        // 渲染完整页面
        std::string fullPage = TemplateEngine::Render("@web/ConfigResolver/templates/base.html", params);
        res.set_content(fullPage, "text/html");
        });

    // 编辑指定文件
    svr.Get("/edit", [&configFiles](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("file")) {
            std::string findFile = req.get_param_value("file");

            if (currentEditingFile.path != findFile) {
                for (const auto& file : configFiles)
                {
                    if (findFile == file.path)
                    {
                        currentEditingFile = file;
                        break;
                    }
                }
                currentConfigType = getConfigType(currentEditingFile.path);
                currentConfig.loadFromFile(currentEditingFile.path, currentConfigType);
            }

            if (req.has_header("X-Requested-With")) {
                // AJAX请求返回JSON
                res.set_header("Content-Type", "application/json");
                res.set_content("{\"status\":\"success\"}", "application/json");
            }
            else {
                // 普通请求重定向
                res.set_redirect("/");
            }
        }
        else {
            res.status = 400;
            res.set_content("Missing file parameter", "text/plain");
        }
        });

    // 保存修改
    svr.Post("/save", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("file")) {
            std::string fileToSave = req.get_param_value("file");

            if (fileToSave == currentEditingFile.path) {
                updateConfigFromPost(req);

                try {
                    if (!currentConfig.saveIncremental()) {
                        std::cout << "[main save] saveIncremental error\n";
                        res.status = 500;
                        res.set_content("Failed to save changes", "text/plain");
                    }
                    //currentConfig.saveConfig(currentEditingFile.path);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error saving config file: " << e.what() << std::endl;
                    res.set_content("Error saving config file", "text/plain");
                    return;
                }
            }
        }

        res.set_redirect("/");
    });

    svr.Get("/get-editor", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(generateConfigEditor(), "text/html");
        });


    std::cout << "Server started at http://localhost:8081\n";
    svr.listen("localhost", 8081);

    return 0;
}