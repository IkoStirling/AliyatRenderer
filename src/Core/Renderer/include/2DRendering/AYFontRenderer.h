#pragma once
#include "AYRenderDevice.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H  // �ɱ�����/��������֧��
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <map>
#include <codecvt>
#include <opencv2/opencv.hpp>


static std::u32string utf8_to_utf32(const std::string& utf8) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.from_bytes(utf8);
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

    /*
        �ɱ����壬��δ֧��
    */
    bool setVariationAxis(const std::string& axisName, float value);
    bool getVariationAxisRange(const std::string& axisName, float& min, float& max, float& def);

    /*
        DEBUG
    */
    void saveAtlas(); 


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
    bool _findChar(Character& theChar, char32_t charCode); 

    void _reloadCharacters();   //�������ͼ�������¼����ַ�

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