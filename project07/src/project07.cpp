#include <cstdio>
#include <unordered_map>
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

cy::GLSLProgram shadow_prog; 
cy::GLRenderDepth2D shadowMap;
GLuint shadow_obj_vao;
GLuint shadow_plane_vao;
GLuint shadow_obj_buffer; 
GLuint shadow_plane_buffer; 


cy::GLSLProgram prog;
GLuint obj_vao;
GLuint vertex_buffer; 
GLuint normal_buffer; 

GLuint plane_vao;
GLuint plane_vertex_buffer; 
GLuint plane_normal_buffer; 


aa::Object obj; 
aa::Camera camera; 
aa::Camera light_camera; 
aa::Mouse mouse;
aa::TransformationMatrices objectTransformations; 
aa::TransformationMatrices planeTransformations; 
aa::Light light;
float a_down; 
float d_down; 

bool swapyz; 
static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 


// OpenGL -----------------------------------
void display() {
    // SET UNIFORMS FOR DPETHMAP
    shadow_prog.Bind(); 
    shadowMap.Bind();
    glClear( GL_DEPTH_BUFFER_BIT );
    shadow_prog["swap"] = swapyz; 
    cy::Matrix4f light_camera_projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(4096)/float(4096), 0.1f, 100.0f);

    aa::TransformationMatrices shadow_obj = objectTransformations; 
    shadow_obj.setProjection(light_camera_projection);
    shadow_obj.setView(cy::Matrix4f::View(light_camera.pos, objectPos, light_camera.up));
    shadow_prog["mlp"] = shadow_obj.mvp; 

    glBindVertexArray(shadow_obj_vao);
    glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
    glBindVertexArray(0);

    aa::TransformationMatrices shadow_plane = planeTransformations; 
    shadow_plane.setProjection(light_camera_projection);
    shadow_plane.setView(cy::Matrix4f::View(light_camera.pos, objectPos, light_camera.up));
    shadow_prog["mlp"] = shadow_plane.mvp; 

    glBindVertexArray(shadow_plane_vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    shadowMap.Unbind();

    glBindVertexArray(0);
    glUseProgram(0);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind(); 

    shadowMap.BindTexture(0);
    prog["shadow"] = 0; 

    // CHECK MATRIX MULT ORDER
    // cy::Vec3f bias = light_camera.pos.GetNormalized() * 0.005f;
    // shadow_obj.mvp = cy::Matrix4f::Translation(cy::Vec3f(0.5f + bias.x, 0.5f + bias.y , 0.5f + bias.z))* cy::Matrix4f::Scale(0.5f) * shadow_obj.mvp; 
    shadow_obj.mvp = cy::Matrix4f::Translation(cy::Vec3f(0.5f, 0.5f, 0.5f ))* cy::Matrix4f::Scale(0.5f) * shadow_obj.mvp; 
    prog["shadowMatrix"] =  shadow_obj.mvp;

    prog["swap"] = swapyz;
    prog["cameraPos"] = camera.pos; 
 
    prog["mvp"] = objectTransformations.mvp;
    prog["m"] = objectTransformations.m; 
    prog["nrm_m"] = objectTransformations.nrm_m; 

    prog["Kd"] = obj.material.Kd; 
    prog["Ks"] = obj.material.Ks; 
    prog["alpha"] = obj.material.alpha; 

    prog["lightPos"] = light.pos; 
    prog["lightSpotDir"] = light.dir; 
    prog["cos_cuttoff"] = light.cos_cutoff; 
    prog["lightColor"] = light.color; 
    prog["lightIntensity"] = light.intensity; 

    prog["att_linear"] = light.att_linear; 
    prog["att_quadratic"] = light.att_quadratic; 

    glBindVertexArray(obj_vao);

    // IMPORTANT DO NOT DELETE
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
    glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
    // glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to solid rendering mode

    // Unbind VA0 and program
    glBindVertexArray(0);

    prog["mvp"] = planeTransformations.mvp;
    prog["m"] = planeTransformations.m; 
    prog["nrm_m"] = planeTransformations.nrm_m; 

    // shadow_plane.mvp = cy::Matrix4f::Translation(cy::Vec3f(0.5f + bias.x, 0.5f + bias.y , 0.5f + bias.z))* cy::Matrix4f::Scale(0.5f) * shadow_plane.mvp ; 
    shadow_plane.mvp = cy::Matrix4f::Translation(cy::Vec3f(0.5f, 0.5f, 0.5f))* cy::Matrix4f::Scale(0.5f) * shadow_plane.mvp ; 
    prog["shadowMatrix"] =  shadow_plane.mvp; 

    glBindVertexArray(plane_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();
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
    objectTransformations.setView(view); 
    planeTransformations.setView(view);

    mouse.last_x = x; 
    mouse.last_y = y; 

    glutPostRedisplay();
}

void keyboardDown(unsigned char key, int x, int y) {
    if (key == 27) {
        glDeleteVertexArrays(1, &obj_vao);
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(1, &normal_buffer);
        exit(0); 
    }
    if (key == 'z') {
        swapyz = !swapyz;
        cy::Matrix4f model; 
        cy::Matrix4f plane_model;

        float scale_factor = (obj.boxMax - obj.boxMin).Length();
        cy::Matrix4f::Scale(2.0f / scale_factor);
        if (swapyz) {
            cy::Vec3f plane_translation(0.0f, -(obj.center.z - obj.boxMin.z), 0.0f);
            plane_model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(plane_translation);
            model = cy::Matrix4f::Scale(2.0f / scale_factor) *  cy::Matrix4f::Translation(cy::Vec3f(-obj.center.x, -obj.center.z,-obj.center.y));
        } else {
            cy::Vec3f plane_translation(0.0f,0.0f,  -(obj.center.z - obj.boxMin.z));
            plane_model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(plane_translation);
            model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(-obj.center);
        }

        light_camera.setPosition(cy::Vec3f(5.0f, 5.0f, 5.0f));
        light.setPosition(light_camera.pos); 
        camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));
        cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
        objectTransformations.setMV(model, view); 
        planeTransformations.setMV(plane_model, view);

        glutPostRedisplay();
    }
    if (key == 'a')
        a_down = true;
    if (key == 'd')
        d_down = true;
}

// GLUT callback for key release.
void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'a')
        a_down = false;
    if (key == 'd')
        d_down = false;
}

void idle() {
    const float rotationSpeed = 0.01f; // Adjust rotation speed as desired
    cy::Vec3f axis(0.0f, 0.0f, 1.0f); 
    if (swapyz)
        axis =  cy::Vec3f(0.0f, 1.0f, 0.0f); 
    if (a_down) {
        light_camera.fixedHorizontalOriginRotation(-rotationSpeed, axis);
        light.pos = light_camera.pos;
        light.dir = light_camera.normal;
    }
    if (d_down) {
        light_camera.fixedHorizontalOriginRotation(rotationSpeed, axis);
        light.setPosition(light_camera.pos);
    }
    
    glutPostRedisplay();
}

// void handleKeypress(unsigned char key, int x, int y) {
//     switch (key) {
//         case 27: 
//             glDeleteVertexArrays(1, &obj_vao);
//             glDeleteBuffers(1, &vertex_buffer);
//             glDeleteBuffers(1, &normal_buffer);
//             exit(0); 
//             break;
//         case 'z':
//             swapyz = !swapyz;
//             cy::Matrix4f model; 
//             cy::Matrix4f plane_model;

//             float scale_factor = (obj.boxMax - obj.boxMin).Length();
//             cy::Matrix4f::Scale(2.0f / scale_factor);
//             if (swapyz) {
//                 cy::Vec3f plane_translation(0.0f, -(obj.center.z - obj.boxMin.z), 0.0f);
//                 plane_model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(plane_translation);
//                 model = cy::Matrix4f::Scale(2.0f / scale_factor) *  cy::Matrix4f::Translation(cy::Vec3f(-obj.center.x, -obj.center.z,-obj.center.y));
//             } else {
//                 cy::Vec3f plane_translation(0.0f,0.0f,  -(obj.center.z - obj.boxMin.z));
//                 plane_model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(plane_translation);
//                 model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(-obj.center);
//             }

//             camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));
//             cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
//             objectTransformations.setMV(model, view); 
//             planeTransformations.setMV(plane_model, view);

//             glutPostRedisplay();
//     }
// }

int main(int argc, char** argv) {
    if (argc < 2) {  // Ensure an argument is provided
        std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
        return -1;
    }

    std::string objFilePath = argv[1];  // Get OBJ file path from the command line
    std::cout << "Loading OBJ file: " << objFilePath << std::endl;


    obj = aa::Object(objFilePath);    
    if (obj.vertices.empty()) {
        std::cerr << "Failed to load OBJ file: " << objFilePath << std::endl;
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

    glGenVertexArrays( 1, &obj_vao );
    glGenBuffers(1, &vertex_buffer); 
    glGenBuffers(1, &normal_buffer); 

    // Scale factor
    float scale_factor = (obj.boxMax - obj.boxMin).Length();


    // camera.setPosition(cy::Vec3f(0.0f, 0.0f, obj.boxDimensions.z * 3.0f));
    camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));


    cy::Matrix4f model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(-obj.center);
    // cy::Matrix4f model = cy::Matrix4f::Translation(-obj.center);
    cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    cy::Matrix4f projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    objectTransformations.setMVP(model, view, projection);

    swapyz = false; 

    prog.Bind(); 

    glBindVertexArray(obj_vao);

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

    cy::Vec3f plane_translation(0.0f,  -0.0f, -(obj.center.z - obj.boxMin.z));
    cy::Matrix4f plane_model = cy::Matrix4f::Scale(2.0f / scale_factor) * cy::Matrix4f::Translation(plane_translation);
    cy::Matrix4f plane_view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    cy::Matrix4f plane_projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    planeTransformations.setMVP(plane_model, plane_view, plane_projection);

    GLfloat plane_vertices[] = {
        -obj.boxDimensions.z * 3.0f, -obj.boxDimensions.z * 3.0f, 0.0f,
        obj.boxDimensions.z * 3.0f, -obj.boxDimensions.z * 3.0f, 0.0f,
        -obj.boxDimensions.z * 3.0f,  obj.boxDimensions.z * 3.0f, 0.0f,
        obj.boxDimensions.z * 3.0f,  obj.boxDimensions.z * 3.0f, 0.0f,
    };

    GLfloat plane_normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    }; 

    glGenVertexArrays( 1, &plane_vao );
    glGenBuffers(1, &plane_vertex_buffer); 
    glGenBuffers(1, &plane_normal_buffer); 

    glBindVertexArray(plane_vao);

    glBindBuffer(GL_ARRAY_BUFFER, plane_vertex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW );
    GLuint plane_pos = glGetAttribLocation(prog.GetID(), "pos" );
    if (plane_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( plane_pos );
    glVertexAttribPointer(plane_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindBuffer(GL_ARRAY_BUFFER, plane_normal_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_normals), plane_normals, GL_STATIC_DRAW );
    GLuint plane_nrm = glGetAttribLocation(prog.GetID(), "nrm" );
    if (plane_nrm == -1) {
        std::cerr << "ERROR: Attribute 'nrm' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( plane_nrm );
    glVertexAttribPointer(plane_nrm, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindVertexArray(0);
    glUseProgram(0);

    // SHADOW PROG
    shadow_prog.BuildFiles( "src/shadow.vert", "src/shadow.frag" );
    glGenVertexArrays( 1, &shadow_obj_vao );
    glGenVertexArrays( 1, &shadow_plane_vao );
    glGenBuffers(1, &shadow_obj_buffer); 
    glGenBuffers(1, &shadow_plane_buffer); 


    // CREATE LIGHT TRANSFORMATIONS
    light_camera.setPosition(cy::Vec3f(light.pos.x, light.pos.y, light.pos.z));
    // light_camera.setPosition(cy::Vec3f(light.pos.x, light.pos.z, light.pos.y));
    shadow_prog.Bind(); 

    glBindVertexArray(shadow_obj_vao);

    glBindBuffer(GL_ARRAY_BUFFER, shadow_obj_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*obj.vertices.size(), obj.vertices.data(), GL_STATIC_DRAW );
    GLuint shadow_pos = glGetAttribLocation(shadow_prog.GetID(), "pos" );
    if (shadow_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( shadow_pos );
    glVertexAttribPointer(shadow_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindVertexArray(0);

    glBindVertexArray(shadow_plane_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, shadow_plane_buffer); 
    // glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW );
    GLuint shadow_plane_pos = glGetAttribLocation(shadow_prog.GetID(), "pos" );
    if (shadow_plane_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( shadow_plane_pos );
    glVertexAttribPointer(shadow_plane_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );


    shadowMap.Initialize(
        true, // depth comparison texture
        4096, // width
        4096 // height
    );
    shadowMap.SetTextureFilteringMode( GL_LINEAR, GL_LINEAR );

    glBindVertexArray(0);
    glUseProgram(0);

    // Function Assignment
    glutDisplayFunc(display);              
    // glutKeyboardFunc(handleKeypress);
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMotion); 

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutIdleFunc(idle);

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