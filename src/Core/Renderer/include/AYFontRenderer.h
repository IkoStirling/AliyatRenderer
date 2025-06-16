#pragma once
#include "AYRenderDevice.h"
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H  // 可变字体/多主字体支持

#include <codecvt>
#include <locale>
#include <iconv.h>

#include <opencv2/opencv.hpp>

static std::string gbk_to_utf8(const std::string& gbkStr) {
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open 失败: " + std::string(strerror(errno)));
    }

    char* inbuf = const_cast<char*>(gbkStr.data());
    size_t inbytes = gbkStr.size();
    std::string utf8Str(gbkStr.size() * 3, '\0'); // 预留足够空间
    char* outbuf = &utf8Str[0];
    size_t outbytes = utf8Str.size();

    if (iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes) == (size_t)-1) {
        iconv_close(cd);
        throw std::runtime_error("iconv 转换失败: " + std::string(strerror(errno)));
    }

    iconv_close(cd);
    utf8Str.resize(utf8Str.size() - outbytes);
    return utf8Str;
}
static std::u32string utf8_to_utf32(const std::string& utf8) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.from_bytes(utf8);
}
static std::u32string safe_utf8_to_utf32(const std::string& utf8) {
    std::u32string result;
    result.reserve(utf8.size()); // 最大可能长度

    for (size_t i = 0; i < utf8.size(); ) {
        unsigned char c = utf8[i];
        char32_t codepoint = 0xFFFD; // 默认替换字符

        if (c < 0x80) {
            // ASCII: 直接映射
            codepoint = c;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2字节序列
            if (i + 1 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1];
            if ((c2 & 0xC0) != 0x80) goto invalid; // 第二字节必须以10开头
            codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
            if (codepoint < 0x80) goto overlong; // 过短编码
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3字节序列
            if (i + 2 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1], c3 = utf8[i + 2];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) goto invalid;
            codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (codepoint < 0x800) goto overlong; // 过短编码
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF) goto surrogate; // 代理区非法
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            // 4字节序列
            if (i + 3 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1], c3 = utf8[i + 2], c4 = utf8[i + 3];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) goto invalid;
            codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
            if (codepoint < 0x10000) goto overlong; // 过短编码
            if (codepoint > 0x10FFFF) goto too_large; // 超出Unicode范围
            i += 4;
        }
        else {
            // 非法首字节
            goto invalid;
        }

        result.push_back(codepoint);
        continue;

    invalid:
        // 非法字节序列，跳过当前字节并继续
        i += 1;
        continue;

    overlong:
        // 过短编码，替换为 U+FFFD
        result.push_back(0xFFFD);
        i += (c < 0xE0 ? 1 : (c < 0xF0 ? 2 : 3));
        continue;

    surrogate:
        // 代理区码点，替换为 U+FFFD
        result.push_back(0xFFFD);
        i += 3;
        continue;

    too_large:
        // 码点超出 Unicode 范围，替换为 U+FFFD
        result.push_back(0xFFFD);
        i += 4;
        continue;
    }

    return result;
}

struct Character {
    GLuint textureID;
    glm::ivec2 size;    //size.y 字形的高度
    glm::ivec2 bearing; //bearing.y基线到顶部的距离
    unsigned int advance;
    glm::vec2 atlasPos; // 纹理图集位置
    glm::vec2 atlasSize; // 在图集中的尺寸
};

/*
    尚未支持可变字体
    请直接使用系统字体
*/
class AYFontRenderer {
public:
    AYFontRenderer(AYRenderDevice* device);
    ~AYFontRenderer();

    /*
        加载整个字体文件
    */
    bool loadFont(const std::string& fontPath, unsigned int fontSize, const std::map<std::string, float>& axisValues = {});
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    bool setVariationAxis(const std::string& axisName, float value);
    bool getVariationAxisRange(const std::string& axisName, float& min, float& max, float& def);

    void saveAtlas()
    {
        static int count = 0;
        for (auto& a : _atlases)
        {
            glBindTexture(GL_TEXTURE_2D, a.textureID);
            int width = a.width;  // 纹理宽度
            int height = a.height; // 纹理高度

            std::vector<unsigned char> pixels(width * height * 4); // RGBA 格式

            // 读取纹理数据（GL_RGBA, GL_UNSIGNED_BYTE）
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

            cv::Mat image(height, width, CV_8UC4, pixels.data());
            //cv::flip(image, image, 0);
            std::string str = "texture_atlas(" + std::to_string(count) + ").png";
            cv::imwrite(str, image);

            count++;
        }
    }

    /*
        弃用
    */
    void preloadCommonChineseCharacters();



private:
    void _setupShader();

private:
    /*
        将字体渲染到图集（每加载一个字就向当前图集的纹理偏移覆盖，然后存储编码与实际字符信息的map）
        当使用时在每个图集中遍历查找目标字符，如果找到则渲染当前字符
    */
    struct TextureAtlas {
        GLuint textureID;
        int width, height;
        int currentX = 0;
        int currentY = 0;
        int nextRowHeight = 0;
        std::unordered_map<char32_t, Character> characters;
    };

    const std::vector<std::pair<uint32_t, uint32_t>> _chineseRanges = {
        {0x4E00, 0x9FFF},   // 基本汉字区（包含"一丁七万丈三上下不与"等）
        {0x4E8D, 0x4EFF},   // 继续补充常用汉字
        {0x3400, 0x4DBF},   // 扩展A区全部（覆盖更多姓氏用字）
        //{0x4F00, 0x4FDF},   // 扩展A区常用部分（"丢乒乓乔乖乘"等）
        //{0x20000, 0x2A6DF}  // 扩展B-F区（按需加载）
    };

    void _addCharToAtlas(FT_Face face, char32_t charCode);
    void _renderCharacter(const Character& ch, float& x, float& y, float scale);
    void _createNewAtlas();
    bool _loadChar(FT_Face face, char32_t charCode);

    void _reloadCharacters();

private:
    AYRenderDevice* _device;
    std::vector<TextureAtlas> _atlases;
    GLuint _vao, _vbo;
    GLuint _shaderProgram;
    unsigned int _atlasSize = 1024; // 纹理图集尺寸

    FT_Library _ftLibrary;
    FT_Face _currentFace =  nullptr; // 需要保存当前字体face
    std::map<std::string, float> _currentAxisValues;
    unsigned int _currentFontSize = 0;
};