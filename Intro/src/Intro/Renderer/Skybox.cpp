#include "itrpch.h"
#include "Skybox.h"
#include <glad/glad.h>
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Intro {

    // 天空盒顶点数据
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    Skybox::Skybox(const std::vector<std::string>& facePaths) {
        // 检查facePaths是否有效
        if (facePaths.size() != 6) {
            ITR_ERROR("Skybox requires exactly 6 face paths, got {}", facePaths.size());
            throw std::runtime_error("Invalid number of skybox face paths");
        }

        // 创建天空盒着色器
        try {
            // 确保着色器文件存在
            std::string vertexPath = "E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/skyboxShader.vert";
            std::string fragmentPath = "E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/skyboxShader.frag";

            if (!std::filesystem::exists(vertexPath) || !std::filesystem::exists(fragmentPath)) {
                ITR_ERROR("Skybox shader files not found: {} or {}", vertexPath, fragmentPath);
                throw std::runtime_error("Skybox shader files not found");
            }

            m_Shader = std::make_shared<Shader>(vertexPath.c_str(), fragmentPath.c_str());
            ITR_INFO("Skybox shader created successfully");
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed to create skybox shader: {}", e.what());
            throw;
        }

        // 加载立方体贴图
        LoadCubemap(facePaths);

        // 设置天空盒几何体
        SetupSkybox();

        ITR_INFO("Skybox initialized successfully with texture: {}", m_CubemapTexture);
    }

    Skybox::~Skybox() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteTextures(1, &m_CubemapTexture);
    }

    void Skybox::LoadCubemap(const std::vector<std::string>& facePaths) {
        glGenTextures(1, &m_CubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false); // 天空盒通常不需要翻转

        for (unsigned int i = 0; i < facePaths.size(); i++) {
            // 检查文件是否存在
            if (!std::filesystem::exists(facePaths[i])) {
                ITR_ERROR("Skybox texture not found: {}", facePaths[i]);
                continue;
            }

            unsigned char* data = stbi_load(facePaths[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                GLenum format = GL_RGB;
                if (nrChannels == 4) format = GL_RGBA;
                else if (nrChannels == 1) format = GL_RED;
                else if (nrChannels == 3) format = GL_RGB;

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                    width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
                ITR_INFO("Loaded skybox face {}: {}x{}", i, width, height);
            }
            else {
                ITR_ERROR("Failed to load skybox texture: {}", facePaths[i]);
                stbi_image_free(data);
            }
        }

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        ITR_INFO("Skybox cubemap created: {}", m_CubemapTexture);
    }

    void Skybox::SetupSkybox() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection) {
        if (!m_Shader) {
            ITR_ERROR("Skybox shader is null");
            return;
        }

        if (m_CubemapTexture == 0) {
            ITR_ERROR("Skybox texture is not initialized");
            return;
        }

        // 保存当前状态
        GLint oldDepthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
        GLboolean oldDepthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);

        // 设置天空盒渲染状态
        glDepthFunc(GL_LEQUAL);  // 确保深度测试通过
        glDepthMask(GL_FALSE);   // 禁用深度写入

        m_Shader->Bind();

        // 检查着色器程序
        GLuint program = m_Shader->GetShaderID();
        if (program == 0) {
            ITR_ERROR("Skybox shader program is invalid");
            return;
        }

        // 设置uniform
        m_Shader->SetUniformMat4("view", view);
        m_Shader->SetUniformMat4("projection", projection);

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);
        m_Shader->SetUniformInt("skybox", 0);

        // 绘制天空盒
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // 恢复状态
        glDepthFunc(oldDepthFunc);
        glDepthMask(oldDepthMask);

        // 检查OpenGL错误
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            ITR_ERROR("OpenGL error in Skybox::Draw: 0x%x", error);
        }
    }

}