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

#ifndef SENSE_PIPELINE_INTERFACE_HPP
#define SENSE_PIPELINE_INTERFACE_HPP

#include "3rdparty/glm/glm.hpp"
#include "DefinitionTypes.hpp"

#include <boost/filesystem/path.hpp>

struct DrawBuffer;
struct Material;
struct DrawableMesh;
struct ShaderProgram;
struct Texture;
struct RenderTarget;
struct Image;
struct Lamp;

struct LoaderImpl;
struct PipelineImpl;

class DataManager;

class Loader
{
public:
  Loader();
  ~Loader();

  // Load the mesh of the given name into a GPU buffer object
  void loadMesh(DrawableMesh*);
  void mainThreadLoadMesh(DrawableMesh*);
  void releaseMesh(DrawableMesh*);

  // Load the given shader strings into a GPU program.
  boost::any queryUniform(ShaderProgram* prog, std::string uni);
  ShaderProgram* loadProgram(std::string vert, std::string frag, std::string geom="");
  void releaseProgram(ShaderProgram*);

  // Load the given image object into a GPU texture
  void loadTexture(Image*);
  // perform any needed updates on the Texture object
  void updateTexture(Image*);
  void releaseTexture(Image*);

  static bool isThreaded();

private:
  LoaderImpl* self;
};


// Interface for the rendering pipeline.
// This abstraction allows most of the engine to just not care about
// the actual platform implementation of the pipeline.
class Pipeline
{
public:
  enum RenderPass {
    PassStandard,
    PassPostLighting,
    PassPostEffect,

    // Start of system passes. You'll throw exceptions if you try to use these outside the pipeline. I'm not joking.
    PassLighting,
    PassPreToneEffect,
    PassToneMap,
    PassPostToneEffect,
    PassCount
  };
  Pipeline();
  ~Pipeline();

  // Add a drawable to be rendered this frame
  // If the platform implementation supports instancing, use_instancing can be set to false to disable
  // that feature. If instancing is not supported, use_instancing has no effect. Instancing is enabled
  // by default for performance reasons.
  void addDrawTask(DrawableMesh* data, Material* mat, glm::mat4 transform, RenderPass pass=PassStandard);
  void addSkinnedDrawTask(DrawableMesh* data, Material* mat, std::vector<glm::mat4>& bones, RenderPass pass=PassStandard);

  // Add a lamp to be used for rendering this frame
  void addLamp(Lamp* lamp);

  // Render the current set of objects to the active RenderTarget
  void render();

  // Do any final processing to ensure that the frame is visible on the screen
  void endFrame();

  // Create a RenderTarget of the specified dimensions
  RenderTarget* createRenderTarget(uint32_t width, uint32_t height, bool mipmap=true); // TODO: enable user selection of render target bit depth

  // Clean up a RenderTarget
  void destroyRenderTarget(RenderTarget*);

  // Set the active RenderTarget
  void setRenderTarget(RenderTarget* target);

  // Set the region of the window to render to
  void setViewport(uint32_t width, uint32_t height);

  // Create a Loader instance to go with this Pipeline
  Loader* createLoader();

  // Load any resources the pipeline needs to have direct references to
  void loadPipelineData(DataManager*);

private:
  PipelineImpl* self;
};

#endif // SENSE_PIPELINE_INTERFACE_HPP
