#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../src/glad.h"
#include <iostream>
#include <vector>

class FBO
{
    public:
        GLuint ID;
        std::vector<GLuint> textureIDs;
        GLuint textureID;
        GLuint renderbufferID;

        FBO();
        void AttatchTexture(int width, int height, GLenum internalFormat);
        void AttatchTextures(int width, int height, int amount, GLenum internalFormat);
        void AttatchRenderBuffer(GLenum internalFormat, GLenum attatchmentType, int width, int height);
        void AttatchTextureMSAA(int samples, int width, int height);
        void AttatchRenderBufferMSAA(int samples, GLenum internalFormat, GLenum attatchmentType, int width, int height);
        void Blit(int width, int height);
        void CheckStatus();
        void BindTexture(int index = 0);
        void BindTextureMSAA();
        void Bind();
        void Unbind();
        void Delete();

};

#endif
