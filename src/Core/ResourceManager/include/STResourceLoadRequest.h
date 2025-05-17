#pragma once
#include <future>

template <typename T>
struct STResourceLoadRequest
{
    std::string mPath;
    std::shared_ptr<std::promise<std::shared_ptr<T>>> mPromise;
    std::function<void(std::shared_ptr<T>)> mCallback;
};