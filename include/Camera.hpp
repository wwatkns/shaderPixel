#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include "Exception.hpp"
#include "Controller.hpp"

typedef std::chrono::duration<double,std::milli> tMilliseconds;
typedef std::chrono::steady_clock::time_point tTimePoint;

class Camera {

public:
    Camera( float fov, float aspect, float near = 0.1f, float far = 100.0f );
    Camera( const Camera& rhs );
    Camera& operator=( const Camera& rhs );
    ~Camera( void );

    // glm::vec3            interpolate( const glm::vec3& v0, const glm::vec3& v1, tTimePoint last, size_t duration = 250 );
    tMilliseconds       getElapsedMilliseconds( tTimePoint last );

    void                handleInputs( const std::array<tKey, N_KEY>& keys, const tMouse& mouse );
    /* Setters */
    void                setFov( float fov );
    void                setAspect( float aspect );
    void                setNear( float near );
    void                setFar( float far );
    /* Getters */
    const glm::mat4&    getProjectionMatrix( void ) const { return (projectionMatrix); };
    const glm::mat4&    getInvProjectionMatrix( void ) const { return (invProjectionMatrix); };
    const glm::mat4&    getViewMatrix( void ) const { return (viewMatrix); };
    const glm::mat4&    getInvViewMatrix( void ) const { return (invViewMatrix); };
    const glm::vec3&    getPosition( void ) const { return (position); };
    const glm::vec3&    getCameraFront( void ) const { return (cameraFront); };
    glm::vec3           getCameraRight( void ) const;
    glm::vec3           getCameraTop( void ) const;
    const float         getFov( void ) const { return (fov); };
    const float         getAspect( void ) const { return (aspect); };
    const float         getNear( void ) const { return (near); };
    const float         getFar( void ) const { return (far); };

private:
    glm::mat4   projectionMatrix;
    glm::mat4   invProjectionMatrix;
    glm::mat4   viewMatrix;
    glm::mat4   invViewMatrix;
    glm::vec3   position;
    glm::vec3   cameraFront;
    float       fov;
    float       aspect;
    float       near;
    float       far;

    float       pitch;
    float       yaw;

    void                handleKeys( const std::array<tKey, N_KEY>& keys );
    void                handleMouse( const tMouse& mouse, float sensitivity = 0.1f );
};
