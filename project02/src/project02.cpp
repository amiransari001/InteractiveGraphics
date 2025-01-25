#include <cstdio>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "../include/cyMatrix.h"
#include "../include/cyTriMesh.h"
#include "../include/cyGL.h"
#include "../include/cyVector.h"

#define DEG2RAD(x) ((x) * 3.14159265359f / 180.0f)

// Probably bad practice. FIX FOR NEXTIME IF IT RUNS
cy::GLSLProgram prog;
GLuint teapot_vao;
GLuint buffer; 

cy::TriMesh mesh; // Need it for mesh.nv
cy::Matrix4f mvp;
cy::Matrix4f perspective;
cy::Matrix4f mv;

float x_cam_angle; 
float y_cam_angel; 
float dist_cam; 

int x_last_mouse;
int y_last_mouse; 
int mouse_button; 

// Amir's Helpers --------------------------
// int SetMVP()) {
//     return 0; 
// }


// OpenGL -----------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    prog.Bind(); 
    glBindVertexArray(teapot_vao);
    glDrawArrays(GL_POINTS, 0, mesh.NV());

    // Unbind VAO. Again not sure.
    glBindVertexArray(0);
    // Unbind prog. Not sure. 
    glUseProgram(0);

    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: 
            exit(0); 
            break;
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

    // Build prog. Understand exact order of operations, i.e. load, compile, link, etc
    // prog = cy::GLSLProgram:: GLSLProgram();
    prog.BuildFiles( "src/shader.vert", "src/shader.frag" );

    glGenVertexArrays( 1, &teapot_vao );
    glGenBuffers(1, &buffer); 

    // Interactive Initis
    x_cam_angle = 0.0f; 
    y_cam_angel = 0.0f; 
    dist_cam = 0.5f; 
    int mouse_button = -1; 

    // Read obj file
    bool success = mesh.LoadFromFileObj("../assets/objs/teapot.obj");
    mesh.ComputeBoundingBox();

    cy::Vec3f min = mesh.GetBoundMin();
    cy::Vec3f max = mesh.GetBoundMax();

    cy::Vec3f center = (min + max) * 0.5f;
    cy::Vec3f dims = (max - min);
    float maxDim = fmax(dims.x, fmax(dims.y, dims.z));

    printf("dims.x: %.5f\n", dims.x);
    printf("dims.y: %.5f\n", dims.y);
    printf("dims.z: %.5f\n", dims.z);
    
    printf("Center.x: %.5f\n", center.x);
    printf("Center.y: %.5f\n", center.y);
    printf("Center.z: %.5f\n", center.z);

    printf("Min: (%.5f, %.5f, %.5f)\n", min.x, min.y, min.z);
    printf("Max: (%.5f, %.5f, %.5f)\n", max.x, max.y, max.z);


    // Bind prog, VAO, and VBO
    prog.Bind(); 

    // cy::Matrix4f m = cy::Matrix4f(0.0f);
    // cy::Matrix4f v = cy::Matrix4f(0.0f);
    // mv = cy::Matrix4f::Translation(-center + cy::Vec3f(0.0f, 0.0f, -1.0f)) * cy::Matrix4f::Scale(1.0f / 5.0f);
    // mv = cy::Matrix4f::Translation(-center) * cy::Matrix4f::Scale(1.0f / 5.0f);
    cy::Matrix4f m = cy::Matrix4f::Translation(center * -1.0f) * cy::Matrix4f::Scale(2.0f / maxDim);
    cy::Matrix4f v = cy::Matrix4f::Translation(cy::Vec3f(1.0f, 0.0f, 0.0f)); // WHY IS THIS WIERD
    
    mv = v * m;
    perspective = cy::Matrix4f::Perspective(DEG2RAD(40.0f), float(800)/float(600), 0.1f, 1000.0f);
    mvp = perspective * mv; 

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
    // MORE TO DO
    glutDisplayFunc(display);              
    glutKeyboardFunc(handleKeypress);
    glutIdleFunc(idle); 

    // OPENGL Inits
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Other Inits

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