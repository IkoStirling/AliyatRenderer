#pragma once
#include <unordered_set>

#define DECLARE_RESOURCE_CLASS(CLASS_NAME, RESOURCE_TYPE_NAME) \
public: \
    virtual std::string getType()const override { \
		static std::string str = RESOURCE_TYPE_NAME; \
		return str; \
	}\
	virtual std::type_index getTypeIndex()const override { \
		return typeid(CLASS_NAME); \
	}\
	static std::string staticGetType(){ \
		static std::string str = RESOURCE_TYPE_NAME; \
		return str; \
	}\
	static std::type_index staticGetTypeIndex(){ \
		return typeid(CLASS_NAME); \
	}\

#define REGISTER_RESOURCE_CLASS(CLASS_NAME, ...) \
struct CLASS_NAME##_Register{ \
	CLASS_NAME##_Register(){ \
		::AYResourceRegistry::getInstance().registerType(CLASS_NAME::staticGetType(), [](){ \
            return std::shared_ptr<CLASS_NAME>(new CLASS_NAME()); \
        }); \
	}\
}; \
static CLASS_NAME##_Register CLASS_NAME##_register; \

class IAYResource
{
public:
	using Tag = std::string;
	using TagSet = std::unordered_set<Tag>;
public:

    //����������Ҫ���ø÷���
    virtual bool load(const std::string& filepath)
    {
        _resourcePath = filepath;
        return true;
    }

	virtual bool unload() = 0;
	virtual bool reload(const std::string& filepath) = 0;
	virtual size_t sizeInBytes() = 0;

public:
    void addTag(const Tag& tag) {
        _tags.insert(tag);
    }

    void removeTag(const Tag& tag) {
        _tags.erase(tag);
    }

    bool hasTag(const Tag& tag) const {
        return _tags.count(tag) > 0;
    }

    const TagSet& getTags() const {
        return _tags;
    }

    const std::string& getPath() const
    {
        return _resourcePath;
    }

protected:
    TagSet _tags;
    std::string _resourcePath;

public:
    virtual ~IAYResource() = default;
    virtual std::string getType()const = 0;
    virtual std::type_index getTypeIndex()const = 0;
};