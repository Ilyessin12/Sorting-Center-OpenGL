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
// Fungsi untuk mengecek collision dengan bounding box sederhana
bool checkCollision(const glm::vec3& position, const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax) {

    return (position.x >= boundingBoxMin.x && position.x <= boundingBoxMax.x) &&
        (position.y >= boundingBoxMin.y && position.y <= boundingBoxMax.y) &&
        (position.z >= boundingBoxMin.z && position.z <= boundingBoxMax.z);
}

// Fungsi untuk mengecek collision dengan semua objek
bool checkCollisionWithObjects(const glm::vec3& newPosition) {
    float margin = 0.1f; // Margin untuk mencegah kamera terlalu dekat dengan objek

	//Manual assigning collision box for each object
    // Conveyor utama
    if (checkCollision(newPosition,
        glm::vec3(-5.33f, -0.01f, 0.26f),
        glm::vec3(-1.51f, 0.93f, 0.97f))) return true;

    // Forklift
    if (checkCollision(newPosition,
        glm::vec3(-4.88f, 0.00f, -3.82f),
        glm::vec3(-4.50f, 0.98f, -2.32f))) return true;

    // Cloth
    if (checkCollision(newPosition,
        glm::vec3(-1.51f, 0.04f, -1.66f),
        glm::vec3(-0.55f, 0.65f, -1.06f))) return true;

    // Dinding L
    if (checkCollision(newPosition,
        glm::vec3(-2.2f, 0.03f, -0.78f),
        glm::vec3(5.8f, 1.04f, 3.9f))) return true;

    // Dinding kanan conveyor
    if (checkCollision(newPosition,
        glm::vec3(-6.2f, 0.03f, -4.76f),
        glm::vec3(-5.1f, 1.98f, 3.25f))) return true;

    // Dinding kanan rak
    if (checkCollision(newPosition,
        glm::vec3(-5.25f, 0.03f, -5.7f),
        glm::vec3(5.43f, 1.01f, -4.7f))) return true;

    // Dinding belakang
    if (checkCollision(newPosition,
        glm::vec3(5.35f, 0.03f, -4.70f),
        glm::vec3(5.81f, 1.0f, -0.79f))) return true;

    // Rak Systems
    // Rak 1
    if (checkCollision(newPosition,
        glm::vec3(-2.0f, -0.01f, -2.17f),
        glm::vec3(3.35f, 0.73f, -1.79f))) return true;

    // Rak 2
    if (checkCollision(newPosition,
        glm::vec3(-1.96f, 0.01f, -3.21f),
        glm::vec3(3.37f, 0.72f, -2.78f))) return true;

    // Rak 3
    if (checkCollision(newPosition,
        glm::vec3(-2.01f, -0.01f, -4.19f),
        glm::vec3(3.37f, 0.69f, -3.80f))) return true;

    return false;
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
		// Set the velocity of the camera
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 newPosition = Position;

        // Movement directions projected onto the XZ plane
        glm::vec3 frontXZ = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 rightXZ = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        if (direction == FORWARD)
            newPosition += Front * velocity;
        if (direction == BACKWARD)
            newPosition -= Front * velocity;
        if (direction == LEFT)
            newPosition -= Right * velocity;
        if (direction == RIGHT)
            newPosition += Right * velocity;

		//Vertical movement
        if (direction == UP)
            newPosition += WorldUp * velocity;
        if (direction == DOWN)
            newPosition -= WorldUp * velocity;

        // Keep the camera at the initial Y position (prevent vertical movement)
        newPosition.y = groundLevelY;

        // Check collision before updating position
        if (!checkCollisionWithObjects(newPosition)) {
            Position = newPosition;
        }
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
