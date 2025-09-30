#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

/*
    
    Load config file
    Create CommandParser
    Set CommandInterface execution callback
    Run CommandInterface
    User input -> MultiLineProcesser -> CommandParser -> execute command
*/

struct STParsedCommand {
    std::string rawCommand;      // 原始命令
    std::string normalized;      // 标准化后的命令
    std::string commandName;     // 提取的命令名称
    std::vector<std::string> args; // 参数列表
    bool isComplete = true;      // 是否完整命令(多行时可能为false)
};

struct STCommandConfig {
    struct {
        std::vector<std::string> continuationChars = { "|", "\\" };
        std::string lineEnding = "auto";
        std::string continuationPrompt = "> ";
    } multiLine;

    struct {
        std::string defaultStrategy = "first_word";
        std::unordered_map<std::string, std::string> templates;
        bool ignoreCase = false;
    } parsing;
};

struct STCommandResult {
    bool success;
    std::string output;
    std::string error;
};

class IAYCommandParser {
public:
    virtual ~IAYCommandParser() = default;

    // 解析单条或多行命令
    virtual STParsedCommand parse(const std::string& input) = 0;

    // 配置解析策略
    virtual void setParseStrategy(const std::string& strategy) = 0;

    // 添加自定义命令模板
    virtual void addCommandTemplate(const std::string& name,
        const std::string& templateStr) = 0;

    void setCmdConfig(const STCommandConfig& config);
protected:
    STCommandConfig _config;
};

class AYFirstWordParser : public IAYCommandParser {
public:
    STParsedCommand parse(const std::string& input) override;

    void setParseStrategy(const std::string& strategy) override;

    void addCommandTemplate(const std::string& name,
        const std::string& templateStr) override;
};

class AYTemplateParser : public IAYCommandParser {
public:
    STParsedCommand parse(const std::string& input) override;

    void setParseStrategy(const std::string& strategy) override;

    void addCommandTemplate(const std::string& name,
        const std::string& templateStr) override;

private:
    std::unordered_map<std::string, std::string> _templates;

    std::string applyTemplate(const std::string& templateStr,
        const std::string& input);
};


class AYCommandParserFactory {
public:
    enum class ParserType { FirstWord, Template };

    static std::unique_ptr<IAYCommandParser> create(ParserType type);
};

class AYMultiLineProcessor {
public:
    AYMultiLineProcessor(const STCommandConfig& config);

    // 添加新行并返回是否构成完整命令
    bool addLine(const std::string& line);

    // 获取当前累积的命令
    std::string getCurrentCommand() const;

    // 重置状态
    void reset();

private:
    std::string _buffer;
    STCommandConfig _config;

    // 检查行尾是否有续行符，由配置文件决定，如无则默认'|' or '\'
    bool hasContinuation(const std::string& line) const;

    // 标准化行尾（处理平台差异）
    std::string normalizeLineEnding(const std::string& line) const;
};

class AYCommandInterface {
public:
    // 一个命令行用户交互界面，基于配置文件定义结束循环语句，如exit或quit
    void run(IAYCommandParser* parser);

    // 结束程序循环
    void shutdown();

    // 加载配置文件
    std::unique_ptr<IAYCommandParser> loadConfig(const std::string& path);

    // 用于定义用户输入前面的符号，如 local> some commands
    void setPrompt(const std::string& prompt);

    // 欢迎信息
    void setWelcomeMsg(const std::string& msg);

    // 提示信息
    void setHintMsg(const std::string& msg);

    // 设置执行回调
    using CommandExecution = std::function<STCommandResult(STParsedCommand&)>;
    void setExecution(CommandExecution execution);
private:
    // 执行回调后打印结果
    void printCommandResult(const STCommandResult& result);

    // 跨平台字符串读取
    std::string readLine();

    STCommandConfig _config;
    bool _shouldClose = false;
    std::unique_ptr<IAYCommandParser> _parser;
    CommandExecution _execution = nullptr;

    std::string _prompt = "> ";
    std::string _welcomeMsg = "Welcome to Command Interface";
    std::string _hintMsg = "Type 'exit' or 'quit' to end session";
};