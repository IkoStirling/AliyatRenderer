#pragma once
#include "IAYEvent.h"
#include "STTransform.h"
#include <iostream>

class Event_InputBase : public IAYEvent {
    DECLARE_EVENT_CLASS(Event_InputBase, "Event_InputBase")
    SUPPORT_MEMORY_POOL(Event_InputBase)
public:
    Event_InputBase(IAYEvent::Builder bulider)
        : IAYEvent(bulider) {
    }

    bool isConsumed() {
        return (isConsumable && curConsumeCount >= maxConsumeCount) ? true: false;
    }

    void consume() { curConsumeCount++; }

protected:
    bool isConsumable = false;
    int maxConsumeCount = 1;
    int curConsumeCount = 0;
};

class Event_MouseButtonDown : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_MouseButtonDown, "Event_MouseButtonDown")
    SUPPORT_MEMORY_POOL(Event_MouseButtonDown)
public:
    Event_MouseButtonDown()
        : Event_InputBase(Builder()
            .setPriority(20) 
            .setMerge(true)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_MouseButtonDown construct\n";
        isConsumable = true;
    }

    virtual void merge(const IAYEvent& other) override {
        if (other.getTypeIndex() == this->getTypeIndex()) {
            const auto& otherEvent = static_cast<const Event_MouseButtonDown&>(other);
            this->mousePos = otherEvent.mousePos;
            this->button = otherEvent.button;
        }
    }

    int button;
    glm::vec2 mousePos;
    int modifiers;
};

REGISTER_EVENT_CLASS(Event_MouseButtonDown);

class Event_MouseButtonUp : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_MouseButtonUp, "Event_MouseButtonUp")
    SUPPORT_MEMORY_POOL(Event_MouseButtonUp)
public:
    Event_MouseButtonUp()
        : Event_InputBase(Builder()
            .setPriority(20)
            .setMerge(true)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_MouseButtonUp construct\n";
        isConsumable = true;
    }
    virtual void merge(const IAYEvent& other) override {
        if (other.getTypeIndex() == this->getTypeIndex()) {
            const auto& otherEvent = static_cast<const Event_MouseButtonUp&>(other);
            this->mousePos = otherEvent.mousePos;
            this->button = otherEvent.button;
        }
    }
    int button;
    glm::vec2 mousePos;
    int modifiers;
};

REGISTER_EVENT_CLASS(Event_MouseButtonUp);

class Event_MouseMove : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_MouseMove, "Event_MouseMove")
    SUPPORT_MEMORY_POOL(Event_MouseMove)
public:
    Event_MouseMove()
        : Event_InputBase(Builder()
            .setPriority(20)
            .setMerge(true)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_MouseMove construct\n";
    }
    virtual void merge(const IAYEvent& other) override {
        if (other.getTypeIndex() == this->getTypeIndex()) {
            this->mousePos = static_cast<const Event_MouseMove&>(other).mousePos;
        }
    }
    glm::vec2 mousePos;
};

REGISTER_EVENT_CLASS(Event_MouseMove);

class Event_KeyDown : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_KeyDown, "Event_KeyDown")
    SUPPORT_MEMORY_POOL(Event_KeyDown)
public:
    Event_KeyDown()
        : Event_InputBase(Builder()
            .setPriority(20) 
            .setMerge(false)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_KeyDown construct\n";
    }

    int key;
    int modifiers;
};

REGISTER_EVENT_CLASS(Event_KeyDown);

class Event_KeyUp : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_KeyUp, "Event_KeyUp")
    SUPPORT_MEMORY_POOL(Event_KeyUp)
public:
    Event_KeyUp()
        : Event_InputBase(Builder()
            .setPriority(20)
            .setMerge(false)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_KeyUp construct\n";
    }

    int key;
    int modifiers;
};

REGISTER_EVENT_CLASS(Event_KeyUp);

class Event_Scroll : public Event_InputBase {
    DECLARE_EVENT_CLASS(Event_Scroll, "Event_Scroll")
    SUPPORT_MEMORY_POOL(Event_Scroll)
public:
    Event_Scroll()
        : Event_InputBase(Builder()
            .setPriority(20) 
            .setMerge(true)
            .setLayer(AYEventLayer::INPUT)){
        //std::cout << "Event_Scroll construct\n";
    }

    virtual void merge(const IAYEvent& other) override {
        if (other.getTypeIndex() == this->getTypeIndex()) {
            this->scrollDelta = static_cast<const Event_Scroll&>(other).scrollDelta;
        }
    }

    glm::vec2 scrollDelta;
    glm::vec2 mousePos;
};

REGISTER_EVENT_CLASS(Event_Scroll);