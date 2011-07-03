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

#include "python/module.hpp"

#include "pipeline/interface.hpp"
#include "Client.hpp"

#include "python/pipeline/PyLoader.hpp"

SenseClient::SenseClient()
  : m_loader_init_complete(false), m_width(800), m_height(600)
{
  platformInit();
  m_pipeline = new Pipeline(this);
  if(m_pipeline->isLoaderThreaded()) {
    platformDetachContext();
    m_loader_thread = boost::thread(std::mem_fun(&SenseClient::runLoaderThread), this);
    while(!m_loader_init_complete) {}
    platformAttachContext();
    if(!loader_error_string.empty()) {
      m_loader_thread.join();
      delete m_pipeline;
      platformFinish();
      throw std::runtime_error(loader_error_string.c_str());
    }    
  } else {
    platformInitLoader();
    m_loader = m_pipeline->createLoader();
  }

  setupPythonModule();
  m_loader->loadMaterialFiles("../data/materials/");
  framebuffer = m_pipeline->createRenderTarget(width(), height(), false);
};

SenseClient::~SenseClient()
{
  m_pipeline->destroyRenderTarget(framebuffer);
  m_loader->finish();
  m_loader_thread.join();
  delete m_pipeline;
  platformFinish();
}

bool SenseClient::tick()
{
  m_pipeline->setRenderTarget(framebuffer);
  m_pipeline->render();
  m_pipeline->endFrame();
  platformSwapBuffers();
  return platformEventLoop();
}

void SenseClient::runLoaderThread()
{
  try {
    platformInitLoader();
    m_loader = m_pipeline->createLoader();
  } catch (std::exception& e) {
    loader_error_string = e.what();
  }
  m_loader_init_complete = true;

  m_loader->exec();

  delete m_loader;
  platformFinishLoader();
}

void SenseClient::setupPythonModule()
{
  PyObject* SensModule = PyImport_ImportModule("SensEngine");
  PyObject* m = PyImport_AddModule("SensEngine.client");
  PyModule_AddObject(m, "__builtins__", PyEval_GetBuiltins());
  PyModule_AddObject(m, "loader", PyLoader_create(m_loader));
  PyModule_AddObject(SensModule, "client", m);
  PyModule_AddStringConstant(SensModule, "__path__", "SensEngine");
}
