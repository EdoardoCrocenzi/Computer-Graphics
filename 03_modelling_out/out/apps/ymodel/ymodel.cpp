//
// LICENSE:
//
// Copyright (c) 2016 -- 2020 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <yocto/yocto_cli.h>
#include <yocto/yocto_scene.h>
#include <yocto/yocto_sceneio.h>
#include <yocto_model/yocto_model.h>
using namespace yocto;

#include <filesystem>

instance_data& get_instance(scene_data& scene, const string& name) {
  for (auto idx = 0; idx < scene.instances.size(); idx++) {
    if (scene.instance_names[idx] == name) return scene.instances[idx];
  }
  throw std::out_of_range{"unknown instance " + name};
}

void run(const vector<string>& args) {
  // command line parameters
  auto terrain      = ""s;
  auto tparams      = terrain_params{};
  auto displacement = ""s;
  auto dparams      = displacement_params{};
  auto voronoise = ""s;
  auto smoothvoronoi = ""s;
  auto vparams      = voronoise_params{};
  auto hair         = ""s;
  auto hairbase     = ""s;
  auto hparams      = hair_params{};
  auto grass        = ""s;
  auto grassbase    = ""s;
  auto gparams      = grass_params{};
  auto output       = "out.json"s;
  auto filename     = "scene.json"s;

  // parse command line
  auto error = string{};
  auto cli   = make_cli("ymodel", "Make procedural scenes");
  add_option(cli, "terrain", terrain, "terrain object");
  add_option(cli, "displacement", displacement, "displacement object");
  add_option(cli, "hair", hair, "hair object");
  add_option(cli, "hairbase", hairbase, "hairbase object");
  add_option(cli, "grass", grass, "grass object");
  add_option(cli, "grassbase", grassbase, "grassbase object");
  add_option(cli, "hairnum", hparams.num, "hair number");
  add_option(cli, "hairlen", hparams.lenght, "hair length");
  add_option(cli, "hairstr", hparams.strength, "hair strength");
  add_option(cli, "hairgrav", hparams.gravity, "hair gravity");
  add_option(cli, "hairstep", hparams.steps, "hair steps");
  add_option(cli, "output", output, "output scene");
  add_option(cli, "scene", filename, "input scene");
  add_option(cli, "voronoise", voronoise, "voronoise object");
  add_option(cli, "smoothvoronoi", smoothvoronoi, "smoothvoroni object");
  add_option(cli, "v", vparams.v, "valore di v");
  add_option(cli, "u", vparams.u, "valore di u");
  add_option(cli, "densita_capelli",hparams.densita_capelli, "densita_capelli");
  add_option(
      cli, "densita_erba", gparams.densita_erba, "densita_erba");
  if (!parse_cli(cli, args, error)) print_fatal(error);

  // load scene
  auto scene = scene_data{};
  if (!load_scene(filename, scene, error)) print_fatal(error);

  // create procedural geometry
  if (smoothvoronoi != "") {
    make_smoothvoronoi(scene.shapes[get_instance(scene, smoothvoronoi).shape], vparams);
  }
  if (voronoise != "") {
    make_voronoise(scene.shapes[get_instance(scene, voronoise).shape], vparams);
  }
  if (terrain != "") {
    make_terrain(scene.shapes[get_instance(scene, terrain).shape], tparams);
  }
  if (displacement != "") {
    make_displacement(
        scene.shapes[get_instance(scene, displacement).shape], dparams);
  }
  if (hair != "") {
    scene.shapes[get_instance(scene, hair).shape]      = {};
    scene.shape_names[get_instance(scene, hair).shape] = "hair";
    make_hair(scene.shapes[get_instance(scene, hair).shape],
        scene.shapes[get_instance(scene, hairbase).shape], hparams);
  }
  if (grass != "") {
    auto grasses = vector<instance_data>{};
    for (auto idx = 0; idx < scene.instances.size(); idx++) {
      if (scene.instance_names[idx].find(grass) != string::npos)
        grasses.push_back(scene.instances[idx]);
    }
    make_grass(scene, get_instance(scene, grassbase), grasses, gparams);
  }

  // make a directory if needed
  if (!make_scene_directories(output, scene, error)) print_fatal(error);

  // save scene
  if (!save_scene(output, scene, error)) print_fatal(error);
}

int main(int argc, const char* argv[]) {
  handle_errors(run, make_cli_args(argc, argv));
}
