#include "Camera.hpp"
#include "glm/ext.hpp"

Camera::Camera( float fov, float aspect, float near, float far ) : aspect(aspect), fov(fov), near(near), far(far) {
    this->projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
    this->position = glm::vec3(0.0f, 0.0f, 8.0f);
    this->target = glm::vec3(0.0f, 0.0f, 0.0f);
    this->viewMatrix = glm::lookAt(this->position, this->target, glm::vec3(0.0f, 1.0f, 0.0f));
}

Camera::Camera( const Camera& rhs ) {
    *this = rhs;
}

Camera& Camera::operator=( const Camera& rhs ) {
    this->projectionMatrix = rhs.getProjectionMatrix();
    this->viewMatrix = rhs.getViewMatrix();
    this->position = rhs.getPosition();
    this->target = rhs.getTarget();
    this->fov = rhs.getFov();
    this->aspect = rhs.getAspect();
    this->near = rhs.getNear();
    this->far = rhs.getFar();
    return (*this);
}

Camera::~Camera( void ) {
}

void    Camera::setFov( float fov ) {
    this->fov = fov;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
}

void    Camera::setAspect( float aspect ) {
    this->aspect = aspect;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
}

void    Camera::setNear( float near ) {
    this->near = near;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
}

void    Camera::setFar( float far ) {
    this->far = far;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
}

void    Camera::handleKeys( const std::array<tKey, N_KEY>& keys ) {
    glm::vec4    translate(
        (float)(keys[GLFW_KEY_A].value - keys[GLFW_KEY_D].value),
        (float)(keys[GLFW_KEY_LEFT_SHIFT].value - keys[GLFW_KEY_SPACE].value),
        (float)(keys[GLFW_KEY_W].value - keys[GLFW_KEY_S].value),
        1.0f
    );
    /* translation is in the same coordinate system as view (moves in same direction) */
    translate = glm::transpose(this->viewMatrix) * glm::normalize(translate);
    this->target = this->target + glm::vec3(this->viewMatrix * glm::vec4(0, 0, -1, 0));
    this->position = this->position - glm::vec3(translate) * 0.5f;
    this->viewMatrix = glm::lookAt(this->position, this->target, glm::vec3(0, 1, 0));
}

// glm::vec3    Camera::interpolate( const glm::vec3& v0, const glm::vec3& v1, tTimePoint last, size_t duration ) {
//     float t = (1 - ((float)duration - this->getElapsedMilliseconds(last).count()) / (float)duration);
//     t = std::min(t, 1.0f);
//     return (mtls::lerp(v0, v1, t));
// }

tMilliseconds   Camera::getElapsedMilliseconds( tTimePoint last ) {
    return (std::chrono::steady_clock::now() - last);
}
