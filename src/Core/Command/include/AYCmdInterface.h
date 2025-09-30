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
    std::string rawCommand;      // ԭʼ����
    std::string normalized;      // ��׼���������
    std::string commandName;     // ��ȡ����������
    std::vector<std::string> args; // �����б�
    bool isComplete = true;      // �Ƿ���������(����ʱ����Ϊfalse)
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

    // �����������������
    virtual STParsedCommand parse(const std::string& input) = 0;

    // ���ý�������
    virtual void setParseStrategy(const std::string& strategy) = 0;

    // ����Զ�������ģ��
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

    // ������в������Ƿ񹹳���������
    bool addLine(const std::string& line);

    // ��ȡ��ǰ�ۻ�������
    std::string getCurrentCommand() const;

    // ����״̬
    void reset();

private:
    std::string _buffer;
    STCommandConfig _config;

    // �����β�Ƿ������з����������ļ�������������Ĭ��'|' or '\'
    bool hasContinuation(const std::string& line) const;

    // ��׼����β������ƽ̨���죩
    std::string normalizeLineEnding(const std::string& line) const;
};

class AYCommandInterface {
public:
    // һ���������û��������棬���������ļ��������ѭ����䣬��exit��quit
    void run(IAYCommandParser* parser);

    // ��������ѭ��
    void shutdown();

    // ���������ļ�
    std::unique_ptr<IAYCommandParser> loadConfig(const std::string& path);

    // ���ڶ����û�����ǰ��ķ��ţ��� local> some commands
    void setPrompt(const std::string& prompt);

    // ��ӭ��Ϣ
    void setWelcomeMsg(const std::string& msg);

    // ��ʾ��Ϣ
    void setHintMsg(const std::string& msg);

    // ����ִ�лص�
    using CommandExecution = std::function<STCommandResult(STParsedCommand&)>;
    void setExecution(CommandExecution execution);
private:
    // ִ�лص����ӡ���
    void printCommandResult(const STCommandResult& result);

    // ��ƽ̨�ַ�����ȡ
    std::string readLine();

    STCommandConfig _config;
    bool _shouldClose = false;
    std::unique_ptr<IAYCommandParser> _parser;
    CommandExecution _execution = nullptr;

    std::string _prompt = "> ";
    std::string _welcomeMsg = "Welcome to Command Interface";
    std::string _hintMsg = "Type 'exit' or 'quit' to end session";
};