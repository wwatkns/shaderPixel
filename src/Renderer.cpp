#include "Renderer.hpp"

Renderer::Renderer( Env* env ) :
    env(env),
    camera(75, (float)env->getWindow().width / (float)env->getWindow().height) {
    this->models.push_back(new Model(
        "./obj/model.obj",
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    ));
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
        this->env->getController()->update();
        this->camera.handleKeys( this->env->getController()->getKeys() );
        for (auto it = this->models.begin(); it != this->models.end(); it++) {
            (*it)->render(this->camera);
        }
        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}
