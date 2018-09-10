#include "Renderer.hpp"
#include "glm/ext.hpp"

Renderer::Renderer( Env* env ) :
env(env),
camera(75, (float)env->getWindow().width / (float)env->getWindow().height) {
    this->shader["default"] = new Shader("./shader/vertex/default.vert.glsl", "./shader/fragment/default.frag.glsl");
    this->shader["skybox"]  = new Shader("./shader/vertex/skybox.vert.glsl", "./shader/fragment/skybox.frag.glsl");
    this->shader["shadowMap"] = new Shader("./shader/vertex/shadowMap.vert.glsl", "./shader/fragment/shadowMap.frag.glsl");
    this->shader["raymarch"] = new Shader("./shader/vertex/raymarch.vert.glsl", "./shader/fragment/raymarch.frag.glsl");
    this->shader["raymarchOnSurface"] = new Shader("./shader/vertex/raymarchSurface.vert.glsl", "./shader/fragment/raymarchSurface.frag.glsl");
    this->shader["2Dtexture"] = new Shader("./shader/vertex/raymarchSurface.vert.glsl", "./shader/fragment/2Dtexture.frag.glsl");
    this->shader["blendTexture"] = new Shader("./shader/vertex/raymarch.vert.glsl", "./shader/fragment/blendTexture.frag.glsl");
    this->lastTime = std::chrono::steady_clock::now();
    this->framerate = 60.0;

    this->initDepthMap();
    this->initShadowDepthMap(4096, 4096);
    this->initRenderbuffer();
    this->initIntermediateTexture();
    
    this->videoCapture = NULL;
    #if 0
    this->videoCapture = new VideoCapture(
        "./test.mp4", // .mov
        this->env->getWindow().width,
        this->env->getWindow().height,
        this->framerate,
        eCodec::avc1,
        16.0
    );
    #endif

    this->useShadows = 0;
}

Renderer::~Renderer( void ) {
    if (this->videoCapture)
        delete this->videoCapture;
}

void	Renderer::loop( void ) {
    static int frames = 0;
    static double last = 0.0;
    glEnable(GL_DEPTH_TEST); /* z-buffering */
    glEnable(GL_FRAMEBUFFER_SRGB); /* gamma correction */
    glEnable(GL_BLEND); /* transparency */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(this->env->getWindow().ptr)) {
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->env->getController()->update();
        this->camera.speedmod = this->env->getRaymarched()->computeSpeedModifier(this->camera.getPosition());
        this->camera.handleInputs(this->env->getController()->getKeys(), this->env->getController()->getMouse());
        this->useShadows = this->env->getController()->getKeyValue(GLFW_KEY_P);

        this->env->getDirectionalLight()->setPosition(
            glm::vec3(glm::sin(glfwGetTime() * 0.125 + 2.) * 50., 20., glm::cos(glfwGetTime()*0.125 + 2.) * 50.)
        );
        /* rendering passes */
        this->updateShadowDepthMap();
        this->renderLights();
        this->renderMeshes();
        this->renderSkybox();
        /* dumb renderbuffer pass... */
        glBindFramebuffer(GL_FRAMEBUFFER, this->renderbuffer.fbo);
        glClear(GL_COLOR_BUFFER_BIT);
        this->render2Dtexture();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, this->renderbuffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->intermediateTexture.fbo);
        glBlitFramebuffer(0, 0, this->renderbuffer.width, this->renderbuffer.height, 0, 0, this->renderbuffer.width, this->renderbuffer.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        this->renderBlendTexture();
        /* order has importance for occlusion */
        this->renderRaymarchedSurfaces();
        this->renderRaymarched();

        glfwSwapBuffers(this->env->getWindow().ptr);
        /* capture video frames */
        if (this->videoCapture)
            this->videoCapture->write();
        /* display framerate */
        tTimePoint current = std::chrono::steady_clock::now();
        frames++;
        if ((static_cast<tMilliseconds>(current - this->lastTime)).count() > 999) {
            std::cout << frames << " fps" << std::endl;
            this->lastTime = current;
            frames = 0;
        }
        /* cap framerate */
        double delta = std::abs(glfwGetTime()/1000.0 - last);
        if (delta < (1000. / this->framerate))
            std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(1000. / this->framerate - delta)));
        last = glfwGetTime()/1000.0;
    }
}

void    Renderer::updateShadowDepthMap( void ) {
    Light*  directionalLight = this->env->getDirectionalLight();
    if (this->useShadows && directionalLight) {
        glm::mat4 lightProjection, lightView;
        lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, this->camera.getNear(), this->camera.getFar());
        lightView = glm::lookAt(directionalLight->getPosition(), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        this->lightSpaceMat = lightProjection * lightView;
        /* render scene from light's point of view */
        this->shader["shadowMap"]->use();
        this->shader["shadowMap"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);

        glViewport(0, 0, this->shadowDepthMap.width, this->shadowDepthMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, this->shadowDepthMap.fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        if (this->env->getModels().size() != 0)
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

    /* render lights for meshes */
    for (auto it = this->env->getLights().begin(); it != this->env->getLights().end(); it++)
        (*it)->render(*this->shader["default"]);

    /* render lights for raymarched objects */
    this->shader["raymarch"]->use();
    for (auto it = this->env->getLights().begin(); it != this->env->getLights().end(); it++)
        (*it)->render(*this->shader["raymarch"]);
    
    /* render lights for raymarched object on surfaces */
    this->shader["raymarchOnSurface"]->use();
    for (auto it = this->env->getLights().begin(); it != this->env->getLights().end(); it++)
        (*it)->render(*this->shader["raymarchOnSurface"]);
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

    if (this->env->getModels().size() != 0)
        /* render models */
        for (auto it = this->env->getModels().begin(); it != this->env->getModels().end(); it++)
            (*it)->render(*this->shader["default"]);

    /* copy the depth buffer to a texture (used in raymarch shader for geometry occlusion of raymarched objects) */
    glBindTexture(GL_TEXTURE_2D, this->depthMap.id);
    glReadBuffer(GL_FRONT);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, this->depthMap.width, this->depthMap.height, 0);
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

void    Renderer::renderRaymarched( void ) {
    glDisable(GL_DEPTH_TEST);

    this->shader["raymarch"]->use();
    this->shader["raymarch"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);
    this->shader["raymarch"]->setIntUniformValue("use_shadows", this->useShadows);
    this->shader["raymarch"]->setMat4UniformValue("invProjection", this->camera.getInvProjectionMatrix());
    this->shader["raymarch"]->setMat4UniformValue("invView", this->camera.getInvViewMatrix());
    this->shader["raymarch"]->setFloatUniformValue("near", this->camera.getNear());
    this->shader["raymarch"]->setFloatUniformValue("far", this->camera.getFar());
    this->shader["raymarch"]->setVec3UniformValue("cameraPos", this->camera.getPosition());

    this->shader["raymarch"]->setVec2UniformValue("uMouse", this->env->getController()->getMousePosition());
    this->shader["raymarch"]->setFloatUniformValue("uTime", glfwGetTime());

    /* geometry depth-buffer */
    glActiveTexture(GL_TEXTURE0);
    this->shader["raymarch"]->setIntUniformValue("depthBuffer", 0);
    glBindTexture(GL_TEXTURE_2D, this->depthMap.id);

    glActiveTexture(GL_TEXTURE1);
    this->shader["raymarch"]->setIntUniformValue("shadowMap", 1);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);

    if (this->env->getRaymarched())
        this->env->getRaymarched()->render(*this->shader["raymarch"]);

    glEnable(GL_DEPTH_TEST);
}

void    Renderer::renderRaymarchedSurfaces( void ) {
    this->shader["raymarchOnSurface"]->use();
    this->shader["raymarchOnSurface"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    this->shader["raymarchOnSurface"]->setMat4UniformValue("view", this->camera.getViewMatrix());
    this->shader["raymarchOnSurface"]->setVec3UniformValue("cameraPos", this->camera.getPosition());
    this->shader["raymarchOnSurface"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);

    this->shader["raymarchOnSurface"]->setIntUniformValue("use_shadows", this->useShadows);
    this->shader["raymarchOnSurface"]->setMat4UniformValue("invProjection", this->camera.getInvProjectionMatrix());
    this->shader["raymarchOnSurface"]->setMat4UniformValue("invView", this->camera.getInvViewMatrix());
    this->shader["raymarchOnSurface"]->setFloatUniformValue("near", this->camera.getNear());
    this->shader["raymarchOnSurface"]->setFloatUniformValue("far", this->camera.getFar());
    this->shader["raymarchOnSurface"]->setFloatUniformValue("uTime", glfwGetTime());

    glActiveTexture(GL_TEXTURE0);
    this->shader["raymarchOnSurface"]->setIntUniformValue("shadowMap", 0);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);

    if (this->env->getRaymarchedSurfaces().size() != 0)
        /* render models */
        for (auto it = this->env->getRaymarchedSurfaces().begin(); it != this->env->getRaymarchedSurfaces().end(); it++)
            (*it)->render(*this->shader["raymarchOnSurface"]);
}

void    Renderer::render2Dtexture( void ) {
    this->shader["2Dtexture"]->use();
    this->shader["2Dtexture"]->setMat4UniformValue("lightSpaceMat", this->lightSpaceMat);
    this->shader["2Dtexture"]->setMat4UniformValue("projection", this->camera.getProjectionMatrix());
    this->shader["2Dtexture"]->setMat4UniformValue("view", this->camera.getViewMatrix());
    this->shader["2Dtexture"]->setFloatUniformValue("near", this->camera.getNear());
    this->shader["2Dtexture"]->setFloatUniformValue("far", this->camera.getFar());
    this->shader["2Dtexture"]->setFloatUniformValue("uTime", glfwGetTime());

    glActiveTexture(GL_TEXTURE0);
    this->shader["2Dtexture"]->setIntUniformValue("shadowMap", 0);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);

    if (this->env->getTexturedSurfaces().size() != 0)
        for (auto it = this->env->getTexturedSurfaces().begin(); it != this->env->getTexturedSurfaces().end(); it++)
            (*it)->render(*this->shader["2Dtexture"]);
}

void    Renderer::renderBlendTexture( void ) {
    // glDisable(GL_DEPTH_TEST);

    this->shader["blendTexture"]->use();
    this->shader["blendTexture"]->setFloatUniformValue("near", this->camera.getNear());
    this->shader["blendTexture"]->setFloatUniformValue("far", this->camera.getFar());

    glActiveTexture(GL_TEXTURE0);
    this->shader["blendTexture"]->setIntUniformValue("tex", 0);
    glBindTexture(GL_TEXTURE_2D, this->intermediateTexture.id);

    if (this->env->getRaymarched())
        this->env->getRaymarched()->render(*this->shader["blendTexture"]);

    // glEnable(GL_DEPTH_TEST);
}

void    Renderer::initShadowDepthMap( const size_t width, const size_t height ) {
    this->shadowDepthMap.width = width;
    this->shadowDepthMap.height = height;

    glGenFramebuffers(1, &this->shadowDepthMap.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->shadowDepthMap.fbo);
    /* create depth texture */
    glGenTextures(1, &this->shadowDepthMap.id);
    glBindTexture(GL_TEXTURE_2D, this->shadowDepthMap.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
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
    this->depthMap.width = this->env->getWindow().width;
    this->depthMap.height = this->env->getWindow().height;

    glGenFramebuffers(1, &this->depthMap.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->depthMap.fbo);
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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMap.id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->shader["raymarch"]->use();
    this->shader["raymarch"]->setIntUniformValue("depthBuffer", 0);
}

void    Renderer::initRenderbuffer( void ) { 
    this->renderbuffer.width = this->env->getWindow().width;
    this->renderbuffer.height = this->env->getWindow().height;

    /* create RenderBuffer */
    glGenRenderbuffers(1, &this->renderbuffer.id);
    glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuffer.id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, this->renderbuffer.width, this->renderbuffer.height);

    /* create FrameBuffer, with renderbuffer binded */
    glGenFramebuffers(1, &this->renderbuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->renderbuffer.fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, this->renderbuffer.id);
    /* attach depth-buffer component that is also associated to another FBO (one depth-buffer, multiple Fbos) */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMap.id, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void    Renderer::initIntermediateTexture( void ) {
    this->intermediateTexture.width = this->env->getWindow().width;
    this->intermediateTexture.height = this->env->getWindow().height;

    glGenFramebuffers(1, &this->intermediateTexture.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->intermediateTexture.fbo);
    /* create depth texture */
    glGenTextures(1, &this->intermediateTexture.id);
    glBindTexture(GL_TEXTURE_2D, this->intermediateTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->intermediateTexture.width, this->intermediateTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // attach depth texture as FBO's color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->intermediateTexture.id, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->shader["blendTexture"]->use();
    this->shader["blendTexture"]->setIntUniformValue("tex", 0);
}