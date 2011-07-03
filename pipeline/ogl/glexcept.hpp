// Copyright 2011 Branan Purvine-Riley and Adam Johnson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SENSE_PIPELINE_GLEXCEPT_HPP
#define SENSE_PIPELINE_GLEXCEPT_HPP

#include "util/util.hpp"
#include <string>
#include <stdexcept>

static const char* glErrorString(GLenum err) {
  switch(err) {
    case GL_INVALID_ENUM: return "Invalid Enum";
    case GL_INVALID_VALUE: return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
    case GL_STACK_OVERFLOW: return "Stack Overflow";
    case GL_STACK_UNDERFLOW: return "Stack Underflow";
    case GL_OUT_OF_MEMORY: return "Out of Memory";
    case GL_TABLE_TOO_LARGE: return "Table too Large";
    default: return "Unknown Error";
  }
}

static const char* fboErrorString(GLenum status) {
  switch(status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "Incomplete Attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "Missing Attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "Incomplete Draw Buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "Incomplete Read Buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "Unsupposed Configuration";
    default: return "Unknown Error";
  }
}

class gl_error : public std::runtime_error {
public:
  gl_error(std::string location, GLenum err) : std::runtime_error(glErrorString(err)+location) {}
};

class fbo_error : public std::runtime_error {
public:
  fbo_error(std::string location, GLenum status) : std::runtime_error(fboErrorString(status)+location) {}
};

// Throw exceptions when OpenGL barfs
#ifndef NDEBUG
#define GL_CHECK(func) func; { GLenum glerr = glGetError(); if(GL_NO_ERROR != glerr) throw gl_error(FILE_LINE, glerr); }
#define FBO_CHECK { GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if(GL_FRAMEBUFFER_COMPLETE != status) throw fbo_error(FILE_LINE, status); }
#else
#define GL_CHECK(func) func;
#define FBO_CHECK
#endif

#endif // SENSE_PIPELINE_GLEXCEPT_HPP
