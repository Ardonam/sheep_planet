#ifndef __GLWIDGET__INCLUDE__
#define __GLWIDGET__INCLUDE__

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QTimer>
#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS

using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec2;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core { 
    Q_OBJECT

    public:
        GLWidget(QWidget *parent=0);
        ~GLWidget();

        GLuint loadShaders(const char* vertf, const char* fragf);
    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

    // Part 2 - add an animate slot
    public slots:
        void animate();

    private:
        void initializeCube();
        void initializeGround();
        void initializeTree();
        void initializeTop();
        void initializeWater();
        void initializeStar();
        void renderStar(mat4 transform);
        void renderWater(mat4 transform);
        void renderTop(mat4 transform);
        void renderTree(mat4 transform);
        void renderGround(mat4 transform);
        void renderCube(mat4 transform);
        void renderBody(mat4 transfom, int x, int y, double h, float s,float up, double fat);
        void renderHead(mat4 transform, int x, int y,double h, float s,float up);
        void renderBackHead(mat4 transform, int x, int y, double h,float s, float up);
        void renderLeg1(mat4 transform, int x, int y);
        void renderLeg2(mat4 transform, int x, int y);
        void renderLeg3(mat4 transform, int x, int y);
        void renderLeg4(mat4 transform, int x, int y);
        void renderFoot1(mat4 transform, int x, int y);
        void renderFoot2(mat4 transform, int x, int y);
        void renderFoot3(mat4 transform, int x, int y);
        void renderFoot4(mat4 transform, int x, int y);

        //void renderParticleSystem(ParticleSystem ps *);

        GLuint cubeProg;
        GLuint cubeVao;
        GLint cubeProjMatrixLoc;
        GLint cubeViewMatrixLoc;
        GLint cubeLightPos;
        GLint cubeModelMatrixLoc;
        GLuint textureObject;

        GLuint groundProg;
        GLuint groundVao;
        GLint groundProjMatrixLoc;
        GLint groundViewMatrixLoc;
        GLint groundLightPos;
        GLint groundModelMatrixLoc;

        GLuint treeProg;
        GLuint treeVao;
        GLint treeProjMatrixLoc;
        GLint treeViewMatrixLoc;
        GLint treeLightPos;
        GLint treeModelMatrixLoc;

        GLuint topProg;
        GLuint topVao;
        GLint topProjMatrixLoc;
        GLint topViewMatrixLoc;
        GLint topLightPos;
        GLint topModelMatrixLoc;

        GLuint waterProg;
        GLuint waterVao;
        GLint waterProjMatrixLoc;
        GLint waterViewMatrixLoc;
        GLint waterLightPos;
        GLint waterModelMatrixLoc;

        GLuint starProg;
        GLuint starVao;
        GLint starProjMatrixLoc;
        GLint starViewMatrixLoc;
        GLint starLightPos;
        GLint starModelMatrixLoc;

        void initializeGrid();
        void renderGrid();

        GLuint gridProg;
        GLuint gridVao;
        GLint gridProjMatrixLoc;
        GLint gridViewMatrixLoc;
        GLint gridModelMatrixLoc;

        GLint shininess;
        GLint speak;
        GLint ambi;

        mat4 projMatrix;
        mat4 viewMatrix;
        mat4 modelMatrix;
        vec3 ltPos;

        // Part 1 - Add two mat4 variables for pitch and yaw.
        // Also add two float variables for the pitch and yaw angles.
        float pitch;
        float yaw;
        mat4 pitchMatrix;
        mat4 yawMatrix;
        mat4 orientation;
        // Part 2 - Add a QTimer variable for our render loop.
        QTimer *timer;
        // Part 3 - Add state variables for keeping track
        //          of which movement keys are being pressed
        //        - Add two vec3 variables for position and velocity.
        //        - Add a variable for toggling fly mode
    bool forward;
    bool back;
    bool left;
    bool right;
    bool up;
    bool down;
    bool fly;

        vec3 velocity;
        vec3 position;
        int width;
        int height;

        glm::vec2 lastPt;
        void updateView();
};

#endif
