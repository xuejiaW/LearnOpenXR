#version 450

layout(std140, binding = 0) uniform CameraConstants {
    mat4 viewProj;
    mat4 modelViewProj;
    mat4 model;
    vec4 color;
    vec4 pad1;
    vec4 pad2;
    vec4 pad3;
};

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out flat uvec2 o_TexCoord;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out flat vec3 o_Color;

void main() {
    gl_Position = modelViewProj * a_Position;
    
    int face = gl_VertexIndex / 6;
    o_TexCoord = uvec2(face, 0);
    
    o_Normal = (model * vec4(a_Normal, 0.0)).xyz;
    
    o_Color = color.rgb;
}
