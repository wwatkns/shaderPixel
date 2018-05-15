#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Exception.hpp"
#include "Shader.hpp"

enum class eLightType {
    directional,
    point,
};

class Light {

public:
    Light( const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, eLightType type );
    Light( const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float lconst, float linear, float quadratic, eLightType type );
    ~Light( void );

    // void            update( void );
    void            render( Shader shader );

    /* getters */
    const eLightType    getType( void ) const { return (type); };
    const glm::vec3&    getPosition( void ) const { return (position); };
    const glm::vec3&    getAmbient( void ) const { return (ambient); };
    const glm::vec3&    getDiffuse( void ) const { return (diffuse); };
    const glm::vec3&    getSpecular( void ) const { return (specular); };
    /* setters */
    void                setPosition( const glm::vec3& t ) { position = t; };
    void                setAmbient( const glm::vec3& t ) { ambient = t; };
    void                setDiffuse( const glm::vec3& t ) { diffuse = t; };
    void                setSpecular( const glm::vec3& t ) { specular = t; };

    static int          pointLightCount;

private:
    glm::vec3           direction;
    glm::vec3           position;
    glm::vec3           ambient;
    glm::vec3           diffuse;
    glm::vec3           specular;
    eLightType          type;

    float               lconst;
    float               linear;
    float               quadratic;
    int                 id;
};
