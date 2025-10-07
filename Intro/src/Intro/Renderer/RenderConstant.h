#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Intro {
    // UBO�󶨵�
    constexpr GLuint CAMERA_UBO_BINDING = 0;
    constexpr GLuint LIGHTS_UBO_BINDING = 1;

    // ����Դ����
    constexpr int MAX_DIR_LIGHTS = 4;
    constexpr int MAX_POINT_LIGHTS = 8;
    constexpr int MAX_SPOT_LIGHTS = 4;
}