#include "Renderer.hpp"
#include "glm/ext.hpp"

Renderer::Renderer( Env* env ) :
env(env),
camera(75, (float)env->getWindow().width / (float)env->getWindow().height),
shader("./shader/vertex.glsl", "./shader/fragment.glsl") {
}

Renderer::~Renderer( void ) {
}

void	Renderer::loop( void ) {
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_MULTISAMPLE);
    while (!glfwWindowShouldClose(this->env->getWindow().ptr)) {
        glfwPollEvents();
        glClearColor(0.09f, 0.08f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->env->getController()->update();
        this->camera.handleKeys( this->env->getController()->getKeys() );

        /* update shader uniforms */
        this->shader.use();
        this->shader.setMat4UniformValue("projection", this->camera.getProjectionMatrix());
        this->shader.setMat4UniformValue("view", this->camera.getViewMatrix());
        /* render models */
        for (auto it = this->env->getModels().begin(); it != this->env->getModels().end(); it++)
            (*it)->render(shader);
        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}
