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

#include "Builtins.hpp"

float builtin_missing_data[] = {
  // cube (missing) vertices/texcoords. 6x4x5 components
  // front
  -1.f, -1.f, -1.f, 0.f, 0.f,
  1.f, -1.f, -1.f, 0.f, 1.f,
  1.f, -1.f, 1.f, 1.f, 1.f,
  -1.f, -1.f, 1.f, 1.f, 0.f,

  // back
  1.f, 1.f, -1.f, 0.f, 0.f,
  -1.f, 1.f, -1.f, 0.f, 1.f,
  -1.f, 1.f, 1.f, 1.f, 1.f,
  1.f, 1.f, 1.f, 1.f, 0.f,

  // top
  -1.f, -1.f, 1.f, 0.f, 0.f,
  1.f, -1.f, 1.f, 0.f, 1.f,
  1.f, 1.f, 1.f, 1.f, 1.f,
  -1.f, 1.f, 1.f, 1.f, 0.f,

  // bottom
  1.f, -1.f, -1.f, 0.f, 0.f,
  -1.f, -1.f, -1.f, 0.f, 1.f,
  -1.f, 1.f, -1.f, 1.f, 1.f,
  1.f, 1.f, -1.f, 1.f, 0.f,

  // left
  -1.f, 1.f, -1.f, 0.f, 0.f,
  -1.f, -1.f, -1.f, 0.f, 1.f,
  -1.f, -1.f, 1.f, 1.f, 1.f,
  -1.f, 1.f, 1.f, 1.f, 0.f,

  // right
  1.f, -1.f, -1.f, 0.f, 0.f,
  1.f, 1.f, -1.f, 0.f, 1.f,
  1.f, 1.f, 1.f, 1.f, 1.f,
  1.f, -1.f, 1.f, 1.f, 0.f,
};

short builtin_missing_indices[] = {
  0, 1, 2,
  0, 2, 3,
  4, 5, 6,
  4, 6, 7,
  8, 9, 10,
  8, 10, 11,
  12, 13, 14,
  12, 14, 15,
  16, 17, 18,
  16, 18, 19,
  20, 21, 22,
  20, 22, 23
};

short builtin_missing_idx_count = 36;

float builtin_quad_data[] = {
  // Data for a screen-aligned quad
  -1.f, -1.f, 0.f, 0.f, 1.f,
  -1.f, 1.f, 0.f, 0.f, 0.f,
  1.f, 1.f, 0.f, 1.f, 0.f,

  -1.f, -1.f, 0.f, 0.f, 1.f,
  1.f, 1.f, 0.f, 1.f, 0.f,
  1.f, -1.f, 0.f, 1.f, 1.f,
};
