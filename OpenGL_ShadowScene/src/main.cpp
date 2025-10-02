#define GLM_ENABLE_EXPERIMENTAL
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GLEW_STATIC

#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "shader.h"
#include "j3a.hpp"
#include "triMesh.hpp"


GLuint program = 0;
GLuint shadowProgram;
GLuint diffuseTexID = 0;
GLuint bumpTexID = 0;
GLuint floorTexID = 0;

GLuint shadowTex, shadowDepth, shadowFBO;

TriMesh model;
TriMesh floorModel;

float camDist = 10.0f;
float phi = 0.0f;
float theta = 0.0f;
float fov = 1.0f;
double lastX, lastY;

glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

glm::mat4 shadowProjMat;
glm::mat4 shadowViewMat;

void render(GLFWwindow* window);
void setupShadowMatrices();
void cursorPosCB(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
void scrollCB(GLFWwindow* window, double xoffset, double yoffset);
void initShadowTexture();
void initDepthBuffer();
void initShadowFramebuffer();

GLuint loadTexture(const std::string& filename) {
    GLuint textureID;
    int w, h, n;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &w, &h, &n, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

int main() {
    glfwInit();

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "202322215", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, cursorPosCB);
    glfwSetMouseButtonCallback(window, mouseButtonCB);
    glfwSetScrollCallback(window, scrollCB);

    program = loadShaders("/Users/minchae/Xcode/202322215/202322215/shader.vert", "/Users/minchae/Xcode/202322215/202322215/shader.frag");


    shadowProgram = loadShaders("/Users/minchae/Xcode/Project/Project/shadow.vs", "/Users/minchae/Xcode/Project/Project/shadow.fs");
    if (!shadowProgram) {
        std::cerr << "Failed to load shadow shaders!" << std::endl;
        return -1;
    }

    loadJ3A("/Users/minchae/Xcode/202322215/202322215/dwarf.j3a");
    model.create(nVertices[0], vertices[0], normals[0], texCoords[0], nTriangles[0], triangles[0]);


    floorModel.create(
        {{-100, -1, -100}, {100, -1, -100}, {-100, -1, 100}, {100, -1, 100}},
        {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{0, 1, 2}, {1, 2, 3}}
    );

    diffuseTexID = loadTexture("/Users/minchae/Xcode/202322215/202322215/"+diffuseMap[0]);
    bumpTexID = loadTexture("/Users/minchae/Xcode/202322215/202322215/"+bumpMap[0]);

    setupShadowMatrices();
    initShadowTexture();
    initDepthBuffer();
    initShadowFramebuffer();

    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
void render(GLFWwindow* window) {
    int w, h;
    GLuint loc;
    glfwGetFramebufferSize(window, &w, &h);


    glm::mat4 shadowViewMat = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 shadowProjMat = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 10.f, 50.0f);


    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowProgram);


    glm::mat4 biasMatrix = glm::translate(glm::vec3(0.5)) * glm::scale(glm::vec3(0.5));


    glm::mat4 modelMat = glm::mat4(1.0f);
    glm::mat4 shadowMVP = shadowProjMat * shadowViewMat * modelMat;
    glm::mat4 shadowBiasMVP = biasMatrix * shadowMVP;

    loc = glGetUniformLocation(shadowProgram, "shadowMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(shadowMVP));


    model.draw();
    floorModel.draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glViewport(0, 0, w, h);
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);


    glm::vec3 cameraPosition = glm::vec3(glm::rotate(glm::mat4(1), theta, glm::vec3(0, 1, 0)) *
                                         glm::rotate(glm::mat4(1), phi, glm::vec3(1, 0, 0)) *
                                         glm::vec4(0, 0, camDist, 1));
    glm::mat4 viewMat = glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 projMat = glm::perspective(fov, w / float(h), 0.01f, 100.0f);


    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    loc = glGetUniformLocation(program, "shadowTex");
    glUniform1i(loc, 2);


    loc = glGetUniformLocation(program, "viewMat");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewMat));

    loc = glGetUniformLocation(program, "projMat");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projMat));

    loc = glGetUniformLocation(program, "lightPos");
    glUniform3fv(loc, 1, glm::value_ptr(lightPos));

    loc = glGetUniformLocation(program, "lightColor");
    glUniform3fv(loc, 1, glm::value_ptr(lightColor));

    loc = glGetUniformLocation(program, "bumpTexEnabled");
    glUniform1i(loc, 1);
    loc = glGetUniformLocation(program, "diffTexEnabled");
    glUniform1i(loc, 1);
    loc = glGetUniformLocation(program, "shadowBiasMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(shadowBiasMVP));

    loc = glGetUniformLocation(program, "diffTex");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTexID);
    glUniform1i(loc, 0);

    loc = glGetUniformLocation(program, "bumpTex");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bumpTexID);
    glUniform1i(loc, 1);

    model.draw();

    loc = glGetUniformLocation(program, "bumpTexEnabled");
    glUniform1i(loc, 0);
    loc = glGetUniformLocation(program, "diffTexEnabled");
    glUniform1i(loc, 0);

    loc = glGetUniformLocation(program, "color");
    glUniform3f(loc, 0.5f, 0.5f, 0.5f);

    floorModel.draw();

    glfwSwapBuffers(window);
}


void setupShadowMatrices() {

    shadowProjMat = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 10.0f, 50.0f);

    shadowViewMat = glm::lookAt(
        lightPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}


void cursorPosCB(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        theta -= (xpos - lastX) / 300;
        phi -= (ypos - lastY) / 300;
        lastX = xpos;
        lastY = ypos;
    }
}

void mouseButtonCB(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &lastX, &lastY);
    }
}

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
    fov = fov * powf(1.01, yoffset);
}

void initShadowTexture() {
    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void initDepthBuffer() {
    glGenTextures(1, &shadowDepth);
    glBindTexture(GL_TEXTURE_2D, shadowDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void initShadowFramebuffer() {
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth, 0);


    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("FBO Error\n");

    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}
