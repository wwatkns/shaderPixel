#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out mat4 invProj;
out mat4 invView;
out float Near;
out float Far;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float near;
uniform float far;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    invProj = inverse(projection);
    invView = inverse(view);
    Near = near;
    Far = far;

    /* spherical billboarding */
    // mat4 modelView = view * model;
    // modelView[0].xyz = vec3(model[0][0], 0, 0);
    // modelView[1].xyz = vec3(0, model[1][1], 0);
    // modelView[2].xyz = vec3(0, 0, model[2][2]);
    // gl_Position = projection * modelView * vec4(aPos, 1.0);

    /* front of camera */
    mat4 Model = model;
    Model[3].xyz = vec3(0.0, 0.0, -Near);
    Model[0][0] = 2.0;
    Model[1][1] = 2.0;
    gl_Position = Model * vec4(aPos, 1.0);
}
