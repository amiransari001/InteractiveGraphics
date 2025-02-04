#include <cstdio>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "../include/cyMatrix.h"
#include "../include/cyTriMesh.h"
#include "../include/cyGL.h"
#include "../include/cyVector.h"
#include "../include/globals.h"
#include "cmath"

#define DEG2RAD(x) ((x) * 3.14159265359f / 180.0f)

// CREATE STRUCTS MAYBE!!

// Probably bad practice. FIX FOR NEXTIME IF IT RUNS
cy::GLSLProgram prog;
GLuint teapot_vao;
GLuint buffer; 

// CLASSES TO MAKE
// OBJ
aa::Object obj; 
aa::Camera camera; 
aa::Mouse mouse;

// cy::TriMesh mesh; 
// cy::Vec3f boxMin;
// cy::Vec3f boxMax;
// cy::Vec3f center;

// CAMERA
// VEC3 camera_pos
// float x_cam_angle; 
// float y_cam_angle; 
// float dist_cam; 

// TRANSFORMATIONS
cy::Matrix4f mvp;
// Prspective transformation matrix
cy::Matrix4f p;
// Model View transformation matrix
cy::Matrix4f v;
cy::Matrix4f m;

// USER IMPUT
// int x_last_mouse;
// int y_last_mouse; 
// int mouse_button; 

bool swapyz; 


static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 
// static cy::Vec3f upVector(0.0f, 1.0f, 0.0f);

// Amir Helpers -----------------------------

// Be careful, dealing with a lot of globals. 
// void resetImage() {

// }


// OpenGL -----------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    prog.Bind(); 

    prog["swap"] = swapyz;
    prog["mvp"] = mvp;
    glBindVertexArray(teapot_vao);
    glDrawArrays(GL_POINTS, 0, obj.mesh.NV());

    // Is this how you unbind? 
    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();
}

void idle() {
    // glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {
    
    if (state == GLUT_DOWN) {
        mouse.button = button; 
        mouse.last_x= x; 
        mouse.last_y = y; 
    }
    else {
        mouse.button = -1; 
    }
}

void myMouseMotion(int x, int y) {

    int x_change = x - mouse.last_x; 
    int y_change = y - mouse.last_y; 

    if (mouse.button == GLUT_LEFT_BUTTON) {
        camera.rotateAroundOrigin(DEG2RAD(y_change), DEG2RAD(-x_change));
    }
    else if (mouse.button == GLUT_RIGHT_BUTTON) {
        camera.move(-y_change);
    }

    v = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 

    mvp = p * v * m; 

    mouse.last_x = x; 
    mouse.last_y = y; 

    glutPostRedisplay();
}

// void glutReshapeFunc

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: 
            exit(0); 
            break;
        case 'z':
            printf("z");
            swapyz = !swapyz;

            if (swapyz) {
                m = cy::Matrix4f::Translation(cy::Vec3f(-obj.center.x, -obj.center.z,-obj.center.y));
            } else {
                m = cy::Matrix4f::Translation(-obj.center);
            }

            camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));

            v = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 

            mvp = p * v * m;

            glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    //GLUT Inits 
    glutInit(&argc, argv); 
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitWindowSize(800, 600);                   
    glutInitWindowPosition(100, 100);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Hello World");

    // GLEW Initializations
    glewExperimental = GL_TRUE;
    glewInit();

    // for debugging. Professor code.
    CY_GL_REGISTER_DEBUG_CALLBACK;

    prog.BuildFiles( "src/shader.vert", "src/shader.frag" );

    glGenVertexArrays( 1, &teapot_vao );
    glGenBuffers(1, &buffer); 

    obj = aa::Object("../assets/objs/teapot.obj");
    camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));

    m = cy::Matrix4f::Translation(-obj.center);
    v = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    p = cy::Matrix4f::Perspective(DEG2RAD(40.0f), float(800)/float(600), 0.1f, 1000.0f);
    mvp = p * v * m;

    swapyz = false; 

    prog.Bind(); 

    prog["swap"] = swapyz; 
    prog["mvp"] = mvp; 

    glBindVertexArray(teapot_vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); 

    // Load buffer data
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.mesh.NV(), &obj.mesh.V(0), GL_STATIC_DRAW );

    // Assign pos attribute buffer
    GLuint pos = glGetAttribLocation(prog.GetID(), "pos" );
    glEnableVertexAttribArray( pos );
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    //Unbind VAO after setup. IDK, maybe good practice.
    glBindVertexArray(0);
    glUseProgram(0);

    // Function Assignment
    glutDisplayFunc(display);              
    glutKeyboardFunc(handleKeypress);
    glutIdleFunc(idle); 
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMotion); 

    // OPENGL Inits
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Main Loop
    glutMainLoop(); 
    return 0; 
}


// CY CODE 

// COMPILE AND LINK SHADERS
// cy::GLSLProgram prog;
// prog.BuildFiles( "shader.vert","shader.frag" );
// UNIFORM SAVIING GOES HERE? 
// prog.Bind();
// glDrawArrays(...);

    // ASSIGNING UNIFORM VARIABLES
// prog["mvp"] = myMatrix;


// glutInit
// glutInitDisplayMode
// glutInitWindowSize
// glutCreateWindow
// glutMainLoop
// glutLeaveMainLoop
// glutPostRedisplay
// glutSwapBuffers
// glutDisplayFunc
// glutKeyboardFunc
// glutIdleFuncs


// NAME="Red Hat Enterprise Linux"
// VERSION="8.10 (Ootpa)"
// ID="rhel"
// ID_LIKE="fedora"
// VERSION_ID="8.10"
// PLATFORM_ID="platform:el8"
// PRETTY_NAME="Red Hat Enterprise Linux 8.10 (Ootpa)"