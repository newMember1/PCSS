#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include "objloader.h"

Scene::ObjModel::ObjModel(std::string path)
{
    if(!objLoader::load(path, verts, vIndexes, normals, nIndexes))
        return;

    // for(int i = 0; i < verts.size(); ++i)
    //     std::cout<<"v " <<verts[i].x<<" "<<verts[i].y << " "<<verts[i].z<<std::endl;
    // for(int j = 0; j < vIndexes.size(); ++j)
    //     std::cout<<"f "<<vIndexes[j][0]<<" "<<vIndexes[j][1]<<" "<<vIndexes[j][2]<<std::endl;
    
    // getchar();

    std::cout<<"objLoader load done..."<<std::endl;

    //verts normalize
    float maxV = 1e-6;
    for(auto v : verts)
        maxV = glm::max(glm::max(v.x, v.y), glm::max(v.z, maxV));
    for(auto & v: verts)
        v = v / maxV;

    std::vector<float> tVerts;
    std::vector<unsigned int> tIndexes;

    for(auto v : verts)
    {
        tVerts.push_back(v.x);
        tVerts.push_back(v.y);
        tVerts.push_back(v.z);
    }
    for(auto i : vIndexes)
    {
        tIndexes.push_back(i[0]);
        tIndexes.push_back(i[1]);
        tIndexes.push_back(i[2]);
    }

    //vao
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tVerts.size(), &tVerts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * tIndexes.size(), &tIndexes[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
}

void Scene::ObjModel::render()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vIndexes.size() * 3, GL_UNSIGNED_INT, 0);
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

    modelA = glm::translate(modelA, glm::vec3(-0.8, 1.7, 0));
    modelB = glm::translate(modelB, glm::vec3(0.8, 1.9, 0));
    modelC = glm::scale(modelC, glm::vec3(10, 0.1, 10));
    bunnyModel = glm::translate(bunnyModel, glm::vec3(0, 0.5, 0));

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

    // //cube A
    // shadowDepthShader.setMat4("model", modelA);
    // glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    // //cube B
    // shadowDepthShader.setMat4("model", modelB);
    // glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    //cube C
    shadowDepthShader.setMat4("model", modelC);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    //bunny
    shadowDepthShader.setMat4("model", bunnyModel);
    model.render();

    shadowDepthShader.release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::render()
{
    renderLightDepth();

    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //draw bunny
    renderShadow.use();
    renderShadow.setVec3("baseColor", {0.5, 0.3, 0.1});
    renderShadow.setMat4("model", bunnyModel);
    model.render();

    glBindVertexArray(cubeVao);
    renderShadow.setInt("shadowMap", 0);
    renderShadow.setMat4("lightSpaceMatrix", lightProjection * lightView);

    // //draw cube A
    // renderShadow.setVec3("baseColor", {0.1, 0.3, 0.5});
    // renderShadow.setMat4("model", modelA);
    // glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);
    
    // //draw cube B
    // renderShadow.setVec3("baseColor", {0.5, 0.3, 0.1});
    // renderShadow.setMat4("model", modelB);
    // glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    //draw cube C
    renderShadow.setVec3("baseColor", {0.3, 0.7, 0.3});
    renderShadow.setMat4("model", modelC);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

    renderShadow.release();
    glBindVertexArray(0);
}
