#pragma once

// OpenGL extensions

#include "ogl.h"
#include "public.h"

OPEN_O2_NAMESPACE

class LogStream;

/** Getting openGL extensions. */
void getGLExtensions(LogStream* log = NULL);

extern PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT;
extern PFNGLFRAMEBUFFERTEXTUREPROC        glFramebufferTexture;
extern PFNGLDRAWBUFFERSPROC               glDrawBuffers;
extern PFNGLDELETEBUFFERSPROC             glDeleteBuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;

CLOSE_O2_NAMESPACE