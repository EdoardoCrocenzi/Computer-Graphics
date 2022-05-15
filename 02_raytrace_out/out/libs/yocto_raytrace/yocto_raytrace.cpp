//
// Implementation for Yocto/RayTrace.
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

#include "yocto_raytrace.h"

#include <yocto/yocto_cli.h>
#include <yocto/yocto_geometry.h>
#include <yocto/yocto_parallel.h>
#include <yocto/yocto_sampling.h>
#include <yocto/yocto_shading.h>
#include <yocto/yocto_shape.h>

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR SCENE EVALUATION
// -----------------------------------------------------------------------------
namespace yocto {

// Generates a ray from a camera for yimg::image plane coordinate uv and
// the lens coordinates luv.
static ray3f eval_camera(const camera_data& camera, const vec2f& uv) {
  auto film = camera.aspect >= 1
                  ? vec2f{camera.film, camera.film / camera.aspect}
                  : vec2f{camera.film * camera.aspect, camera.film};
  auto q    = transform_point(camera.frame,
      {film.x * (0.5f - uv.x), film.y * (uv.y - 0.5f), camera.lens});
  auto e    = transform_point(camera.frame, {0, 0, 0});
  return {e, normalize(e - q)};
}

}  // namespace yocto

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR PATH TRACING
// -----------------------------------------------------------------------------
namespace yocto {



// Raytrace renderer.
static vec4f shade_raytrace(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  auto isec = intersect_bvh(bvh, scene, ray);
  if (!isec.hit) {
    vec3f ciao     = eval_environment(scene, ray.d);
    vec4f ciaociao = {ciao.x, ciao.y, ciao.z, 0};
    return ciaociao;
  }
  auto& instance = scene.instances[isec.instance];
  auto material = eval_material(scene, instance, isec.element, isec.uv);
  auto& shape    = scene.shapes[instance.shape];

  auto  normal   = transform_direction(
      instance.frame, eval_normal(shape, isec.element, isec.uv));
  auto position = transform_point(
      instance.frame, eval_position(shape, isec.element, isec.uv));
  vec2f coord    = eval_texcoord(shape, isec.element, isec.uv);
  
  auto radiance = material.emission;

  vec4f barabozzo = {radiance.x, radiance.y, radiance.z, 0};
  vec4f material2 = {material.color.x, material.color.y, material.color.z, 0};

  auto color = material.color;
  material2  = {color.x, color.y, color.z, 0};
  
  if (!shape.points.empty()) {
    normal = -ray.d;
  } else if (!shape.lines.empty()) {
    normal = orthonormalize(-ray.d, normal);
  } else if (!shape.triangles.empty()) {
    if (dot(-ray.d, normal) < 0) {
      normal = -normal;
    }
  }

  if (rand1f(rng) < 1 - material.opacity) {
    return shade_raytrace(scene, bvh, ray3f{position, ray.d},
                            bounce + 1, rng, params);
  }

  if (bounce >= params.bounces) {
    vec4f barabozzo = {radiance.x, radiance.y, radiance.z, 0};
    return barabozzo;
  }
  if (material.type == material_type::matte) {
    auto incoming = sample_hemisphere_cos(normal, rand2f(rng));
    barabozzo += rgb_to_rgba(material.color) * shade_raytrace(scene, bvh,
                                 ray3f{position, incoming}, bounce + 1, rng,
                                 params);
    return barabozzo;
  } else if (material.type == material_type::reflective &&
             material.roughness == 0) {
    auto  incoming  = reflect(-ray.d, normal);
    vec3f bla       = fresnel_schlick(material.color, normal, -ray.d);
    vec4f bla2      = {bla.x, bla.y, bla.z, material2.w};
    barabozzo += bla2 * shade_raytrace(scene, bvh, ray3f{position, incoming},
                            bounce + 1, rng, params);
    return barabozzo;
  } else if (material.type == material_type::reflective &&
             material.roughness != 0) {
    auto  exponent  = 2 / (material.roughness * material.roughness);
    auto halfway  = sample_hemisphere_cospower(
        exponent, normal, rand2f(rng));
    auto  incoming  = reflect(-ray.d, halfway);
    vec3f bla       = fresnel_schlick(material.color, halfway, -ray.d);
    vec4f bla2      = {bla.x, bla.y, bla.z, material2.w};
    barabozzo += bla2 * shade_raytrace(scene, bvh, ray3f{position, incoming},
                            bounce + 1, rng, params); 
    return barabozzo;
  } else if (material.type == material_type::glossy) {
    auto  exponent = 2 / (material.roughness * material.roughness);
    auto  halfway  = sample_hemisphere_cospower(exponent, normal, rand2f(rng));
    vec3f fresnel  = {0.04, 0.04, 0.04};
    vec3f f        = fresnel_schlick(fresnel, halfway, -ray.d);
    if (rand1f(rng) < f.x) {
      auto incoming = reflect(-ray.d, halfway);
      barabozzo += shade_raytrace(
          scene, bvh, ray3f{position, incoming}, bounce + 1, rng, params);
    } else {
      auto incoming = sample_hemisphere_cos(normal, rand2f(rng));
      barabozzo += rgb_to_rgba(material.color) * shade_raytrace(scene, bvh,
                                   ray3f{position, incoming}, bounce + 1, rng,
                                   params);
    }
    return barabozzo;
  } else if (material.type == material_type::transparent) {
    vec3f fresnel  = {0.04, 0.04, 0.04};
    vec3f f        = fresnel_schlick(fresnel, normal, ray.d);
    if (rand1f(rng) < f.x) {
      auto incoming = reflect(-ray.d, normal);
      barabozzo += shade_raytrace(
          scene, bvh, ray3f{position, incoming}, bounce + 1, rng, params);
    } else {
      auto incoming = ray.d;
      barabozzo += rgb_to_rgba(material.color) * shade_raytrace(scene, bvh, ray3f{position, incoming}, bounce + 1, rng, params);
    }
    return barabozzo;
  } else if (material.type == material_type::refractive) {
    vec3f fresnel = {0.04, 0.04, 0.04};
    vec3f f       = fresnel_schlick(fresnel, normal, -ray.d);
    if (rand1f(rng) < f.x) {
      auto incoming = reflect(-ray.d, normal);
      barabozzo += shade_raytrace(scene, bvh, ray3f{position, incoming}, bounce + 1, rng, params);
    } else {
      auto   normale          = normal;
      double refraction_ratio = 0.58;
      if (dot(-ray.d, normal) < 0) {
          refraction_ratio = (1.0 / 0.58);
        normale          = -normal;
      } else {
        refraction_ratio = 0.58;
        normale          = normal;
      }
      auto incoming = refract(-ray.d, normale, refraction_ratio);
      
      barabozzo += material2 * shade_raytrace(scene, bvh, ray3f{position, incoming}, bounce + 1, rng, params);
    }
    return barabozzo;
  }
}

// Matte renderer.
static vec4f shade_matte(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  return {0, 0, 0, 0};
}

// Eyelight renderer.
static vec4f shade_eyelight(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  auto isec = intersect_bvh(bvh,scene, ray);
  if (!isec.hit) {
    return {0, 0, 0};
  }
  auto& instance = scene.instances[isec.instance];
  auto& material = scene.materials[instance.material];
  auto& shape    = scene.shapes[instance.shape];
  auto  normal   = transform_direction(
      instance.frame, eval_normal(shape, isec.element, isec.uv));
  vec4f barabozzo = {material.color.x, material.color.y, material.color.z, 0};
  return barabozzo*dot(normal,-ray.d);
}

static vec4f shade_normal(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  auto isec = intersect_bvh(bvh, scene, ray);
  if (!isec.hit) {
    return {0, 0, 0};
  }
  auto& instance = scene.instances[isec.instance];
  auto& shape   = scene.shapes[instance.shape];
  auto  normal   = transform_direction(
      instance.frame, eval_normal(shape, isec.element, isec.uv));
  vec4f barabozzo = {normal.x, normal.y, normal.z, 0};
  return barabozzo*0.5+0.5;
}

static vec4f shade_texcoord(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  auto  isec     = intersect_bvh(bvh, scene, ray);
  auto& instance = scene.instances[isec.instance];
  auto& shape    = scene.shapes[instance.shape];
  vec2f ciao     = eval_texcoord(shape, isec.element, isec.uv);
  auto  barabozzo = fmod(ciao.x, 1);
  auto  barabozzo2 = fmod(ciao.y, 1);
  return {barabozzo, barabozzo2, 0, 0};
}

static vec4f shade_color(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
  auto isec = intersect_bvh(bvh, scene, ray);
  if (!isec.hit) {
    return {0, 0, 0};
  }
  auto& instance = scene.instances[isec.instance];
  auto& material = scene.materials[instance.material];
  vec4f barabozzo = {material.color.x, material.color.y, material.color.z, 0};
  return barabozzo;
}

static vec4f matcap(const scene_data& scene, const bvh_scene& bvh, const ray3f& ray, int bounce, rng_state& rng, const raytrace_params& params) {
  auto isec = intersect_bvh(bvh, scene, ray);
  if (!isec.hit) {
    return {0, 0, 0};
  }
  auto& instance = scene.instances[isec.instance];
  auto  normal   = transform_direction(instance.frame, eval_normal(scene.shapes[instance.shape], isec.element, isec.uv));
  auto& material = eval_material(scene, instance, isec.element, isec.uv);
  if (isec.element == 0) {
    vec4f barabozzo = rgb_to_rgba(material.color);
    return barabozzo;
  }
  vec3f  reflected   = reflect(-ray.d, normal);
  double m           = 2 * sqrt(pow(reflected.x, 2) + pow(reflected.y, 2) + pow(reflected.z + 1, 2));
  float ciao = (reflected.x + reflected.y + reflected.z) / m + 0.5;
  float ciao2     = (reflected.x + reflected.y + reflected.z) / m + 0.5;
  float ciao3   = (reflected.x + reflected.y + reflected.z) / m + 0.5;
  vec3f  ciaociao  = {ciao, ciao2, ciao3};
  vec4f  barabozzo   = rgb_to_rgba(ciaociao) * rgb_to_rgba(material.color);
  return barabozzo;
  }
static vec4f cellshading(const scene_data& scene, const bvh_scene& bvh,
    const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params) {
    auto isec = intersect_bvh(bvh, scene, ray);
    if (!isec.hit) {
      return {0, 0, 0, 0};
    }
    auto& instance = scene.instances[isec.instance];
    auto  normal   = transform_direction(instance.frame,
        eval_normal(scene.shapes[instance.shape], isec.element, isec.uv));
    auto&  material        = scene.materials[instance.material];
    auto& shape    = scene.shapes[instance.shape];
    auto   position        = transform_point(
        instance.frame, eval_position(shape, isec.element, isec.uv));

    float shadow        = 1;
    vec3f light         = {0.4, 0.8, 0.8};
    vec4f light_color   = {0.85, 0.85, 0.5, 1};
    vec4f ambient_color = {0.4, 0.4, 0.4, 1};
    float scalare       = dot(light, normal);




    auto intersezione = intersect_bvh(bvh, scene, ray3f{position, light});
    auto& instance2    = scene.instances[intersezione.instance];
    auto& materiale    = scene.materials[instance2.material];
    vec3f  zero         = {0, 0, 0};

    float light_intensity = 0.8;
    vec4f luce            = light_color * light_intensity;


    vec4f  specular_color  = {0.9, 0.9, 0.9, 1};
    auto   glossines       = 32;
    vec3f  half_vector     = normalize(light + (-ray.d));
    float  scalare2        = dot(normal, half_vector);
    float  specular_intensity = pow(
        scalare2 * light_intensity, glossines * glossines);
    float specular_intensity_smooth = smoothstep(
        0.005f, 0.1f, specular_intensity);
    vec4f specular = specular_intensity_smooth * specular_color;

    auto rimDot   = 1 - dot(-ray.d, normal);
    float  RimThreShold    = 0.1;
    float RimAmount        = 0.716;
    float rimIntensity = rimDot * pow(scalare, RimThreShold);
    rimIntensity           = smoothstep(
        RimAmount - 0.01f, RimAmount + 0.01f, rimIntensity);
    float rim = rimIntensity;
    if (intersezione.hit && materiale.emission == zero) {
      shadow = rand1f(rng);
    }
    if (isec.element == 0) {
      return rgb_to_rgba(material.color) * (luce * shadow);
    } else {
      return rgb_to_rgba(material.color) *
             (luce * shadow + ambient_color + specular + rim);
    }
  }

// Trace a single ray from the camera using the given algorithm.
using raytrace_shader_func = vec4f (*)(const scene_data& scene,
    const bvh_scene& bvh, const ray3f& ray, int bounce, rng_state& rng,
    const raytrace_params& params);
static raytrace_shader_func get_shader(const raytrace_params& params) {
  switch (params.shader) {
    case raytrace_shader_type::matcap: return matcap;
    case raytrace_shader_type::raytrace: return shade_raytrace;
    case raytrace_shader_type::matte: return shade_matte;
    case raytrace_shader_type::eyelight: return shade_eyelight;
    case raytrace_shader_type::normal: return shade_normal;
    case raytrace_shader_type::texcoord: return shade_texcoord;
    case raytrace_shader_type::color: return shade_color;
    case raytrace_shader_type::cellshading: return cellshading;
    default: {
      throw std::runtime_error("sampler unknown");
      return nullptr;
    }
  }
}

// Build the bvh acceleration structure.
bvh_scene make_bvh(const scene_data& scene, const raytrace_params& params) {
  return make_bvh(scene, false, false, params.noparallel);
}

// Init a sequence of random number generators.
raytrace_state make_state(
    const scene_data& scene, const raytrace_params& params) {
  auto& camera = scene.cameras[params.camera];
  auto  state  = raytrace_state{};
  if (camera.aspect >= 1) {
    state.width  = params.resolution;
    state.height = (int)round(params.resolution / camera.aspect);
  } else {
    state.height = params.resolution;
    state.width  = (int)round(params.resolution * camera.aspect);
  }
  state.samples = 0;
  state.image.assign(state.width * state.height, {0, 0, 0, 0});
  state.hits.assign(state.width * state.height, 0);
  state.rngs.assign(state.width * state.height, {});
  auto rng_ = make_rng(1301081);
  for (auto& rng : state.rngs) {
    rng = make_rng(961748941ull, rand1i(rng_, 1 << 31) / 2 + 1);
  }
  return state;
}

// Progressively compute an image by calling trace_samples multiple times.
void raytrace_samples(raytrace_state& state, const scene_data& scene,
    const bvh_scene& bvh, const raytrace_params& params) {
  if (state.samples >= params.samples) return;
  auto& camera = scene.cameras[params.camera];
  auto  shader = get_shader(params);
  state.samples += 1;
  if (params.samples == 1) {
    for (auto idx = 0; idx < state.width * state.height; idx++) {
      auto i = idx % state.width, j = idx / state.width;
      auto u = (i + 0.5f) / state.width, v = (j + 0.5f) / state.height;
      auto ray      = eval_camera(camera, {u, v});
      auto radiance = shader(scene, bvh, ray, 0, state.rngs[idx], params);
      if (!isfinite(radiance)) radiance = {0, 0, 0};
      state.image[idx] += radiance;
      state.hits[idx] += 1;
    }
  } else if (params.noparallel) {
    for (auto idx = 0; idx < state.width * state.height; idx++) {
      auto i = idx % state.width, j = idx / state.width;
      auto u        = (i + rand1f(state.rngs[idx])) / state.width,
           v        = (j + rand1f(state.rngs[idx])) / state.height;
      auto ray      = eval_camera(camera, {u, v});
      auto radiance = shader(scene, bvh, ray, 0, state.rngs[idx], params);
      if (!isfinite(radiance)) radiance = {0, 0, 0};
      state.image[idx] += radiance;
      state.hits[idx] += 1;
    }
  } else {
    parallel_for(state.width * state.height, [&](int idx) {
      auto i = idx % state.width, j = idx / state.width;
      auto u        = (i + rand1f(state.rngs[idx])) / state.width,
           v        = (j + rand1f(state.rngs[idx])) / state.height;
      auto ray      = eval_camera(camera, {u, v});
      auto radiance = shader(scene, bvh, ray, 0, state.rngs[idx], params);
      if (!isfinite(radiance)) radiance = {0, 0, 0};
      state.image[idx] += radiance;
      state.hits[idx] += 1;
    });
  }
}

// Check image type
static void check_image(
    const color_image& image, int width, int height, bool linear) {
  if (image.width != width || image.height != height)
    throw std::invalid_argument{"image should have the same size"};
  if (image.linear != linear)
    throw std::invalid_argument{
        linear ? "expected linear image" : "expected srgb image"};
}

// Get resulting render
color_image get_render(const raytrace_state& state) {
  auto image = make_image(state.width, state.height, true);
  get_render(image, state);
  return image;
}
void get_render(color_image& image, const raytrace_state& state) {
  check_image(image, state.width, state.height, true);
  auto scale = 1.0f / (float)state.samples;
  for (auto idx = 0; idx < state.width * state.height; idx++) {
    image.pixels[idx] = state.image[idx] * scale;
  }
}

}  // namespace yocto
