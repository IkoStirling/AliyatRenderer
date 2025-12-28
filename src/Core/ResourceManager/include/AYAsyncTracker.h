#pragma once
#include "STAsyncTask.h"
#include <future>
#include <iostream>
#include <vector>
#include "AYLogger.h"
namespace ayt::engine::resource
{
    class AsyncTracker {
    public:
        static AsyncTracker& getInstance() {
            static AsyncTracker instance;
            return instance;
        }

        void addTask(const std::string& path,
            std::shared_future<std::shared_ptr<IResource>> future,
            std::function<void(std::shared_ptr<IResource>)> callback,
            std::chrono::milliseconds timeout = std::chrono::seconds(5))
        {
            _tasks.push_back({
                path,
                std::chrono::steady_clock::now(),
                timeout,
                future,
                callback
                });
        }

        void update(float delta_time) {
            auto now = std::chrono::steady_clock::now();
            auto it = _tasks.begin();

            while (it != _tasks.end()) {
                if (it->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    // 完成回调
                    try {
                        auto result = it->future.get();
                        if (it->callback) {
                            it->callback(result);
                        }
                    }
                    catch (const std::exception& e) {
                        AYLOG_ERR("[AsyncTracker] Future exception: {}", e.what());
                    }
                    it = _tasks.erase(it);
                }
                else if (now - it->startTime > it->timeout) {
                    AYLOG_ERR("[AsyncTracker] Track timeout, doesn't mean the mission must fail: {}", it->resourcePath);
                    it = _tasks.erase(it); // 丢弃
                }
                else {
                    ++it;
                }
            }
        }

    private:
        std::vector<AsyncTask> _tasks;
    };
}