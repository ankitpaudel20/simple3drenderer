#include "core.h"

class node;
struct pointLight {

  vec3 position;
  float intensity;
  vec3 diffusecolor;
  vec3 specularcolor;

  float constant;
  float linear;
  float quadratic;

  node *model;

  pointLight(const vec3 &pos, const float &intensity, const vec3 &diffcol = 1, const vec3 &speccol = 1) : position(pos), diffusecolor(diffcol), specularcolor(speccol), intensity(intensity) {}
};
