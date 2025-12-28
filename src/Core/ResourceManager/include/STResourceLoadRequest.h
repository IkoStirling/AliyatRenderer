#pragma once
#include <future>
namespace ayt::engine::resource
{
    template <typename T, typename... Args>
    struct ResourceLoadRequest
    {
        std::string mPath;
        std::shared_ptr<std::promise<std::shared_ptr<T>>> mPromise;
        std::function<void(std::shared_ptr<T>)> mCallback;
        std::tuple<Args...> mArgs;

        ResourceLoadRequest(std::string path,
            std::shared_ptr<std::promise<std::shared_ptr<T>>> promise,
            std::function<void(std::shared_ptr<T>)> callback,
            std::tuple<Args...> args)
            : mPath(std::move(path))
            , mPromise(std::move(promise))
            , mCallback(std::move(callback))
            , mArgs(std::move(args))
        {
        }

        template<typename Func>
        auto applyArgs(Func&& func) {
            return std::apply(std::forward<Func>(func), mArgs);
        }

        // 移动构造函数
        ResourceLoadRequest(ResourceLoadRequest&& other) noexcept
            : mPath(std::move(other.mPath)),
            mPromise(std::move(other.mPromise)),
            mCallback(std::move(other.mCallback)),
            mArgs(std::move(other.mArgs)) {
        }

        // 移动赋值运算符
        ResourceLoadRequest& operator=(ResourceLoadRequest&& other) noexcept {
            if (this != &other) {
                mPath = std::move(other.mPath);
                mPromise = std::move(other.mPromise);
                mCallback = std::move(other.mCallback);
                mArgs = std::move(other.mArgs);
            }
            return *this;
        }

        // 禁用拷贝（因为包含promise）
        ResourceLoadRequest(const ResourceLoadRequest&) = delete;
        ResourceLoadRequest& operator=(const ResourceLoadRequest&) = delete;
    };
}