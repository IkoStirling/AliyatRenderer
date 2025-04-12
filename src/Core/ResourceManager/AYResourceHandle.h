#pragma once
#include <memory>
#include <string>


//��ֹ��Դ�����ڲ��첽����ǰ��ʹ����ָ�뱻���������ڲ�Ҳʹ��shared_ptr�����ӳ���������
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