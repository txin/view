/*
 * cube class for openGL cube
 * interact to change the camera position 
 *
 *
 */
#pragma once
#define GLM_FORCE_RADIANS

#include <iostream>
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
    //float verticalAngle = 0.0f;
    float verticalAngle = 0.0f; //testing
    float initialFoV = 45.0f;

    // eyeMoveSpeed relative to the movement of the cube
    // TODO: eyemovement speed testing
    float eyeMoveSpeed = 0.003f;

public:
    Cube() {};
    int run();
    // update view matrix with the values from the x, y positions
    // and depth information from the eyeTracking class
    int setEyePosition(int xpos, int ypos, int zpos);
};
