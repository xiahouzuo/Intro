#pragma once

#include "Shader.h"
#include "Texture.h"
#include <memory>

namespace Intro {
    class ITR_API Material {
    public:
        Material(std::shared_ptr<Shader> shader) : m_Shader(std::move(shader)) {
            BindTextures();
        }

        void Bind() const {
            m_Shader->Bind();

            // ���������� uniform
            if (m_Diffuse) {
                glActiveTexture(GL_TEXTURE0);
                m_Diffuse->Bind();
                m_Shader->SetUniformInt("material_diffuse", 0);
            }
            else {
                // ���û������������������Ҫ����һ��Ĭ��ֵ
                // ����ȷ����ɫ���ܴ���û����������
            }

            if (m_Specular) {
                glActiveTexture(GL_TEXTURE1);
                m_Specular->Bind();
                m_Shader->SetUniformInt("material_specular", 1);
            }
            else {
                // ���û�и߹���������һ��Ĭ��ֵ
                m_Shader->SetUniformInt("material_specular", 1); // ��Ȼ�󶨵�����Ԫ1
                // �������Ҫһ��Ĭ�ϵİ�ɫ����
            }

            // ���ò�������
            m_Shader->SetUniformFloat("material_shininess", m_Shininess);
        }

        void SetDiffuse(std::shared_ptr<Texture> texture) { m_Diffuse = std::move(texture); }
        void SetSpecular(std::shared_ptr<Texture> texture) { m_Specular = std::move(texture); }
        void SetShininess(float shininess) { m_Shininess = shininess; }

        std::shared_ptr<Shader> GetShader() const { return m_Shader; }

    private:
        void BindTextures() {
            m_Shader->Bind();
            m_Shader->SetUniformInt("material.diffuse", 0);
            m_Shader->SetUniformInt("material.specular", 1);
        }

        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<Texture> m_Diffuse;
        std::shared_ptr<Texture> m_Specular;
        float m_Shininess = 32.0f;
    };
}