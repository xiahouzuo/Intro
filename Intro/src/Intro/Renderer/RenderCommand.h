#pragma once
#ifndef INTRO_RENDERCOMMAND_H
#define INTRO_RENDERCOMMAND_H

#include <cstdint>

namespace Intro {
    class Mesh;
    class Model;
    class Shader;

    struct RenderCommand {
        // 推荐：传入 Shader 引用（但该函数**不会**在内部 bind/unbind shader，
        // 因为在大量对象时在外部统一 bind shader 更高效）
        static void Draw(const Mesh& mesh, Shader& shader);

        static void Draw(const Model& model, Shader& shader);
    };
} // namespace Intro

#endif // INTRO_RENDERCOMMAND_H
