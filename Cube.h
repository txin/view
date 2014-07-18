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

    // eyeMoveSpeed relative to the movement of the cube
    // TODO: eyemovement speed testing
    float eyeMoveSpeed = 0.05f;

public:
    Cube(){};
    int run();
    // set view matrix from the x, y positions of eyeTracking class
    int setViewMatrix(int xpos, int ypos);
};
