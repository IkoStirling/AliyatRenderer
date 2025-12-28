#pragma once
#include "IAYEvent.h"
#include "AYEventRegistry.h"
#include "STResourceLoadRequest.h"

#include "AYTexture.h"
#include "AYAudio.h"
#include "AYAudioStream.h"
#include "AYVideo.h"
#include "AYModel.h"
namespace ayt::engine::resource
{
    using namespace ::ayt::engine::event;

    template <typename T, typename... Args>
    class Event_ResourceLoadAsync : public ayt::engine::event::IEvent
    {
        SUPPORT_MEMORY_POOL(Event_ResourceLoadAsync)
            DECLARE_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, "Event_ResourceLoadAsync_", T)
    public:
        virtual void merge(const IEvent& other) override {}

        Event_ResourceLoadAsync() :
            IEvent(IEvent::Builder()
                .setLayer(EventLayer::RESOURCE)
                .setMerge(false)
                .setPriority(99)
            )
        {
        }

        Event_ResourceLoadAsync(ResourceLoadRequest<T, Args...>&& request)
            : mRequest(std::make_shared<ResourceLoadRequest<T, Args...>>(std::move(request)))
        {
        }

        explicit Event_ResourceLoadAsync(std::shared_ptr<ResourceLoadRequest<T, Args...>> request)
            : mRequest(std::move(request)) {
        }

    public:
        std::shared_ptr<ResourceLoadRequest<T, Args...>> mRequest;
    };

    REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYTexture)
    REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYAudio)
    REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYAudioStream)
    REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYVideo)
    REGISTER_TEMPLATE_EVENT_CLASS(Event_ResourceLoadAsync, AYModel)
}