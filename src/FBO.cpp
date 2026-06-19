#include "../headers/FBO.h"
#include "camera.h"
#include <cstddef>

FBO::FBO()
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void FBO::AttatchTexture(int width, int height, GLenum internalFormat)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FBO::AttatchTextures(int width, int height, int amount, GLenum internalFormat)
{
    for(int i = 0; i < amount; i++) {
        unsigned int colorBuffer;
        glGenTextures(1, &colorBuffer);

        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer, 0);
        textureIDs.push_back(colorBuffer);
    }
}

void FBO::AttatchRenderBuffer(GLenum internalFormat, GLenum attatchmentType, int width, int height)
{
    glGenRenderbuffers(1, &renderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attatchmentType, GL_RENDERBUFFER, renderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBO::AttatchTextureMSAA(int samples, int width, int height)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void FBO::AttatchRenderBufferMSAA(int samples, GLenum internalFormat, GLenum attatchmentType, int width, int height)
{
    glGenRenderbuffers(1, &renderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attatchmentType, GL_RENDERBUFFER, renderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBO::Blit(int width, int height)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FBO::CheckStatus()
{
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

void FBO::BindTexture(int index)
{
    if(index == 0)
        glBindTexture(GL_TEXTURE_2D, textureID);
    else
        glBindTexture(GL_TEXTURE_2D, textureIDs[index - 1]);
}

void FBO::BindTextureMSAA()
{
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
}

void FBO::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void FBO::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::Delete()
{
    glDeleteFramebuffers(1, &ID);
    glDeleteTextures(1, &textureID);
    glDeleteRenderbuffers(1, &renderbufferID);
}
