#include <cstdio>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "../include/cyMatrix.h"
#include "../include/cyTriMesh.h"
#include "../include/cyGL.h"
#include "../include/cyVector.h"
#include "cmath"

#define DEG2RAD(x) ((x) * 3.14159265359f / 180.0f)

// CREATE STRUCTS MAYBE!!

// Probably bad practice. FIX FOR NEXTIME IF IT RUNS
cy::GLSLProgram prog;
GLuint teapot_vao;
GLuint buffer; 

cy::TriMesh mesh; // Need it for mesh.nv
cy::Matrix4f mvp;
// Prspective transformation matrix
cy::Matrix4f p;
// Model View transformation matrix
cy::Matrix4f v;
cy::Matrix4f m;
// interactive transformation matrix
bool swapyz; 

// Object Info
cy::Vec3f boxMin;
cy::Vec3f boxMax;
cy::Vec3f center;

float x_cam_angle; 
float y_cam_angle; 
float dist_cam; 

int x_last_mouse;
int y_last_mouse; 
int mouse_button; 

static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 
static cy::Vec3f upVector(0.0f, 1.0f, 0.0f);

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
    glDrawArrays(GL_POINTS, 0, mesh.NV());

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
        mouse_button = button; 
        x_last_mouse = x; 
        y_last_mouse = y; 
    }
    else {
        mouse_button = -1; 
    }
}

void myMouseMotion(int x, int y) {

    int x_change = x - x_last_mouse; 
    int y_change = y - y_last_mouse; 

    if (mouse_button == GLUT_LEFT_BUTTON) {
        y_cam_angle += x_change; 
        x_cam_angle += -y_change;

        x_cam_angle = std::max(-89.0f, std::min(89.0f, x_cam_angle));
    }
    else if (mouse_button == GLUT_RIGHT_BUTTON) {
        dist_cam += -y_change; 
    }

    cy::Vec3f camPos;

    camPos.x = dist_cam * cos(DEG2RAD(x_cam_angle)) * sin(DEG2RAD(y_cam_angle));  
    camPos.y = dist_cam * sin(DEG2RAD(x_cam_angle));                              
    camPos.z = dist_cam * cos(DEG2RAD(x_cam_angle)) * cos(DEG2RAD(y_cam_angle)); 

    // Calculate forward, right, and up vectors
    cy::Vec3f forward = (objectPos - camPos).GetNormalized();
    cy::Vec3f right = cy::Vec3f(0.0f, 1.0f, 0.0f).Cross(forward).GetNormalized();
    cy::Vec3f newUp = forward.Cross(right);

    // Camera at camPos, looking at objectPos. I think
    v = cy::Matrix4f::View(camPos, objectPos, newUp); 

    mvp = p * v * m; 

    // USEFULPRINTS
    // printf("old-coords: %d, %d \n", x_last_mouse, y_last_mouse);
    // printf("current-coords: %d, %d \n", x, y);
    // printf("change-in-coords: %d, %d \n", x_change, y_change);
    // printf("Yaw: %f, Pitch: %f, Dist: %f\n", y_cam_angle, x_cam_angle, dist_cam);
    // printf("Camera Position: (%.2f, %.2f, %.2f)\n", camPos.x, camPos.y, camPos.z);

    x_last_mouse = x; 
    y_last_mouse = y; 

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

            x_cam_angle = 0.0f;
            y_cam_angle = 0.0f; 

            cy::Vec3f dims = boxMax - boxMin;

            // swap, boxmax, boxmin, distC_cam)

            if (swapyz) {
                m = cy::Matrix4f::Translation(cy::Vec3f(-center.x, -center.z,-center.y));
            } else {
                m = cy::Matrix4f::Translation(cy::Vec3f(-center.x, -center.y,-center.z));
            }

            dist_cam = dims.z * 3.0f;
            cy::Vec3f camPos(0.0f, 0.0f, dist_cam);
            v = cy::Matrix4f::View(camPos, objectPos, upVector); 

            mvp = p * v *m;


            glutPostRedisplay();
    }
}

// cy::Matrix4f OrthographicMatrix(float left, float right, float bottom, float top, float near, float far) {
//     cy::Matrix4f ortho;
//     ortho.SetIdentity();  // Initialize to identity matrix
//     ortho[0]  = 2.0f / (right - left);
//     ortho[5]  = 2.0f / (top - bottom);
//     ortho[10] = -2.0f / (far - near);
//     ortho[12] = -(right + left) / (right - left);
//     ortho[13] = -(top + bottom) / (top - bottom);
//     ortho[14] = -(far + near) / (far - near);
//     ortho[15] = 1.0f;
//     return ortho;
// }

int main(int argc, char** argv) {
    float width = 800.0f; 
    float height = 600.0f; 
    //GLUT Inits 
    glutInit(&argc, argv); 
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitWindowSize((int)width, (int)height);                   
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
    
    bool success = mesh.LoadFromFileObj("../assets/objs/teapot.obj");
    mesh.ComputeBoundingBox();

    boxMin = mesh.GetBoundMin();
    boxMax = mesh.GetBoundMax();

    center = (boxMin + boxMax) * 0.5f;

    cy::Vec3f dims = (boxMax - boxMin);
    float maxDim = fmax(dims.x, fmax(dims.y, dims.z));

    // FOR SCALING
    // float box_diagnol = dims.Length(); 
    // float view_diagnol = (cy::Vec3f(-(width/2.0f), -(height/2.0f), -500.0f) - cy::Vec3f(width/2.0f, height/2.0f, 500.0f)).Length();

    // USEFUL PRINTS
    // printf("dims.x: %.5f\n", dims.x);
    // printf("dims.y: %.5f\n", dims.y);
    // printf("dims.z: %.5f\n", dims.z);
    // printf("Center.x: %.5f\n", center.x);
    // printf("Center.y: %.5f\n", center.y);
    // printf("Center.z: %.5f\n", center.z);
    // printf("Min: (%.5f, %.5f, %.5f)\n", box_min.x, box_min.y, box_min.z);
    // printf("Max: (%.5f, %.5f, %.5f)\n", box_max.x, box_max.y, box_max.z);

    // SCALING, WARPS CENTER
    // m = cy::Matrix4f::Translation(cy::Vec3f(-center.x, -center.y, -center.z)) * cy::Matrix4f::Scale(2.0f / box_diagnol);
    m = cy::Matrix4f::Translation(cy::Vec3f(-center.x, -center.y,-center.z));
    // cy::Matrix4f v = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -dims.z * 3.0f)); 
    
    x_cam_angle = 0.0f; 
    y_cam_angle = 0.0f; 
    dist_cam = dims.z * 3.0f; 
    mouse_button = -1; 

    cy::Vec3f camPos(0.0f, 0.0f, dist_cam);
    v = cy::Matrix4f::View(camPos, objectPos, upVector); 
    
    p = cy::Matrix4f::Perspective(DEG2RAD(40.0f), float(800)/float(600), 0.1f, 1000.0f);
    mvp = p * v * m;
    swapyz = false; 

    prog.Bind(); 

    prog["swap"] = swapyz; 
    prog["mvp"] = mvp; 

    glBindVertexArray(teapot_vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); 

    // Load buffer data
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*mesh.NV(), &mesh.V(0), GL_STATIC_DRAW );

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