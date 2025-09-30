#include "itrpch.h" // ��������Ŀʹ��Ԥ����ͷ���������У���������Ƴ�
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
