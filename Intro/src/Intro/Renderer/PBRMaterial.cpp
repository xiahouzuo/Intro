#include "itrpch.h"
#include "PBRMaterial.h"
#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>

namespace Intro {

    PBRMaterial::PBRMaterial(std::shared_ptr<Shader> shader)
        : Material(shader)
        , m_Albedo(0.5f, 0.5f, 0.5f)
        , m_Metallic(0.0f)
        , m_Roughness(0.5f)
        , m_AO(1.0f)
        , m_Emissive(0.0f, 0.0f, 0.0f)
        , m_Exposure(1.0f)
        , m_UseAlbedoMap(false)
        , m_UseNormalMap(false)
        , m_UseMetallicMap(false)
        , m_UseRoughnessMap(false)
        , m_UseAOMap(false)
        , m_UseEmissiveMap(false)
    {
        
    }

    void PBRMaterial::Bind() {
        std::cout << "=== PBRMaterial::Bind() 被调用 ===" << std::endl;
        auto shaderPtr = GetShader();
        if (!shaderPtr) return;

        shaderPtr->Bind();
        SetPBRUniforms();
        BindPBRTextures();
    }

    void PBRMaterial::SetPBRUniforms() {
        auto shaderPtr = GetShader();
        if (!shaderPtr) return;

        // 检查关键的PBR Uniform是否存在
        GLint albedoLoc = shaderPtr->GetUniformLocation("u_AlbedoColor");
        GLint metallicLoc = shaderPtr->GetUniformLocation("u_Metallic");
        GLint roughnessLoc = shaderPtr->GetUniformLocation("u_Roughness");
        GLint useAlbedoMapLoc = shaderPtr->GetUniformLocation("u_UseAlbedoMap");

        std::cout << "PBR Uniform Locations:" << std::endl;
        std::cout << "u_AlbedoColor: " << albedoLoc << std::endl;
        std::cout << "u_Metallic: " << metallicLoc << std::endl;
        std::cout << "u_Roughness: " << roughnessLoc << std::endl;
        std::cout << "u_UseAlbedoMap: " << useAlbedoMapLoc << std::endl;

        // 如果任何关键Uniform返回-1，说明Shader有问题
        if (albedoLoc == -1 || metallicLoc == -1) {
            std::cout << "ERROR: 关键PBR Uniform未找到！" << std::endl;
        }


        // 设置PBR材质参数
        shaderPtr->SetUniformVec3("u_AlbedoColor", m_Albedo);
        shaderPtr->SetUniformFloat("u_Metallic", m_Metallic);
        shaderPtr->SetUniformFloat("u_Roughness", m_Roughness);
        shaderPtr->SetUniformFloat("u_AO", m_AO);
        shaderPtr->SetUniformVec3("u_EmissiveColor", m_Emissive);
        shaderPtr->SetUniformFloat("u_Exposure", m_Exposure);
        shaderPtr->SetUniformVec3("u_AmbientColor", GetAmbient());

        // 设置纹理使用标志
        shaderPtr->SetUniformInt("u_UseAlbedoMap", m_UseAlbedoMap ? 1 : 0);
        shaderPtr->SetUniformInt("u_UseNormalMap", m_UseNormalMap ? 1 : 0);
        shaderPtr->SetUniformInt("u_UseMetallicMap", m_UseMetallicMap ? 1 : 0);
        shaderPtr->SetUniformInt("u_UseRoughnessMap", m_UseRoughnessMap ? 1 : 0);
        shaderPtr->SetUniformInt("u_UseAOMap", m_UseAOMap ? 1 : 0);
        shaderPtr->SetUniformInt("u_UseEmissiveMap", m_UseEmissiveMap ? 1 : 0);
    }

    void PBRMaterial::BindPBRTextures() {
        // 绑定PBR纹理到对应的纹理单元
        // 纹理单元分配：
        // 0: albedo, 1: normal, 2: metallic, 3: roughness, 4: ao, 5: emissive

        GLuint whiteTex = GetOrCreateWhiteTexture();

        // Albedo map
        glActiveTexture(GL_TEXTURE0);
        if (m_UseAlbedoMap && m_AlbedoMap) {
            glBindTexture(GL_TEXTURE_2D, m_AlbedoMap->GetID());
        }
        else {
            glBindTexture(GL_TEXTURE_2D, whiteTex);
        }

        // Normal map
        glActiveTexture(GL_TEXTURE1);
        if (m_UseNormalMap && m_NormalMap) {
            glBindTexture(GL_TEXTURE_2D, m_NormalMap->GetID());
        }
        else {
            // 对于法线贴图，默认使用中性法线 (0.5, 0.5, 1.0)
            static GLuint defaultNormalTex = 0;
            if (defaultNormalTex == 0) {
                glGenTextures(1, &defaultNormalTex);
                glBindTexture(GL_TEXTURE_2D, defaultNormalTex);
                unsigned char normal[3] = { 128, 128, 255 }; // (0.5, 0.5, 1.0) in normalized
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, normal);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glBindTexture(GL_TEXTURE_2D, defaultNormalTex);
        }

        // Metallic map
        glActiveTexture(GL_TEXTURE2);
        if (m_UseMetallicMap && m_MetallicMap) {
            glBindTexture(GL_TEXTURE_2D, m_MetallicMap->GetID());
        }
        else {
            // 对于金属度，默认使用黑色纹理 (0.0)
            static GLuint defaultBlackTex = 0;
            if (defaultBlackTex == 0) {
                glGenTextures(1, &defaultBlackTex);
                glBindTexture(GL_TEXTURE_2D, defaultBlackTex);
                unsigned char black[1] = { 0 };
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, black);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glBindTexture(GL_TEXTURE_2D, defaultBlackTex);
        }

        // Roughness map
        glActiveTexture(GL_TEXTURE3);
        if (m_UseRoughnessMap && m_RoughnessMap) {
            glBindTexture(GL_TEXTURE_2D, m_RoughnessMap->GetID());
        }
        else {
            // 对于粗糙度，默认使用白色纹理 (1.0)
            glBindTexture(GL_TEXTURE_2D, whiteTex);
        }

        // AO map
        glActiveTexture(GL_TEXTURE4);
        if (m_UseAOMap && m_AOMap) {
            glBindTexture(GL_TEXTURE_2D, m_AOMap->GetID());
        }
        else {
            glBindTexture(GL_TEXTURE_2D, whiteTex);
        }

        // Emissive map
        glActiveTexture(GL_TEXTURE5);
        if (m_UseEmissiveMap && m_EmissiveMap) {
            glBindTexture(GL_TEXTURE_2D, m_EmissiveMap->GetID());
        }
        else {
            static GLuint defaultBlackTex = 0;
            if (defaultBlackTex == 0) {
                glGenTextures(1, &defaultBlackTex);
                glBindTexture(GL_TEXTURE_2D, defaultBlackTex);
                unsigned char black[3] = { 0, 0, 0 };
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, black);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glBindTexture(GL_TEXTURE_2D, defaultBlackTex);
        }

        // 设置纹理采样器uniform
        auto shaderPtr = GetShader();
        if (shaderPtr) {
            shaderPtr->SetUniformInt("material_albedo", 0);
            shaderPtr->SetUniformInt("material_normal", 1);
            shaderPtr->SetUniformInt("material_metallic", 2);
            shaderPtr->SetUniformInt("material_roughness", 3);
            shaderPtr->SetUniformInt("material_ao", 4);
            shaderPtr->SetUniformInt("material_emissive", 5);
        }

        // 重置到纹理单元0
        glActiveTexture(GL_TEXTURE0);
    }

    // PBR参数设置方法实现
    void PBRMaterial::SetAlbedo(const glm::vec3& albedo) { m_Albedo = albedo; }
    void PBRMaterial::SetMetallic(float metallic) { m_Metallic = metallic; }
    void PBRMaterial::SetRoughness(float roughness) { m_Roughness = roughness; }
    void PBRMaterial::SetAO(float ao) { m_AO = ao; }
    void PBRMaterial::SetEmissive(const glm::vec3& emissive) { m_Emissive = emissive; }
    void PBRMaterial::SetExposure(float exposure) { m_Exposure = exposure; }

    // PBR纹理设置方法实现
    void PBRMaterial::SetAlbedoMap(std::shared_ptr<Texture> texture) {
        m_AlbedoMap = std::move(texture);
        m_UseAlbedoMap = (m_AlbedoMap != nullptr);
    }
    void PBRMaterial::SetNormalMap(std::shared_ptr<Texture> texture) {
        m_NormalMap = std::move(texture);
        m_UseNormalMap = (m_NormalMap != nullptr);
    }
    void PBRMaterial::SetMetallicMap(std::shared_ptr<Texture> texture) {
        m_MetallicMap = std::move(texture);
        m_UseMetallicMap = (m_MetallicMap != nullptr);
    }
    void PBRMaterial::SetRoughnessMap(std::shared_ptr<Texture> texture) {
        m_RoughnessMap = std::move(texture);
        m_UseRoughnessMap = (m_RoughnessMap != nullptr);
    }
    void PBRMaterial::SetAOMap(std::shared_ptr<Texture> texture) {
        m_AOMap = std::move(texture);
        m_UseAOMap = (m_AOMap != nullptr);
    }
    void PBRMaterial::SetEmissiveMap(std::shared_ptr<Texture> texture) {
        m_EmissiveMap = std::move(texture);
        m_UseEmissiveMap = (m_EmissiveMap != nullptr);
    }

    // 纹理使用标志设置方法
    void PBRMaterial::SetUseAlbedoMap(bool use) { m_UseAlbedoMap = use; }
    void PBRMaterial::SetUseNormalMap(bool use) { m_UseNormalMap = use; }
    void PBRMaterial::SetUseMetallicMap(bool use) { m_UseMetallicMap = use; }
    void PBRMaterial::SetUseRoughnessMap(bool use) { m_UseRoughnessMap = use; }
    void PBRMaterial::SetUseAOMap(bool use) { m_UseAOMap = use; }
    void PBRMaterial::SetUseEmissiveMap(bool use) { m_UseEmissiveMap = use; }

    // Getter方法实现
    glm::vec3 PBRMaterial::GetAlbedo() const { return m_Albedo; }
    float PBRMaterial::GetMetallic() const { return m_Metallic; }
    float PBRMaterial::GetRoughness() const { return m_Roughness; }
    float PBRMaterial::GetAO() const { return m_AO; }
    glm::vec3 PBRMaterial::GetEmissive() const { return m_Emissive; }
    float PBRMaterial::GetExposure() const { return m_Exposure; }

    std::shared_ptr<Texture> PBRMaterial::GetAlbedoMap() const { return m_AlbedoMap; }
    std::shared_ptr<Texture> PBRMaterial::GetNormalMap() const { return m_NormalMap; }
    std::shared_ptr<Texture> PBRMaterial::GetMetallicMap() const { return m_MetallicMap; }
    std::shared_ptr<Texture> PBRMaterial::GetRoughnessMap() const { return m_RoughnessMap; }
    std::shared_ptr<Texture> PBRMaterial::GetAOMap() const { return m_AOMap; }
    std::shared_ptr<Texture> PBRMaterial::GetEmissiveMap() const { return m_EmissiveMap; }

    bool PBRMaterial::UseAlbedoMap() const { return m_UseAlbedoMap; }
    bool PBRMaterial::UseNormalMap() const { return m_UseNormalMap; }
    bool PBRMaterial::UseMetallicMap() const { return m_UseMetallicMap; }
    bool PBRMaterial::UseRoughnessMap() const { return m_UseRoughnessMap; }
    bool PBRMaterial::UseAOMap() const { return m_UseAOMap; }
    bool PBRMaterial::UseEmissiveMap() const { return m_UseEmissiveMap; }

    // 兼容性方法
    void PBRMaterial::SetDiffuse(std::shared_ptr<Texture> texture) {
        SetAlbedoMap(std::move(texture));
    }

    std::shared_ptr<Texture> PBRMaterial::GetDiffuseTexture() const {
        return GetAlbedoMap();
    }

} // namespace Intro