#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out float Near;
out float Far;

uniform mat4 model;
uniform float near;
uniform float far;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    Near = near;
    Far = far;

    // put quad in front of camera
    mat4 Model = model;
    Model[3].xyz = vec3(0.0, 0.0, -Near);
    Model[0][0] = 2.0;
    Model[1][1] = 2.0;
    gl_Position = Model * vec4(aPos, 1.0);
}