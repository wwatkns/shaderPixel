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

        this->shader.use();
        this->env->getController()->update();
        // this->camera.handleKeys( this->env->getController()->getKeys() );

        /* update shader uniforms */
        this->shader.setMat4UniformValue("projection", this->camera.getProjectionMatrix());
        this->shader.setMat4UniformValue("view", this->camera.getViewMatrix());

        // std::cout << glm::to_string(this->camera.getProjectionMatrix()) << std::endl;
        // std::cout << glm::to_string(this->camera.getViewMatrix()) << std::endl;
        /* render model */
        this->env->getModel()->render(shader);

        // for (auto it = this->models.begin(); it != this->models.end(); it++)
            // (*it)->render(shader, this->camera);

        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}
