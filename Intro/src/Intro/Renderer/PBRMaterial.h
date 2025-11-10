#pragma once

#include "Material.h"
#include <memory>
#include <glm/glm.hpp>

namespace Intro {

    class ITR_API PBRMaterial : public Material {
    public:
        PBRMaterial(std::shared_ptr<Shader> shader = nullptr);

        // 重写Bind方法以实现PBR特定的绑定逻辑
        void Bind() override;

        // PBR参数设置
        void SetAlbedo(const glm::vec3& albedo);
        void SetMetallic(float metallic);
        void SetRoughness(float roughness);
        void SetAO(float ao);
        void SetEmissive(const glm::vec3& emissive);
        void SetExposure(float exposure);

        // PBR纹理设置
        void SetAlbedoMap(std::shared_ptr<Texture> texture);
        void SetNormalMap(std::shared_ptr<Texture> texture);
        void SetMetallicMap(std::shared_ptr<Texture> texture);
        void SetRoughnessMap(std::shared_ptr<Texture> texture);
        void SetAOMap(std::shared_ptr<Texture> texture);
        void SetEmissiveMap(std::shared_ptr<Texture> texture);

        // 纹理使用标志
        void SetUseAlbedoMap(bool use);
        void SetUseNormalMap(bool use);
        void SetUseMetallicMap(bool use);
        void SetUseRoughnessMap(bool use);
        void SetUseAOMap(bool use);
        void SetUseEmissiveMap(bool use);

        // Getters
        glm::vec3 GetAlbedo() const;
        float GetMetallic() const;
        float GetRoughness() const;
        float GetAO() const;
        glm::vec3 GetEmissive() const;
        float GetExposure() const;

        std::shared_ptr<Texture> GetAlbedoMap() const;
        std::shared_ptr<Texture> GetNormalMap() const;
        std::shared_ptr<Texture> GetMetallicMap() const;
        std::shared_ptr<Texture> GetRoughnessMap() const;
        std::shared_ptr<Texture> GetAOMap() const;
        std::shared_ptr<Texture> GetEmissiveMap() const;

        bool UseAlbedoMap() const;
        bool UseNormalMap() const;
        bool UseMetallicMap() const;
        bool UseRoughnessMap() const;
        bool UseAOMap() const;
        bool UseEmissiveMap() const;

        // 兼容性方法
        void SetDiffuse(std::shared_ptr<Texture> texture) override;
        std::shared_ptr<Texture> GetDiffuseTexture() const override;

    private:
        void SetPBRUniforms();
        void BindPBRTextures();

        // PBR材质参数
        glm::vec3 m_Albedo;
        float m_Metallic;
        float m_Roughness;
        float m_AO;
        glm::vec3 m_Emissive;
        float m_Exposure;

        // PBR纹理
        std::shared_ptr<Texture> m_AlbedoMap;
        std::shared_ptr<Texture> m_NormalMap;
        std::shared_ptr<Texture> m_MetallicMap;
        std::shared_ptr<Texture> m_RoughnessMap;
        std::shared_ptr<Texture> m_AOMap;
        std::shared_ptr<Texture> m_EmissiveMap;

        // 纹理使用标志
        bool m_UseAlbedoMap;
        bool m_UseNormalMap;
        bool m_UseMetallicMap;
        bool m_UseRoughnessMap;
        bool m_UseAOMap;
        bool m_UseEmissiveMap;
    };

} // namespace Intro