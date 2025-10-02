#version 410 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;

uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat = mat4(1);
uniform mat4 projMat = mat4(1);
uniform mat4 shadowBiasMVP;

out vec3 worldCoord;
out vec3 normal;
out vec2 texCoord;
out vec4 shadowCoord;

void main(void)
{

    vec4 worldPosition = modelMat * vec4(in_Position, 1.0);
    worldCoord = worldPosition.xyz;
    normal = mat3(modelMat) * in_Normal;
    texCoord = in_TexCoord;


    shadowCoord = shadowBiasMVP * vec4(in_Position, 1.0);


    gl_Position = projMat * viewMat * worldPosition; 
}
