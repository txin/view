/*
 * cube class for openGL cube
 * interact to change the camera position 
 *
 *
 */
#pragma once
#define GLM_FORCE_RADIANS

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Cube {
private:
    GLFWwindow* window;
 
    float horizontalAngle = 3.14f;
    float verticalAngle = 0.0f;
    float initialFoV = 45.0f;

public:
    Cube(){};
    int run();
    int setViewMatrix();
};
