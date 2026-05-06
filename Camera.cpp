#include "Camera.hpp"
#include <cmath>

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->worldUp = glm::normalize(cameraUp);

        
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);

        
        if (glm::length(this->cameraFrontDirection) > 0.0001f) {
            pitch = glm::degrees(asinf(this->cameraFrontDirection.y));
            yaw = glm::degrees(atan2f(this->cameraFrontDirection.z, this->cameraFrontDirection.x));
        }

        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;
        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        case MOVE_UP:
            cameraPosition += worldUp * speed;   
            break;
        case MOVE_DOWN:
            cameraPosition -= worldUp * speed;
            break;
        }
    }

    void Camera::rotate(float pitchDelta, float yawDelta) {
        
        yawDelta *= mouseSensitivity;
        pitchDelta *= mouseSensitivity;

        yaw += yawDelta;
        pitch += pitchDelta;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

       
        if (yaw > 180.0f) yaw -= 360.0f;
        if (yaw < -180.0f) yaw += 360.0f;

        updateCameraVectors();
    }

    void Camera::updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, worldUp));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

}
