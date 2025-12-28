#pragma once
#include <string>
#include <chrono>
#include <future>
#include <functional>

namespace ayt::engine::resource
{
    struct AsyncTask {
        std::string resourcePath;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::milliseconds timeout;
        std::shared_future<std::shared_ptr<IResource>> future;
        std::function<void(std::shared_ptr<IResource>)> callback;
    };
}