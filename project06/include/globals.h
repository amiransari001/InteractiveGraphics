#ifndef GLOBALS_H
#define GLOBALS_H

#include "cyMatrix.h"
#include "cyTriMesh.h"
#include "cyVector.h"
#include <string>
#include <iostream>
#include <vector>
#include "lodepng.h"

namespace aa {


    class Texture {
    public: 
        unsigned width; 
        unsigned height; 
        std::vector<unsigned char> data; 

        bool loadTexture(const char* filename) {
            unsigned error = lodepng::decode(data, width, height, filename);
                if (error) {
                    std::cerr << "LodePNG Error: " << lodepng_error_text(error) << std::endl;
                    return false;
                }

                // Successfully loaded texture data
                std::cout << "Loaded PNG: " << filename << " (" << width << "x" << height << ")\n";
                return true;
        }

    };

    class Light {
    public: 
        cy::Vec3f dir; 
        // cy::Vec3f intensity; 
        float intensity; 
        cy::Vec3f color; 
        // cy::Vec3f pos; 

        Light() { 
            dir = cy::Vec3f(-1.0f, 0.0f, 0.0f); 
            // intensity = cy::Vec3f(1.0f, 1.0f, 1.0f);
            intensity = 1.0; 
            color = cy::Vec3f(1.0f, 1.0f, 1.0f);
        };
    };

    class Material {
    public:
        cy::Vec3f Kd; 
        cy::Vec3f Ks; 
        float alpha;

        Material () {
            Kd = cy::Vec3f(0.0f, 0.5f, 0.5f);
            // Kd = cy::Vec3f(0.0f, 1.0f, 0.0f);
            Ks = cy::Vec3f(1.0f, 1.0f, 1.0f); 
            alpha = 30.0f; 
        }; 

        void setMaterial(cy::Vec3f diff_coef, cy::Vec3f spec_coef, float shine) {
            Kd = diff_coef; 
            Ks = spec_coef; 
            alpha = shine; 
        }
    };

    class Object {
    public:
        cy::TriMesh mesh; 
        cy::Vec3f boxMin;
        cy::Vec3f boxMax;
        cy::Vec3f center;
        cy::Vec3f boxDimensions;
        std::vector<cy::Vec3f> vertices; 
        std::vector<cy::Vec3f> normals; 
        std::vector<cy::Vec3f> textureCoords; 
        std::vector<unsigned int> indices; 
        aa::Material material; 


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

            for (unsigned int i = 0; i < mesh.NF(); i ++) {
                for (unsigned int j = 0; j < 3; j ++) {                    
                    unsigned int vIndex = mesh.F(i).v[j]; 
                    unsigned int nIndex = mesh.FN(i).v[j]; 
                    unsigned int tIndex = mesh.FT(i).v[j]; 

                    vertices.push_back(mesh.V(vIndex));
                    textureCoords.push_back(mesh.VT(tIndex));

                    if (mesh.HasNormals()) {

                        normals.push_back(mesh.VN(nIndex));
                    }
                    indices.push_back(vertices.size() - 1);
                }
            }

            // DRAWING WITH INDICES CORRECT
            // for (unsigned int i = 0; i < mesh.NF(); i ++) {
            //     for (unsigned int j = 0; j < 3; j ++) {
            //         indices.push_back(mesh.F(i).v[j]);
            //     }
            // }
            // for (unsigned int i = 0; i < mesh.NV(); i++)
            // {
            //     vertices.push_back(mesh.V(i)); 
            //     normals.push_back(mesh.VN(i)); 
            // }

  



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

        cy::Matrix4f createView() {
            float Tr = pos.Dot(right);
            float Tu = pos.Dot(up);
            float Tf = pos.Dot(normal);

            cy::Matrix4f view(
                right.x, right.y, right.z, -Tr, 
                up.x, up.y, up.z, -Tu, 
                -normal.x, -normal.y, -normal.z, Tf, 
                0.0f, 0.0f, 0.0f, 1.0f
            );

            return view;
        }

    };

    class TransformationMatrices {
    public:
        cy::Matrix4f mvp, mv, m, v, p;
        cy::Matrix3f nrm_m, nrm_mv; 
        // cy::Matrix4f o;
        // char projectionType;

        TransformationMatrices(cy::Matrix4f model, cy::Matrix4f view, cy::Matrix4f projection) {
            setMVP(model, view, projection); 
        }
        TransformationMatrices() = default;

        void setModel(const cy::Matrix4f& model) { m = model; }
        void setProjection(const cy::Matrix4f& projection) { p = projection; }
        void setMVP(cy::Matrix4f model, cy::Matrix4f view, cy::Matrix4f projection) {
            m = model; 
            v = view; 
            p = projection;
            mv = v * m; 
            mvp = p * v * m; 
            nrm_m = m.GetSubMatrix3().GetInverse().GetTranspose(); 
            nrm_mv = mv.GetSubMatrix3().GetInverse().GetTranspose(); 
        }
        void setMV(cy::Matrix4f model, cy::Matrix4f view)  {
            m = model; 
            v = view; 
            mv = v * m; 
            mvp = p * v * m; 
            nrm_m = m.GetSubMatrix3().GetInverse().GetTranspose(); 
            nrm_mv = mv.GetSubMatrix3().GetInverse().GetTranspose(); 
        }
        void setView(const cy::Matrix4f view) {
            v = view; 
            mv = v * m; 
            mvp = p * v * m; 
            nrm_m = m.GetSubMatrix3().GetInverse().GetTranspose(); 
            nrm_mv = mv.GetSubMatrix3().GetInverse().GetTranspose(); 
        }


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