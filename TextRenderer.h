#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height);

    void loadFont(const char* fontPath);

    void renderText(unsigned int shader, const std::string& text, float x, float y, float scale, glm::vec3 color);
    void setProjection(unsigned int shader, unsigned int width, unsigned int height);
    float calculateTextWidth(const std::string& text, float scale);

private:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;

    void initRenderData();
};

#endif
