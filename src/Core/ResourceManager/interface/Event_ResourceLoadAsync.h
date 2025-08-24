#pragma once
#include "IAYEvent.h"
#include "AYEventRegistry.h"
#include "STResourceLoadRequest.h"

#include "AYTexture.h"
#include "AYAudio.h"
#include "AYAudioStream.h"
#include "AYVideo.h"
#include "AYModel.h"

template <typename T, typename... Args>
class Event_ResourceLoadAsync : public IAYEvent
{
    SUPPORT_MEMORY_POOL(Event_ResourceLoadAsync)
    DECLARE_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, "Event_ResourceLoadAsync_", T)
public:
    virtual void merge(const IAYEvent& other) override {}

    Event_ResourceLoadAsync() :
        IAYEvent(IAYEvent::Builder()
            .setLayer(AYEventLayer::RESOURCE)
            .setMerge(false)
            .setPriority(99)
        )
    {
    }

    Event_ResourceLoadAsync(STResourceLoadRequest<T, Args...>&& request)
        : mRequest(std::make_shared<STResourceLoadRequest<T, Args...>>(std::move(request)))
    {
    }

    explicit Event_ResourceLoadAsync(std::shared_ptr<STResourceLoadRequest<T, Args...>> request)
        : mRequest(std::move(request)) {
    }

public:
    std::shared_ptr<STResourceLoadRequest<T, Args...>> mRequest;
};

REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYTexture)
REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYAudio)
REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYAudioStream)
REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYVideo)
REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYModel)