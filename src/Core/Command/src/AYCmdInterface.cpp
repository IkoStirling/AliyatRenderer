#include "AYCmdInterface.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

void IAYCommandParser::setCmdConfig(const STCommandConfig& config)
{
    _config = config;
}

// AYFirstWordParser 实现
STParsedCommand AYFirstWordParser::parse(const std::string& input) {
    std::string inputLower = input;
    if (_config.parsing.ignoreCase) {
        std::transform(inputLower.begin(), inputLower.end(), inputLower.begin(), ::tolower);
    }
    STParsedCommand result;
    result.rawCommand = inputLower;

    std::istringstream iss(inputLower);
    iss >> result.commandName;

    std::string arg;
    while (iss >> arg) {
        result.args.push_back(arg);
    }

    result.normalized = result.commandName;
    for (const auto& a : result.args) {
        result.normalized += " " + a;
    }

    return result;
}

void AYFirstWordParser::setParseStrategy(const std::string& strategy) {
    // 此策略无需额外配置
}

void AYFirstWordParser::addCommandTemplate(const std::string& name,
    const std::string& templateStr) {
    // 此策略不使用模板
}

// AYTemplateParser 实现
STParsedCommand AYTemplateParser::parse(const std::string& input) {
    STParsedCommand result;
    result.rawCommand = input;

    for (const auto& [name, templateStr] : _templates) {
        if (_config.parsing.ignoreCase) {
            std::string inputLower = input;
            std::transform(inputLower.begin(), inputLower.end(), inputLower.begin(), ::tolower);
            std::string nameLower = name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (inputLower.find(nameLower) == 0) {
                result.commandName = name; // 保留原始大小写
                result.normalized = applyTemplate(templateStr, input);
                break;
            }
        }
        else {
            if (input.find(name) == 0) {
                result.commandName = name;
                result.normalized = applyTemplate(templateStr, input);
                break;
            }
        }
    }

    if (result.commandName.empty()) {
        AYFirstWordParser fallback;
        return fallback.parse(input);
    }

    return result;
}

void AYTemplateParser::setParseStrategy(const std::string& strategy) {
    // 此策略无需额外配置
}

void AYTemplateParser::addCommandTemplate(const std::string& name,
    const std::string& templateStr) {
    _templates[name] = templateStr;
}

std::string AYTemplateParser::applyTemplate(const std::string& templateStr,
    const std::string& input) {
    std::string result = templateStr;
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    size_t pos = 0;
    int argIndex = 0;
    while ((pos = result.find("${" + std::to_string(argIndex) + "}")) != std::string::npos) {
        std::string arg;
        if (iss >> arg) {
            result.replace(pos, 3 + std::to_string(argIndex).length(), arg);
            argIndex++;
        }
        else {
            break;
        }
    }
    return result;
}

// AYCommandParserFactory 实现
std::unique_ptr<IAYCommandParser> AYCommandParserFactory::create(ParserType type) {
    switch (type) {
    case ParserType::FirstWord:
        return std::make_unique<AYFirstWordParser>();
    case ParserType::Template:
        return std::make_unique<AYTemplateParser>();
    default:
        return nullptr;
    }
}

// AYMultiLineProcessor 实现
AYMultiLineProcessor::AYMultiLineProcessor(const STCommandConfig& config)
    : _config(config) {
}

bool AYMultiLineProcessor::addLine(const std::string& line) {
    std::string normalized = normalizeLineEnding(line);

    //if (!_buffer.empty()) _buffer += " ";
    //_buffer += normalized;
    if (!_buffer.empty()) {
        _buffer += "\n" + normalized;
    }
    else {
        _buffer = normalized;
    }

    return !hasContinuation(normalized);
}

std::string AYMultiLineProcessor::getCurrentCommand() const {
    std::string result;
    std::istringstream iss(_buffer);
    std::string line;

    while (std::getline(iss, line)) {
        // 移除每行末尾的续行符
        for (const auto& c : _config.multiLine.continuationChars) {
            if (!line.empty() && line.back() == c[0]) {
                line.pop_back();
                // 移除续行符后的空白
                while (!line.empty() && isspace(line.back())) {
                    line.pop_back();
                }
                break;
            }
        }

        if (!result.empty()) {
            result += " "; // 用空格连接多行
        }
        result += line;
    }

    return result;
}

void AYMultiLineProcessor::reset() {
    _buffer.clear();
}

bool AYMultiLineProcessor::hasContinuation(const std::string& line) const {
    if (line.empty()) return false;

    size_t pos = line.find_last_not_of(" \t\r\n");
    if (pos == std::string::npos) return false;

    char lastChar = line[pos];
    for (const auto& c : _config.multiLine.continuationChars) {
        if (c.size() == 1 && lastChar == c[0]) {
            return true;
        }
    }
    return false;
}

std::string AYMultiLineProcessor::normalizeLineEnding(const std::string& line) const {
    std::string result;
    result.reserve(line.size());

    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\r' && i + 1 < line.size() && line[i + 1] == '\n') {
            result += '\n';
            ++i;
        }
        else if (line[i] == '\r') {
            result += '\n';
        }
        else {
            result += line[i];
        }
    }

    return result;
}

// AYCommandInterface 实现
void AYCommandInterface::run(IAYCommandParser* parser) {
    _parser.reset(parser);

    std::cout << _welcomeMsg << std::endl;
    std::cout << _hintMsg << std::endl;

    AYMultiLineProcessor mlp(_config);

    bool isContinuation = false;
    while (!_shouldClose) {
        std::cout << (isContinuation ? _config.multiLine.continuationPrompt : _prompt);
        std::string line = readLine();

        if (line == "exit" || line == "quit") {
            shutdown();
            continue;
        }

        if (!mlp.addLine(line)) {
            isContinuation = true;
            continue;
        }

        isContinuation = false;
        std::string fullCommand = mlp.getCurrentCommand();
        auto parsed = _parser->parse(fullCommand);

        if (_execution) {
            auto result = _execution(parsed);
            printCommandResult(result);
        }

        mlp.reset();
    }
}

void AYCommandInterface::shutdown() {
    _shouldClose = true;
}

std::unique_ptr<IAYCommandParser> AYCommandInterface::loadConfig(const std::string& path)
{
    using json = nlohmann::json;
    try {
        std::ifstream ifs(path);
        json config = json::parse(ifs);

        // 设置接口属性
        if (config.contains("interface")) {
            auto& interfaceConfig = config["interface"];
            if (interfaceConfig.contains("welcomeMsg")) {
                setWelcomeMsg(interfaceConfig["welcomeMsg"]);
            }
            if (interfaceConfig.contains("prompt")) {
                setPrompt(interfaceConfig["prompt"]);
            }
            if (interfaceConfig.contains("hintMsg")) {
                setHintMsg(interfaceConfig["hintMsg"]);
            }
        }

        // 创建解析器
        STCommandConfig cmdConfig;

        // 加载多行配置
        if (config.contains("multiLine")) {
            auto& multiLine = config["multiLine"];
            if (multiLine.contains("continuationChars")) {
                cmdConfig.multiLine.continuationChars =
                    multiLine["continuationChars"].get<std::vector<std::string>>();
            }
            if (multiLine.contains("continuationPrompt")) {
                cmdConfig.multiLine.continuationPrompt =
                    multiLine["continuationPrompt"];
            }
        }

        // 加载解析配置
        if (config.contains("parsing")) {
            auto& parsing = config["parsing"];
            if (parsing.contains("ignoreCase")) {
                cmdConfig.parsing.ignoreCase = parsing["ignoreCase"];
            }
            if (parsing.contains("defaultStrategy")) {
                cmdConfig.parsing.defaultStrategy = parsing["defaultStrategy"];
            }
            if (parsing.contains("templates")) {
                cmdConfig.parsing.templates =
                    parsing["templates"].get<std::unordered_map<std::string, std::string>>();
            }
        }

        // 创建解析器
        std::unique_ptr<IAYCommandParser> parser;
        if (cmdConfig.parsing.defaultStrategy == "template") {
            auto templateParser = std::make_unique<AYTemplateParser>();
            for (const auto& [name, templateStr] : cmdConfig.parsing.templates) {
                templateParser->addCommandTemplate(name, templateStr);
            }
            parser = std::move(templateParser);
        }
        else {
            parser = std::make_unique<AYFirstWordParser>();
        }
        parser->setCmdConfig(cmdConfig);
        _config = cmdConfig;

        return parser;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return nullptr;
    }
}

void AYCommandInterface::setPrompt(const std::string& prompt) {
    _prompt = prompt;
}

void AYCommandInterface::setWelcomeMsg(const std::string& msg) {
    _welcomeMsg = msg;
}

void AYCommandInterface::setHintMsg(const std::string& msg)
{
    _hintMsg = msg;
}

void AYCommandInterface::setExecution(CommandExecution execution) {
    _execution = execution;
}

void AYCommandInterface::printCommandResult(const STCommandResult& result) {
    if (result.success) {
        if (!result.output.empty()) {
            std::cout << result.output << std::endl;
        }
    }
    else {
        std::cerr << "Error: " << result.error << std::endl;
    }
}

std::string AYCommandInterface::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}