#include "AYRenderer.h"
#include "2DRendering/AYFontRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> ​​
#include <vector>
#include "AYLogger.h"
namespace ayt::engine::render
{
    const char* textVertexShader = R"(
#version 460 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

    const char* textFragmentShader = R"(
#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {    
    color = vec4(textColor, texture(text, TexCoords).r);
}
)";

    FontRenderer::FontRenderer(RenderDevice* device, Renderer* renderer) :
        _device(device),
        _renderer(renderer)
    {
        //初始化FT库
        if (FT_Init_FreeType(&_ftLibrary)) {
            AYLOG_ERR("[FontRenderer] Could not init FreeType Library");
            return;
        }

        _setupShader();

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        _createNewAtlas();
    }

    FontRenderer::~FontRenderer()
    {

    }

    void FontRenderer::shutdown()
    {
        if (_currentFace) {
            FT_Done_Face(_currentFace);
        }

        for (auto& atlas : _atlases) {
            glDeleteTextures(1, &atlas.textureID);
        }
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteProgram(_shaderProgram);
    }

    bool FontRenderer::loadFont(const std::string& fontPath, unsigned int fontSize, const std::map<std::string, float>& axisValues)
    {
        //加载字体文件到_currentFace
        if (FT_New_Face(_ftLibrary, fontPath.c_str(), 0, &_currentFace)) {
            AYLOG_ERR("[FontRenderer] Failed to load font");
            FT_Done_FreeType(_ftLibrary);
            return false;
        }

        // 检查是否为可变字体
        if (FT_IS_VARIATION(_currentFace)) {
            // 获取可变轴信息
            FT_MM_Var* mm_var = nullptr;
            if (FT_Get_MM_Var(_currentFace, &mm_var)) {
                AYLOG_ERR("[FontRenderer] Could not get MM Var data");
                FT_Done_Face(_currentFace);
                return false;
            }

            // 准备轴设置数组
            std::vector<FT_Fixed> coords(mm_var->num_axis);

            // 初始化为默认值
            for (FT_UInt i = 0; i < mm_var->num_axis; ++i) {
                coords[i] = mm_var->axis[i].def;
            }

            // 应用用户指定的轴值
            for (const auto& [axisName, value] : axisValues) {
                for (FT_UInt i = 0; i < mm_var->num_axis; ++i) {
                    if (axisName == mm_var->axis[i].name) {
                        // 确保值在有效范围内
                        float clampedValue = std::clamp(value,
                            mm_var->axis[i].minimum / 65536.0f,
                            mm_var->axis[i].maximum / 65536.0f);
                        coords[i] = static_cast<FT_Fixed>(clampedValue * 65536.0f);
                        break;
                    }
                }
            }

            // 设置可变轴值
            if (FT_Set_Var_Design_Coordinates(_currentFace, mm_var->num_axis, coords.data())) {
                AYLOG_ERR("[FontRenderer] Could not set variation coordinates");
                FT_Done_MM_Var(_ftLibrary, mm_var);
                FT_Done_Face(_currentFace);
                return false;
            }

            FT_Done_MM_Var(_ftLibrary, mm_var);
        }


        //设置字体大小
        _currentFontSize = fontSize;
        //加载英文，中文使用时加载
        _reloadCharacters();

        return true;
    }

    void FontRenderer::renderText(const std::string& text, float x, float y, float scale, const math::Vector3& color)
    {
        glfwMakeContextCurrent(_device->getWindow());

        glUseProgram(_shaderProgram);
        glUniform3f(glGetUniformLocation(_shaderProgram, "textColor"), color.r, color.g, color.b);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(_vao);

        int width, height;
        glfwGetFramebufferSize(_device->getWindow(), &width, &height);
        math::Matrix4 projection = glm::ortho(0.0f, static_cast<float>(width),
            static_cast<float>(height), 0.0f); // 注意Y轴翻转
        glUniformMatrix4fv(glGetUniformLocation(_shaderProgram, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        std::u32string utf32 = utf8_to_utf32(text);
        for (char32_t c : utf32) {
            Character theChar;

            if (findChar(theChar, c)) {
                _renderCharacter(theChar, x, y, scale);
            }
            else
            {
                loadChar(_currentFace, c);
                findChar(theChar, c);
                _renderCharacter(theChar, x, y, scale);
            }
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        //状态验证
        AY_CHECK_GL_ERROR("FontRenderer found somthing wrong");
    }

    void FontRenderer::getCharacterQuatInfo(const Character& ch, math::Vector3& render_pos, std::vector<math::Vector3>& result_pos, std::vector<math::Vector2>& result_uv, float scale)
    {
        float xpos = render_pos.x + ch.bearing.x * scale;
        float ypos = render_pos.y - ch.bearing.y * scale;  // 修正Y坐标计算

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        //纹理uv
        float u1 = ch.atlasPos.x;
        float v1 = ch.atlasPos.y + ch.atlasSize.y; // 原始V坐标
        float u2 = u1 + ch.atlasSize.x;
        float v2 = ch.atlasPos.y; // 原始V坐标

        result_pos = {
            { xpos,     ypos + h,   render_pos.z }, // 左下
            { xpos + w, ypos + h,   render_pos.z }, // 右下
            { xpos + w, ypos,       render_pos.z }, // 右上
            { xpos,     ypos,       render_pos.z }  // 左上
        };

        result_uv = {
            { u1, v1 }, // 左下
            { u2, v1 }, // 右下
            { u2, v2 }, // 右上
            { u1, v2 }  // 左上
        };

        //使字符偏移 （水平间距/64）* 缩放
        render_pos.x += (ch.advance >> 6) * scale;
    }

    void FontRenderer::_renderCharacter(const Character& ch, float& x, float& y, float scale) {
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - ch.bearing.y * scale;  // 修正Y坐标计算

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        //纹理uv
        float u1 = ch.atlasPos.x;
        float v1 = ch.atlasPos.y + ch.atlasSize.y; // 原始V坐标
        float u2 = u1 + ch.atlasSize.x;
        float v2 = ch.atlasPos.y; // 原始V坐标

        // 修正的顶点数据
        /*
            两个三角形组成字符渲染的面

          1、  →
             ↓  屏幕空间坐标方向

          2、↑  uv坐标方向
               →

          3、 GL右手坐标系
                ↑
                 . →  z轴朝向屏幕内

            (u1,v1+h)   (u1+w,v1+h)
            (x,y)       (x+w,y)
                x  x  x  x
                x      x x
                x    x   x
                x x      x
                x  x  x  x
            (x,y+h)    (x+w,y+h)
            (u1,v1)    (u1+w,v1)
        */
        float vertices[6][4] = {
            { xpos,     ypos + h,   u1, v1 }, // 左下
            { xpos + w, ypos + h,   u2, v1 }, // 右下
            { xpos + w, ypos,       u2, v2 }, // 右上

            { xpos,     ypos + h,   u1, v1 }, // 左下
            { xpos + w, ypos,       u2, v2 }, // 右上
            { xpos,     ypos,       u1, v2 }  // 左上
        };

        //上传并绘制
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //使字符偏移 （水平间距/64）* 缩放
        x += (ch.advance >> 6) * scale;
    }

    void FontRenderer::_createNewAtlas()
    {
        TextureAtlas newAtlas;
        newAtlas.width = _atlasSize;
        newAtlas.height = _atlasSize;

        glGenTextures(1, &newAtlas.textureID);
        glBindTexture(GL_TEXTURE_2D, newAtlas.textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _atlasSize, _atlasSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 确保纹理正确初始化
        std::vector<unsigned char> emptyData(4 * _atlasSize * _atlasSize, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _atlasSize, _atlasSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptyData.data());

        _atlases.push_back(newAtlas);
    }

    bool FontRenderer::loadChar(FT_Face face, char32_t charCode)
    {
        if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
            return false;
        }

        _addCharToAtlas(face, charCode);
        return true;
    }

    void FontRenderer::_reloadCharacters()
    {
        // 清空现有图集
        for (auto& atlas : _atlases) {
            glDeleteTextures(1, &atlas.textureID);
            atlas.characters.clear();
        }
        _atlases.clear();
        _createNewAtlas();

        //设置上传纹理无对齐（默认4字节）
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // 重新加载face中的字符
        FT_Set_Pixel_Sizes(_currentFace, 0, _currentFontSize);

        // 重新加载ASCII字符
        for (char32_t c = 32; c < 128; c++) {
            loadChar(_currentFace, c);
        }
    }

    bool FontRenderer::findChar(Character& theChar, char32_t charCode)
    {
        for (const auto& atlas : _atlases) {
            auto it = atlas.characters.find(charCode);
            if (it != atlas.characters.end()) {
                theChar = it->second;
                return true;
            }
        }
        return false;
    }

    void FontRenderer::_addCharToAtlas(FT_Face face, char32_t charCode) {
        // 检查是否已存在
        for (auto& atlas : _atlases) {
            if (atlas.characters.find(charCode) != atlas.characters.end()) {
                return;
            }
        }

        const int padding = 1;

        // 尝试添加到现有图集
        for (auto& atlas : _atlases) {
            if (face->glyph->bitmap.width + padding + atlas.currentX > atlas.width) {
                atlas.currentX = 0;
                atlas.currentY += atlas.nextRowHeight + padding;
                atlas.nextRowHeight = 0;
            }

            if (face->glyph->bitmap.rows + atlas.currentY > atlas.height) {
                continue; // 换下一个图集
            }

            FT_Bitmap& bitmap = face->glyph->bitmap;
            int width = bitmap.width;
            int height = bitmap.rows;
            unsigned char* data = bitmap.buffer;

            // 准备 RGBA 数据缓冲区
            std::vector<unsigned char> rgbaData(width * height * 4, 0);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int idx = y * width + x;
                    unsigned char gray = data[idx]; // FreeType 提供的灰度值 [0, 255]

                    // RGBA
                    rgbaData[idx * 4 + 0] = gray;     // R = 灰度值
                    rgbaData[idx * 4 + 1] = 0;        // G = 0
                    rgbaData[idx * 4 + 2] = 0;        // B = 0
                    rgbaData[idx * 4 + 3] = 255;      // A = 255（不透明）
                }
            }
            // 有足够空间
            glBindTexture(GL_TEXTURE_2D, atlas.textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                atlas.currentX, atlas.currentY,
                width, height,
                GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());

            Character character = {
                atlas.textureID,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x),
                math::Vector2(
                    static_cast<float>(atlas.currentX) / atlas.width,
                    static_cast<float>(atlas.currentY) / atlas.height
                ),
                math::Vector2(
                    static_cast<float>(face->glyph->bitmap.width) / atlas.width,
                    static_cast<float>(face->glyph->bitmap.rows) / atlas.height
                )
            };

            atlas.characters[charCode] = character;
            atlas.currentX += face->glyph->bitmap.width + padding;
            atlas.nextRowHeight = std::max(atlas.nextRowHeight, (int)face->glyph->bitmap.rows);
            return;
        }

        // 没有足够空间，创建新图集
        _createNewAtlas();
        _addCharToAtlas(face, charCode);
    }

    bool FontRenderer::setVariationAxis(const std::string& axisName, float value)
    {
        if (!FT_IS_VARIATION(_currentFace)) {
            return false;
        }

        FT_MM_Var* mm_var = nullptr;
        if (FT_Get_MM_Var(_currentFace, &mm_var)) {
            return false;
        }

        bool found = false;
        std::vector<FT_Fixed> coords(mm_var->num_axis);

        // 获取当前坐标
        if (FT_Get_Var_Design_Coordinates(_currentFace, mm_var->num_axis, coords.data())) {
            FT_Done_MM_Var(_ftLibrary, mm_var);
            return false;
        }

        // 查找并设置指定轴
        for (FT_UInt i = 0; i < mm_var->num_axis; ++i) {
            if (axisName == mm_var->axis[i].name) {
                float min = mm_var->axis[i].minimum / 65536.0f;
                float max = mm_var->axis[i].maximum / 65536.0f;
                value = std::clamp(value, min, max);
                coords[i] = static_cast<FT_Fixed>(value * 65536.0f);
                found = true;
                break;
            }
        }

        if (!found) {
            FT_Done_MM_Var(_ftLibrary, mm_var);
            return false;
        }

        // 应用新坐标
        if (FT_Set_Var_Design_Coordinates(_currentFace, mm_var->num_axis, coords.data())) {
            FT_Done_MM_Var(_ftLibrary, mm_var);
            return false;
        }

        _currentAxisValues[axisName] = value;
        FT_Done_MM_Var(_ftLibrary, mm_var);

        // 重新加载字符以应用新样式
        _reloadCharacters();
        return true;
    }

    bool FontRenderer::getVariationAxisRange(const std::string& axisName, float& min, float& max, float& def)
    {
        return false;
    }

    void FontRenderer::_setupShader()
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &textVertexShader, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &textFragmentShader, NULL);
        glCompileShader(fragmentShader);

        _shaderProgram = glCreateProgram();
        glAttachShader(_shaderProgram, vertexShader);
        glAttachShader(_shaderProgram, fragmentShader);
        glLinkProgram(_shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void FontRenderer::saveAtlas()
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
}