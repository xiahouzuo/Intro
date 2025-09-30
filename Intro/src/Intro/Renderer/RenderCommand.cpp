#include "itrpch.h" // 如果你的项目使用预编译头，保留此行；否则可以移除
#include "RenderCommand.h"

#include "Mesh.h"
#include "Intro/Renderer/Model.h"
#include "Shader.h"
namespace Intro {

    void RenderCommand::Draw(const Mesh& mesh, Shader& shader) {
        mesh.Draw(shader);
    }

    void RenderCommand::Draw(const Model& model, Shader& shader)
    {
        model.Draw(shader);
    }

} // namespace Intro
