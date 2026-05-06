#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN };

    class Camera {

    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        glm::mat4 getViewMatrix();

        
        void move(MOVE_DIRECTION direction, float speed);

        
        void rotate(float pitchDelta, float yawDelta);

        glm::vec3 getPosition() const { return cameraPosition; }
        glm::vec3 getFrontDirection() const { return cameraFrontDirection; }
        glm::vec3 getRightDirection() const { return cameraRightDirection; }
        glm::vec3 getUpDirection() const { return cameraUpDirection; }

        
        glm::vec3 cameraFrontDirection;

    private:
        void updateCameraVectors();

        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;

        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;

        
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        float yaw = -90.0f;
        float pitch = 0.0f;

       
        float mouseSensitivity = 0.12f;
    };
}

#endif /* Camera_hpp */
