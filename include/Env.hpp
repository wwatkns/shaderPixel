#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

#include "Exception.hpp"
#include "Controller.hpp"
#include "Model.hpp"
#include "RaymarchedObject.hpp"
#include "Light.hpp"

typedef struct  s_window {
    GLFWwindow* ptr;
    int         width;
    int         height;
}               t_window;

class Env {

public:
    Env( void );
    ~Env( void );

    const t_window&         getWindow( void ) const { return (window); };
    Controller*             getController( void ) { return (controller); };
    std::vector<Model*>&    getModels( void ) { return (models); };
    std::vector<RaymarchedObject*>&    getRaymarchedObjects( void ) { return (raymarchedObjects); };
    std::vector<Light*>&    getLights( void ) { return (lights); };
    Model*                  getSkybox( void ) { return (skybox); };
    Light*                  getDirectionalLight( void );

    // Model*              quad; // NEW

private:
    t_window            window;
    Controller*         controller;
    std::vector<Model*> models;
    std::vector<Light*> lights;
    Model*              skybox;
    std::vector<RaymarchedObject*> raymarchedObjects;

    void        initGlfwEnvironment( const std::string& glVersion = "4.0" );
    void        initGlfwWindow( size_t width, size_t height );
    void        setupController( void );
    // callback to be called each time the window is resized to update the viewport size as well
    static void framebufferSizeCallback( GLFWwindow* window, int width, int height );

};
