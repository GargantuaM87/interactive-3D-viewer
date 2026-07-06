#include <iostream>
#include "../headers/imgui/imgui.h"
#include "../headers/imgui/imgui_impl_glfw.h"
#include "../headers/imgui/imgui_impl_opengl3.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <filesystem>
#include <string>
#include "../headers/shaderClass.h"
#include "../headers/EBO.h"
#include "../headers/VBO.h"
#include "../headers/VAO.h"
#include "../headers/texture.h"
#include "../headers/camera.h"
#include "../headers/model.h"
#include "../headers/FBO.h"
#include "../headers/guiVariables.h"
#include "../include/stb/stb_image.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include "imgui.h"


const unsigned int width = 800;
const unsigned int height = 800;

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
     glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

unsigned int LoadCubeMap(std::vector<std::string> faces)
{
     unsigned int textureID;
     glGenTextures(1, &textureID);
     glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

     int width, height, nrChannels;
     for(unsigned int i = 0; i < faces.size(); i++)
     {
          unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
          if(data)
          {
               glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
               stbi_image_free(data);
          }
          else
          {
               std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
               stbi_image_free(data);
          }
     }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

int main(int, char **)
{
     float quadVertices[] = {
          // positions     // texCoords
          -1.0f,  1.0f,  0.0f, 1.0f,
          -1.0f, -1.0f,  0.0f, 0.0f,
           1.0f, -1.0f,  1.0f, 0.0f,

          -1.0f,  1.0f,  0.0f, 1.0f,
          1.0f, -1.0f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f
     };

     float skyboxVertices[] = {
          // positions
              -1.0f,  1.0f, -1.0f,
              -1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,

              -1.0f, -1.0f,  1.0f,
              -1.0f, -1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f, -1.0f,  1.0f,

               1.0f, -1.0f, -1.0f,
               1.0f, -1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,

              -1.0f, -1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f, -1.0f,  1.0f,
              -1.0f, -1.0f,  1.0f,

              -1.0f,  1.0f, -1.0f,
               1.0f,  1.0f, -1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f, -1.0f,

             -1.0f, -1.0f, -1.0f,
             -1.0f, -1.0f,  1.0f,
              1.0f, -1.0f, -1.0f,
              1.0f, -1.0f, -1.0f,
             -1.0f, -1.0f,  1.0f,
              1.0f, -1.0f,  1.0f
     };
     GLFWwindow *window;

     if (!glfwInit())
     {
         throw 1;
     }
     // Create the window that appears on the screen
     glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
     window = glfwCreateWindow(width, height, "Window", NULL, NULL);
     // Tells GLFW to add the window to the current context
     // A context being an object that encapsulates the OpenGL state machine
     glfwMakeContextCurrent(window);

     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
     {
          std::cout << "Couldn't load OpenGL" << std::endl;
          glfwTerminate();
          return -1;
     }

     int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
     if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
     {
         glEnable(GL_DEBUG_OUTPUT);
         glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
         glDebugMessageCallback(glDebugOutput, nullptr);
         glDebugMessageControl(GL_DEBUG_SOURCE_API,
                               GL_DEBUG_TYPE_ERROR,
                               GL_DEBUG_SEVERITY_HIGH,
                               0, nullptr, GL_TRUE);
     }

     // Shaders
     Shader toTextureShader("../assets/shaders/framebuffer.vert", "../assets/shaders/framebuffer.frag");
     Shader defaultShader("../assets/shaders/default.vert", "../assets/shaders/default.frag");
     Shader depthShader("../assets/shaders/shadows/depth.vert", "../assets/shaders/shadows/depth.frag");
     Shader depthCubeShader("../assets/shaders/shadows/pointShadow.vert", "../assets/shaders/shadows/pointShadow.frag");
     depthCubeShader.LinkGeometry("../assets/shaders/shadows/pointShadow.geom");
     // Models
     Model cafe("../assets/ModularModel/modular.obj");
     Model cube("../assets/Models/cube.obj");
     Model sphere("../assets/Models/subsphere.obj");
     Model plane("../assets/Models/plane.obj");
     // Textures
     TextureUnit marbleText("../assets/textures/textures/marble_cliff_06_diff_2k.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

     // quad geometry
     VAO quadVAO;
     VBO quadVBO(quadVertices, sizeof(quadVertices));
     quadVAO.Bind();
     quadVAO.LinkAttrib(quadVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
     quadVAO.LinkAttrib(quadVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
     quadVAO.Unbind();
     // sky box geometry
     VAO skyboxVAO;
     VBO skyboxVBO(skyboxVertices, sizeof(skyboxVertices));
     skyboxVAO.Bind();
     skyboxVAO.LinkAttrib(skyboxVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
     skyboxVAO.Unbind();

     //Render to Texture
     FBO fbo;
     fbo.AttatchTexture(width, height, GL_RGBA);
     fbo.AttatchRenderBuffer(GL_DEPTH24_STENCIL8, GL_DEPTH_ATTACHMENT, width, height);
     fbo.CheckStatus();
     // Depth fbo and texture for directional shadow map
     const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
     unsigned int depthMapFBO;
     glGenFramebuffers(1, &depthMapFBO);
     // for directional shadows
     unsigned int depthMap;
     glGenTextures(1, &depthMap);
     glBindTexture(GL_TEXTURE_2D, depthMap);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
     float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
     glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
     // attatch texture
     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
     glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
     glDrawBuffer(GL_NONE);
     glReadBuffer(GL_NONE);
     glCheckError();
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
     // for omni-directional shadows
     unsigned int depthCubemap;
     glGenTextures(1, &depthCubemap);
     glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
     for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);




     glEnable(GL_DEPTH_TEST);
     // -----------RENDER LOOP VARIABLES-----------
     Camera camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));
     glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
     GUIVariables GUI;

     float deltaTime = 0.0f;
     float lastFrame = 0.0f;

     //-----------END OF RENDER LOOP VARIABLES-----------
     IMGUI_CHECKVERSION();
     ImGui::CreateContext();
     ImGuiIO &io = ImGui::GetIO();
     (void)io;
     ImGui::StyleColorsClassic();
     ImGui_ImplGlfw_InitForOpenGL(window, true);
     ImGui_ImplOpenGL3_Init("#version 330");

     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


     // Presetting Uniforms here
     defaultShader.Activate();
     defaultShader.SetToInt("depthMap", 0);
     defaultShader.SetToInt("depthCubeMap", 1);

     toTextureShader.Activate();
     toTextureShader.SetToInt("screenTexture", 0);
     // Main Render Loop
     while (!glfwWindowShouldClose(window))
     {
          glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glBindFramebuffer(GL_FRAMEBUFFER, 0);

          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          float crntFrame = glfwGetTime();
          deltaTime = crntFrame - lastFrame;
          lastFrame = crntFrame;

           if (!io.WantCaptureMouse)
               camera.Inputs(window);

          camera.Matrix(45, 0.1, 100);
          glm::mat4 model = glm::mat4(1.0);
          glm::mat4 view = camera.GetViewMatrix();
          glm::mat4 proj = camera.GetProjMatrix();

          //1.  render depth of scene to texture from light's perspective
          glm::mat4 lightProj, lightView;
          glm::mat4 lightSpaceMatrix;
          float nearPlane = 1.0f, farPlane = 25.0f;
          lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
          lightView = glm::lookAt(GUI.lightDir, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
          lightSpaceMatrix = lightProj * lightView;
          depthShader.Activate();
          depthShader.SetToMat4("lightSpaceMatrix", lightSpaceMatrix);

          glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
          glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
          glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
          glDrawBuffer(GL_NONE);
          glReadBuffer(GL_NONE);
          glClear(GL_DEPTH_BUFFER_BIT);

          model = glm::scale(model, glm::vec3(0.5));
          depthShader.SetToMat4("model", model);
          sphere.Draw(depthShader);
          model = glm::mat4(1.0);
          model = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
          depthShader.SetToMat4("model", model);
          plane.Draw(depthShader);
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          // ----END of depth map rendering----
          // reset viewport
          glViewport(0, 0, width, height);

          // ---create depth map cubemap transformation matrices---
          glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
          std::vector<glm::mat4> shadowTransforms;
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
          shadowTransforms.push_back(shadowProj * glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
          //2. render scene into depth cube map
          glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
          glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
          glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
          glDrawBuffer(GL_NONE);
          glReadBuffer(GL_NONE);
          glClear(GL_DEPTH_BUFFER_BIT);

          depthCubeShader.Activate();
          for(unsigned int i = 0; i < 6; ++i)
              depthCubeShader.SetToMat4(&("shadowMatrices[" + std::to_string(i) + "]")[i], shadowTransforms[i]);
          depthCubeShader.SetToFloat("far_plane", farPlane);
          depthCubeShader.SetToVec3("lightPos", &GUI.lightPos[0]);
          model = glm::mat4(1.0);
          model = glm::scale(model, glm::vec3(0.5));
          depthCubeShader.SetToMat4("model", model);
          sphere.Draw(depthCubeShader);
          model = glm::mat4(1.0);
          model = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
          depthCubeShader.SetToMat4("model", model);
          plane.Draw(depthCubeShader);
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glViewport(0, 0, width, height);

          //3. render scene as normal
          fbo.Bind();
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          defaultShader.Activate();
          defaultShader.SetToMat4("view", view);
          defaultShader.SetToMat4("proj", proj);
          defaultShader.SetToMat4("lightSpaceMatrix", lightSpaceMatrix);
          // directional light
          defaultShader.SetToVec3("dLight.direction", &GUI.lightDir[0]);
          defaultShader.SetToVec3("dLight.ambient", &glm::vec3(0.4f)[0]);
          defaultShader.SetToVec3("dLight.diffuse", &glm::vec3(0.8f)[0]);
          defaultShader.SetToVec3("dLight.specular", &glm::vec3(0.2)[0]);
          //point light
          defaultShader.SetToVec3("pLight.position", &GUI.lightPos[0]);
          defaultShader.SetToVec3("pLight.color", &GUI.lightColor[0]);
          defaultShader.SetToFloat("pLight.constant", 1.0f);
          defaultShader.SetToFloat("pLight.linear", 0.09f);
          defaultShader.SetToFloat("pLight.quadratic", 0.032f);
          defaultShader.SetToVec3("u_viewPos", &camera.Position[0]);
          defaultShader.SetToFloat("far_plane", farPlane);

          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, depthMap);
          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

          model = glm::mat4(1.0);
          model = glm::scale(model, glm::vec3(0.5));
          defaultShader.SetToMat4("model", model);
          sphere.Draw(defaultShader);
          model = glm::mat4(1.0);
          model = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
          defaultShader.SetToMat4("model", model);
          plane.Draw(defaultShader);
          model = glm::mat4(1.0);
          model = glm::translate(model, GUI.lightPos);
          model = glm::scale(model, glm::vec3(0.2));
          defaultShader.SetToMat4("model", model);
          cube.Draw(defaultShader);

          fbo.Unbind();

          toTextureShader.Activate();
          glActiveTexture(GL_TEXTURE0);
          fbo.BindTexture(0);
          quadVAO.Bind();
          glDrawArrays(GL_TRIANGLES, 0, 6);
          quadVAO.Unbind();

          //--------------END OF SHADERS & MODEL DRAWING--------------
          // ---------IMGUI---------
          ImGui::Begin("OpenGL Settings Panel");
          ImGui::Text("Tweaks");

          ImGui::Separator();
          // directional light pos var
          ImGui::SliderFloat3("Directional Light Dir", &GUI.lightDir[0], -10.0, 10.0f);
          ImGui::SliderFloat3("Point Light Pos", &GUI.lightPos[0], -10.0, 10.0);

          ImGui::End();

          ImGui::Render();
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
          // ---------END OF IMGUI---------

          // Swap back buffer with front buffer
          glfwSwapBuffers(window);
          // Makes sure our window is responsive (such as resizing it and moving it)
          glfwPollEvents();
     }
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplGlfw_Shutdown();
     ImGui::DestroyContext();

     glfwTerminate();
     return 0;
}
