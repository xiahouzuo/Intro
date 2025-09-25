#pragma once
#ifndef INTRO_RENDERCOMMAND_H
#define INTRO_RENDERCOMMAND_H

#include <cstdint>

namespace Intro {
    class Mesh;
    class Model;
    class Shader;

    struct RenderCommand {
        // �Ƽ������� Shader ���ã����ú���**����**���ڲ� bind/unbind shader��
        // ��Ϊ�ڴ�������ʱ���ⲿͳһ bind shader ����Ч��
        static void Draw(const Mesh& mesh, Shader& shader);

        static void Draw(const Model& model, Shader& shader);
    };
} // namespace Intro

#endif // INTRO_RENDERCOMMAND_H
