#pragma once
namespace ayt::engine::resource
{
    struct STAsyncTask {
        std::string resourcePath;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::milliseconds timeout;
        std::shared_future<std::shared_ptr<IAYResource>> future;
        std::function<void(std::shared_ptr<IAYResource>)> callback;
    };
}