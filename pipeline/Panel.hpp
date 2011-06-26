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

#ifndef SENSE_PIPELINE_PANEL_HPP
#define SENSE_PIPELINE_PANEL_HPP

#include <memory>
#include "3rdparty/glm/glm.hpp"

class Material;
class DrawBuffer;

struct Panel {
  std::shared_ptr<Material> mat;
  std::shared_ptr<DrawBuffer> buf;
  glm::mat4 matrix;
};

#endif // SENSE_PIPELINE_PANEL_HPP
