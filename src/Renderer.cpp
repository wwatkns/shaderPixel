#include "Renderer.hpp"
#include "glm/ext.hpp"

Renderer::Renderer( Env* env ) :
env(env),
camera(75, (float)env->getWindow().width / (float)env->getWindow().height) {
    this->shader["default"] = new Shader("./shader/vertex/default_vert.glsl", "./shader/fragment/default_frag.glsl");
    this->shader["skybox"]  = new Shader("./shader/vertex/skybox_vert.glsl", "./shader/fragment/skybox_frag.glsl");
    this->shader["shadowMap"] = new Shader("./shader/vertex/shadow_mapping_vert.glsl", "./shader/fragment/shadow_mapping_frag.glsl");
    // this->shader["cloud"] = new Shader("./shader/vertex/default_vert.glsl", "./shader/fragment/cloud_frag.glsl");
    this->shader["fractal"] = new Shader("./shader/vertex/fractal_vert.glsl", "./shader/fragment/fractal_frag.glsl");
    this->initDepthMap();
    this->initShadowDepthMap(4096, 4096);
    this->useShadows = 0;
}

Renderer::~Renderer( void ) {
}

void	Renderer::loop( void ) {
    glEnable(GL_DEPTH_TEST); /* z-buffering */
    glEnable(GL_FRAMEBUFFER_SRGB); /* gamma correction */
    // glEnable(GL_MULTISAMPLE); /* multisampling MSAA */
    glEnable(GL_BLEND); /* transparency */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE); /* enable face-culling (back faces of triangles are not rendered) */
    while (!glfwWindowShouldClose(this->env->getWindow().ptr)) {
        glfwPollEvents();
        glClearColor(0.09f, 0.08f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->env->getController()->update();
        this->camera.handleInputs(this->env->getController()->getKeys(), this->env->getController()->getMouse());

        this->useShadows = this->env->getController()->getKeyValue(GLFW_KEY_P);

        this->updateShadowDepthMap();
        this->renderLights();
        this->renderMeshes();
        this->renderSkybox();
        this->renderShaders();
        glfwSwapBuffers(this->env->getWindow().ptr);
    }
}

void    Renderer::updateShadowDepthMap( void ) {
    Light*  directionalLight = this->env->getDirectionalLight();
    if (this->useShadows && directionalLight) {
        glm::mat4 lightProjection, lightView;
        float near_plane = 0.1f, far_plane = 35.0f;
        lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
        lightView = glm::lookAt(directionalLight->getPosition(), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        this->lightSpaceMat = lightProjection * lightView;
        /* render scene from light's point of view */
        this->shader["shadowMap"]->use();
        this->shader["shadowMap"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);

        glViewport(0, 0, this->shadowDepthMap.width, this->shadowDepthMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, this->shadowDepthMap.fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        /* render meshes on shadowMap shader */
        for (auto it = this->env->getModels().begin(); it != this->env->getModels().end(); it++)
            (*it)->render(*this->shader["shadowMap"]);

        /* reset viewport and framebuffer*/
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, this->env->getWindow().width, this->env->getWindow().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    this->shader["default"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);
    glActiveTexture(GL_TEXTURE0);
    this->shader["default"]->setIntUniformValue("shadowMap", 0);
    this->shader["default"]->setIntUniformValue("state.use_shadows", this->useShadows);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);

    /* render models */
    for (auto it = this->env->getModels().begin(); it != this->env->getModels().end(); it++)
        (*it)->render(*this->shader["default"]);

    /* copy the depth buffer to a texture (used in fractal shader for geometry occlusion of raymarched objects) */
    glBindTexture(GL_TEXTURE_2D, this->depthMap.id);
    glReadBuffer(GL_FRONT);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, this->depthMap.width, this->depthMap.height, 0);
}

void    Renderer::renderSkybox( void ) {
    glDisable(GL_CULL_FACE); /* disable face-culling as the skybox shows back-faces */
    glDepthFunc(GL_LEQUAL);
    this->shader["skybox"]->use();
    this->shader["skybox"]->setMat4UniformValue("view", glm::mat4(glm::mat3(this->camera.getViewMatrix())));
    this->shader["skybox"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    /* render skybox */
    this->env->getSkybox()->render(*this->shader["skybox"]);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
}

void    Renderer::renderShaders( void ) {
    glDisable(GL_CULL_FACE);
    this->shader["fractal"]->use();
    this->shader["fractal"]->setMat4UniformValue("view", this->camera.getViewMatrix());
    this->shader["fractal"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    this->shader["fractal"]->setFloatUniformValue("near", this->camera.getNear());
    this->shader["fractal"]->setFloatUniformValue("far", this->camera.getFar());

    this->shader["fractal"]->setVec2UniformValue("uResolution", glm::vec2(0.5, 0.5));
    this->shader["fractal"]->setVec2UniformValue("uMouse", this->env->getController()->getMousePosition());
    this->shader["fractal"]->setFloatUniformValue("uTime", glfwGetTime());
    this->shader["fractal"]->setVec3UniformValue("uCameraPos", this->camera.getPosition());
    // NEW
    glActiveTexture(GL_TEXTURE0);
    this->shader["default"]->setIntUniformValue("depthBuffer", 0);
    glBindTexture(GL_TEXTURE_2D, this->depthMap.id);

    this->env->quad->render(*this->shader["fractal"]);
    glEnable(GL_CULL_FACE);
}

void    Renderer::initShadowDepthMap( const size_t width, const size_t height ) {
    glEnable(GL_DEPTH_TEST);

    this->shadowDepthMap.width = width;
    this->shadowDepthMap.height = height;

    glGenFramebuffers(1, &this->shadowDepthMap.fbo);
    /* create depth texture */
    glGenTextures(1, &this->shadowDepthMap.id);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->shadowDepthMap.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->shadowDepthMap.id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->shader["default"]->use();
    this->shader["default"]->setIntUniformValue("shadowMap", 0);
    this->shader["default"]->setIntUniformValue("texture_diffuse1", 1);
    this->shader["default"]->setIntUniformValue("texture_normal1", 2);
    this->shader["default"]->setIntUniformValue("texture_specular1", 3);
    this->shader["default"]->setIntUniformValue("texture_emissive1", 4);
}

void    Renderer::initDepthMap( void ) {
    glEnable(GL_DEPTH_TEST);

    this->depthMap.width = this->env->getWindow().width;
    this->depthMap.height = this->env->getWindow().height;

    glGenFramebuffers(1, &this->depthMap.fbo);
    /* create depth texture */
    glGenTextures(1, &this->depthMap.id);
    glBindTexture(GL_TEXTURE_2D, this->depthMap.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->depthMap.width, this->depthMap.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->depthMap.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMap.id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->shader["fractal"]->use();
    this->shader["fractal"]->setIntUniformValue("depthBuffer", 0);
}
