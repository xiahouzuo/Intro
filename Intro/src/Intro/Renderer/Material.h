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

            // 绑定纹理并设置 uniform
            if (m_Diffuse) {
                glActiveTexture(GL_TEXTURE0);
                m_Diffuse->Bind();
                m_Shader->SetUniformInt("material_diffuse", 0);
            }
            else {
                // 如果没有漫反射纹理，可能需要设置一个默认值
                // 或者确保着色器能处理没有纹理的情况
            }

            if (m_Specular) {
                glActiveTexture(GL_TEXTURE1);
                m_Specular->Bind();
                m_Shader->SetUniformInt("material_specular", 1);
            }
            else {
                // 如果没有高光纹理，设置一个默认值
                m_Shader->SetUniformInt("material_specular", 1); // 仍然绑定到纹理单元1
                // 你可能需要一个默认的白色纹理
            }

            // 设置材质属性
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