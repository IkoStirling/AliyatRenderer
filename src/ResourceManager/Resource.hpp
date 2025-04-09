#include <string>
#include <memory>
#include <unordered_map>

class IAYReource
{
public:
	virtual bool load(const std::string& filepath) = 0;
	virtual bool unload() = 0;
};

class AYTexture : public IAYReource
{
public:
	virtual bool load(const std::string& filepath)override
	{

	}
	virtual bool unload()override
	{

	}
};

class AYResourceManager {
private:
    std::unordered_map<std::string, std::shared_ptr<IAYReource>> _resources;

public:
    template <typename T>
    std::shared_ptr<T> loadResource(const std::string& filepath) {
        auto it = _resources.find(filepath);
        if (it != _resources.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }

        std::shared_ptr<T> resource = std::shared_ptr<T>(new T());
        if (resource->load(filepath)) {
            _resources[filepath] = resource;
            return resource;
        }
        return nullptr;
    }

    void unloadResource(const std::string& filepath) {
        auto it = _resources.find(filepath);
        if (it != _resources.end()) {
            it->second->unload();
            _resources.erase(it);
        }
    }
};