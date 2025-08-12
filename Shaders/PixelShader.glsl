#version 450
layout(location = 0) in flat uvec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in flat vec3 i_Color;
layout(location = 0) out vec4 o_Color;

void main() {
    uint i = i_TexCoord.x;
    float light = 0.1 + 0.9 * clamp(i_Normal.g, 0.0, 1.0);
    o_Color = vec4(light * i_Color.rgb, 1.0);
}
