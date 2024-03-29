#include "Cube.h"
#include "shader.hpp"
#include "Global.h"
#include "StereoView.h"

using namespace glm;

float speed = 0.1f;  
glm::vec3 right;
float deltaTime;
// movement of x y coordinates of the eye, int pixels
int deltaXpos, deltaYpos, deltaZpos;
glm::vec3 direction;
glm::vec3 position(0, 0, 5);

// for testing
static void key_callback(GLFWwindow* window, int key, int scancode,
                         int action, int modes) {
// Move forward
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        position += direction * deltaTime * speed;
    }
// Move backward
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        position -= direction * deltaTime * speed;
    }
// Strafe right
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        position += right * deltaTime * speed;
    }
// Strafe left
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        position -= right * deltaTime * speed;
    }
}

int Cube::setEyePosition(int deltaXpos, int deltaYpos, int deltaZpos) {

    int thresh = 5;
    // TODO: add threshold for the change of X, Y position
    if (deltaXpos > thresh || deltaXpos < 0 - thresh) {
        position += right * deltaTime * eyeMoveSpeed * float(deltaXpos);
    }
 
    if (deltaYpos > thresh || deltaYpos < 0 - thresh) {
        glm::vec3 yAxis(0, 1, 0);
        position += yAxis * deltaTime * eyeMoveSpeed * float(deltaYpos);
    }
    
    // set eyeDepth
    // depth scaling factor
    if (deltaZpos > thresh || deltaZpos < 0 - thresh) {
        float depthChangeSpeed = 0.001;
        position += direction * deltaTime * depthChangeSpeed * float(deltaZpos);
    }
    return 0;
}

int Cube::run() {
 
    // Initialise GLFW
    if(!glfwInit()) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(640, 480, "Colored Cube", NULL, NULL);
    if(window == NULL){
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

   // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("TransformVertexShader.vert", 
                                   "ColorFragmentShader.frag");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Our vertices. Tree consecutive floats give a 3D vertex; 
//Three consecutive vertices give a triangle.
// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles,
// and 12*3 vertices
    static const GLfloat g_vertex_buffer_data[] = { 
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };

    // One color for each vertex. They were generated randomly.
    static const GLfloat g_color_buffer_data[] = { 
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);


    double lastTime = glfwGetTime();

    Global global = Global::getInstance();
   
    // position of eye
    // initialise eye position
    int xpos = CAMERA_WIDTH / 2;
    int ypos = CAMERA_HEIGHT / 2;
    int zpos = DEFAULT_DEPTH;
    int tempX, tempY, tempZ;
    // display main loop
    bool runningStatus = global.getRunningStatus();
    do {
        tempX = global.getEyePosition().x;
        tempY = global.getEyePosition().y;
        tempZ = global.getEyeDepth();

        deltaXpos = tempX - xpos;
        deltaYpos = tempY - ypos;
        deltaZpos = tempZ - zpos;

        xpos = tempX;
        ypos = tempY;
        zpos = tempZ;

        setEyePosition(deltaXpos, deltaYpos, deltaZpos);

        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);

        direction = glm::vec3(cos(verticalAngle) * sin(horizontalAngle),
                              sin(verticalAngle),
                              cos(verticalAngle) * cos(horizontalAngle));
        
        // Right vector
        // PI/2
        right = glm::vec3(
            sin(horizontalAngle - 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f));
        
        glm::vec3 up = glm::cross(right, direction);


// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix

        glm::mat4 View       = glm::lookAt(
            //glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
            position,
            position + direction, // and looks at the origin
            up  // Head is up (set to 0,-1,0 to look upside-down)
            );
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 Model      = glm::mat4(1.0f);
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around



        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use our shader
        glUseProgram(programID);
        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
            );


        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        runningStatus = global.getRunningStatus();
       
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 
        && runningStatus != false);


    std::cout << "glfw ends" << std::endl;

    global.setRunningStatus(false);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}
