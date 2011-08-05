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

#ifndef SENSE_CLIENT_HPP
#define SENSE_CLIENT_HPP

#ifdef _WIN32
// it's this or duplicate typedefs. I'd rather go with this >_>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/thread.hpp>
#include <string>

class Pipeline;
class Loader;
class DataManager;
class EntityManager;

struct RenderTarget;

struct ClientPlatform;

struct Material;
struct DrawableMesh;

struct Entity;

class SenseClient
{
public:
  SenseClient();
  ~SenseClient();

  bool tick();

  unsigned int width() const { return m_width; }
  unsigned int height() const { return m_height; }

private:
  void platformInit();
  void platformInitLoader();
  void platformFinish();
  void platformFinishLoader();
  void platformSwapBuffers();
  void platformAttachContext();
  void platformDetachContext();
  bool platformEventLoop();

  void runLoaderThread();

  void setupPythonModule();
  void readScriptsDir(boost::filesystem::path, std::string);

  const char* displayName();

  // general members
  DataManager* m_datamgr;
  EntityManager* m_manager;
  boost::thread m_loader_thread;
  volatile bool m_loader_init_complete;
  std::string loader_error_string;
  
  // Pipeline related members
  Pipeline* m_pipeline;
  Loader* m_loader;
  RenderTarget* framebuffer;

  // Temp entity for draw test (until we have a World)
  Entity* m_test_ent;

  // Information about the client window
  uint32_t m_new_width, m_new_height;
  uint32_t m_width, m_height;
  ClientPlatform* m_platform_info;

// Various platform-specific bits that need to be handled
#ifdef _WIN32
  bool platformWndProcReturn;
  const char* windowClass();
  friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
};

#endif // SENSE_CLIENT_HPP
