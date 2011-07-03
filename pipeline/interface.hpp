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
struct RenderTarget;
struct Lamp;

struct LoaderImpl;
struct PipelineImpl;

class SenseClient;

class Loader
{
public:
  Loader(SenseClient*);
  ~Loader();

  // Load the mesh of the given name into a GPU buffer object
  DrawBuffer* loadMesh(std::string name);
  void releaseMesh(DrawBuffer*);

  // Load the textures and shaders associated with the given material
  Material* loadMaterial(std::string name);
  void releaseMaterial(Material*);

  // Add a material definition with the given name
  void addMaterial(MaterialDef def, std::string name);

  // Load material files from the given directory
  void loadMaterialFiles(boost::filesystem::path);

  // If the Loader implementation is threaded, this will run the threaded proc
  // If not, this must be implemented as an empty function.
  void exec();

  // If the Loader implementation is threaded, this signals the loader to terminate
  // If not, this must be implemented as a no-op
  void finish();

private:
  LoaderImpl* self;
};


// Interface for the rendering pipeline.
// This abstraction allows most of the engine to just not care about
// the actual platform implementation of the pipeline.
class Pipeline
{
public:
  Pipeline(SenseClient*);
  ~Pipeline();

  // Add a drawable to be rendered this frame
  // If the platform implementation supports instancing, use_instancing can be set to false to disable
  // that feature. If instancing is not supported, use_instancing has no effect. Instancing is enabled
  // by default for performance reasons.
  void addDrawTask(DrawBuffer* data, Material* mat, glm::mat4 transform, bool use_instancing=true);

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

  // Check if the loader is threaded.
  // This information is in the Pipeline, rather than the Loader, because the info may be needed for platform init before the Loader is created
  bool isLoaderThreaded();

  // Create a Loader instance to go with this Pipeline
  Loader* createLoader();

private:
  PipelineImpl* self;
};

#endif // SENSE_PIPELINE_INTERFACE_HPP
