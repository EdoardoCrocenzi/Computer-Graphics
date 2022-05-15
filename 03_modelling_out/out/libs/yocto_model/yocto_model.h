//
// # Yocto/Model: Examples of procedural modeling
//

//
// LICENSE:
//
// Copyright (c) 2016 -- 2021 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#ifndef _YOCTO_MODEL_H_
#define _YOCTO_MODEL_H_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <yocto/yocto_geometry.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_scene.h>

#include <array>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// USING DIRECTIVES
// -----------------------------------------------------------------------------
namespace yocto {

// using directives
using std::array;
using std::string;
using std::vector;

}  // namespace yocto

// -----------------------------------------------------------------------------
// EXAMPLE OF PROCEDURAL MODELING
// -----------------------------------------------------------------------------
namespace yocto {

struct terrain_params {
  float size    = 0.1f;
  vec3f center  = zero3f;
  float height  = 0.1f;
  float scale   = 10;
  int   octaves = 8;
  vec4f bottom  = srgb_to_rgb(vec4f{154, 205, 50, 255} / 255);
  vec4f middle  = srgb_to_rgb(vec4f{205, 133, 63, 255} / 255);
  vec4f top     = srgb_to_rgb(vec4f{240, 255, 255, 255} / 255);
};

void make_terrain(shape_data& shape, const terrain_params& params);

struct displacement_params {
  float height  = 0.02f;
  float scale   = 50;
  int   octaves = 8;
  vec4f bottom  = srgb_to_rgb(vec4f{64, 224, 208, 255} / 255);
  vec4f top     = srgb_to_rgb(vec4f{244, 164, 96, 255} / 255);
};

struct voronoise_params {
  float height  = 0.02f;
  float scale   = 50;
  int   octaves = 8;
  vec4f bottom  = srgb_to_rgb(vec4f{64, 224, 208, 255} / 255);
  vec4f top     = srgb_to_rgb(vec4f{244, 164, 96, 255} / 255);
  float u       = 1.0f;
  float v       = 1.0f;
};

void make_voronoise(shape_data& shape, const voronoise_params& params);

void make_smoothvoronoi(shape_data& shape, const voronoise_params& params);

void make_displacement(shape_data& shape, const displacement_params& params);



struct hair_params {
  int   num      = 100000;
  int   steps    = 1;
  float lenght   = 0.02f;
  float scale    = 250;
  float strength = 0.01f;
  float gravity  = 0.0f;
  vec4f bottom   = srgb_to_rgb(vec4f{25, 25, 25, 255} / 255);
  vec4f top      = srgb_to_rgb(vec4f{244, 164, 96, 255} / 255);
  float densita_capelli = 0.0f;
};

void make_hair(
    shape_data& hair, const shape_data& shape, const hair_params& params);

struct grass_params {
  int num = 10000;
  float densita_erba = 0.0f;
};

void make_grass(scene_data& scene, const instance_data& object,
    const vector<instance_data>& grasses, const grass_params& params);

}  // namespace yocto

#endif
