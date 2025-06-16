#pragma once
#include "AYRenderDevice.h"
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H  // �ɱ�����/��������֧��

#include <codecvt>
#include <locale>
#include <iconv.h>

#include <opencv2/opencv.hpp>

static std::string gbk_to_utf8(const std::string& gbkStr) {
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open ʧ��: " + std::string(strerror(errno)));
    }

    char* inbuf = const_cast<char*>(gbkStr.data());
    size_t inbytes = gbkStr.size();
    std::string utf8Str(gbkStr.size() * 3, '\0'); // Ԥ���㹻�ռ�
    char* outbuf = &utf8Str[0];
    size_t outbytes = utf8Str.size();

    if (iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes) == (size_t)-1) {
        iconv_close(cd);
        throw std::runtime_error("iconv ת��ʧ��: " + std::string(strerror(errno)));
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
    result.reserve(utf8.size()); // �����ܳ���

    for (size_t i = 0; i < utf8.size(); ) {
        unsigned char c = utf8[i];
        char32_t codepoint = 0xFFFD; // Ĭ���滻�ַ�

        if (c < 0x80) {
            // ASCII: ֱ��ӳ��
            codepoint = c;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2�ֽ�����
            if (i + 1 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1];
            if ((c2 & 0xC0) != 0x80) goto invalid; // �ڶ��ֽڱ�����10��ͷ
            codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
            if (codepoint < 0x80) goto overlong; // ���̱���
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3�ֽ�����
            if (i + 2 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1], c3 = utf8[i + 2];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) goto invalid;
            codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (codepoint < 0x800) goto overlong; // ���̱���
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF) goto surrogate; // �������Ƿ�
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            // 4�ֽ�����
            if (i + 3 >= utf8.size()) goto invalid;
            unsigned char c2 = utf8[i + 1], c3 = utf8[i + 2], c4 = utf8[i + 3];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) goto invalid;
            codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
            if (codepoint < 0x10000) goto overlong; // ���̱���
            if (codepoint > 0x10FFFF) goto too_large; // ����Unicode��Χ
            i += 4;
        }
        else {
            // �Ƿ����ֽ�
            goto invalid;
        }

        result.push_back(codepoint);
        continue;

    invalid:
        // �Ƿ��ֽ����У�������ǰ�ֽڲ�����
        i += 1;
        continue;

    overlong:
        // ���̱��룬�滻Ϊ U+FFFD
        result.push_back(0xFFFD);
        i += (c < 0xE0 ? 1 : (c < 0xF0 ? 2 : 3));
        continue;

    surrogate:
        // ��������㣬�滻Ϊ U+FFFD
        result.push_back(0xFFFD);
        i += 3;
        continue;

    too_large:
        // ��㳬�� Unicode ��Χ���滻Ϊ U+FFFD
        result.push_back(0xFFFD);
        i += 4;
        continue;
    }

    return result;
}

struct Character {
    GLuint textureID;
    glm::ivec2 size;    //size.y ���εĸ߶�
    glm::ivec2 bearing; //bearing.y���ߵ������ľ���
    unsigned int advance;
    glm::vec2 atlasPos; // ����ͼ��λ��
    glm::vec2 atlasSize; // ��ͼ���еĳߴ�
};

/*
    ��δ֧�ֿɱ�����
    ��ֱ��ʹ��ϵͳ����
*/
class AYFontRenderer {
public:
    AYFontRenderer(AYRenderDevice* device);
    ~AYFontRenderer();

    /*
        �������������ļ�
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
            int width = a.width;  // ������
            int height = a.height; // ����߶�

            std::vector<unsigned char> pixels(width * height * 4); // RGBA ��ʽ

            // ��ȡ�������ݣ�GL_RGBA, GL_UNSIGNED_BYTE��
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

            cv::Mat image(height, width, CV_8UC4, pixels.data());
            //cv::flip(image, image, 0);
            std::string str = "texture_atlas(" + std::to_string(count) + ").png";
            cv::imwrite(str, image);

            count++;
        }
    }

    /*
        ����
    */
    void preloadCommonChineseCharacters();



private:
    void _setupShader();

private:
    /*
        ��������Ⱦ��ͼ����ÿ����һ���־���ǰͼ��������ƫ�Ƹ��ǣ�Ȼ��洢������ʵ���ַ���Ϣ��map��
        ��ʹ��ʱ��ÿ��ͼ���б�������Ŀ���ַ�������ҵ�����Ⱦ��ǰ�ַ�
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
        {0x4E00, 0x9FFF},   // ����������������"һ�������������²���"�ȣ�
        {0x4E8D, 0x4EFF},   // �������䳣�ú���
        {0x3400, 0x4DBF},   // ��չA��ȫ�������Ǹ����������֣�
        //{0x4F00, 0x4FDF},   // ��չA�����ò��֣�"��ƹ���ǹԳ�"�ȣ�
        //{0x20000, 0x2A6DF}  // ��չB-F����������أ�
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
    unsigned int _atlasSize = 1024; // ����ͼ���ߴ�

    FT_Library _ftLibrary;
    FT_Face _currentFace =  nullptr; // ��Ҫ���浱ǰ����face
    std::map<std::string, float> _currentAxisValues;
    unsigned int _currentFontSize = 0;
};