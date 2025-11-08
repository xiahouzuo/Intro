#pragma once
#include "Intro/Core.h"
#include "Shader.h"
#include "Texture.h"
#include <vector>
#include <string>
#include <memory>

namespace Intro {

    class ITR_API Skybox {
    public:
        Skybox(const std::vector<std::string>& facePaths);
        ~Skybox();

        void Draw(const glm::mat4& view, const glm::mat4& projection);

        std::shared_ptr<Shader> GetShader() const { return m_Shader; }
        void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }

    private:
        void LoadCubemap(const std::vector<std::string>& facePaths);
        void SetupSkybox();

        unsigned int m_CubemapTexture;
        unsigned int m_VAO, m_VBO;
        std::shared_ptr<Shader> m_Shader;
    };

}