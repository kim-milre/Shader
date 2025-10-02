//vs
#version 410 core

layout(location = 0) in vec3 in_Position;

uniform mat4 shadowMVP;

void main(void) {

    vec4 worldPosition = vec4(in_Position, 1.0);

    gl_Position = shadowMVP * worldPosition;
}
