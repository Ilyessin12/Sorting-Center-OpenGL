#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  1.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;
float groundLevelY;

//COLLISION DETECTION LOGICS
//radius
float cameraRadius = 0.3;

// Bounding box structure
struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

BoundingBox computeBoundingBox(const glm::vec3& position, const glm::vec3& scale) {
    glm::vec3 halfScale = scale * 0.5f;
    BoundingBox box;
    box.min = position - halfScale;
    box.max = position + halfScale;
    return box;
}

bool checkCollisionSphereAABB(const glm::vec3& sphereCenter, float sphereRadius, const BoundingBox& box) {
    glm::vec3 closestPoint;
    closestPoint.x = glm::clamp(sphereCenter.x, box.min.x, box.max.x);
    closestPoint.y = glm::clamp(sphereCenter.y, box.min.y, box.max.y);
    closestPoint.z = glm::clamp(sphereCenter.z, box.min.z, box.max.z);

    //use dot product to find the distance squared because no length2 somehow
    glm::vec3 difference = sphereCenter - closestPoint;
    float distanceSquared = glm::dot(difference, difference);
    return distanceSquared < (sphereRadius * cameraRadius);
}

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        groundLevelY = position.y; // Set ground level to initial Y position
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system.
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;

        // Movement directions projected onto the XZ plane
        glm::vec3 frontXZ = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 rightXZ = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

		//Vertical movement
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;

        // Keep the camera at the initial Y position (prevent vertical movement)
        Position.y = groundLevelY;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

	// Check if a sphere and an AABB are colliding (Collision detection)
    void ProcessKeyboardCollision(Camera_Movement direction, float deltaTime, const std::vector<BoundingBox>& worldBoundingBoxes)
    {
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 newPosition = Position;

        // Movement directions projected onto the XZ plane
        glm::vec3 frontXZ = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 rightXZ = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        if (direction == FORWARD)
            newPosition += frontXZ * velocity;
        if (direction == BACKWARD)
            newPosition -= frontXZ * velocity;
        if (direction == LEFT)
            newPosition -= rightXZ * velocity;
        if (direction == RIGHT)
            newPosition += rightXZ * velocity;

        // Keep the camera at the ground level Y position
        newPosition.y = Position.y; // Assuming you want to preserve the camera's Y position

        // Check for collisions with each bounding box
        bool collisionDetected = false;
        for (const BoundingBox& box : worldBoundingBoxes) {
            if (checkCollisionSphereAABB(newPosition, cameraRadius, box)) {
                collisionDetected = true;
                break;
            }
        }

        // If no collision, update the position
        if (!collisionDetected) {
            Position = newPosition;
        }
        // Optionally, implement sliding along the surfaces if a collision is detected
    }


private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
