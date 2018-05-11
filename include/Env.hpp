#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>

#include "Exception.hpp"
#include "Controller.hpp"
#include "Model.hpp"

typedef struct  s_window {
    GLFWwindow* ptr;
    int         width;
    int         height;
}               t_window;

class Env {

public:
    Env( void );
    ~Env( void );

    const t_window& getWindow( void ) const { return (window); };
    Controller*     getController( void ) { return (controller); };
    Model*          getModel( void ) { return (model); };

private:
    t_window    window;
    Controller* controller;
    Model*      model;

    void        initGlfwEnvironment( const std::string& glVersion = "4.0" );
    void        initGlfwWindow( size_t width, size_t height );
    void        setupController( void );
    // callback to be called each time the window is resized to update the viewport size as well
    static void framebufferSizeCallback( GLFWwindow* window, int width, int height );

};
