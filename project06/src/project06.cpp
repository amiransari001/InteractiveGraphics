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

cy::GLSLProgram skybox_prog; 
GLuint skybox_vao; 
GLuint skybox_vertex_buffer; 

cy::GLSLProgram prog;
GLuint vao;
GLuint vertex_buffer; 
GLuint normal_buffer; 

cy::GLTextureCubeMap envmap;

aa::Object obj; 
aa::Camera camera; 
aa::Mouse mouse;
aa::TransformationMatrices tMatrices; 
aa::Light light;

bool swapyz; 
static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 


// OpenGL -----------------------------------
void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind(); 
    glDepthMask(GL_TRUE);
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

    glActiveTexture(GL_TEXTURE0);
    envmap.Bind(0);
    prog["env"] = 0;

    glBindVertexArray(vao);

    // IMPORTANT DO NOT DELETE
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
    glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
    // glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to solid rendering mode

    glBindVertexArray(0);
    glUseProgram(0);

    glDepthMask(GL_FALSE);
    skybox_prog.Bind(); 
    glActiveTexture(GL_TEXTURE1);
    envmap.Bind(1);
    skybox_prog["skybox"] = 1;
    cy::Matrix4f skybox_view(tMatrices.v); 
    skybox_view.SetNoTranslation(); 
    skybox_prog["v"] = skybox_view; 
    // skybox_prog["v"] = tMatrices.v; 
    skybox_prog["p"] = tMatrices.p; 

    glBindVertexArray(skybox_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);
    // Unbind VA0 and program
    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();
}

// void idle() {
//     // glutPostRedisplay();
// }

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
            exit(0); 
            break;
        case 'z':
            swapyz = !swapyz;
            cy::Matrix4f model; 
            if (swapyz) {
                model = cy::Matrix4f::Translation(-obj.center) * cy::Matrix4f::Scale(1.0/((obj.boxMax - obj.boxMin).Length()));

                model = cy::Matrix4f::Translation(cy::Vec3f(-obj.center.x, -obj.center.z,-obj.center.y));
            } else {
                // model = cy::Matrix4f::Translation(-obj.center) * cy::Matrix4f::Scale(1.0/((obj.boxMax - obj.boxMin).Length()));;
                model = cy::Matrix4f::Translation(-obj.center);
            }

            camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));
            // camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));
            cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
            tMatrices.setMV(model, view); 

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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutCreateWindow("Hello World");

    // GLEW Initializations
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    CY_GL_REGISTER_DEBUG_CALLBACK;


// LOADING DATA
    // LOAD COMMAND LINE ARGS (OBJ FILE PATH)
    if (argc < 2) {  
        std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
        return -1;
    }

    // Get AND LOAD OBJ FILE    
    std::string objFilePath = argv[1];  
    
    obj = aa::Object(objFilePath);    
    if (obj.vertices.empty()) {
        std::cerr << "Failed to load OBJ file: " << objFilePath << std::endl;
        return -1;
    }

    // CREATE SKYBOX COORDS
    float skybox_vertices[] = {
    // +X face (B, F, G, G, C, B)
     1.0f, -1.0f, -1.0f,   // B
     1.0f, -1.0f,  1.0f,   // F
     1.0f,  1.0f,  1.0f,   // G
     1.0f,  1.0f,  1.0f,   // G
     1.0f,  1.0f, -1.0f,   // C
     1.0f, -1.0f, -1.0f,   // B

    // -X face (D, A, E, E, H, D)
    -1.0f,  1.0f, -1.0f,   // D
    -1.0f, -1.0f, -1.0f,   // A
    -1.0f, -1.0f,  1.0f,   // E
    -1.0f, -1.0f,  1.0f,   // E
    -1.0f,  1.0f,  1.0f,   // H
    -1.0f,  1.0f, -1.0f,   // D

    // +Y face (D, C, G, G, H, D)
    -1.0f,  1.0f, -1.0f,   // D
     1.0f,  1.0f, -1.0f,   // C
     1.0f,  1.0f,  1.0f,   // G
     1.0f,  1.0f,  1.0f,   // G
    -1.0f,  1.0f,  1.0f,   // H
    -1.0f,  1.0f, -1.0f,   // D

    // -Y face (A, B, F, F, E, A)
    -1.0f, -1.0f, -1.0f,   // A
     1.0f, -1.0f, -1.0f,   // B
     1.0f, -1.0f,  1.0f,   // F
     1.0f, -1.0f,  1.0f,   // F
    -1.0f, -1.0f,  1.0f,   // E
    -1.0f, -1.0f, -1.0f,   // A

    // +Z face (E, F, G, G, H, E)
    -1.0f, -1.0f,  1.0f,   // E
     1.0f, -1.0f,  1.0f,   // F
     1.0f,  1.0f,  1.0f,   // G
     1.0f,  1.0f,  1.0f,   // G
    -1.0f,  1.0f,  1.0f,   // H
    -1.0f, -1.0f,  1.0f,   // E

    // -Z face (C, B, A, A, D, C)
     1.0f,  1.0f, -1.0f,   // C
     1.0f, -1.0f, -1.0f,   // B
    -1.0f, -1.0f, -1.0f,   // A
    -1.0f, -1.0f, -1.0f,   // A
    -1.0f,  1.0f, -1.0f,   // D
     1.0f,  1.0f, -1.0f    // C
};

    for (int i = 0; i < 108; i++) {
        skybox_vertices[i] = skybox_vertices[i] * 500.0f;
    }


    // LOAD AND INITIALIZE ENVIRONMENT CUBE MAP 
    aa::Texture posx;
    posx.loadTexture("background01/cubemap_posx.png");
    aa::Texture negx;
    negx.loadTexture("background01/cubemap_negx.png");
    aa::Texture posy;
    posy.loadTexture("background01/cubemap_posy.png");
    aa::Texture negy;
    negy.loadTexture("background01/cubemap_negy.png");
    aa::Texture posz;
    posz.loadTexture("background01/cubemap_posz.png");
    aa::Texture negz;
    negz.loadTexture("background01/cubemap_negz.png");
    
    envmap.Initialize();

    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 0, posx.data.data(), posx.width, posx.height );
    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 1, negx.data.data(), negx.width, negx.height );
    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 2, posy.data.data(), posy.width, posy.height );
    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 3, negy.data.data(), negy.width, negy.height );
    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 4, posz.data.data(), posz.width, posz.height );
    envmap.SetImageRGBA((cy::GLTextureCubeMap::Side) 5, negz.data.data(), negz.width, negz.height );

    glActiveTexture(GL_TEXTURE1);
    envmap.Bind(1);
    envmap.BuildMipmaps();
    envmap.SetSeamless();
    glActiveTexture(GL_TEXTURE0);
    envmap.Bind(0);
    envmap.BuildMipmaps();
    envmap.SetSeamless();
    
    // UNIFORM VARIABLE DEFINITION
    camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));
    // camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));
    cy::Matrix4f model = cy::Matrix4f::Translation(-obj.center);
    // cy::Matrix4f model = cy::Matrix4f::Translation(-obj.center) * cy::Matrix4f::Scale(1.0/((obj.boxMax - obj.boxMin).Length()));
    cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    cy::Matrix4f projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    tMatrices.setMVP(model, view, projection);
    swapyz = false; 
    
    // OBJECT PROG INITIALIZATIONS
    prog.BuildFiles( "src/shader.vert", "src/shader.frag" );

    glGenVertexArrays( 1, &vao );    
    glGenBuffers(1, &vertex_buffer); 
    glGenBuffers(1, &normal_buffer); 

    // OBJ PROG SETUP 
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
    
    glBindVertexArray(0);
    glUseProgram(0);

    // SKYCUBE PROGRAM SETUP
    skybox_prog.BuildFiles("src/skybox.vert", "src/skybox.frag" );
    
    cy::Matrix4f skybox_view(tMatrices.v); 
    skybox_view.SetNoTranslation(); 
    skybox_prog["v"] = skybox_view; 
    skybox_prog["p"] = tMatrices.p; 
    
    glGenVertexArrays(1, &skybox_vao); 
    glGenBuffers(1, &skybox_vertex_buffer); 

    skybox_prog.Bind(); 
    glBindVertexArray(skybox_vao); 

    // SKYBOX VERTEX BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW );
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36, skybox_vertices, GL_STATIC_DRAW );


    GLuint skybox_pos = glGetAttribLocation(skybox_prog.GetID(), "skybox_pos" );
    if (skybox_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' from skybox prog not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( skybox_pos );
    glVertexAttribPointer(skybox_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );


    //Unbind VAO after setup. IDK, maybe good practice.
    glBindVertexArray(0);
    glUseProgram(0);

    // Function Assignment
    glutDisplayFunc(display);              
    glutKeyboardFunc(handleKeypress);
    // glutIdleFunc(idle); 
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMotion); 

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