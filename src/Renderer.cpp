#include "Renderer.hpp"
#include "glm/ext.hpp"

Renderer::Renderer( Env* env ) :
env(env),
camera(75, (float)env->getWindow().width / (float)env->getWindow().height),
shader("./shader/vertex.glsl", "./shader/fragment.glsl"),
model(
    "/Users/wwatkins/Downloads/nanosuit/nanosuit.obj",
    // "/Users/wwatkins/Downloads/amaravati-guardian-lion/source/amaravatiGuardianLion/lion.obj",
    // "/Users/wwatkins/Downloads/rpg-reptile-mage/source/LizPosed.obj",
    // "/Users/wwatkins/Downloads/swimsuit-succubus/source/SS.obj",
    // glm::vec3(0.0f, -1.75f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f)
    // glm::vec3(0.2f, 0.2f, 0.2f)
) {
    // this->models.push_back(new Model(
    //     "/Users/wwatkins/Downloads/nanosuit/nanosuit.obj",
    //     // "/Users/wwatkins/Downloads/amaravati-guardian-lion/source/amaravatiGuardianLion/lion.obj",
    //     // "/Users/wwatkins/Downloads/rpg-reptile-mage/source/LizPosed.obj",
    //     // "/Users/wwatkins/Downloads/swimsuit-succubus/source/SS.obj",
    //     glm::vec3(0.0f, -1.75f, 0.0f),
    //     glm::vec3(0.0f, 0.0f, 0.0f),
    //     glm::vec3(0.2f, 0.2f, 0.2f)
    // ));
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

        // std::cout << glm::to_string(this->camera.getProjectionMatrix()) << std::endl;
        // std::cout << glm::to_string(this->camera.getViewMatrix()) << std::endl;

        /* update shader values */
        this->shader.use();
        this->shader.setMat4UniformValue("projection", this->camera.getProjectionMatrix());
        this->shader.setMat4UniformValue("view", this->camera.getViewMatrix());
        /* render model */
        this->model.render(shader, this->camera);

        // for (auto it = this->models.begin(); it != this->models.end(); it++)
            // (*it)->render(shader, this->camera);

        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}
