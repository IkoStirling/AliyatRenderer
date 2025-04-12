#pragma once
#include "Core/EventSystem/IAYEvent.h"
#include "Core/EventSystem/AYEventRegistry.h"
#include "Core/ResourceManager/STResourceLoadRequest.h"
#include "Core/SystemResourceType/AYTexture.h"

template <typename T>
class Event_ResourceLoadAsync : public IAYEvent
{
    SUPPORT_MEMORY_POOL(Event_ResourceLoadAsync)
    DECLARE_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, "Event_ResourceLoadAsync_", T)
public:
    virtual void merge(const IAYEvent& other) override {}

    Event_ResourceLoadAsync() :
        IAYEvent(IAYEvent::Builder().setLayer(AYEventLayer::RESOURCE))
    {
    }

public:
    STResourceLoadRequest<T> mRequest;
};

REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYTexture)