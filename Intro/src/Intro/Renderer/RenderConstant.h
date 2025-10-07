#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Intro {
    // UBO绑定点
    constexpr GLuint CAMERA_UBO_BINDING = 0;
    constexpr GLuint LIGHTS_UBO_BINDING = 1;

    // 最大光源数量
    constexpr int MAX_DIR_LIGHTS = 4;
    constexpr int MAX_POINT_LIGHTS = 8;
    constexpr int MAX_SPOT_LIGHTS = 4;
}