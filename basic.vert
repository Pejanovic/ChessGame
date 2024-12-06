#version 330 core
layout(location = 0) in vec2 aPos; // pozicija verteksa
layout(location = 1) in vec2 aTexCoord; // teksture koordinate

uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = model * vec4(aPos, 0.0, 1.0); // pretvaramo 2d prostor u 4d potreban za transformacije i projekcije u OpnGL - u, model transformice vertekse iz lokalnih u world space koordinate
    TexCoord = aTexCoord;
}
