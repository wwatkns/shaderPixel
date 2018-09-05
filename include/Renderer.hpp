#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <thread>

#include "Exception.hpp"
#include "Env.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "VideoCapture.hpp"

typedef struct  sDepthMap {
    unsigned int    id;
    unsigned int    fbo;
    size_t          width;
    size_t          height;
}               tDepthMap;

typedef std::unordered_map<std::string, Shader*> tShaderMap;
typedef std::chrono::duration<double,std::milli> tMilliseconds;
typedef std::chrono::steady_clock::time_point tTimePoint;

class Renderer {

public:
    Renderer( Env* env );
    ~Renderer( void );

    void	loop( void );
    void    updateShadowDepthMap( void );
    void    renderLights( void );
    void    renderMeshes( void );
    void    renderSkybox( void );
    void    renderShaders( void );

private:
    Env*            env;
    Camera          camera;
    tShaderMap      shader;
    tDepthMap       depthMap;       // depth-map for the view fustrum
    tDepthMap       shadowDepthMap; // depth-map for the shadows
    glm::mat4       lightSpaceMat;
    int             useShadows;
    float           framerate;
    VideoCapture*   videoCapture;

    tTimePoint      lastTime;

    void    initShadowDepthMap( const size_t width = 1024, const size_t height = 1024 );
    void    initDepthMap( void );

};
