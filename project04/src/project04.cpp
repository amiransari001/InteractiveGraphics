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


// Probably bad practice. FIX FOR NEXTIME IF IT RUNS
cy::GLSLProgram prog;
GLuint vao;
GLuint ebo; 
GLuint vertex_buffer; 
GLuint normal_buffer; 
GLuint tex_coord_buffer; 
cyGLTexture2D tex;


aa::Object obj; 
aa::Camera camera; 
aa::Mouse mouse;
aa::TransformationMatrices tMatrices; 
aa::Light light;
aa::Texture texture; 

bool swapyz; 
static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 


// OpenGL -----------------------------------
void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind(); 
    
    prog["swap"] = swapyz;
    prog["cameraPos"] = camera.pos; 
 
    prog["mvp"] = tMatrices.mvp;
    prog["m"] = tMatrices.m; 
    prog["nrm_m"] = tMatrices.nrm_m; 

    prog["Kd"] = obj.material.Kd; 
    prog["Ks"] = obj.material.Ks; 
    prog["alpha"] = obj.material.alpha; 

    prog["lightDir"] = light.dir; 
    prog["lightColor"] = light.color; 
    prog["lightIntensity"] = light.intensity; 

    glBindVertexArray(vao);

    // IMPORTANT DO NOT DELETE
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
    glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
    // glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to solid rendering mode

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

    cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    // cy::Matrix4f view = camera.createView(); 
    tMatrices.setView(view); 

    mouse.last_x = x; 
    mouse.last_y = y; 

    glutPostRedisplay();
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: 
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vertex_buffer);
            glDeleteBuffers(1, &normal_buffer);
            glDeleteBuffers(1, &ebo);
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

            camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));
            cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
            tMatrices.setMV(model, view); 

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

    prog.BuildFiles( "src/shader.vert", "src/shader.frag" );

    glGenVertexArrays( 1, &vao );    
    glGenBuffers(1, &ebo); 
    glGenBuffers(1, &vertex_buffer); 
    glGenBuffers(1, &normal_buffer); 
    glGenBuffers(1, &tex_coord_buffer); 
    tex.Initialize();


    // obj = aa::Object("../assets/objs/teapot.obj");
    // texture.loadTexture("../assets/txts/brick.png");


    camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));

    cy::Matrix4f model = cy::Matrix4f::Translation(-obj.center);
    cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    cy::Matrix4f projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    tMatrices.setMVP(model, view, projection);

    swapyz = false; 

    prog.Bind(); 

    prog["swap"] = swapyz;
    prog["cameraPos"] = camera.pos; 
    prog["mvp"] = tMatrices.mvp; 
    prog["m"] = tMatrices.m; 
    prog["nrm_m"] = tMatrices.nrm_m; 

    prog["Kd"] = obj.material.Kd; 
    prog["Ks"] = obj.material.Ks; 
    prog["alpha"] = obj.material.alpha; 

    prog["lightDir"] = light.dir; 
    prog["lightColor"] = light.color; 
    prog["lightIntensity"] = light.intensity; 

    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size() * sizeof(unsigned int), obj.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.vertices.size(), obj.vertices.data(), GL_STATIC_DRAW );

    GLuint pos = glGetAttribLocation(prog.GetID(), "pos" );
    if (pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( pos );
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );


    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.normals.size(), obj.normals.data(), GL_STATIC_DRAW );
    
    GLuint nrm = glGetAttribLocation(prog.GetID(), "nrm" );
    if (nrm == -1) {
        std::cerr << "ERROR: Attribute 'nrm' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( nrm );
    glVertexAttribPointer(nrm, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindBuffer(GL_ARRAY_BUFFER, tex_coord_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.textureCoords.size(), obj.textureCoords.data(), GL_STATIC_DRAW );
    
    GLuint tCoord = glGetAttribLocation(prog.GetID(), "tCoord" );
    if (tCoord == -1) {
        std::cerr << "ERROR: Attribute 'tCoord' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( tCoord );
    glVertexAttribPointer(tCoord, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
 
    tex.SetImage(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, texture.data.data(), texture.width, texture.height );
    tex.BuildMipmaps();
    tex.Bind( 0 );
    prog["tex"] = 0;

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

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