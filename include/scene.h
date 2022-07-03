#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include "shader.h"
#include "camera.h"

class Scene
{
public:
    Scene();
    void render();

private:
    void init();
    void setupShadowTexture();
    void setupFramebuffer();
    void renderLightDepth();

    void initMatrix();
    void updateMatrix(Camera & cam, glm::mat4 rotate);

    Shader renderShadow{"../shaders/renderShadow.vert", "../shaders/renderShadow.frag"};
    Shader shadowDepthShader{"../shaders/genShadow.vert", "../shaders/genShadow.frag"};

    glm::mat4 lightProjection;
    glm::mat4 lightView;

    Camera cam{glm::vec3(0.0f, 3.0f, 5.0f)};
    glm::vec3 lightPos{3, 10, 3};
    glm::mat4 modelA{1.0f};
    glm::mat4 modelB{1.0f};
    glm::mat4 modelC{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};

    unsigned int framebuffer;

    unsigned int shadowMap;
    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    unsigned int cubeVao;
    unsigned int cubeVbo;

    unsigned int width;
    unsigned int height;

    unsigned int tex3D;

    unsigned int texWidth = 64;
    unsigned int texHeight = 64;
    unsigned int texDepth = 64;

    std::vector<float> sliceDatas;

    std::vector<float> cubeVertices = {
    // positions          // normals           // texture coords
    // Back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // Front face
    -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
     0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, // top-left
    -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
    // Right face
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
    // Top face
    -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
     0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
     0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
    -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left 
    };
};

#endif