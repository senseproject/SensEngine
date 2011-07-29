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

#ifndef SENSE_CLIENT_DATAMANAGER_HPP
#define SENSE_CLIENT_DATAMANAGER_HPP

class Loader;

// The data manager loads anything that exists inside
// a game data package. It should be run on a worker thread,
// and "mainThreadTick" should be called every frame.
class DataManager
{
public:
  DataManager(Loader*);
  ~DataManager();

  void exec();
  void finish();
  void mainThreadTick();

private:
  Loader* m_loader;
  volatile bool m_finished;
};


#endif // SENSE_CLIENT_DATAMANAGER_HPP
