#pragma once

#include "Shader.h"
#include "Texture.h"
#include <memory>
#include <glm/glm.hpp>

namespace Intro {

    class ITR_API Material {
    public:
        Material(std::shared_ptr<Shader> shader)
            : m_Shader(std::move(shader)), m_Diffuse(nullptr), m_Specular(nullptr),
            m_Shininess(32.0f), m_Ambient(glm::vec3(0.2f))
        {
            // ��� shader �ڹ���ʱ�Ѿ����ã���һ��Ĭ�� sampler ָ������ȫ��
            if (m_Shader) {
                m_Shader->Bind();
                m_Shader->SetUniformInt("material_diffuse", 0);
                m_Shader->SetUniformInt("material_specular", 1);
            }
        }

        // Bind: ÿ�λ���ʱ���ã�ȷ�� shader �Ѱ󶨲� sampler/texture ��ȷ����
        void Bind() {
            auto shaderPtr = GetShader();
            if (!shaderPtr) return;
            shaderPtr->Bind();
            GLuint program = shaderPtr->GetShaderID();

            // ֧�ֶ��ֳ��� sampler ���ƣ������ԣ�
            const char* diffuseNames[] = { "material_diffuse", "texture_diffuse1", "texture_diffuse", nullptr };
            const char* specNames[] = { "material_specular", "texture_specular1", "texture_specular", nullptr };

            for (int i = 0; diffuseNames[i] != nullptr; ++i) {
                GLint loc = glGetUniformLocation(program, diffuseNames[i]);
                if (loc >= 0) {
                    glUniform1i(loc, 0); // diffuse -> unit 0
                }
            }
            for (int i = 0; specNames[i] != nullptr; ++i) {
                GLint loc = glGetUniformLocation(program, specNames[i]);
                if (loc >= 0) {
                    glUniform1i(loc, 1); // specular -> unit 1
                }
            }

            // �������� uniform����� shader ʹ�ã�
            GLint locSh = glGetUniformLocation(program, "material_shininess");
            if (locSh >= 0) glUniform1f(locSh, m_Shininess);

            GLint locAmb = glGetUniformLocation(program, "u_AmbientColor");
            if (locAmb >= 0) glUniform3f(locAmb, m_Ambient.r, m_Ambient.g, m_Ambient.b);

            // ����������������� 1x1 ��ɫ�󱸣�
            GLuint white = GetOrCreateWhiteTexture();

            GLuint diffuseID = white;
            GLuint specID = white;

            if (m_Diffuse) {
                // Texture ������ṩ GetID() �� GetRendererID()
                diffuseID = m_Diffuse->GetID();
            }
            if (m_Specular) {
                specID = m_Specular->GetID();
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specID);
            glActiveTexture(GL_TEXTURE0);
        }

        // setters / getters
        void SetDiffuse(std::shared_ptr<Texture> texture) { m_Diffuse = std::move(texture); }
        void SetSpecular(std::shared_ptr<Texture> texture) { m_Specular = std::move(texture); }
        void SetShininess(float shininess) { m_Shininess = shininess; }
        void SetAmbient(const glm::vec3& ambient) { m_Ambient = ambient; }

        std::shared_ptr<Texture> GetDiffuseTexture() const { return m_Diffuse; }
        std::shared_ptr<Texture> GetSpecularTexture() const { return m_Specular; }

        // ���ݾ���
        std::shared_ptr<Texture> GetDiffuseTextureID() const { return m_Diffuse; }
        std::shared_ptr<Texture> GetSpecularTextureID() const { return m_Specular; }

        float GetShininess() const { return m_Shininess; }
        std::shared_ptr<Shader> GetShader() const { return m_Shader; }
        glm::vec3 GetAmbient() const { return m_Ambient; }

        // ����������һ�� 1x1 ��ɫ������������
        static GLuint GetOrCreateWhiteTexture() {
            static GLuint whiteTex = 0;
            if (whiteTex == 0) {
                glGenTextures(1, &whiteTex);
                glBindTexture(GL_TEXTURE_2D, whiteTex);
                unsigned char white[4] = { 255,255,255,255 };
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            return whiteTex;
        }

    private:
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<Texture> m_Diffuse;
        std::shared_ptr<Texture> m_Specular;
        float m_Shininess;
        glm::vec3 m_Ambient;
    };

} // namespace Intro
