//fs
#version 410 core

out vec3 fragColor;

void main(void) {
    
    fragColor = vec3(gl_FragCoord.z);
}
