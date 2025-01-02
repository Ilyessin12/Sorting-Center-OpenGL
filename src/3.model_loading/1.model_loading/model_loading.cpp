#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

//function to update the windows title so we could see the x y z value in real time
void updateWindowTitle(GLFWwindow* window, const Camera& camera)
{
	// Update window title with camera position, use stringstream to format the string
    std::stringstream ss;
    ss << "Sorting-Center-|-Camera-Position: ("
        << std::fixed << std::setprecision(2)
        << camera.Position.x << ", "
        << camera.Position.y << ", "
        << camera.Position.z << ")";
	// Set the window title to the new string
    glfwSetWindowTitle(window, ss.str().c_str());
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-3.70f, 0.30f, -3.08f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//model
std::vector<Model> models;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//BOX MOVEMENT LOGICS
//movement speed for box object logic, alter if needed
float moveSpeed = 1.0f;
//movement logic for box object
bool MoveTowards(glm::vec3& currentPos, const glm::vec3& targetPos, float speed, float deltaTime)
{
    glm::vec3 direction = targetPos - currentPos;
    float distance = glm::length(direction);
    if (distance <= 0.001f) // Threshold to consider as reached
    {
        currentPos = targetPos;
        return true;
    }

    glm::vec3 move = glm::normalize(direction) * speed * deltaTime;
    if (glm::length(move) >= distance)
    {
        currentPos = targetPos;
        return true;
    }
    else
    {
        currentPos += move;
        return false;
    }
}

//simple jump mechanics
bool isJumping = false;  //to check if the player is currently jumping or not
float jumpVelocity = 0.0f; //to store the current jump velocity
const float gravity = -9.81f; //gravity value
const float jumpStrength = 3.0f;  //jump strength, can be adjusted
float groundLevel = camera.Position.y; //ground level so we cannot float freely in the air
float baseYPosition = 0.30f; // Initial Y position (matches camera's starting Y)
glm::vec3 cameraVelocity = glm::vec3(0.0f); // Camera's current velocity to apply it to the position



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sorting-Center", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------

	// Shaders for the models
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");

    // load models
    // -----------

    // Background [0-1]
    models.push_back(Model(FileSystem::getPath("resources/objects/building/WallAndFloor/dindingdanlantai.obj")));   // Dinding
    models.push_back(Model(FileSystem::getPath("resources/objects/building/roof/atap.obj")));                       // Atap

    // Box spawned [2-3]
    models.push_back(Model(FileSystem::getPath("resources/objects/boxdetail/boxbesarterbuka/boxbesarterbuka.obj")));    // Box besar
    models.push_back(Model(FileSystem::getPath("resources/objects/boxdetail/boxkeciltertutup/boxkeciltertutup.obj")));  // Box kecil

    // Conveyor [4-5]
    models.push_back(Model(FileSystem::getPath("resources/objects/conveyor/newconveyor/untitled.obj")));            // Frame
    models.push_back(Model(FileSystem::getPath("resources/objects/conveyor/newconveyor/belt/beltconveyor.obj")));   // Belt

    // Spawner [6-7]
    models.push_back(Model(FileSystem::getPath("resources/objects/spawnernew/untitled.obj")));  // Kanan
    models.push_back(Model(FileSystem::getPath("resources/objects/spawnernew/untitled.obj")));  // Kiri

    // Conveyor tinggi [8-9]
    models.push_back(Model(FileSystem::getPath("resources/objects/conveyor/untitled.obj")));        // Frame
    models.push_back(Model(FileSystem::getPath("resources/objects/conveyortinggi/belt/belt.obj"))); // belt

    // Spawner [10]
    models.push_back(Model(FileSystem::getPath("resources/objects/spawnernew/untitled.obj")));

    // Rak 1 [11-16]
    models.push_back(Model(FileSystem::getPath("resources/objects/rak/rak_tangga/rak_tangga.obj")));

    // Box Rak 1 [17-36]
    models.push_back(Model(FileSystem::getPath("resources/objects/boxdetail/boxkeciltertutup/boxkeciltertutup.obj")));     // Frame
    models.push_back(Model(FileSystem::getPath("resources/objects/boxdetail/boxkeciltertutup/boxkeciltertutup.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/boxdetail/boxkeciltertutup/boxkeciltertutup.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/2boxbesar/duaboxbesar.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/2boxbesar/duaboxbesar.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/2boxbesar/duaboxbesar.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/2boxbesar/duaboxbesar.obj")));

    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));
    models.push_back(Model(FileSystem::getPath("resources/objects/kumpulan_box/4boxbesartengahkosong/4boxbesartengahkosong.obj")));



    // Rak 2 [37]
    models.push_back(Model(FileSystem::getPath("resources/objects/rak/rakonly.obj")));     // Frame

    // Rak 3 [38]
    models.push_back(Model(FileSystem::getPath("resources/objects/rak/rakonly.obj")));     // Frame

    /***********************************************************************/
	// Decorations [39-41]
    models.push_back(Model(FileSystem::getPath("resources/objects/Decorations/cloth/cloth.obj")));     // Frame
    models.push_back(Model(FileSystem::getPath("resources/objects/Decorations/pallet/pallet.obj")));     // Frame
    models.push_back(Model(FileSystem::getPath("resources/objects/Decorations/forklift/forklift.obj")));     // Frame

    //position for each models
    std::vector<glm::vec3> modelPositions = {
        // horizontal | Vertikal | Far
                  // Background
        glm::vec3(0.0f, 0.0f, 0.0f),    // Frame
        glm::vec3(0.0f, -3.8f, 3.0f),    // atap

        // Box
        glm::vec3(-5.5f, 0.2f, 0.8f),    // besar
        glm::vec3(-4.3f, 0.55f, 3.43f),    // kecil


        // Conveyor
        glm::vec3(-3.5f, -0.012f, 0.55f),    // Frame
        glm::vec3(3.5f, 0.079f, -0.69f),    // belt 

        // Spawner conveyor
        glm::vec3(-5.16f, 0.14f, 0.63f),    // Kanan
        glm::vec3(2.17f, 0.14f, -0.63f),    // Kiri

        // Conveyor tinggi
        glm::vec3(-4.5f, 0.048f, 1.43f),    // Frame
        glm::vec3(-4.5f, -0.13f, 1.32f),    // belt


        glm::vec3(-3.2f, 0.48f, -4.53f),    // belt

        // Rak 1
        glm::vec3(-1.5f, 0.02f, -4.0f),

        // Box Rak 1 (1 DEPAN, 2 TENGAH, 3 BELAKANG)
        glm::vec3(4.4f, 0.058f, -1.6f), 
        glm::vec3(4.4f, 0.058f, -1.3f), 
        glm::vec3(4.4f, 0.058f, -1.0f), 
        glm::vec3(3.97f, 0.07f, -0.67f),
        glm::vec3(3.97f, 0.07f, 1.97f),
        glm::vec3(2.97f, 0.07f, -1.57f),
        glm::vec3(2.97f, 0.07f, 1.963f),
        glm::vec3(1.97f, 0.07f, -0.67f),
        glm::vec3(1.97f, 0.07f, 1.97f),
        glm::vec3(2.97f, 0.545f, -0.65f),
        glm::vec3(2.97f, 0.545f, 1.94f),
        glm::vec3(1.97f, 0.545f, -1.557f),
        glm::vec3(2.97f, 0.302f, -1.55f),
        glm::vec3(1.97f, 0.545f, 1.557f),
        glm::vec3(3.97f, 0.302f, 1.557f),
        glm::vec3(3.97f, 0.545f, -1.557f),
        glm::vec3(1.97f, 0.302f, 0.675f),
        glm::vec3(3.97f, 0.302f, -1.557f),
        glm::vec3(2.97f, 0.302f, 0.875f),

        // Rak 2
        glm::vec3(-1.5f, 0.02f, -3.0f),

        // Rak 3
        glm::vec3(-1.5f, 0.02f, -2.0f),

        // Cloth
        glm::vec3(-1.0f, 0.02f, -1.2f),

        // Pallet
        glm::vec3(4.9f, 0.03f, -1.0f),

        // Forklift
        glm::vec3(-4.7f, 0.0f, -3.2f),
    };

    // scale for each models
    std::vector<glm::vec3> modelScales = {
        // Background (dinding, atap)
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.17f, 0.17f, 0.17f),    // Belt

        // Box
        glm::vec3(0.45f, 0.45f, 0.45f),    // Besar
        glm::vec3(0.26f, 0.26f, 0.26f),    // Kecil

        // Conveyor
        glm::vec3(0.04f, 0.04f, 0.04f),    // Frame
        glm::vec3(0.04f, 0.04f, 0.04f),    // Belt

        // Spawner conveyor
        glm::vec3(0.01f, 0.06f, 0.04f),     // Kanan
        glm::vec3(0.01f, 0.06f, 0.04f),     // Kiri

        // Conveyor tinggi
        glm::vec3(0.04f, 0.04f, 0.04f),    // Frame
        glm::vec3(0.04f, 0.04f, 0.04f),    // Belt

        // Spawner conveyor tinggi
        glm::vec3(0.01f, 0.035f, 0.035f),    // Belt

        // Rak 1 (6)
        glm::vec3(0.17f, 0.17f, 0.17f),

        // Box Rak 1(3)
         glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),glm::vec3(0.5f, 0.5f, 0.5f),

         // Rak 2 (6)
         glm::vec3(0.17f, 0.17f, 0.17f),
         // Rak 3
         glm::vec3(0.17f, 0.17f, 0.17f),

         // Cloth
         glm::vec3(0.22f, 0.22f, 0.22f),

         // Pallet
         glm::vec3(0.05f, 0.05f, 0.05f),

         glm::vec3(0.11f, 0.11f, 0.11f),
    };


    //index for the model that wants its texture to be animated
    std::vector<size_t> animationModelIndices = { 5, 9 };
	// model rotations so that the model faces the right direction
    std::vector<size_t> rotate90Y = { 10, 12,13, 14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 }; // [10] spawner conveyor tinggi, box rak for the rest
    std::vector<size_t> rotate180Y = { 1,5, 7 };           // [1] atap, [5] belt conveyor, [10] spawner kiri
    std::vector<size_t> rotate270Y = {};

    //for box animation logic
    float meltTimer = 0.0f; // Timer to track time after melting
    bool isMelted = false;  // Flag to indicate if the box has melted

    // Positions for boxBesar (Index 2)
    glm::vec3 initialBoxBesarPos = modelPositions[2];
    glm::vec3 firstStopPosBoxBesar = glm::vec3(-4.7f, 0.2f, 0.8f); // Designated position where boxBesar stops in Phase 1
    glm::vec3 finalPosBoxBesar = glm::vec3(-2.0f, 0.2f, 0.8f);      // Final position after Phase 3

    // Positions for boxKecil (Index 3)
    glm::vec3 initialBoxKecilPos = modelPositions[3];
    glm::vec3 meltPosition = firstStopPosBoxBesar; // Position where boxKecil melts into boxBesar

    // Movement phase tracking
    int movementPhase = 1; // Start with Phase 1

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        // processInput(window);
        processInput(window);

        // Apply jump physics
        if (isJumping)
        {
            jumpVelocity += gravity * deltaTime;
            camera.Position.y += jumpVelocity * deltaTime;

            // Check if landed
            if (camera.Position.y <= baseYPosition)
            {
                camera.Position.y = baseYPosition;
                isJumping = false;
                jumpVelocity = 0.0f;
            }
        }

        // *Update the window title with the camera position*
        // Update every 0.5 seconds
        static float lastTitleUpdateTime = 0.0f;
        if (currentFrame - lastTitleUpdateTime >= 0.5f)
        {
            updateWindowTitle(window, camera);
            lastTitleUpdateTime = currentFrame;
        }

        // Update the movement phases
        switch (movementPhase)
        {
        case 1: // Phase 1: Move boxBesar to firstStopPosBoxBesar
        {
            if (MoveTowards(modelPositions[2], firstStopPosBoxBesar, moveSpeed, deltaTime))
            {
                // Transition to Phase 2
                movementPhase = 2;
            }

            // Ensure boxKecil stays at its initial position
            modelPositions[3] = initialBoxKecilPos;
            break;
        }

        case 2: // Phase 2: boxKecil moves towards boxBesar, boxBesar waits
        {
            // Keep boxBesar at firstStopPosBoxBesar
            modelPositions[2] = firstStopPosBoxBesar;

            // Update the small box's position using the previous logic
            glm::vec3& smallBoxPos = modelPositions[3]; // Index of the small box
            glm::vec3& largeBoxPos = modelPositions[2]; // Index of the large box

            if (!isMelted)
            {
                // Move along Z-axis until Z positions are equal
                if (smallBoxPos.z != largeBoxPos.z)
                {
                    float zDirection = (largeBoxPos.z - smallBoxPos.z) > 0 ? 1.0f : -1.0f;
                    smallBoxPos.z += zDirection * moveSpeed * deltaTime;

                    // Clamp the position to the large box's Z position
                    if ((zDirection > 0 && smallBoxPos.z > largeBoxPos.z) ||
                        (zDirection < 0 && smallBoxPos.z < largeBoxPos.z))
                    {
                        smallBoxPos.z = largeBoxPos.z;
                    }
                }
                // Once Z positions are equal, move along Y-axis
                else if (smallBoxPos.y != largeBoxPos.y)
                {
                    float yDirection = (largeBoxPos.y - smallBoxPos.y) > 0 ? 1.0f : -1.0f;
                    smallBoxPos.y += yDirection * moveSpeed * deltaTime;

                    // Clamp the position to the large box's Y position
                    if ((yDirection > 0 && smallBoxPos.y > largeBoxPos.y) ||
                        (yDirection < 0 && smallBoxPos.y < largeBoxPos.y))
                    {
                        smallBoxPos.y = largeBoxPos.y;
                    }
                }
                else
                {
                    // The small box has melted with the large box
                    isMelted = true;
                    meltTimer = 0.9f; // Start the timer

                    // Transition to Phase 3
                    movementPhase = 3;
                }
            }
            break;
        }

        case 3: // Phase 3: boxBesar moves to final position, boxKecil respawns
        {
            // Reset boxKecil to initial position after a delay
            meltTimer += deltaTime;
            if (meltTimer >= 1.0f)
            {
                modelPositions[3] = initialBoxKecilPos; // Reset to initial position
                isMelted = false;
            }

            if (MoveTowards(modelPositions[2], finalPosBoxBesar, moveSpeed, deltaTime))
            {
                // Reset positions and restart the movement
                modelPositions[2] = initialBoxBesarPos;
                movementPhase = 1;
            }
            break;
        }

        default:
        {
            break;
        }
   
        }


        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // This is related to the texture animation speed
        // Calculate a time offset for texture animation
        float speed = 0.04f; // Adjust this value to control the speed
        float timeValue = glfwGetTime() * speed;
        ourShader.setFloat("time", timeValue);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render each model
        for (size_t i = 0; i < models.size(); i++)
        {
            // Model transformation
            glm::mat4 model = glm::mat4(1.0f);


            if (std::find(rotate180Y.begin(), rotate180Y.end(), i) != rotate180Y.end()) {
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotasi di sumbu Y
            }
            if (std::find(rotate270Y.begin(), rotate270Y.end(), i) != rotate270Y.end()) {
                model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotasi di sumbu Y
            }
            if (std::find(rotate90Y.begin(), rotate90Y.end(), i) != rotate90Y.end()) {
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotasi di sumbu Y
            }

            model = glm::translate(model, modelPositions[i]);
            model = glm::scale(model, modelScales[i]);
            ourShader.setMat4("model", model);

            // Determine if the current model should have animated texture
            bool animateTexture = false;
            glm::vec2 movementDirection = glm::vec2(0.0f, 0.0f);

            // Only animate the texture for the objects that is set beforehand

            // Check if the current model is a conveyor belt
            if (i == 5) // Conveyor belt (Index 5)
            {
                // Conveyor's texture moves during phases 1 and 3
                if (movementPhase != 2)
                {
                    animateTexture = true;
                    movementDirection = glm::vec2(0.0f, 1.0f); // Adjust direction if needed
                }
            }
            else if (i == 9) // Conveyor tinggi belt (Index 9)
            {
                // Conveyor tinggi's texture moves during phase 2
                if (movementPhase == 2)
                {
                    animateTexture = true;
                    movementDirection = glm::vec2(0.0f, 1.0f); // Adjust direction if needed
                }
            }


            // Set the animation uniforms (very importantE)
            ourShader.setBool("animateTexture", animateTexture);
            ourShader.setVec2("movementDirection", movementDirection);

            // Draw the model
            models[i].Draw(ourShader);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

void processInput(GLFWwindow* window)
{
    // Close window on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Store original Y position before processing WASD movement
    float originalY = camera.Position.y;

    // Process WASD movement normally
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Restore Y position after WASD movement
    camera.Position.y = originalY;

    // Jump initiation
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping)
    {
        isJumping = true;
        jumpVelocity = jumpStrength;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
