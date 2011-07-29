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

#include "DataManager.hpp"

#include <boost/thread/thread.hpp>

DataManager::DataManager(Loader* loader)
  : m_loader(loader)
{}

DataManager::~DataManager()
{}

void DataManager::exec()
{
  m_finished = false;
  while(!m_finished)
    boost::this_thread::yield();
}

void DataManager::finish()
{
  m_finished = true;
}

void DataManager::mainThreadTick()
{}