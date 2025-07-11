#pragma once
#include <future>

template <typename T, typename... Args>
struct STResourceLoadRequest
{
    std::string mPath;
    std::shared_ptr<std::promise<std::shared_ptr<T>>> mPromise;
    std::function<void(std::shared_ptr<T>)> mCallback;
    std::tuple<Args...> mArgs;

    STResourceLoadRequest(std::string path,
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

    // �ƶ����캯��
    STResourceLoadRequest(STResourceLoadRequest&& other) noexcept
        : mPath(std::move(other.mPath)),
        mPromise(std::move(other.mPromise)),
        mCallback(std::move(other.mCallback)),
        mArgs(std::move(other.mArgs)) {
    }

    // �ƶ���ֵ�����
    STResourceLoadRequest& operator=(STResourceLoadRequest&& other) noexcept {
        if (this != &other) {
            mPath = std::move(other.mPath);
            mPromise = std::move(other.mPromise);
            mCallback = std::move(other.mCallback);
            mArgs = std::move(other.mArgs);
        }
        return *this;
    }

    // ���ÿ�������Ϊ����promise��
    STResourceLoadRequest(const STResourceLoadRequest&) = delete;
    STResourceLoadRequest& operator=(const STResourceLoadRequest&) = delete;
};