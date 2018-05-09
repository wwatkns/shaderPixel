#include "Renderer.hpp"

Renderer::Renderer( Env* env ) :
    env(env),
    camera(75, (float)env->getWindow().width / (float)env->getWindow().height),
    shader("./shader/vertex.glsl", "./shader/fragment.glsl") {
}

Renderer::~Renderer( void ) {
}

void	Renderer::loop( void ) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    while (!glfwWindowShouldClose(this->env->getWindow().ptr)) {
        glfwPollEvents();
        glClearColor(0.09f, 0.08f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->shader.use();
        this->env->getController()->update();
        this->camera.handleKeys( this->env->getController()->getKeys() );
        this->updateShaderUniforms();
        // TODO: update the models
        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}

void    Renderer::updateShaderUniforms( void ) {
    this->shader.setMat4UniformValue("view", this->camera.getViewMatrix());
    this->shader.setMat4UniformValue("projection", this->camera.getProjectionMatrix());
}
