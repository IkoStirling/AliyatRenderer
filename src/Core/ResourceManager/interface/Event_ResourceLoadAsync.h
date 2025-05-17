#pragma once
#include "IAYEvent.h"
#include "AYEventRegistry.h"
#include "STResourceLoadRequest.h"
#include "AYTexture.h"

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