#pragma once
#include "AYRenderDevice.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H  // 可变字体/多主字体支持
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

    /*
        可变轴体，暂未支持
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
    bool _findChar(Character& theChar, char32_t charCode); 

    void _reloadCharacters();   //清空所有图集，重新加载字符

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