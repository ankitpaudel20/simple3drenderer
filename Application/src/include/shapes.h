//
// Created by ankit on 9/12/20.
//

#ifndef OPENGL_CORE_SHAPES_H
#define OPENGL_CORE_SHAPES_H

#include "core.h"
#include "drawable.h"

namespace cube
{
        const std::vector<vec3> pos{

                    vec3(-0.5f, -0.5f, -0.5f),
                    vec3(0.5f, -0.5f, -0.5f),
                    vec3(0.5f,  0.5f, -0.5f),
                    vec3(0.5f,  0.5f, -0.5f),
                    vec3(-0.5f,  0.5f, -0.5f),
                    vec3(-0.5f, -0.5f, -0.5f),

                    vec3(-0.5f, -0.5f,  0.5f),
                    vec3(0.5f, -0.5f,  0.5f),
                    vec3(0.5f,  0.5f,  0.5f),
                    vec3(0.5f,  0.5f,  0.5f),
                    vec3(-0.5f,  0.5f,  0.5f),
                    vec3(-0.5f, -0.5f,  0.5f),

                    vec3(-0.5f,  0.5f,  0.5f),
                    vec3(-0.5f,  0.5f, -0.5f),
                    vec3(-0.5f, -0.5f, -0.5f),
                    vec3(-0.5f, -0.5f, -0.5f),
                    vec3(-0.5f, -0.5f,  0.5f),
                    vec3(-0.5f,  0.5f,  0.5f),

                    vec3(0.5f,  0.5f,  0.5f),
                    vec3(0.5f,  0.5f, -0.5f),
                    vec3(0.5f, -0.5f, -0.5f),
                    vec3(0.5f, -0.5f, -0.5f),
                    vec3(0.5f, -0.5f,  0.5f),
                    vec3(0.5f,  0.5f,  0.5f),

                    vec3(-0.5f, -0.5f, -0.5f),
                    vec3(0.5f, -0.5f, -0.5f),
                    vec3(0.5f, -0.5f,  0.5f),
                    vec3(0.5f, -0.5f,  0.5f),
                    vec3(-0.5f, -0.5f,  0.5f),
                    vec3(-0.5f, -0.5f, -0.5f),

                    vec3(-0.5f,  0.5f, -0.5f),
                    vec3(0.5f,  0.5f, -0.5f),
                    vec3(0.5f,  0.5f,  0.5f),
                    vec3(0.5f,  0.5f,  0.5f),
                    vec3(-0.5f,  0.5f,  0.5f),
                    vec3(-0.5f,  0.5f, -0.5f),
        };
}


#endif //OPENGL_CORE_SHAPES_H
