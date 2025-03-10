#include <cstdio>
#include "lodepng.h" 
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "../include/cyMatrix.h"
#include "../include/cyTriMesh.h"
#include "../include/cyGL.h"
#include "../include/cyVector.h"
#include "../include/globals.h"
#include "cmath"

#define DEG2RAD(x) ((x) * 3.14159265359f / 180.0f)
const int NUM_PASSES = 2;

cy::GLSLProgram progs[NUM_PASSES];
GLuint vaos[NUM_PASSES];

cy::GLSLProgram prog;
GLuint vao;
GLuint vertex_buffer; 
GLuint normal_buffer; 
GLuint tex_coord_buffer; 
cy::GLTexture2D tex;

cy::GLRenderTexture2D renderBuffer;


aa::Object obj; 
aa::Object plane; 
aa::Camera camera[NUM_PASSES];
// aa::Camera camera; 
aa::Mouse mouse;
// aa::TransformationMatrices tMatrices; 
aa::TransformationMatrices mats[NUM_PASSES];
aa::Light light;
aa::Texture texture; 

bool swapyz; 
static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 
int keyModifier = 0; 


// OpenGL -----------------------------------
void display() {
    progs[1].Bind();
    renderBuffer.Bind();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    progs[1]["swap"] = swapyz;
    progs[1]["cameraPos"] = camera[1].pos; 
 
    progs[1]["mvp"] = mats[1].mvp;
    progs[1]["m"] = mats[1].m; 
    progs[1]["nrm_m"] = mats[1].nrm_m; 

    progs[1]["Kd"] = obj.material.Kd; 
    progs[1]["Ks"] = obj.material.Ks; 
    progs[1]["alpha"] = obj.material.alpha; 

    progs[1]["lightDir"] = light.dir; 
    progs[1]["lightColor"] = light.color; 
    progs[1]["lightIntensity"] = light.intensity; 

    glBindVertexArray(vaos[1]);
    tex.Bind(0);
    progs[1]["tex"] = 0;
    // IMPORTANT DO NOT DELETE
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
    glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
    // glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to solid rendering mode

    renderBuffer.Unbind();
    renderBuffer.BuildTextureMipmaps();

    glBindVertexArray(0);
    glUseProgram(0);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    progs[0].Bind(); 
    glBindVertexArray(vaos[0]);
    progs[0]["mvp"] = mats[0].mvp;
    // progs[0]["mvp"] = cy::Matrix4f::Identity();
    
    renderBuffer.BindTexture(0);
    progs[0]["renderedTexture"] = 0; 

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Unbind VA0 and program
    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();
}

void idle() {
    // glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {

    if (state == GLUT_DOWN) {
        keyModifier = glutGetModifiers();
        mouse.button = button; 
        mouse.last_x= x; 
        mouse.last_y = y; 
    }
    else {
        keyModifier = 0; 
        mouse.button = -1; 
    }
}

void myMouseMotion(int x, int y) {
    int x_change = x - mouse.last_x; 
    int y_change = y - mouse.last_y; 
    
    if(keyModifier == 4) {
        if (mouse.button == GLUT_LEFT_BUTTON) {
            camera[0].rotateAroundOrigin(DEG2RAD(y_change), DEG2RAD(-x_change));
        }
        else if (mouse.button == GLUT_RIGHT_BUTTON) {
            camera[0].move(-y_change);
        }
        cy::Matrix4f view = cy::Matrix4f::View(camera[0].pos, objectPos, camera[0].up); 
        mats[0].setView(view); 
    } else {
        if (mouse.button == GLUT_LEFT_BUTTON) {
            camera[1].rotateAroundOrigin(DEG2RAD(y_change), DEG2RAD(-x_change));
        }
        else if (mouse.button == GLUT_RIGHT_BUTTON) {
            camera[1].move(-y_change);
        }
        cy::Matrix4f view = cy::Matrix4f::View(camera[1].pos, objectPos, camera[1].up); 
        mats[1].setView(view); 
    }

    mouse.last_x = x; 
    mouse.last_y = y; 

    glutPostRedisplay();
}

void handleKeypress(unsigned char key, int x, int y) {
    int mods = glutGetModifiers();
    switch (key) {
        case 27: 
            glDeleteVertexArrays(NUM_PASSES, vaos);
            glDeleteBuffers(1, &vertex_buffer);
            glDeleteBuffers(1, &normal_buffer);
            glDeleteBuffers(1, &tex_coord_buffer);
            exit(0); 
            break;
        case 'z':
            swapyz = !swapyz;
            cy::Matrix4f model; 
            if (swapyz) {
                model = cy::Matrix4f::Translation(cy::Vec3f(-obj.center.x, -obj.center.z,-obj.center.y));
            } else {
                model = cy::Matrix4f::Translation(-obj.center);
            }

            camera[1].setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));
            cy::Matrix4f view = cy::Matrix4f::View(camera[1].pos, objectPos, camera[1].up); 
            mats[1].setMV(model, view); 

            glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {  // Ensure an argument is provided
        std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
        return -1;
    }

    std::string objFilePath = argv[1];  // Get OBJ file path from the command line
    std::string texturePath = (argc > 2) ? argv[2] : "brick.png";
    std::cout << "Loading OBJ file: " << objFilePath << std::endl;


    obj = aa::Object(objFilePath);    
    if (obj.vertices.empty()) {
        std::cerr << "Failed to load OBJ file: " << objFilePath << std::endl;
        return -1;
    }

    if (!texture.loadTexture(texturePath.c_str())) {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        return -1;
    }



    //GLUT Inits 
    glutInit(&argc, argv); 
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitWindowSize(800, 600);                   
    glutInitWindowPosition(100, 100);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutCreateWindow("Hello World");

    // GLEW Initializations
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    CY_GL_REGISTER_DEBUG_CALLBACK;

    progs[0].BuildFiles("src/shader.vert", "src/shader.frag");
    progs[1].BuildFiles( "src/rendertex.vert", "src/rendertex.frag" );
    // prog.BuildFiles( "src/shader.vert", "src/shader.frag" );

    glGenVertexArrays(NUM_PASSES, vaos); 
    // glGenVertexArrays( 1, &vao );    
    glGenBuffers(1, &vertex_buffer); 
    glGenBuffers(1, &normal_buffer); 
    glGenBuffers(1, &tex_coord_buffer); 
    tex.Initialize();
    camera[1].setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));

    // cy::Matrix4f model = cy::Matrix4f::Translation(-obj.center) * cy::Matrix4f::Scale(1.0 / (obj.boxMax - obj.boxMin).Length());
    cy::Matrix4f tr_model = cy::Matrix4f::Translation(-obj.center);
    cy::Matrix4f tr_view = cy::Matrix4f::View(camera[1].pos, objectPos, camera[1].up); 
    cy::Matrix4f tr_projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(600)/float(600), 0.1f, 1000.0f);
    mats[1].setMVP(tr_model, tr_view, tr_projection);

    swapyz = false; 

    progs[1].Bind(); 
    // prog.Bind(); 

    progs[1]["swap"] = swapyz;
    progs[1]["cameraPos"] = camera[1].pos; 
    progs[1]["mvp"] = mats[1].mvp; 
    progs[1]["m"] = mats[1].m; 
    progs[1]["nrm_m"] = mats[1].nrm_m; 

    progs[1]["Kd"] = obj.material.Kd; 
    progs[1]["Ks"] = obj.material.Ks; 
    progs[1]["alpha"] = obj.material.alpha; 

    progs[1]["lightDir"] = light.dir; 
    progs[1]["lightColor"] = light.color; 
    progs[1]["lightIntensity"] = light.intensity; 

    glBindVertexArray(vaos[1]);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.vertices.size(), obj.vertices.data(), GL_STATIC_DRAW );

    GLuint pos = glGetAttribLocation(progs[1].GetID(), "pos" );
    if (pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( pos );
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );


    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.normals.size(), obj.normals.data(), GL_STATIC_DRAW );
    
    GLuint nrm = glGetAttribLocation(progs[1].GetID(), "nrm" );
    if (nrm == -1) {
        std::cerr << "ERROR: Attribute 'nrm' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( nrm );
    glVertexAttribPointer(nrm, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindBuffer(GL_ARRAY_BUFFER, tex_coord_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.textureCoords.size(), obj.textureCoords.data(), GL_STATIC_DRAW );
    
    GLuint tCoord = glGetAttribLocation(progs[1].GetID(), "tCoord" );
    if (tCoord == -1) {
        std::cerr << "ERROR: Attribute 'tCoord' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( tCoord );
    glVertexAttribPointer(tCoord, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
 
    tex.SetImage(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, texture.data.data(), texture.width, texture.height );
    tex.BuildMipmaps();
    tex.Bind( 0 );
    progs[1]["tex"] = 0;

    glBindVertexArray(0);
    glUseProgram(0);

    // OTHER THING
    progs[0].Bind(); 
    glBindVertexArray(vaos[0]);
    camera[0].setPosition(cy::Vec3f(0.0f, 0.0f, 4.0));
    cy::Matrix4f model = cy::Matrix4f::Identity();
    cy::Matrix4f view = cy::Matrix4f::View(camera[0].pos, objectPos, camera[0].up); 
    cy::Matrix4f projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    mats[0].setMVP(model, view, projection);
    progs[0]["mvp"] = mats[0].mvp;

    renderBuffer.Initialize(
        true, // create depth buffer
        3, // RGB
        600, // width
        600 // height
    );

    //Unbind VAO after setup. IDK, maybe good practice.
    glBindVertexArray(0);
    glUseProgram(0);

    // Function Assignment
    glutDisplayFunc(display);          
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMotion);     
    glutKeyboardFunc(handleKeypress);
    // glutIdleFunc(idle); 
    // glutKeyboardFunc(keyboardDown);
    // glutKeyboardUpFunc(keyboardUp);

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