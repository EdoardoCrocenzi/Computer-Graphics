//
// Implementation for Yocto/Model
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

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "yocto_model.h"

#include <yocto/yocto_sampling.h>

#include "ext/perlin-noise/noise1234.h"

// -----------------------------------------------------------------------------
// USING DIRECTIVES
// -----------------------------------------------------------------------------
namespace yocto {

// using directives
using std::array;
using std::string;
using std::vector;
using namespace std::string_literals;

}  // namespace yocto

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR EXAMPLE OF PROCEDURAL MODELING
// -----------------------------------------------------------------------------
namespace yocto {

float noise(const vec3f& p) { return ::noise3(p.x, p.y, p.z); }
vec2f noise2(const vec3f& p) {
  return {noise(p + vec3f{0, 0, 0}), noise(p + vec3f{3, 7, 11})};
}
vec3f noise3(const vec3f& p) {
  return {noise(p + vec3f{0, 0, 0}), noise(p + vec3f{3, 7, 11}),
      noise(p + vec3f{13, 17, 19})};
}
float fbm(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 1.0f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * fabs(noise(p * scale));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}
float turbulence(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 1.0f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * fabs(noise(p * scale));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}
float ridge(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 0.5f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * (1 - fabs(noise(p * scale))) * (1 - fabs(noise(p * scale)));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}

void add_polyline(shape_data& shape, const vector<vec3f>& positions,
    const vector<vec4f>& colors, float thickness = 0.0001f) {
  auto offset = (int)shape.positions.size();
  shape.positions.insert(
      shape.positions.end(), positions.begin(), positions.end());
  shape.colors.insert(shape.colors.end(), colors.begin(), colors.end());
  shape.radius.insert(shape.radius.end(), positions.size(), thickness);
  for (auto idx = 0; idx < positions.size() - 1; idx++) {
    shape.lines.push_back({offset + idx, offset + idx + 1});
  }
}

void sample_shape(vector<vec3f>& positions, vector<vec3f>& normals,
    vector<vec2f>& texcoords, const shape_data& shape, int num) {
  auto triangles  = shape.triangles;
  auto qtriangles = quads_to_triangles(shape.quads);
  triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
  auto cdf = sample_triangles_cdf(triangles, shape.positions);
  auto rng = make_rng(19873991);
  for (auto idx = 0; idx < num; idx++) {
    auto [elem, uv] = sample_triangles(cdf, rand1f(rng), rand2f(rng));
    auto q          = triangles[elem];
    positions.push_back(interpolate_triangle(
        shape.positions[q.x], shape.positions[q.y], shape.positions[q.z], uv));
    normals.push_back(normalize(interpolate_triangle(
        shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
    if (!texcoords.empty()) {
      texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
          shape.texcoords[q.y], shape.texcoords[q.z], uv));
    } else {
      texcoords.push_back(uv);
    }
  }
}

float voronoise(vec3f& position, const voronoise_params& params) {
  float va = 0.0;
  float wt = 0.0;
  vec3f ciao = {floor(position.x), floor(position.y), floor(position.z)};
  vec3f ciao2  = {position.x - ciao.x, position.y - ciao.y, position.z - ciao.z};
  float k  = 1.0 + 63.0 * pow(1.0 - params.v, 4.0);
  if (params.v == 0) {
    k = 1.0 + 63.0 * pow(1.0 - 0.1, 4.0);
  }
  for (int z = -2; z <= 2; z++) {
    for (int y = -2; y <= 2; y++) {
      for (int x = -2; x <= 2; x++) {
        vec3f g = {float(x), float(y), float(z)};
        vec3f o = {dot(ciao + g, vec3f{127.1, 311.7, 294.5}),
            dot(ciao + g, vec3f{269.5, 183.3, 104.8}),
            dot(ciao + g, vec3f{419.2, 371.9, 304.5})};
        o.x     = o.x - floor((o.x));
        o.y     = o.y - floor((o.y));
        o.z     = o.z - floor((o.z));
        o       = o * vec3f{params.u, params.u, 1.0};
        vec3f r = g - ciao2 + o;
        float d = dot(r, r);
        float w = pow(float(1.0 - smoothstep(0.0f, 1.414f, sqrt(d))), k);
        va += w * o.z;
        wt += w;
      }
    }
  }
  return va / wt;
}

float smoothvoronoi(vec3f& position) {
  vec3f ciao   = {floor(position.x), floor(position.y), floor(position.z)};
  vec3f ciao2   = {position.x - ciao.x, position.y - ciao.y, position.z - ciao.z};
  float res = 0.0;
  for (int z = -1; z <= 1; z++) {
    for (int y = -1; y <= 1; y++) {
      for (int x = -1; x <= 1; x++) {
        vec3f b = {float(x), float(y), float(z)};
        vec3f o = {dot((ciao + b), vec3f{127.1, 311.7, 0.5}),
            dot((ciao + b), vec3f{269.5, 183.3, 0.5}),
            dot((ciao + b), vec3f{419.2, 371.9, 0.5})};
        o.x     = o.x - floor(sin(o.x) * 43758.5453);
        o.y     = o.y - floor(sin(o.y) * 43758.5453);
        o.z     = o.z - floor(sin(o.z) * 43758.5453);
        o.x     = o.x - floor(o.x);
        o.y     = o.y - floor(o.y);
        o.z     = o.z - floor(o.z);
        vec3f r = b - ciao2 + o;
        float d = dot(r, r);
        res += 1.0 / pow(d, 8.0f);
      }
    }
  }
  return pow(1.0 / res, 1.0 / 16.0);
}

void make_terrain(shape_data& shape, const terrain_params& params) {
  int i = 0;
  for  (auto posizione : shape.positions) {
    auto& normale = shape.normals[i];
    shape.positions[i] +=
        normale *
                 ridge(shape.positions[i] * params.scale, params.octaves) * params.height *
                 (1.f - length(shape.positions[i] - params.center) / params.size);
    auto percentuale = shape.positions[i].y / params.height;
    if (percentuale <= 0.33) {
      shape.colors.push_back(params.bottom);
    } else if (percentuale <= 0.66) {
      shape.colors.push_back(params.middle);
    } else {
      shape.colors.push_back(params.top);
    }
    i++;
  }
  shape.normals = compute_normals(shape);
  
}

void make_displacement(shape_data& shape, const displacement_params& params) {
  int i = 0;
  for (auto posizione : shape.positions) {
    auto& normale = shape.normals[i];
      shape.positions[i] +=
          normale *
          turbulence(shape.positions[i] * params.scale, params.octaves) *
          params.height;

      shape.colors.push_back(interpolate_line(params.bottom, params.top,
          turbulence(posizione * params.scale, params.octaves)));
      i++;
  }

  shape.normals = compute_normals(shape);
}

void make_voronoise(shape_data& shape, const voronoise_params& params) {
  int i = 0;
  for (auto posizione : shape.positions) {
    auto  senza_misura = shape.positions[i];
    auto& normal = shape.normals[i];
    shape.positions[i] +=
        normal *
        voronoise(vec3f{posizione.x, posizione.y, posizione.z} * params.scale,
            params) *
        params.height;
    shape.colors.push_back(interpolate_line(params.bottom, params.top,
        (distance(senza_misura, shape.positions[i]) / params.height)));
    i++;
  }
  compute_normals(shape.normals, shape);
}

void make_smoothvoronoi(
    shape_data& shape, const voronoise_params& params) {
  int i = 0;
  for (auto posizione : shape.positions) {
    auto  senza_misura = shape.positions[i];
    auto& normal = shape.normals[i];
    shape.positions[i] +=
        normal *
        smoothvoronoi(
            vec3f{posizione.x, posizione.y, posizione.z} * params.scale) *
        params.height;

    shape.colors.push_back(interpolate_line(params.bottom, params.top,
        (distance(senza_misura, shape.positions[i]) / params.height)));
    i++;
  }
  compute_normals(shape.normals, shape);
}

void make_hair(shape_data& hair, const shape_data& shape, const hair_params& params) {
  shape_data capelli = shape;
  auto       size = capelli.positions.size();
  auto       rng     = make_rng(34000);
  sample_shape(capelli.positions, capelli.normals, capelli.texcoords, capelli, params.num);
  for (auto i = size; i < capelli.positions.size(); i++) {
    auto          casuale = rand1f(rng);
    if (params.densita_capelli > casuale) {
      continue;
    } else {
      vector<vec4f> colori;
      vector<vec3f> posizioni;
      auto          normale = capelli.normals[i];
      posizioni.push_back(capelli.positions[i]);
      colori.push_back(params.bottom);
      for (auto j = 0; j < params.steps; j++) {
        auto barabozzo = posizioni[j] +
                         (params.lenght / params.steps) * normale +
                         noise3(posizioni[j] * params.scale) * params.strength;
        barabozzo.y -= params.gravity;
        normale = normalize(barabozzo - posizioni[j]);
        posizioni.push_back(barabozzo);
        colori.push_back(interpolate_line(params.bottom, params.top,
            distance(barabozzo, posizioni[0]) / params.lenght));
      }
      colori[params.steps] = params.top;
      add_polyline(hair, posizioni, colori);
    }
  }
  auto tangente = lines_tangents(hair.lines, hair.positions);
  for (auto i = 0; i < tangente.size(); i++) {
    hair.tangents.push_back({tangente[i].x, tangente[i].y, tangente[i].z, 0});
  }

  
}

void make_grass(scene_data& scene, const instance_data& object,
    const vector<instance_data>& grasses, const grass_params& params) {
  auto  densita   = 0.6;
  auto  rng   = make_rng(34000);
  auto& barabozzo = scene.shapes[object.shape];
  sample_shape(
      barabozzo.positions, barabozzo.normals, barabozzo.texcoords, barabozzo, params.num);
  int i = 0;
  for (auto posizione: barabozzo.positions) {
    auto          casuale = rand1f(rng);
    if (params.densita_erba > casuale) {
      continue;
    } else {
      instance_data ciao;
      const auto&   erba = grasses[rand1i(rng, grasses.size())];
      ciao.shape         = erba.shape;
      ciao.material      = erba.material;

      ciao.frame.y = barabozzo.normals[i];
      ciao.frame.x = normalize(
          vec3f{1, 0, 0} - dot({1, 0, 0}, ciao.frame.y) * ciao.frame.y);
      ciao.frame.z = cross(ciao.frame.x, ciao.frame.y);
      ciao.frame.o = posizione;

      float random = 0.9f + rand1f(rng) * 0.1f;
      ciao.frame *= scaling_frame({random, random, random});

      random = rand1f(rng) * 2 * pif;
      ciao.frame *= rotation_frame(ciao.frame.y, random);

      random = 0.1f + rand1f(rng) * 0.1f;
      ciao.frame *= rotation_frame(ciao.frame.z, random);

      scene.instances.push_back(ciao);
    }
    i++;
  }
}



}  // namespace yocto
