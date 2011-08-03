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
#include "DataManager.hpp"

#include "python/PyDataManager.hpp"
#include "python/entity/api.hpp"

#include "entity/Entity.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

SenseClient::SenseClient()
  : m_loader_init_complete(false), m_new_width(800), m_new_height(600), m_width(800), m_height(600)
{
  m_manager = new EntityManager;

  platformInit();
  m_pipeline = new Pipeline;
  if(!Loader::isThreaded()) {
    platformInitLoader();
    m_loader = m_pipeline->createLoader();
  }
  platformDetachContext(); // Some platforms (notably X11 with OpenGL) require contexts be unbound before sharing resources
  m_loader_thread = boost::thread(std::mem_fun(&SenseClient::runLoaderThread), this);
  while(!m_loader_init_complete) {}
  platformAttachContext();
  if(!loader_error_string.empty()) {
    m_loader_thread.join();
    delete m_pipeline;
    platformFinish();
    throw std::runtime_error(loader_error_string.c_str());
  }

  framebuffer = m_pipeline->createRenderTarget(width(), height(), false);
  m_pipeline->setViewport(width(), height());

  setupPythonModule();

  readScriptsDir("../data/materials", ".smtl");
  readScriptsDir("../data/definitions", ".sdef");

  mesh = m_datamgr->loadMesh("__quad__");
  mat = m_datamgr->loadMaterial("simple");
};

SenseClient::~SenseClient()
{
  m_pipeline->destroyRenderTarget(framebuffer);
  m_datamgr->finish();
  m_loader_thread.join();
  if(!Loader::isThreaded()) {
    delete m_loader;
    platformFinishLoader();
  }
  delete m_pipeline;
  platformFinish();
}

bool SenseClient::tick()
{
  if (m_new_width != m_width || m_new_height != m_height) {
    m_width = m_new_width;
    m_height = m_new_height;
    m_pipeline->destroyRenderTarget(framebuffer);
    framebuffer = m_pipeline->createRenderTarget(width(), height(), false);
    m_pipeline->setViewport(width(), height());
  }

  m_datamgr->mainThreadTick();
  m_pipeline->addDrawTask(mesh, mat, glm::mat4(1.f));
  m_pipeline->setRenderTarget(framebuffer);
  m_pipeline->render();
  m_pipeline->endFrame();
  platformSwapBuffers();
  return platformEventLoop();
}

void SenseClient::runLoaderThread()
{
  try {
    if(Loader::isThreaded()) {
      platformInitLoader();
      m_loader = m_pipeline->createLoader();
    }
    m_datamgr = new DataManager(m_loader);
  } catch (std::exception& e) {
    loader_error_string = e.what();
    return;
  }
  m_loader_init_complete = true;

  m_datamgr->exec();

  delete m_datamgr;
  if(Loader::isThreaded()) {
    delete m_loader;
    platformFinishLoader();
  }
}

void SenseClient::setupPythonModule()
{
  PyObject* SensModule = PyImport_ImportModule("SensEngine");
  PyObject* m = PyImport_AddModule("SensEngine.client");
  PyModule_AddObject(m, "__builtins__", PyEval_GetBuiltins());
  initDataManager(m);
  PyModule_AddObject(m, "loader", PyDataManager_create(m_datamgr));
  PyModule_AddObject(m, "manager", PyEntityManager_create(m_manager));
  PyModule_AddObject(SensModule, "client", m);
}

void SenseClient::readScriptsDir(fs::path dir, std::string ext)
{
  if(!exists(dir))
    return;

  PyObject* old_sys_path = appendSysPath(dir.string().c_str(), true);
  fs::directory_iterator end_itr;
  for(fs::directory_iterator itr(dir); itr != end_itr; ++itr) {
    if(itr->path().extension() == ext) {
      fs::ifstream stream;
      stream.open(itr->path());
      std::string pydefcode((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
      PyRun_SimpleString(pydefcode.c_str());
    }
   }
  restoreSysPath(old_sys_path);
}
