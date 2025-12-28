#pragma once
#include <memory>
#include <string>

namespace ayt::engine::resource
{
    //防止资源在类内部异步操作前因使用裸指针被析构，在内部也使用shared_ptr进行延长生命周期
    template<typename T>
    class ResourceHandle : public std::enable_shared_from_this<ResourceHandle<T>>
    {
    public:
        ResourceHandle(const std::string& path, bool isLazyLoad = true);

        std::shared_ptr<T> get();

        T* operator->();
    private:
        std::string _path;
        std::shared_ptr<T> _resource;
    };












    //ResourceHandle.inl
#include "AYResourceManager.h"
    template<typename T>
    inline ResourceHandle<T>::ResourceHandle(const std::string& path, bool isLazyLoad)
        :
        _path(path)
    {
        if (!isLazyLoad)
        {
            _resource = ResourceManager::getInstance().load<T>(_path);
        }
    }

    template<typename T>
    inline std::shared_ptr<T> ResourceHandle<T>::get()
    {
        if (!_resource)
        {
            _resource = ResourceManager::getInstance().load<T>(_path);
        }
        return _resource;
    }

    template<typename T>
    inline T* ResourceHandle<T>::operator->()
    {
        return get().get();
    }
}