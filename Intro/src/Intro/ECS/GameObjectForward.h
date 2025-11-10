#pragma once

#include <entt/entt.hpp>

namespace Intro {
    class Scene;
    class ECS;
    class GameObject;
    class GameObjectManager;

    using Entity = entt::entity;

    struct Transform;
    struct ActiveComponent;
    struct TagComponent;
    struct LightComponent;
    // ... 其他组件的前置声明
}