#ifndef GLOBALS_H
#define GLOBALS_H

#include "cyMatrix.h"
#include "cyTriMesh.h"
#include "cyVector.h"
#include <string>
#include <iostream>

namespace aa {

    class Object {
    public:
        cy::TriMesh mesh; 
        cy::Vec3f boxMin;
        cy::Vec3f boxMax;
        cy::Vec3f center;
        cy::Vec3f boxDimensions;


         Object(const std::string& objFilePath) {
            if (!mesh.LoadFromFileObj(objFilePath.c_str())) {
                std::cerr << "Error: Could not load OBJ file: " << objFilePath << std::endl;
                return;
            }

            mesh.ComputeBoundingBox();
            boxMin = mesh.GetBoundMin();
            boxMax = mesh.GetBoundMax();
            center = (boxMin + boxMax) * 0.5f;
            boxDimensions = (boxMax - boxMin);

            std::cout << "Loaded OBJ: " << objFilePath << std::endl;
        }
        Object() = default;
    };

    class Camera {
    public: 
        cy::Vec3f pos;
        cy::Vec3f normal;
        cy::Vec3f right;
        cy::Vec3f up;

        // Initializes position at origin? Not sure
        Camera(cy::Vec3f position) {
            setPosition(position);
        }
        Camera() = default;


        void setPosition(cy::Vec3f position) {
            pos = position; 
            normal = -position.GetNormalized();
            
            cy::Vec3f worldUp(0.0f, 1.0f, 0.0f);
            right = worldUp.Cross(normal).GetNormalized();
            up = normal.Cross(right).GetNormalized();
        }

        void move(int distance) {
            float d = distance; 
            pos += normal*d; 
        }

        void rotateAroundOrigin(float angleChangeX, float angleChangeY) {
            cy::Matrix4f rotX = cy::Matrix4f::Rotation(right, angleChangeX);
            cy::Matrix4f rotY = cy::Matrix4f::Rotation(up, angleChangeY);

            normal = (rotX * rotY * cy::Vec4f(normal, 0.0f)).XYZ().GetNormalized();
            up = (rotX * rotY * cy::Vec4f(up, 0.0f)).XYZ().GetNormalized();
            right = (rotX * rotY * cy::Vec4f(right, 0.0f)).XYZ().GetNormalized();

            pos = -normal * pos.Length();
        }

        cy::Matrix4f getViewMatrix() {
            // Ensure vectors are normalized
            normal.Normalize();
            right.Normalize();
            up.Normalize();

            // Create a view matrix manually
            cy::Matrix4f view(
                right.x, up.x, -normal.x, 0.0f,
                right.y, up.y, -normal.y, 0.0f,
                right.z, up.z, -normal.z, 0.0f,
                -right.Dot(pos), -up.Dot(pos), normal.Dot(pos), 1.0f
            );

            return view;
        }

    };

    class Trans {
    public:
        cy::Matrix4f mvp, m, v, p;
        // cy::Matrix4f o;
        // char projectionType;

        Trans(); 

        void setModel(const cy::Matrix4f& model) { m = model; }
        void setView(const cy::Matrix4f& view) { v = view; }
        void setProjection(const cy::Matrix4f& projection) { p = projection; }
        void setMVP(const cy::Matrix4f& modelViewProjection) { mvp = modelViewProjection; }

        // void setOrthographic(float left, float right, float bottom, float top, float near, float far) {
        //     cy::Matrix4f ortho;
        //     ortho.SetIdentity();  // Initialize to identity matrix
        //     ortho[0]  = 2.0f / (right - left);
        //     ortho[5]  = 2.0f / (top - bottom);
        //     ortho[10] = -2.0f / (far - near);
        //     ortho[12] = -(right + left) / (right - left);
        //     ortho[13] = -(top + bottom) / (top - bottom);
        //     ortho[14] = -(far + near) / (far - near);
        //     ortho[15] = 1.0f;
        //     o = ortho; 
        // }       
    };

    class Mouse {
    public: 
        int last_x; 
        int last_y;
        int button; 

        Mouse () {
            button = -1; 
        }
    };



}

#endif