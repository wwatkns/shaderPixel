#version 400 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMat;
uniform mat4 model;

void main() {
    gl_Position = lightSpaceMat * model * vec4(aPos, 1.0);
}
