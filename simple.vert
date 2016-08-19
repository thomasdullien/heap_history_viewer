#version 130
in ivec3 position;
in highp vec3 color;
out vec4 vColor;

uniform mat2 projectVertexToScreen;
uniform vec2 translationPart;

void main(void)
{
    vec2 result = (position * projectVertexToScreen) + translationPart;
    gl_Position = vec4(result, 0.0, 1.0);
    vColor = vec4(color, 1.0);
}
