#pragma once
#include "IAYComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>>

namespace ayt::engine::game
{
    class TransformComponent : public IComponent
    {
    public:
        virtual void beginPlay() override {}
        virtual void update(float delta_time) override {}
        virtual void endPlay() override {}

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        glm::mat4 getWorldMatrix() const {
            glm::mat4 model(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
            // ...ÆäËû±ä»»
            model = glm::scale(model, scale);
            return model;
        }
    };
}