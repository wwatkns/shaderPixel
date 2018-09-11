#include "Camera.hpp"
#include "glm/ext.hpp"

Camera::Camera( float fov, float aspect, float near, float far ) : aspect(aspect), fov(fov), near(near), far(far) {
    this->projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
    this->invProjectionMatrix = glm::inverse(this->projectionMatrix);
    this->position = glm::vec3(0.0f, 0.0f, 3.0f);
    this->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    this->viewMatrix = glm::lookAt(this->position, this->position + this->cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));
    this->invViewMatrix = glm::inverse(this->viewMatrix);
    this->pitch = 0;
    this->yaw = 0;
    this->last = std::chrono::steady_clock::now();
    this->speed = 0.02;
    this->speedmod = 1.0;
}

Camera::Camera( const Camera& rhs ) {
    *this = rhs;
}

Camera& Camera::operator=( const Camera& rhs ) {
    this->projectionMatrix = rhs.getProjectionMatrix();
    this->viewMatrix = rhs.getViewMatrix();
    this->position = rhs.getPosition();
    this->cameraFront = rhs.getCameraFront();
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
    this->invProjectionMatrix = glm::inverse(this->projectionMatrix);
}

void    Camera::setAspect( float aspect ) {
    this->aspect = aspect;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
    this->invProjectionMatrix = glm::inverse(this->projectionMatrix);
}

void    Camera::setNear( float near ) {
    this->near = near;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
    this->invProjectionMatrix = glm::inverse(this->projectionMatrix);
}

void    Camera::setFar( float far ) {
    this->far = far;
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->near, this->far);
    this->invProjectionMatrix = glm::inverse(this->projectionMatrix);
}

void    Camera::handleInputs( const std::array<tKey, N_KEY>& keys, const tMouse& mouse ) {
    this->handleKeys(keys);
    this->handleMouse(mouse);
    this->viewMatrix = glm::lookAt(this->position, this->position + this->cameraFront, glm::vec3(0, 1, 0));
    this->invViewMatrix = glm::inverse(this->viewMatrix);
    this->last = std::chrono::steady_clock::now();
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
    this->position = this->position - glm::vec3(translate) * this->getElapsedMilliseconds(this->last).count() * this->speed * this->speedmod;
}

void    Camera::handleMouse( const tMouse& mouse, float sensitivity ) {
    this->pitch += (mouse.prevPos.y - mouse.pos.y) * sensitivity;
    this->pitch = std::min(std::max(this->pitch, -89.0f), 89.0f);
    this->yaw += (mouse.pos.x - mouse.prevPos.x) * sensitivity;
    glm::vec3 front(
        std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw)),
        std::sin(glm::radians(pitch)),
        std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw))
    );
    this->cameraFront = glm::normalize(front);
}

tMilliseconds   Camera::getElapsedMilliseconds( tTimePoint last ) {
    return (std::chrono::steady_clock::now() - last);
}
