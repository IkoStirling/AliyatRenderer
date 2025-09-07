﻿#pragma once
#include "BaseRendering/AYUIRenderer.h"

class AYUIEventHandler
{
public:
    AYUIEventHandler(AYUIRenderer* uiRenderer) :
        _uiRenderer(uiRenderer)
    {

    }

    void handleMouseMove(Event_MouseMove& event) {
        //std::cout << "Mouse move!\n";
        for (size_t i = 0; i < _uiRenderer->_uiElements.size(); ++i) {
            AYUIRenderer::UIElement& elem = _uiRenderer->_uiElements[i];
            auto& renderData = elem.renderData;
            auto& interaction = elem.interaction;
            auto& callback = elem.callback;
            if (!renderData.visible || !interaction.interactive) continue;

            glm::vec3 pos = renderData.position;
            glm::vec3 size = renderData.size;

            bool inside = _uiRenderer->isInsideElement(elem.id, event.mousePos);

            if (inside) {
                if (!interaction.isHovered) {
                    interaction.isHovered = true;
                    if (callback.onHovered)
                        callback.onHovered();
                }
                interaction.isPressed = false; // 鼠标移动时，如果未按下，则确保 pressed 重置
            }
            else {
                if (interaction.isHovered)
                {
                    interaction.isHovered = false;
                    if (callback.onUnhovered)
                        callback.onUnhovered();
                }
            }
        }
    }

    void handleMouseButton(Event_MouseButtonDown& event) {
        //std::cout << "MouseButtonDown!\n";
        if (event.button != 0) return; // 假设只处理左键，button==0

        for (size_t i = 0; i < _uiRenderer->_uiElements.size(); ++i) {
            AYUIRenderer::UIElement& elem = _uiRenderer->_uiElements[i];
            auto& renderData = elem.renderData;
            auto& interaction = elem.interaction;
            auto& callback = elem.callback;
            if (!renderData.visible || !interaction.interactive || !interaction.isHovered) continue;

            glm::vec3 pos = renderData.position;
            glm::vec3 size = renderData.size;

            bool inside = _uiRenderer->isInsideElement(elem.id, event.mousePos);

            if (inside) {
                interaction.isPressed = true;
                if (callback.onPressed)
                    callback.onPressed();
                event.consume();
                break;
            }
        }
    }

    void handleMouseButton(Event_MouseButtonUp& event) {
        //std::cout << "MouseButtonUp!\n";
        if (event.button != 0) return; // 只处理左键

        for (size_t i = 0; i < _uiRenderer->_uiElements.size(); ++i) {
            AYUIRenderer::UIElement& elem = _uiRenderer->_uiElements[i];
            auto& renderData = elem.renderData;
            auto& interaction = elem.interaction;
            auto& callback = elem.callback;
            if (!renderData.visible || !interaction.interactive || !interaction.isPressed) continue;

            glm::vec3 pos = renderData.position;
            glm::vec3 size = renderData.size;

            bool inside = _uiRenderer->isInsideElement(elem.id, event.mousePos);

            if (inside) {
                if (interaction.isHovered)
                {
                    if (callback.onClicked)
                        callback.onClicked();
                }
                else
                {
                    if (callback.onReleased)
                        callback.onReleased();
                }
                interaction.isPressed = false;
                event.consume(); // 阻止事件继续传递
                break;
            }
        }
    }

	void subscribeToInputEvents()
	{
        auto eventSystem = GET_CAST_MODULE(AYEventSystem, "EventSystem");

        // 订阅事件的方式
        _eventTokens.push_back(
            std::unique_ptr<AYEventToken>(
                eventSystem->subscribe(Event_MouseMove::staticGetType(),
                    [this](IAYEvent& event) {
                        handleMouseMove(static_cast<Event_MouseMove&>(event));
                    })
            ));

        _eventTokens.push_back(
            std::unique_ptr<AYEventToken>(
                eventSystem->subscribe(Event_MouseButtonDown::staticGetType(),
                    [this](IAYEvent& event) {
                        handleMouseButton(static_cast<Event_MouseButtonDown&>(event));
                    })
            ));

        _eventTokens.push_back(
            std::unique_ptr<AYEventToken>(
                eventSystem->subscribe(Event_MouseButtonUp::staticGetType(),
                    [this](IAYEvent& event) {
                        handleMouseButton(static_cast<Event_MouseButtonUp&>(event));
                    })
            ));
	}

private:
    std::vector<std::unique_ptr<AYEventToken>> _eventTokens;
    AYUIRenderer* _uiRenderer = nullptr;
};