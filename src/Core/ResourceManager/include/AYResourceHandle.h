#pragma once
#include <memory>
#include <string>


//防止资源在类内部异步操作前因使用裸指针被析构，在内部也使用shared_ptr进行延长生命周期
template<typename T>
class AYResourceHandle : public std::enable_shared_from_this<AYResourceHandle<T>>
{
public:
    AYResourceHandle(const std::string& path, bool isLazyLoad = true);

    std::shared_ptr<T> get();

    T* operator->();
private:
    std::string _path;
    std::shared_ptr<T> _resource;
};












//AYResourceHandle.inl
#include "AYResourceManager.h"
template<typename T>
inline AYResourceHandle<T>::AYResourceHandle(const std::string& path, bool isLazyLoad)
    :
    _path(path)
{
    if (!isLazyLoad)
    {
        _resource = AYResourceManager::getInstance().load<T>(_path);
    }
}

template<typename T>
inline std::shared_ptr<T> AYResourceHandle<T>::get()
{
    if (!_resource)
    {
        _resource = AYResourceManager::getInstance().load<T>(_path);
    }
    return _resource;
}

template<typename T>
inline T* AYResourceHandle<T>::operator->()
{
    return get().get();
}