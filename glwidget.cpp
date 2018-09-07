#include "glwidget.h"
#include <iostream>
#include <QOpenGLTexture>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QTextStream>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

using glm::inverse;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::perspective;
using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;
using glm::rotate;
using glm::value_ptr;
using glm::lookAt;

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) { 
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(16);

    forward = false;
    back = false;
    left = false;
    right = false;
    up = false;
    down = false;
    fly = false;
}

GLWidget::~GLWidget() {
}

void GLWidget::animate() {
    float dt = .016;
    vec3 forwardVec = -vec3(yawMatrix[2]);
    vec3 upVec = vec3(0,1,0);

    velocity = vec3(0,0,0);

    if(!fly) {
        forwardVec = -vec3(yawMatrix[2]);
        velocity += -upVec;
    }
    if(fly)
        forwardVec = -vec3(orientation[2]);

    vec3 rightVec = vec3(orientation[0]);

    float speed = 3;

    if(forward) {
        velocity += forwardVec;
    }
    if(right) {
        velocity += rightVec;
    }
    if(left) {
        velocity += -rightVec;
    }
    if(back) {
        velocity += -forwardVec;
    }

    if(length(velocity) > 0) {
        velocity = normalize(velocity);
    }

    position += velocity*speed*dt;

    updateView();
    update();
}

void GLWidget::initializeGrid() {
    glGenVertexArrays(1, &gridVao);
    glBindVertexArray(gridVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    vec3 pts[84];
    for(int i = -10; i <= 10; i++) {

        pts[2*(i+10)] = vec3(i, -.5f, 10);
        pts[2*(i+10)+1] = vec3(i, -.5f, -10);

        pts[2*(i+10)+42] = vec3(10,-.5f, i);
        pts[2*(i+10)+43] = vec3(-10,-.5f, i);
    }

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/grid_vert.glsl", ":/grid_vert.glsl");
    glUseProgram(program);
    gridProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

    gridProjMatrixLoc = glGetUniformLocation(program, "projection");
    gridViewMatrixLoc = glGetUniformLocation(program, "view");
    gridModelMatrixLoc = glGetUniformLocation(program, "model");
}

void GLWidget::initializeCube() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // bottom
        vec3(.5f,.5f,.5f),
        vec3(.5f,.5f,.5f),
        vec3(.5f,.5f,.5f),
        vec3(.5f,.5f,.5f),

        // front
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // back
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // right
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),


        // left
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    cubeProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    cubeProjMatrixLoc = glGetUniformLocation(program, "projection");
    cubeViewMatrixLoc = glGetUniformLocation(program, "view");
    cubeModelMatrixLoc = glGetUniformLocation(program, "model");
    cubeLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 1);
    glUniform3f(cubeLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .1);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, 3);
}


void GLWidget::initializeGround() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &groundVao);
    glBindVertexArray(groundVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(50,1,50),    // 0
        vec3(50,1,-50),   // 1
        vec3(-50,1,-50),  // 2
        vec3(-50,1,50),   // 3

        // bottom
        vec3(50,-1,50),   // 4
        vec3(-50,-1,50),  // 5
        vec3(-50,-1,-50), // 6
        vec3(50,-1,-50),  // 7

        // front
        vec3(50,1,50),    // 8
        vec3(-50,1,50),   // 9
        vec3(-50,-1,50),  // 10
        vec3(50,-1,50),   // 11

        // back
        vec3(-50,-1,-50), // 12
        vec3(-50,1,-50),  // 13
        vec3(50,1,-50),   // 14
        vec3(50,-1,-50),  // 15

        // right
        vec3(50,-1,50),   // 16
        vec3(50,-1,-50),  // 17
        vec3(50,1,-50),   // 18
        vec3(50,1,50),     // 19

        // left
        vec3(-50,-1,50),  // 20
        vec3(-50,1,50),   // 21
        vec3(-50,1,-50),  // 22
        vec3(-50,-1,-50) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),

        // bottom
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),
        vec3(.22,.4,.2),
        // front
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // back
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // right
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // left
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    //std::cout << "ERRRR 488";
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    groundProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    groundProjMatrixLoc = glGetUniformLocation(program, "projection");
    groundViewMatrixLoc = glGetUniformLocation(program, "view");
    groundModelMatrixLoc = glGetUniformLocation(program, "model");
    groundLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 1);
    glUniform3f(groundLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .1);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, 1);
}

void GLWidget::initializeTree() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &treeVao);
    glBindVertexArray(treeVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(.5,2,.5),    // 0
        vec3(.5,2,-.5),   // 1
        vec3(-.5,2,-.5),  // 2
        vec3(-.5,2,.5),   // 3

        // bottom
        vec3(.5,-1,.5),   // 4
        vec3(-.5,-1,.5),  // 5
        vec3(-.5,-1,-.5), // 6
        vec3(.5,-1,-.5),  // 7

        // front
        vec3(.5,2,.5),    // 8
        vec3(-.5,2,.5),   // 9
        vec3(-.5,-1,.5),  // 10
        vec3(.5,-1,.5),   // 11

        // back
        vec3(-.5,-1,-.5), // 12
        vec3(-.5,2,-.5),  // 13
        vec3(.5,2,-.5),   // 14
        vec3(.5,-1,-.5),  // 15

        // right
        vec3(.5,-1,.5),   // 16
        vec3(.5,-1,-.5),  // 17
        vec3(.5,2,-.5),   // 18
        vec3(.5,2,.5),     // 19

        // left
        vec3(-.5,-1,.5),  // 20
        vec3(-.5,2,.5),   // 21
        vec3(-.5,2,-.5),  // 22
        vec3(-.5,-1,-.5) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),

        // bottom
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),

        // front
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),

        // back
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),

        // right
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),


        // left
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0),
        vec3(.23,.15,0)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    treeProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    treeProjMatrixLoc = glGetUniformLocation(program, "projection");
    treeViewMatrixLoc = glGetUniformLocation(program, "view");
    treeModelMatrixLoc = glGetUniformLocation(program, "model");
    treeLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 1);
    glUniform3f(treeLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .1);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, .1);
}

void GLWidget::initializeTop() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &topVao);
    glBindVertexArray(topVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(1,3,1),    // 0
        vec3(1,3,-1),   // 1
        vec3(-1,3,-1),  // 2
        vec3(-1,3,1),   // 3

        // bottom
        vec3(1,2,1),   // 4
        vec3(-1,2,1),  // 5
        vec3(-1,2,-1), // 6
        vec3(1,2,-1),  // 7

        // front
        vec3(1,3,1),    // 8
        vec3(-1,3,1),   // 9
        vec3(-1,2,1),  // 10
        vec3(1,2,1),   // 11

        // back
        vec3(-1,2,-1), // 12
        vec3(-1,3,-1),  // 13
        vec3(1,3,-1),   // 14
        vec3(1,2,-1),  // 15

        // right
        vec3(1,2,1),   // 16
        vec3(1,2,-1),  // 17
        vec3(1,3,-1),   // 18
        vec3(1,3,1),     // 19

        // left
        vec3(-1,2,1),  // 20
        vec3(-1,3,1),   // 21
        vec3(-1,3,-1),  // 22
        vec3(-1,2,-1) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // bottom
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // front
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // back
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),

        // right
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),


        // left
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1),
        vec3(.11,.2,.1)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    topProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    topProjMatrixLoc = glGetUniformLocation(program, "projection");
    topViewMatrixLoc = glGetUniformLocation(program, "view");
    topModelMatrixLoc = glGetUniformLocation(program, "model");
    topLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 1);
    glUniform3f(topLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .1);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, .1);
}

void GLWidget::initializeStar() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &starVao);
    glBindVertexArray(starVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // bottom
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // front
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // back
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // right
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),


        // left
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    starProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    starProjMatrixLoc = glGetUniformLocation(program, "projection");
    starViewMatrixLoc = glGetUniformLocation(program, "view");
    starModelMatrixLoc = glGetUniformLocation(program, "model");
    starLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 1);
    glUniform3f(starLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .1);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, 10);
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glPointSize(4.0f);

    glEnable(GL_DEPTH_TEST);
    GLuint restart = 0xFFFFFFFF;
    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

    initializeCube();
    initializeGrid();
    initializeGround();
    initializeTree();
    initializeTop();
    initializeWater();
    initializeStar();

    viewMatrix = mat4(1.0f);
    modelMatrix = mat4(1.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(groundProg);
    glUniformMatrix4fv(groundViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(groundModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(treeProg);
    glUniformMatrix4fv(treeViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(treeModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(topProg);
    glUniformMatrix4fv(topViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(topModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(waterProg);
    glUniformMatrix4fv(waterViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(waterModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(starProg);
    glUniformMatrix4fv(starViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(starModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(gridModelMatrixLoc, 1, false, value_ptr(modelMatrix));
}

void GLWidget::initializeWater() {
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &waterVao);
    glBindVertexArray(waterVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(10,1,10),    // 0
        vec3(10,1,-10),   // 1
        vec3(-10,1,-10),  // 2
        vec3(-10,1,10),   // 3

        // bottom
        vec3(10,-1,10),   // 4
        vec3(-10,-1,10),  // 5
        vec3(-10,-1,-10), // 6
        vec3(10,-1,-10),  // 7

        // front
        vec3(10,1,10),    // 8
        vec3(-10,1,10),   // 9
        vec3(-10,-1,10),  // 10
        vec3(10,-1,10),   // 11

        // back
        vec3(-10,-1,-10), // 12
        vec3(-10,1,-10),  // 13
        vec3(10,1,-10),   // 14
        vec3(10,-1,-10),  // 15

        // right
        vec3(10,-1,10),   // 16
        vec3(10,-1,-10),  // 17
        vec3(10,1,-10),   // 18
        vec3(10,1,10),     // 19

        // left
        vec3(-10,-1,10),  // 20
        vec3(-10,1,10),   // 21
        vec3(-10,1,-10),  // 22
        vec3(-10,-1,-10) // 23

    };

    vec3 norPts[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),   // 1
        vec3(0,1,0),  // 2
        vec3(0,1,0),   // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),  // 5
        vec3(0,-1,0), // 6
        vec3(0,-1,0),  // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),   // 9
        vec3(0,0,1),  // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1), // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),   // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),  // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),     // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),   // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),

        // bottom
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),

        // front
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),

        // back
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),

        // right
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),


        // left
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38),
        vec3(.15,.44,.38)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(norPts), norPts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/cube_vert.glsl", ":/cube_frag.glsl");
    glUseProgram(program);
    waterProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    waterProjMatrixLoc = glGetUniformLocation(program, "projection");
    waterViewMatrixLoc = glGetUniformLocation(program, "view");
    waterModelMatrixLoc = glGetUniformLocation(program, "model");
    waterLightPos = glGetUniformLocation(program, "lightPos");
    shininess = glGetUniformLocation(program, "shininess");
    glUniform1f(shininess, 2);
    glUniform3f(waterLightPos, 17*3,30,-17*3);
    speak = glGetUniformLocation(program, "speck");
    glUniform1f(speak, .7);
    ambi = glGetUniformLocation(program, "ambient");
    glUniform1f(ambi, .1);
}

void GLWidget::resizeGL(int w, int h) {
    width = w;
    height = h;

    float aspect = (float)w/h;

    projMatrix = perspective(45.0f, aspect, .01f, 100.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(groundProg);
    glUniformMatrix4fv(groundProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(treeProg);
    glUniformMatrix4fv(treeProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(topProg);
    glUniformMatrix4fv(topProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(waterProg);
    glUniformMatrix4fv(waterProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(starProg);
    glUniformMatrix4fv(starProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridProjMatrixLoc, 1, false, value_ptr(projMatrix));
}

void GLWidget::renderCube(mat4 transform) {
    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(transform));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderGrid() {
    glUseProgram(gridProg);
    glBindVertexArray(gridVao);
    glDrawArrays(GL_LINES, 0, 84);
}

void GLWidget::renderGround(mat4 transform) {
    glUseProgram(groundProg);
    glBindVertexArray(groundVao);
    glUniformMatrix4fv(groundModelMatrixLoc, 1, false, value_ptr(transform));
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderTree(mat4 transform) {
    glUseProgram(treeProg);
    glBindVertexArray(treeVao);
    glUniformMatrix4fv(treeModelMatrixLoc, 1, false, value_ptr(transform));
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderTop(mat4 transform) {
    glUseProgram(topProg);
    glBindVertexArray(topVao);
    glUniformMatrix4fv(topModelMatrixLoc, 1, false, value_ptr(transform));
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderWater(mat4 transform) {
    glUseProgram(waterProg);
    glBindVertexArray(waterVao);
    glUniformMatrix4fv(waterModelMatrixLoc, 1, false, value_ptr(transform));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderStar(mat4 transform) {
    glUseProgram(starProg);
    glBindVertexArray(starVao);
    glUniformMatrix4fv(starModelMatrixLoc, 1, false, value_ptr(transform));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //renderGrid();

    mat4 scale = glm::scale(mat4(1.0),vec3(.1,.1,.1));

        renderBody(scale,  2, -5, 1, 1, 3.0f, 1.0);
        renderBody(scale, 3 , 2, 1, 8,-3.0f, 1.3 );
        renderBody(scale, -3 , 6,1, 1,-8.0f, .95 );
        renderBody(scale, 6 , 7, 1, 3,-3.0f, .95 );
        renderBody(scale, -6 , 2,1, -5,10.0f, 1.2 );
        //translate the sheep
        int t = 7;
        renderBody(scale, t+ 2 ,t -5, 1, 1, 3.0f, 1.0);
        renderBody(scale, t+3 , t+2, 1, 8,-3.0f, 1.3 );
        renderBody(scale, t-3 , t+6,1, 1,-8.0f, .95 );
        renderBody(scale, t+6 , t+7, 1, 3,-3.0f, .95 );
        renderBody(scale,t -6 , t+2,1, -5,10.0f, 1.2 );

        t = 40;
        renderBody(scale, t+ 2 ,t -5, 1, 1, 3.0f, 1.0);
        renderBody(scale, t+3 , t+2, 1, 8,-3.0f, 1.3 );
        renderBody(scale, t-3 , t+6,1, 1,-8.0f, .95 );
        renderBody(scale, t+6 , t+7, 1, 3,-3.0f, .95 );
        renderBody(scale,t -6 , t+2,1, -5,10.0f, 1.2 );

        //render ground
        scale = glm::scale(mat4(1.0),vec3(.25,.5,.25));
        mat4 trans = glm::translate(mat4(1.0), vec3(-18.75, .75, -18.75));
        renderGround( trans* scale);
        scale = glm::scale(mat4(1.0),vec3(.3,.5,.3));
        trans = glm::translate(mat4(1.0), vec3(-17.5, .25, -17.5));
        renderGround(trans * scale);
        scale = glm::scale(mat4(1.0),vec3(.325,.5,.325));
        trans = glm::translate(mat4(1.0), vec3(-16.80, -.25, -16.80));
        renderGround( trans* scale);
        trans = glm::translate(mat4(1.0), vec3(0, -1, 0));
        renderGround(trans);

        //render trees
        trans = glm::translate(mat4(1.0), vec3(18, 0, 11));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(15, 0, 12));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(17, 0, 7));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(10, 0, 16));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(8, 0, 18));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(15, 0, 15));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(10, 0, 8));
        renderTree(trans);
        renderTop(trans);
        trans = glm::translate(mat4(1.0), vec3(6, 0, 9));
        renderTree(trans);
        renderTop(trans);

        //Water render
        trans = glm::translate(mat4(1.0), vec3(10, -.99, -10));
        renderWater(trans);
        //Hang the Moon
        trans = glm::translate(mat4(1.0), vec3(17, 15, -17));
        renderStar(trans);
}

void GLWidget::renderBody(mat4 transform,int x,int y, double h,float s, float u, double f) {
    mat4 scale = glm::scale(mat4(1.0),vec3(f*2,f*2,f*3));
    mat4 trans = glm::translate(mat4(1.0), vec3(x, -2, y));
    renderCube(transform * trans * scale);
    renderHead(transform,x,y,h,s,u);
}

void GLWidget::renderHead(mat4 transform,int x,int y,double h, float s, float u) {

    mat4 scale = glm::scale(mat4(1.0),vec3(1,1,1));
    mat4 trans = glm::translate(mat4(1.0), vec3(x,-1,-1.8 + y));

    if(h == 1){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube( transform * trans *rot*rot2* scale);
    }
    else if(h == 2){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),70.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans*rot2 * scale);
    }
    else if(h == 3){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),70.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),-70.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),-70.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans *rot2* scale);
    }
    else if(h == 4){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),70.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),-70.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),-70.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans *rot2* scale);
        rot = glm::rotate(mat4(1.0),140.0f/45, vec3(0,1,0) );
        rot2 = glm::rotate(mat4(1.0),140.0f/45, vec3(1,0,0) );
        renderCube( transform *rot* trans *rot2* scale);
    }

    renderBackHead(transform,x,y,h,s,u);
}

void GLWidget::renderBackHead(mat4 transform,int x,int y,double h,float s, float u ) {
    mat4 scale = glm::scale(mat4(1.0),vec3(1.25,1.25,1.25));
    mat4 trans = glm::translate(mat4(1.0), vec3(x,-1,-1.50 + y));

    if(h == 1){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube(transform  * trans* rot * rot2 * scale);
    }
    else if (h ==2 ){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube(transform * rot * trans  * rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);
    }
    else if (h ==3){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube(transform * rot * trans  * rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);
        rot = glm::rotate(mat4(1.0),-70.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);

    }
    else if (h ==4){
        mat4 rot2 = glm::rotate(mat4(1.0),u/45, vec3(1,0,0) );
        mat4 rot = glm::rotate(mat4(1.0),s/45, vec3(0,1,0) );
        renderCube(transform * rot * trans  * rot2* scale);
        rot = glm::rotate(mat4(1.0),70.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);
        rot = glm::rotate(mat4(1.0),-70.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);
        rot = glm::rotate(mat4(1.0),140.0f/45, vec3(0,1,0) );
        renderCube(transform * rot * trans * scale);
    }


    renderLeg1(transform,x,y);
}

void GLWidget::renderLeg1(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.85,.85,.85));
    mat4 trans = glm::translate(mat4(1.0), vec3(-.50 + x,.65-4,-1 +y));
    renderCube(transform * trans * scale);
    renderLeg2(transform,x,y);
}

void GLWidget::renderLeg2(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.85,.85,.85));
    mat4 trans = glm::translate(mat4(1.0), vec3(.50 +x,.65-4,-1 +y));
    renderCube(transform * trans * scale);
    renderLeg3(transform,x,y);
}

void GLWidget::renderLeg3(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.85,.85,.85));
    mat4 trans = glm::translate(mat4(1.0), vec3(-.50 +x,.65-4,1 +y));
    renderCube(transform * trans * scale);
    renderLeg4(transform,x,y);
}

void GLWidget::renderLeg4(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.85,.85,.85));
    mat4 trans = glm::translate(mat4(1.0), vec3(.50 +x,.65-4,1 +y));
    renderCube(transform * trans * scale);
    renderFoot1(transform,x,y);
}

void GLWidget::renderFoot1(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.7,1,.7));
    mat4 trans = glm::translate(mat4(1.0), vec3(-.50 +x,-4,-1 +y));
    renderCube(transform * trans * scale);
    renderFoot2(transform,x,y);
}

void GLWidget::renderFoot2(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.7,1,.7));
    mat4 trans = glm::translate(mat4(1.0), vec3(-.50 +x,-4,1 +y));
    renderCube(transform * trans * scale);
    renderFoot3(transform,x,y);
}

void GLWidget::renderFoot3(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.7,1,.7));
    mat4 trans = glm::translate(mat4(1.0), vec3(.50 +x,-4,-1 +y));
    renderCube(transform * trans * scale);
    renderFoot4(transform,x,y);
}

void GLWidget::renderFoot4(mat4 transform,int x,int y) {
    mat4 scale = glm::scale(mat4(1.0),vec3(.7,1,.7));
    mat4 trans = glm::translate(mat4(1.0), vec3(.50 +x,-4,1 +y));
    renderCube(transform * trans * scale);
}

GLuint GLWidget::loadShaders(const char* vertf, const char* fragf) {
    GLuint program = glCreateProgram();

    // read vertex shader from Qt resource file
    QFile vertFile(vertf);
    vertFile.open(QFile::ReadOnly | QFile::Text);
    QString vertString;
    QTextStream vertStream(&vertFile);
    vertString.append(vertStream.readAll());
    std::string vertSTLString = vertString.toStdString();

    const GLchar* vertSource = vertSTLString.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);
    {
        GLint compiled;
        glGetShaderiv( vertShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( vertShader, len, &len, log );
            std::cout << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, vertShader);

    // read fragment shader from Qt resource file
    QFile fragFile(fragf);
    fragFile.open(QFile::ReadOnly | QFile::Text);
    QString fragString;
    QTextStream fragStream(&fragFile);
    fragString.append(fragStream.readAll());
    std::string fragSTLString = fragString.toStdString();

    const GLchar* fragSource = fragSTLString.c_str();

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    {
        GLint compiled;
        glGetShaderiv( fragShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( fragShader, len, &len, log );
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    {
        GLint linked;
        glGetProgramiv( program, GL_LINK_STATUS, &linked );
        if ( !linked ) {
            GLsizei len;
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetProgramInfoLog( program, len, &len, log );
            std::cout << "Shader linker failed: " << log << std::endl;
            delete [] log;
        }
    }

    return program;
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_W:
            // forward
            forward = true;
            break;
        case Qt::Key_A:
            // left
            left = true;
            break;
        case Qt::Key_D:
            // right
            right = true;
            break;
        case Qt::Key_S:
            // backward
            back = true;
            break;
        case Qt::Key_Tab:
            // toggle fly mode
            if(fly)
                fly = false;
            else if(!fly)
                fly = true;
            break;
        case Qt::Key_Shift:
            // down
            down = true;
            break;
        case Qt::Key_Space:
            // up or jump
            up = true;
            break;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_W:
            // forward
            forward = false;
            break;
        case Qt::Key_A:
            // left
            left = false;
            break;
        case Qt::Key_D:
            // right
            right = false;
            break;
        case Qt::Key_S:
            // backward
            back = false;
            break;
        case Qt::Key_Tab:
            // toggle fly mode
            break;
        case Qt::Key_Shift:
            // down
            down = false;
            break;
        case Qt::Key_Space:
            // up or jump
            up = true;
            break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    lastPt = pt;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    vec2 d = pt-lastPt;

    yaw += d.x/100;
    pitch += d.y/100;

    if(pitch > M_PI/2) {
        pitch = M_PI/2;
    } else if (pitch < -M_PI/2) {
        pitch = -M_PI/2;
    }

    yawMatrix = glm::rotate(mat4(1.0f), yaw, vec3(0,1,0));
    pitchMatrix = glm::rotate(mat4(1.0f), pitch, vec3(1,0,0));

    orientation = yawMatrix*pitchMatrix;

    updateView();

    // Part 1 - use d.x and d.y to modify your pitch and yaw angles
    // before constructing pitch and yaw rotation matrices with them

    lastPt = pt;
}

void GLWidget::updateView() {
    if(position.x > 25) {
        position.x = 25;
    } else if (position.x < -25) {
        position.x = -25;
    }
    if(position.z > 25) {
        position.z = 25;
    } else if (position.z < -25) {
        position.z = -25;
    }
    if(position.y < 0)
        position.y = 0;

    mat4 trans = glm::translate(mat4(1.0f), position);

    viewMatrix = inverse(trans*orientation);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(groundProg);
    glUniformMatrix4fv(groundViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(treeProg);
    glUniformMatrix4fv(treeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(topProg);
    glUniformMatrix4fv(topViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(waterProg);
    glUniformMatrix4fv(waterViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(starProg);
    glUniformMatrix4fv(starViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridViewMatrixLoc, 1, false, value_ptr(viewMatrix));

}
