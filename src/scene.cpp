#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include "objloader.h"

Scene::ObjModel::ObjModel(std::string path)
{
    if(!loadObjModel(path, verts, normals, true))
        return;

    std::cout<<"objLoader load done..."<<std::endl;

    //arrange vao
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &nbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), &verts[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
}

void Scene::ObjModel::render(Shader & shader)
{
    glBindVertexArray(vao);
    shader.use();
    glDrawArrays(GL_TRIANGLES, 0, verts.size() / 3);
    shader.release();
    glBindVertexArray(0);
}

Scene::Scene()
{
    width = 800;
    height = 600;
    init();
}

void Scene::init()
{
    //global settings
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //set up framebuffer and texture
    setupShadowTexture();
    setupFramebuffer();

    //cube VAO
    glGenVertexArrays(1, &cubeVao);
    glGenBuffers(1, &cubeVbo);
    glBindVertexArray(cubeVao);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVertices.size(), &cubeVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)3);

    /*initialize MVP matrix*/
    planeModel = glm::scale(planeModel, glm::vec3(10, 0.1, 10));
    objectModel = glm::scale(objectModel, glm::vec3(1.2, 1.2, 1.2));

    // view = cam.GetViewMatrix();
    view = glm::lookAt(cam.Position, glm::vec3(0), glm::vec3(0, 1, 0));
    projection = glm::perspective(cam.Zoom, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    renderShadow.use();
    renderShadow.setMat4("view", view);
    renderShadow.setMat4("projection", projection);
    renderShadow.release();

    float near = 1.0f;
    float far = 20.0f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
}

void Scene::setupShadowTexture()
{
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Scene::setupFramebuffer()
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMap, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SHADOW_WIDTH, SHADOW_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::renderLightDepth()
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    shadowDepthShader.use();
    shadowDepthShader.setMat4("lightSpaceMatrix", lightProjection * lightView);
    glBindVertexArray(cubeVao);

    //cube
    shadowDepthShader.setMat4("model", planeModel);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    //obj model
    shadowDepthShader.setMat4("model", objectModel);
    model.render(shadowDepthShader);

    shadowDepthShader.release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::render()
{
    //first render a depth map from light position
    renderLightDepth();

    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //draw obj model
    renderShadow.use();
    renderShadow.setVec3("lightDir", lightPos);
    renderShadow.setVec3("viewPos", cam.Position);
    renderShadow.setIvec2("LIGHTSIZE", lightSize);
    renderShadow.setMat4("model", objectModel);
    renderShadow.setVec3("baseColor", {0.5, 0.3, 0.1});
    renderShadow.setInt("shadowMap", 0);
    renderShadow.setMat4("lightSpaceMatrix", lightProjection * lightView);
    model.render(renderShadow);

    //draw cube
    glBindVertexArray(cubeVao);
    renderShadow.use();
    renderShadow.setMat4("model", planeModel);
    renderShadow.setVec3("baseColor", {0.3, 0.7, 0.3});
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    renderShadow.release();
    glBindVertexArray(0);
}

void Scene::updateLightSize(glm::ivec2 size)
{
    lightSize = size;
}

void Scene::updateWidthAndHeight(float width, float height)
{
    this->width = width;
    this->height = height;
    projection = glm::perspective(cam.Zoom, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    shadowDepthShader.use();
    shadowDepthShader.setMat4("projection", projection);
    shadowDepthShader.release();

    renderShadow.use();
    renderShadow.setMat4("projection", projection);
    renderShadow.release();
}
