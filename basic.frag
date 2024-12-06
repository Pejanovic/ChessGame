#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 moveColor;

void main() {
    vec4 texColor = texture(texture1, TexCoord);

    // Ako je fragment transparentan, odbaci ga
    if (texColor.a < 0.1) {
        discard;
    }

    // Ako je fragment dio poteza (crna tekstura), koristi moveColor
    if (texColor.rgb == vec3(0.0, 0.0, 0.0)) {
        FragColor = moveColor; // Boja poteza
    } else {
        FragColor = texColor; // Originalna boja figure
    }
}
