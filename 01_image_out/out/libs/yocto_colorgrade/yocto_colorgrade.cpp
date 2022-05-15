//
// Implementation for Yocto/Grade.
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
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

#include "yocto_colorgrade.h"

#include <yocto/yocto_color.h>
#include <yocto/yocto_sampling.h>

// -----------------------------------------------------------------------------
// COLOR GRADING FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto {
color_image grade_image(const color_image& image, const grade_params& params) {
  auto img     = image;
  int  j       = 0;
  int  i       = 0;
  auto casuale = make_rng(57462);
  for (auto& pixel : image.pixels) {
    auto color = xyz(pixel);
    color      = color * pow(2, params.exposure);
    if (params.filmic == true) {
      color = color * 0.6;
      color = (pow(color, 2) * 2.51 + color * 0.03) /
              (pow(color, 2) * 2.43 + color * 0.59 + 0.14);
    }
    if (params.srgb == true) {
      color = rgb_to_srgb(color);
    }
    color          = clamp(color, 0, 1);
    color          = color * params.tint;  // da rivedere se va male
    auto g         = (color.x + color.y + color.z) / 3;
    color          = g + (color - g) * (params.saturation * 2);
    color          = gain(color, 1 - params.contrast);
    auto  vr       = 1 - params.vignette;
    float x        = image.width;
    float y        = image.height;
    auto  size     = vec2f{x, y};
    float i1       = i;
    float j1       = j;
    auto  ij       = vec2f{i1, j1};
    auto  r        = length((ij - (size) / 2)) / length((size / 2));
    color          = color * (1 - smoothstep(vr, 2 * vr, r));
    color          = color + (rand1f(casuale) - 0.5) * params.grain;
    vec4f modifica = {color.x, color.y, color.z, pixel.w};
    set_pixel(img, i, j, modifica);
    i = i + 1;
    if (i == image.width) {
      j = j + 1;
      i = 0;
    }
  }
  i = 0;
  j = 0;
  if (params.mosaic != 0) {
    for (auto& pixel : img.pixels) {
      auto ciao   = i - i % params.mosaic;
      auto pippo  = j - j % params.mosaic;
      auto pixel2 = get_pixel(img, ciao, pippo);
      set_pixel(img, i, j, pixel2);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
  }
  i = 0;
  j = 0;
  if (params.grid != 0) {
    for (auto& pixel : img.pixels) {
      auto pixel3 = get_pixel(img, i, j);
      auto colore = xyz(pixel3);
      if (0 == i % params.grid || 0 == j % params.grid) {
        colore = colore * 0.5;
      } else {
        colore = colore;
      }
      vec4f barabozzo = {colore.x, colore.y, colore.z, pixel3.w};
      set_pixel(img, i, j, barabozzo);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
  }

  if (params.sigma > 0.0) {
    auto immagine   = img;
    int  matrix_len = pow(
        1 + 2 * pow((-2 * pow(2, params.sigma) * -5.298317), 0.5), 0.5);
    if (int(matrix_len) % 2 == 0) {
      matrix_len += 1;
    }
    int gen = matrix_len / 2;

    double matrice[50][50];
    double sum = 0.0;
    double s   = 2.0 * params.sigma * params.sigma;

    for (int x = -gen; x <= gen; x++) {
      for (int y = -gen; y <= gen; y++) {
        double r                  = x * x + y * y;
        matrice[x + gen][y + gen] = (exp(-r / s)) / (3.14159265 * s);
        sum += matrice[x + gen][y + gen];
      }
    }

    for (int i = 0; i < int(matrix_len); i++) {
      for (int j = 0; j < int(matrix_len); j++) {
        matrice[i][j] /= sum;
      }
    }
    i = 0;
    j = 0;
    for (j = 0; j < img.height - 1; j++) {
      for (i = 0; i < img.width - 1; i++) {
        if ((i - gen >= 0) && (j + gen < img.height) && (j - gen >= 0) &&
            (i + gen < img.width)) {
          vec3f color;
          int   z = 0;
          for (int y = j - gen; y < j - gen + matrix_len; y++) {
            int w = 0;
            for (int x = i - gen; x < i - gen + matrix_len; x++) {
              color += xyz(get_pixel(img, x, y)) * matrice[z][w];
              w += 1;
            }
            z += 1;
          }
          vec4f colore = {color.x, color.y, color.z, get_pixel(img, i, j).w};
          set_pixel(immagine, i, j, colore);
        } else {
          set_pixel(immagine, i, j, {0, 0, 0, 0});
        }
      }
    }
    img = immagine;
  }
  if (params.sobel == true) {
    double matrix1[3][3];
    matrix1[0][0] = 2.0*2;
    matrix1[0][1] = 0.0*2;
    matrix1[0][2] = -2.0*2;
    matrix1[1][0] = 4.0*2;
    matrix1[1][1] = 0.0*2;
    matrix1[1][2] = -4.0*2;
    matrix1[2][0] = 2.0*2;
    matrix1[2][1] = 0.0;
    matrix1[2][2] = -2.0*2;

    double matrix2[3][3];
    matrix2[0][0] = 2.0*2;
    matrix2[0][1] = 4.0*2;
    matrix2[0][2] = 2.0*2;
    matrix2[1][0] = 0.0*2;
    matrix2[1][1] = 0.0*2;
    matrix2[1][2] = 0.0*2;
    matrix2[2][0] = -2.0*2;
    matrix2[2][1] = -4.0*2;
    matrix2[2][2] = -2.0*2;

    int i = 0;
    int j = 0;
    for (auto& pixel : image.pixels) {
      auto  colore    = xyz(pixel);
      auto  ciao      = colore.x * 0.299 + colore.y * 0.587 + colore.z * 0.114;
      vec4f barabozzo = {ciao, ciao, ciao, pixel.w};
      set_pixel(img, i, j, barabozzo);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }

    auto immagine = img;

    i = 0;
    j = 0;
    for (auto& pixel : img.pixels) {
      if ((i - 3 >= 0) && (j - 3 >= 0) && (i + 3 <= image.width) &&
          (j + 3 <= image.height)) {
        vec3f Gx;
        vec3f Gy;
        int   z = 0;
        for (int y = j - 3; y < j - 3 + 3; y++) {
          int w = 0;
          for (int x = i - 3; x < i - 3 + 3; x++) {
            Gx += xyz(get_pixel(img, x, y)) * matrix1[z][w];
            Gy += xyz(get_pixel(img, x, y)) * matrix2[z][w];
            w += 1;
          }
          z += 1;
        }
        auto G       = pow(Gx, 2) + pow(Gy, 2);
        G            = pow(G, 0.5);
        vec4f colore = {G.x, G.y, G.z, get_pixel(img, i, j).w};
        set_pixel(immagine, i, j, colore);
      } else {
        set_pixel(immagine, i, j, {0, 0, 0, 0});
      }
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
    img = immagine;
  }
  if (params.seppia == true) {
    int i = 0;
    int j = 0;
    for (auto& pixel : image.pixels) {
      auto  colore    = xyz(pixel);
      vec4f barabozzo = {
          (colore.x * 0.393 + colore.y * 0.769 + colore.z * 0.189),
          (colore.x * 0.349 + colore.y * 0.686 + colore.z * 0.168),
          (colore.x * 0.272 + colore.y * 0.534 + colore.z * 0.131), pixel.w};
      set_pixel(img, i, j, barabozzo);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
  }
  if (params.bianco_nero == true) {
    int i = 0;
    int j = 0;
    for (auto& pixel : image.pixels) {
      auto  colore    = xyz(pixel);
      auto  ciao      = colore.x * 0.299 + colore.y * 0.587 + colore.z * 0.114;
      vec4f barabozzo = {ciao, ciao, ciao, pixel.w};
      set_pixel(img, i, j, barabozzo);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
  }
  if (params.inverso == true) {
    int  i   = 0;
    int  j   = 0;
    for (auto& pixel : image.pixels) {
      auto colore     = xyz(pixel);
      colore          = 1 - colore;
      vec4f barabozzo = {colore.x, colore.y, colore.z, pixel.w};
      set_pixel(img, i, j, barabozzo);
      i = i + 1;
      if (i == image.width) {
        j = j + 1;
        i = 0;
      }
    }
  }
  return img;
  }
}  // namespace yocto


