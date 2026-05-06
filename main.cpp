#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

gps::Window myWindow;

// Matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightSpaceMatrix;

// Lights
glm::vec3 lightDir = glm::vec3(0.5f, -1.0f, 0.3f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 pointLightPos = glm::vec3(50.0f, 30.0f, 50.0f);

// Uniform locations
GLint modelLoc, viewLoc, projectionLoc, normalMatrixLoc;
GLint lightDirLoc, lightColorLoc, pointLightPosLoc, spotLightPosLoc, spotLightDirLoc;
GLint lightSpaceMatrixLoc, timeLoc, windStrengthLoc, fogDensityLoc;
GLint fogEnabledLoc, spotLightEnabledLoc;

// Camera
gps::Camera myCamera(
    glm::vec3(0.0f, 50.0f, 100.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));


float cameraSpeed = 50.0f;  

// Frame timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 640, lastY = 360;

// Models
gps::Model3D terrain, tree, rock, campfire, flower1, flower2;
glm::mat4 terrainModel = glm::mat4(1.0f);
glm::mat4 campfireModel = glm::mat4(1.0f);

// Multiple trees and rocks
std::vector<glm::mat4> treeModels;
std::vector<glm::mat4> rockModels;
std::vector<glm::mat4> flower1Models;
std::vector<glm::mat4> flower2Models;

// Shaders
gps::Shader myBasicShader, skyboxShader, shadowShader;

// Skybox
GLuint skyboxVAO, skyboxVBO, cubemapTexture;
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
};

// Shadow mapping
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLuint shadowFBO, shadowDepthMap;

// Animation and effects
bool animationTour = false;
float tourAngle = 0.0f;
float tourRadius = 150.0f;  
float tourHeight = 80.0f;   

bool rainEnabled = false;
bool fogEnabled = true;
bool spotLightEnabled = true;
float windStrength = 0.05f;
float fogDensity = 0.015f;

// Visualization modes
GLenum polygonMode = GL_FILL;

// Rain particles
struct RainParticle {
    glm::vec3 position;
    float speed;
};
std::vector<RainParticle> rainParticles;
GLuint rainVAO, rainVBO;

GLboolean pressedKeys[1024];
double lastTime = 0.0;
float gameTime = 0.0f;

// Point light animation
float pointLightAngle = 0.0f;

void glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        default: error = "UNKNOWN"; break;
        }
        std::cerr << "OpenGL error: " << error << " | " << file << " (" << line << ")\n";
    }
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "Cubemap failed to load at: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void initSkybox() {
    std::vector<std::string> faces{
        "textures/skybox/posx.jpg",  
        "textures/skybox/negx.jpg",  
        "textures/skybox/posy.jpg",  
        "textures/skybox/negy.jpg",  
        "textures/skybox/posz.jpg",  
        "textures/skybox/negz.jpg"   
    };
    cubemapTexture = loadCubemap(faces);

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void initShadowFramebuffer() {
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowDepthMap);
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initRain() {
    for (int i = 0; i < 2000; ++i) {  
        RainParticle p;
        p.position = glm::vec3(
            (rand() % 2000 - 1000),  
            (rand() % 200),          
            (rand() % 2000 - 1000)
        );
        p.speed = 0.3f + static_cast<float>(rand() % 20) / 100.0f;
        rainParticles.push_back(p);
    }

    glGenVertexArrays(1, &rainVAO);
    glGenBuffers(1, &rainVBO);
}

void updateRain(float deltaTime) {
    if (!rainEnabled) return;

    for (auto& p : rainParticles) {
        p.position.y -= p.speed * deltaTime * 10.0f;
        if (p.position.y < 0.0f) {
            p.position.y = 200.0f;
            p.position.x = (rand() % 2000 - 1000);
            p.position.z = (rand() % 2000 - 1000);
        }
    }
}

void renderRain() {
    if (!rainEnabled) return;

    std::vector<float> vertices;
    for (const auto& p : rainParticles) {
        vertices.push_back(p.position.x);
        vertices.push_back(p.position.y);
        vertices.push_back(p.position.z);
    }

    glBindVertexArray(rainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glDrawArrays(GL_POINTS, 0, rainParticles.size());
    glBindVertexArray(0);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    myWindow.setWindowDimensions({ width, height });
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        pressedKeys[key] = true;

        if (key == GLFW_KEY_1) polygonMode = GL_FILL;
        if (key == GLFW_KEY_2) polygonMode = GL_LINE;
        if (key == GLFW_KEY_3) polygonMode = GL_POINT;
        if (key == GLFW_KEY_R) rainEnabled = !rainEnabled;
        if (key == GLFW_KEY_F) fogEnabled = !fogEnabled;
        if (key == GLFW_KEY_L) spotLightEnabled = !spotLightEnabled;
        if (key == GLFW_KEY_T) animationTour = !animationTour;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (action == GLFW_RELEASE) pressedKeys[key] = false;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    if (!animationTour) {
       
        myCamera.rotate(yoffset * 0.15f, xoffset * 0.15f);
    }
}

void processMovement() {
    if (animationTour) return;

    
    float velocity = cameraSpeed * deltaTime;

    if (pressedKeys[GLFW_KEY_W]) myCamera.move(gps::MOVE_FORWARD, velocity);
    if (pressedKeys[GLFW_KEY_S]) myCamera.move(gps::MOVE_BACKWARD, velocity);
    if (pressedKeys[GLFW_KEY_A]) myCamera.move(gps::MOVE_LEFT, velocity);
    if (pressedKeys[GLFW_KEY_D]) myCamera.move(gps::MOVE_RIGHT, velocity);
    if (pressedKeys[GLFW_KEY_SPACE]) myCamera.move(gps::MOVE_UP, velocity);
    if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) myCamera.move(gps::MOVE_DOWN, velocity);
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

   
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void initModels() {
    terrain.LoadModel("models/terrain/terrain.obj");
    tree.LoadModel("models/tree/tree.obj");
    rock.LoadModel("models/rock/rock.obj");
    campfire.LoadModel("models/campfire/campfire_stones.obj");
    flower1.LoadModel("models/flower/flower1.obj");
    flower2.LoadModel("models/flower/flower2.obj");

   
    terrainModel = glm::scale(glm::mat4(1.0f), glm::vec3(200.0f, 1.0f, 200.0f));

    
    campfireModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    
    campfireModel = glm::scale(campfireModel, glm::vec3(3.5f));

    srand(static_cast<unsigned>(time(nullptr)));

    
    for (int i = 0; i < 40; ++i) {
        glm::mat4 mat = glm::mat4(1.0f);
        float x = static_cast<float>(rand() % 200 - 100);
        float z = static_cast<float>(rand() % 200 - 100);

       
        if (abs(x) < 10 && abs(z) < 10) continue;

        mat = glm::translate(mat, glm::vec3(x, 0.0f, z));
        float scale = 3.0f + static_cast<float>(rand() % 10) / 2.0f;
        mat = glm::scale(mat, glm::vec3(scale));
        float rotY = static_cast<float>(rand() % 360);
        mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        treeModels.push_back(mat);
    }

    
    for (int i = 0; i < 30; ++i) {
        glm::mat4 mat = glm::mat4(1.0f);
        float x = static_cast<float>(rand() % 200 - 100);
        float z = static_cast<float>(rand() % 200 - 100);

        
        if (abs(x) < 10 && abs(z) < 10) continue;

        mat = glm::translate(mat, glm::vec3(x, 0.0f, z));
        float scale = 2.0f + static_cast<float>(rand() % 10) / 2.0f;
        mat = glm::scale(mat, glm::vec3(scale));
        float rotY = static_cast<float>(rand() % 360);
        mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        rockModels.push_back(mat);
    }

    
    for (int i = 0; i < 150; ++i) {
        glm::mat4 mat = glm::mat4(1.0f);
        float x = static_cast<float>(rand() % 200 - 100);
        float z = static_cast<float>(rand() % 200 - 100);
        mat = glm::translate(mat, glm::vec3(x, 0.0f, z));
        float scale = 2.2f + static_cast<float>(rand() % 10) / 10.0f; 
        mat = glm::scale(mat, glm::vec3(scale));
        float rotY = static_cast<float>(rand() % 360);
        mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        flower1Models.push_back(mat);
    }

    for (int i = 0; i < 150; ++i) {
        glm::mat4 mat = glm::mat4(1.0f);
        float x = static_cast<float>(rand() % 200 - 100);
        float z = static_cast<float>(rand() % 200 - 100);
        mat = glm::translate(mat, glm::vec3(x, 0.0f, z));
        float scale = 2.2f + static_cast<float>(rand() % 10) / 10.0f; 
        mat = glm::scale(mat, glm::vec3(scale));
        float rotY = static_cast<float>(rand() % 360);
        mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        flower2Models.push_back(mat);
    }
}

void initShaders() {
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
    shadowShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    lightSpaceMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceMatrix");

    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
    spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos");
    spotLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir");

    timeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "time");
    windStrengthLoc = glGetUniformLocation(myBasicShader.shaderProgram, "windStrength");
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled");
    spotLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightEnabled");

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);
}

void renderShadowMap() {
    shadowShader.useShaderProgram();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    float near_plane = 1.0f, far_plane = 2000.0f;  
    glm::mat4 lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(-lightDir * 500.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    model = terrainModel;
    glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    terrain.Draw(shadowShader);

    model = campfireModel;
    glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    campfire.Draw(shadowShader);

    for (const auto& m : treeModels) {
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
        tree.Draw(shadowShader);
    }

    for (const auto& m : rockModels) {
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
        rock.Draw(shadowShader);
    }

    for (const auto& m : flower1Models) {
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
        flower1.Draw(shadowShader);
    }

    for (const auto& m : flower2Models) {
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
        flower2.Draw(shadowShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {
    renderShadowMap();

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glUniform1f(timeLoc, gameTime);
    glUniform1f(windStrengthLoc, windStrength);
    glUniform1f(fogDensityLoc, fogDensity);
    glUniform1i(fogEnabledLoc, fogEnabled ? 1 : 0);
    glUniform1i(spotLightEnabledLoc, spotLightEnabled ? 1 : 0);

  
    glm::vec3 spotLightPosView = glm::vec3(0.0f, 0.0f, 0.0f); 
    glm::vec3 spotLightDirView = glm::vec3(0.0f, 0.0f, -1.0f);

    
    glm::vec3 lightDirView = glm::normalize(glm::mat3(view) * lightDir);
    glm::vec3 pointLightPosView = glm::vec3(view * glm::vec4(pointLightPos, 1.0f));
    

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirView));
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPosView));
    glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(spotLightPosView));
    glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(spotLightDirView));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap);

    model = terrainModel;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    terrain.Draw(myBasicShader);

    for (const auto& m : treeModels) {
        model = m;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        tree.Draw(myBasicShader);
    }

    for (const auto& m : rockModels) {
        model = m;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        rock.Draw(myBasicShader);
    }

    // Campfire
    model = campfireModel;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    campfire.Draw(myBasicShader);

    
    glDisable(GL_CULL_FACE);
    for (const auto& m : flower1Models) {
        model = m;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        flower1.Draw(myBasicShader);
    }

    for (const auto& m : flower2Models) {
        model = m;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        flower2.Draw(myBasicShader);
    }
    glEnable(GL_CULL_FACE);

    renderRain();

    // Skybox (render last) 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthFunc(GL_LEQUAL);
    skyboxShader.useShaderProgram();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void cleanup() {
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowDepthMap);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    glDeleteVertexArrays(1, &rainVAO);
    glDeleteBuffers(1, &rainVBO);
    myWindow.Delete();
}

int main(int argc, const char* argv[]) {
    try {
        myWindow.Create(1280, 720, "Padure Mistice - OpenGL Project");
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    initSkybox();
    initShadowFramebuffer();
    initRain();
    setWindowCallbacks();

    projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(myWindow.getWindowDimensions().width) /
        static_cast<float>(myWindow.getWindowDimensions().height),
        0.1f, 1000.0f);

    lastTime = glfwGetTime();

    glCheckError();

    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        // Frame timing
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        double currentTime = glfwGetTime();
        float dtForAnim = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        gameTime += dtForAnim;

        if (animationTour) {
            tourAngle += 0.3f * dtForAnim;
            glm::vec3 camPos;
            camPos.x = sin(tourAngle) * tourRadius;
            camPos.y = tourHeight;
            camPos.z = cos(tourAngle) * tourRadius;

            myCamera = gps::Camera(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

       
        pointLightPos = glm::vec3(sin(pointLightAngle) * 150.0f, 30.0f, cos(pointLightAngle) * 150.0f);
        pointLightAngle += dtForAnim * 0.5f;

        updateRain(dtForAnim);
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
        glCheckError();
    }

    cleanup();
    return 0;
}