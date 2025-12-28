#pragma once
#include "IAYEvent.h"
#include "STTransform.h"
#include <iostream>
namespace ayt::engine::event
{
    class Event_UIMouseEnter : public IEvent {
        DECLARE_EVENT_CLASS(Event_UIMouseEnter, "Event_UIMouseEnter")
        SUPPORT_MEMORY_POOL(Event_UIMouseEnter)
    public:
        Event_UIMouseEnter()
            : IEvent(Builder()
                .setPriority(20) // UI事件优先级较高
                .setMerge(false)
                .setLayer(EventLayer::UI)) {
        }

        uint32_t elementId;
        glm::vec2 position;
    };

    REGISTER_EVENT_CLASS(Event_UIMouseEnter);

    class Event_UIMouseLeave : public ayt::engine::event::IEvent {
        DECLARE_EVENT_CLASS(Event_UIMouseLeave, "Event_UIMouseLeave")
        SUPPORT_MEMORY_POOL(Event_UIMouseLeave)
    public:
        Event_UIMouseLeave()
            : IEvent(Builder()
                .setPriority(20)
                .setMerge(false)
                .setLayer(EventLayer::UI)) {
        }

        uint32_t elementId;
    };

    REGISTER_EVENT_CLASS(Event_UIMouseLeave);

    class Event_UIMouseClick : public ayt::engine::event::IEvent {
        DECLARE_EVENT_CLASS(Event_UIMouseClick, "Event_UIMouseClick")
        SUPPORT_MEMORY_POOL(Event_UIMouseClick)
    public:
        Event_UIMouseClick()
            : IEvent(Builder()
                .setPriority(20)
                .setMerge(false)
                .setLayer(EventLayer::UI)) {
        }

        uint32_t elementId;
        int button;
        glm::vec2 position;
        bool doubleClick;
    };

    REGISTER_EVENT_CLASS(Event_UIMouseClick);
}