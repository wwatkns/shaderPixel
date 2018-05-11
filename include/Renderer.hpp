#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "Exception.hpp"
#include "Env.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model.hpp"

class Renderer {

public:
    Renderer( Env* env );
    ~Renderer( void );

    void	loop( void );

private:
    Env*                env;
    Camera              camera;
    // std::vector<Model*> models;
    Model               model;
    Shader              shader;

};
