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
cy::GLSLProgram wireframeProg;
GLuint wireframe_vao;
GLuint wireframe_buffer;
GLuint wireframe_tex_buffer;

cy::GLSLProgram prog;
GLuint plane_vao;
GLuint plane_vertex_buffer; 
GLuint plane_normal_buffer; 
GLuint nrm_tex_coord_buffer;

cyGLTexture2D normal_tex;
cyGLTexture2D displacement_tex;

aa::Object plane; 
aa::Camera camera; 
aa::Camera light_camera; 
aa::Mouse mouse;
aa::TransformationMatrices planeTransformations; 
aa::Light light;
float a_down; 
float d_down; 
bool showWF; 
bool swapyz; 
bool use_displacement;
float tess_scale = 0.0;  
float max_tess_scale = 6.0; 
float disp_scale = 0.0f; 
static cy::Vec3f objectPos(0.0f, 0.0f, 0.0f); 


// OpenGL -----------------------------------
void display() {
    cy::Matrix4f light_camera_projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(4096)/float(4096), 0.1f, 100.0f);

    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind(); 

    glActiveTexture(GL_TEXTURE0);
    normal_tex.Bind(0);
    prog["normal_tex"] = 0; 

    glActiveTexture(GL_TEXTURE1);
    displacement_tex.Bind(1);
    prog["displacement_tex"] = 1; 

    prog["swap"] = swapyz;
    prog["cameraPos"] = camera.pos; 

    prog["Kd"] = plane.material.Kd; 
    prog["Ks"] = plane.material.Ks; 
    prog["alpha"] = plane.material.alpha; 

    prog["lightPos"] = light.pos; 
    prog["lightSpotDir"] = light.dir; 
    prog["cos_cuttoff"] = light.cos_cutoff; 
    prog["lightColor"] = light.color; 
    prog["lightIntensity"] = light.intensity; 

    prog["att_linear"] = light.att_linear; 
    prog["att_quadratic"] = light.att_quadratic; 

    prog["mvp"] = planeTransformations.mvp;
    prog["m"] = planeTransformations.m; 
    prog["nrm_m"] = planeTransformations.nrm_m; 

    prog["tess_scale"] = tess_scale; 
    prog["disp_scale"] = disp_scale; 

    glBindVertexArray(plane_vao);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArrays(GL_PATCHES, 0, 4); 

    glBindVertexArray(0);
    glUseProgram(0);

    if (showWF) {
        wireframeProg.Bind(); 
        wireframeProg["normal_tex"] = 0; 
        wireframeProg["displacement_tex"] = 1; 
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        wireframeProg["tess_scale"] = tess_scale; 
        wireframeProg["disp_scale"] = disp_scale; 
        wireframeProg["mvp"] = planeTransformations.mvp;
        wireframeProg["swap"] = swapyz; 
        glBindVertexArray(plane_vao);
        glDrawArrays(GL_PATCHES, 0, 4); 
        glBindVertexArray(0);
        glUseProgram(0);
        glEnable(GL_DEPTH_TEST);
    }

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
    planeTransformations.setView(view);

    mouse.last_x = x; 
    mouse.last_y = y; 

    glutPostRedisplay();
}

void keyboardDown(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0); 
    }
    if (key == 'z') {
        swapyz = !swapyz;
        cy::Matrix4f model; 

        light_camera.setPosition(cy::Vec3f(5.0f, 5.0f, 5.0f));
        light.setPosition(light_camera.pos); 
        camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));
        cy::Matrix4f view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
        planeTransformations.setMV(cy::Matrix4f::Identity(), view);

        glutPostRedisplay();
    }
    if (key == ' ') {
        showWF = !showWF;
        glutPostRedisplay();
    }
    if (key == 'a')
        a_down = true;
    if (key == 'd')
        d_down = true;
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'a')
        a_down = false;
    if (key == 'd')
        d_down = false;
}

void specialKeyCallback(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  {
        if (tess_scale == 0.0) {
            return; 
        }
        tess_scale--; 
        glutPostRedisplay();
    }
    if (key == GLUT_KEY_RIGHT) {
        if (tess_scale == max_tess_scale) {
            return;
        }
        tess_scale++; 
        glutPostRedisplay();
    }
    if (key == GLUT_KEY_DOWN)  {
        if (disp_scale <= 0.0f) {
            disp_scale = 0.0f;
            return; 
        }
        disp_scale = disp_scale - 0.055f; 
        glutPostRedisplay();
    }
    if (key == GLUT_KEY_UP) {
        if (disp_scale >= 0.33f) {
            disp_scale = 0.33f;
            return;
        }
        disp_scale = disp_scale + 0.055f; 
        glutPostRedisplay();
    }
    glutPostRedisplay(); 
}


void idle() {
    const float rotationSpeed = 0.01f; 
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

int main(int argc, char** argv) {
    use_displacement = false;

    aa::Texture normal_map; 
    aa::Texture displacement_map; 
    const char*  normalMapPath;
    const char*  displacementMapPath;
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <normal_map.png> [displacement_map.png]" << std::endl;
        return EXIT_FAILURE;
    }

    normalMapPath = argv[1];
    if (!normal_map.loadTexture(normalMapPath))
    {
        std::cerr << "Failed to load: " << argv[1] << std::endl;
    }

    if (argc >= 3) {
        displacementMapPath = argv[2];
        if (!displacement_map.loadTexture(displacementMapPath))
        {
            std::cerr << "Failed to load: " << argv[1] << std::endl;
        }
        use_displacement = true;
        std::cout << "Using normal map: " << normalMapPath << std::endl;
        std::cout << "Using displacement map: " << displacementMapPath << std::endl;
    } else {
        std::cout << "Using normal map: " << normalMapPath << std::endl;
        std::cout << "No displacement map provided. Displacement mapping will be disabled." << std::endl;
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

    prog.BuildFiles( "src/shader.vert", "src/shader.frag","src/shader.gs","src/shader.tcs","src/shader.tes");

    camera.setPosition(cy::Vec3f(0.0f, 0.0f, 2.0f));

    swapyz = false; 
    showWF = false; 
    prog.Bind(); 

    glGenVertexArrays( 1, &plane_vao );
    glGenBuffers(1, &plane_vertex_buffer); 
    glGenBuffers(1, &plane_normal_buffer); 
    glGenBuffers(1, &nrm_tex_coord_buffer);

    cy::Matrix4f plane_view = cy::Matrix4f::View(camera.pos, objectPos, camera.up); 
    cy::Matrix4f plane_projection = cy::Matrix4f::Perspective(DEG2RAD(45.0f), float(800)/float(600), 0.1f, 1000.0f);
    planeTransformations.setMVP(cy::Matrix4f::Identity(), plane_view, plane_projection);

    plane.vertices.push_back(cy::Vec3f(-1.0f, -1.0f, 0.0f)); // Bottom-left
    plane.vertices.push_back(cy::Vec3f(1.0f, -1.0f, 0.0f));  // Bottom-right
    plane.vertices.push_back(cy::Vec3f(1.0f, 1.0f, 0.0f));   // Top-right
    plane.vertices.push_back(cy::Vec3f(-1.0f, 1.0f, 0.0f));  // Top-left

    plane.material = aa::Material();

    GLfloat nrm_tex_coords[] = {
        0.0f, 1.0f,   // Bottom-left
        1.0f, 1.0f,   // Bottom-right
        1.0f, 0.0f,   // Top-right
        0.0f, 0.0f,   // Top-left
    };

    glBindVertexArray(plane_vao);

    glBindBuffer(GL_ARRAY_BUFFER, plane_vertex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*plane.vertices.size(), plane.vertices.data(), GL_STATIC_DRAW );
    GLuint plane_pos = glGetAttribLocation(prog.GetID(), "pos" );
    if (plane_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( plane_pos );
    glVertexAttribPointer(plane_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

    glBindBuffer(GL_ARRAY_BUFFER, nrm_tex_coord_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(nrm_tex_coords), nrm_tex_coords, GL_STATIC_DRAW );
    
    GLuint tCoord = glGetAttribLocation(prog.GetID(), "tCoord" );
    if (tCoord == -1) {
        std::cerr << "ERROR: Attribute 'tCoord' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( tCoord );
    glVertexAttribPointer(tCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
 

    normal_tex.Initialize();
    normal_tex.SetImage(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, normal_map.data.data(), normal_map.width, normal_map.height );
    normal_tex.BuildMipmaps();

    if (use_displacement){
        displacement_tex.Initialize();
        displacement_tex.SetImage(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, displacement_map.data.data(), displacement_map.width, displacement_map.height );
        displacement_tex.BuildMipmaps();
    }

    
    glBindVertexArray(0);
    glUseProgram(0);

    light_camera.setPosition(cy::Vec3f(light.pos.x, light.pos.y, light.pos.z));
    

    wireframeProg.BuildFiles( 
        "src/wireframe.vs", 
        "src/wireframe.fs", 
        "src/wireframe.gs",
        "src/wireframe.tcs",
        "src/wireframe.tes"
    );


    wireframeProg.Bind();

    glGenVertexArrays( 1, &wireframe_vao );
    glBindVertexArray(wireframe_vao);
    glGenBuffers(1, &wireframe_buffer); 
    glGenBuffers(1, &wireframe_tex_buffer); 


    glBindBuffer(GL_ARRAY_BUFFER, wireframe_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*plane.vertices.size(), plane.vertices.data(), GL_STATIC_DRAW );
    GLuint wireframe_pos = glGetAttribLocation(wireframeProg.GetID(), "pos" );
    if (wireframe_pos == -1) {
        std::cerr << "ERROR: Attribute 'pos' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( wireframe_pos );
    glVertexAttribPointer(wireframe_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );




    glBindBuffer(GL_ARRAY_BUFFER, wireframe_tex_buffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(nrm_tex_coords), nrm_tex_coords, GL_STATIC_DRAW );
    
    GLuint wtCoord = glGetAttribLocation(wireframeProg.GetID(), "tCoord" );
    if (wtCoord == -1) {
        std::cerr << "ERROR: Attribute 'tCoord' not found in shader!" << std::endl;
    }
    glEnableVertexAttribArray( wtCoord );
    glVertexAttribPointer(wtCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
 
    glBindVertexArray(0);
    glUseProgram(0);
    // Function Assignment
    glutDisplayFunc(display);              
    // glutKeyboardFunc(handleKeypress);
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMotion); 

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyCallback);
    glutIdleFunc(idle);

    // OPENGL Inits
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    // Main Loop
    glutMainLoop(); 
    return 0; 
}

