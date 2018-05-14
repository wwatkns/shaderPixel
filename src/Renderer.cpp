#include "Renderer.hpp"
#include "glm/ext.hpp"

Renderer::Renderer( Env* env ) :
env(env),
camera(75, (float)env->getWindow().width / (float)env->getWindow().height) {
    this->shader["default"] = new Shader("./shader/default_vert.glsl", "./shader/default_frag.glsl");
    this->shader["skybox"]  = new Shader("./shader/skybox_vert.glsl", "./shader/skybox_frag.glsl");
}

Renderer::~Renderer( void ) {
}

void	Renderer::loop( void ) {
    /* z-buffering */
    glEnable(GL_DEPTH_TEST);
    /* gamma correction */
    glEnable(GL_FRAMEBUFFER_SRGB);
    /* multisampling MSAA */
    glEnable(GL_MULTISAMPLE);
    /* transparency */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(this->env->getWindow().ptr)) {
        glfwPollEvents();
        glClearColor(0.09f, 0.08f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->env->getController()->update();
        this->camera.handleKeys( this->env->getController()->getKeys() );

        this->renderLights();
        this->renderMeshes();
        this->renderSkybox();

        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}

void    Renderer::renderLights( void ) {
    /* update shader uniforms */
    this->shader["default"]->use();
    this->shader["default"]->setIntUniformValue("nPointLights", Light::pointLightCount);
    /* render lights */
    for (auto it = this->env->getLights().begin(); it != this->env->getLights().end(); it++)
        (*it)->render(*this->shader["default"]);
}

void    Renderer::renderMeshes( void ) {
    /* update shader uniforms */
    this->shader["default"]->use();
    this->shader["default"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    this->shader["default"]->setMat4UniformValue("view", this->camera.getViewMatrix());
    this->shader["default"]->setVec3UniformValue("viewPos", this->camera.getPosition());
    /* render models */
    for (auto it = this->env->getModels().begin(); it != this->env->getModels().end(); it++)
        (*it)->render(*this->shader["default"]);
}

void    Renderer::renderSkybox( void ) {
    glDepthFunc(GL_LEQUAL);
    this->shader["skybox"]->use();
    this->shader["skybox"]->setMat4UniformValue("view", glm::mat4(glm::mat3(this->camera.getViewMatrix())));
    this->shader["skybox"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    /* render skybox */
    this->env->getSkybox()->render(*this->shader["skybox"]);
    glDepthFunc(GL_LESS);
}
