#include "BaseRendering/AYBgfxCreator.h"
#include <bimg/decode.h>
#include "AYLogger.h"
namespace ayt::engine::render
{
	bx::FileReader AYBgfxCreator::_reader;
	bx::DefaultAllocator AYBgfxCreator::_defaultAllocator;

	void AYBgfxCreator::update(float delta_time) {

	}

	bgfx::ProgramHandle AYBgfxCreator::getProgram(const std::string& key)
	{
		static int count = 0;
		AYLOG_INFO("getProgram call: {}times", ++count);
		size_t delimiterPos = key.find('|');
		if (delimiterPos == std::string::npos) {
			AYLOG_ERR("Invalid shader key format: {}", key);
			return BGFX_INVALID_HANDLE;
		}

		std::string vsPath = key.substr(0, delimiterPos);
		std::string fsPath = key.substr(delimiterPos + 1);

		if (auto it = _shaderCache.find(key); it != _shaderCache.end())
		{
			return _shaderCache[key].program;
		}

		return loadProgram(vsPath.c_str(), fsPath.c_str());
	}

	const char* AYBgfxCreator::convertShaderPath(const bx::StringView& _name)
	{
		bx::FilePath filePath("include/generated/shaders/");

		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: filePath.join("dx11");  break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:        filePath.join("pssl");  break;
		case bgfx::RendererType::Metal:      filePath.join("metal"); break;
		case bgfx::RendererType::Nvn:        filePath.join("nvn");   break;
		case bgfx::RendererType::OpenGL:     filePath.join("glsl");  break;
		case bgfx::RendererType::OpenGLES:   filePath.join("essl");  break;
		case bgfx::RendererType::Vulkan:     filePath.join("spirv"); break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
		}

		char fileName[512];
		bx::strCopy(fileName, BX_COUNTOF(fileName), _name);
		bx::strCat(fileName, BX_COUNTOF(fileName), ".bin");

		filePath.join(fileName);

		return filePath.getCPtr();
	}

	void AYBgfxCreator::setUniformAndBind(const std::string& key, BgfxUniformCallback callback)
	{
		if (auto it = _shaderCache.find(key); callback && it != _shaderCache.end())
		{
			it->second.callback = callback;
			callback(it->second.program);
		}
	}

	const bgfx::Memory* AYBgfxCreator::loadMem(const bx::FilePath& _filePath)
	{
		if (bx::open(&_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(&_reader);
			const bgfx::Memory* mem = bgfx::alloc(size + 1);
			bx::read(&_reader, mem->data, size, bx::ErrorAssert{});
			bx::close(&_reader);
			mem->data[mem->size - 1] = '\0';
			return mem;
		}

		AYLOG_ERR("Failed to load {}.", _filePath.getCPtr());
		return NULL;
	}

	void* AYBgfxCreator::loadMem(const bx::FilePath& _filePath, uint32_t* _size)
	{
		if (bx::open(&_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(&_reader);
			void* data = bx::alloc(&_defaultAllocator, size);
			bx::read(&_reader, data, size, bx::ErrorAssert{});
			bx::close(&_reader);

			if (NULL != _size)
			{
				*_size = size;
			}
			return data;
		}

		AYLOG_ERR("Failed to load {}.", _filePath.getCPtr());
		return NULL;
	}

	bgfx::ShaderHandle AYBgfxCreator::loadShader(const bx::StringView& _name)
	{
		bgfx::ShaderHandle handle = bgfx::createShader(loadMem(convertShaderPath(_name)));
		bgfx::setName(handle, _name.getPtr(), _name.getLength());

		return handle;
	}

	bgfx::ProgramHandle AYBgfxCreator::loadProgram(const std::string& _vsName, const std::string& _fsName)
	{
		bgfx::ShaderHandle vsh = loadShader(_vsName.c_str());
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!_fsName.empty())
		{
			fsh = loadShader(_fsName.c_str());
		}
		bgfx::ProgramHandle pro = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		std::string key = _vsName + "|" + _fsName;

		auto vs = convertShaderPath(_vsName.c_str());
		auto fs = convertShaderPath(_fsName.c_str());
		bx::FilePath vsPath(vs);
		bx::FilePath fsPath(fs);

		_shaderCache[key] = {
			vsh,
			fsh,
			pro,
			std::filesystem::last_write_time(vsPath.getCPtr()),
			std::filesystem::last_write_time(fsPath.getCPtr()),
			nullptr
		};
		return pro;
	}

	void AYBgfxCreator::imageReleaseCb(void* _ptr, void* _userData)
	{
		BX_UNUSED(_ptr);
		bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
		bimg::imageFree(imageContainer);
	}


	bgfx::TextureHandle AYBgfxCreator::loadTexture(const bx::FilePath& _filePath, uint64_t _flags, uint8_t _skip, bgfx::TextureInfo* _info, bimg::Orientation::Enum* _orientation)
	{
		BX_UNUSED(_skip);
		bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;

		uint32_t size;
		void* data = load(_filePath, &size);
		if (NULL != data)
		{
			bimg::ImageContainer* imageContainer = bimg::imageParse(&_defaultAllocator, data, size);

			if (NULL != imageContainer)
			{
				if (NULL != _orientation)
				{
					*_orientation = imageContainer->m_orientation;
				}

				const bgfx::Memory* mem = bgfx::makeRef(
					imageContainer->m_data
					, imageContainer->m_size
					, imageReleaseCb
					, imageContainer
				);
				unload(data);

				if (NULL != _info)
				{
					bgfx::calcTextureSize(
						*_info
						, uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, uint16_t(imageContainer->m_depth)
						, imageContainer->m_cubeMap
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
					);
				}

				if (imageContainer->m_cubeMap)
				{
					handle = bgfx::createTextureCube(
						uint16_t(imageContainer->m_width)
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, _flags
						, mem
					);
				}
				else if (1 < imageContainer->m_depth)
				{
					handle = bgfx::createTexture3D(
						uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, uint16_t(imageContainer->m_depth)
						, 1 < imageContainer->m_numMips
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, _flags
						, mem
					);
				}
				else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), _flags))
				{
					handle = bgfx::createTexture2D(
						uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, _flags
						, mem
					);
				}

				if (bgfx::isValid(handle))
				{
					const bx::StringView name(_filePath);
					bgfx::setName(handle, name.getPtr(), name.getLength());
				}
			}
		}

		return handle;
	}

	bimg::ImageContainer* AYBgfxCreator::imageLoad(const bx::FilePath& _filePath, bgfx::TextureFormat::Enum _dstFormat)
	{
		uint32_t size = 0;
		void* data = loadMem(_filePath, &size);

		return bimg::imageParse(&_defaultAllocator, data, size, bimg::TextureFormat::Enum(_dstFormat));
	}

	void AYBgfxCreator::calcTangents(void* _vertices, uint16_t _numVertices, bgfx::VertexLayout _layout, const uint16_t* _indices, uint32_t _numIndices)
	{
	}

	void* AYBgfxCreator::load(const bx::FilePath& _filePath, uint32_t* _size)
	{
		if (bx::open(&_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(&_reader);
			void* data = bx::alloc(&_defaultAllocator, size);
			bx::read(&_reader, data, size, bx::ErrorAssert{});
			bx::close(&_reader);
			if (NULL != _size)
			{
				*_size = size;
			}
			return data;
		}
		else
		{
			AYLOG_ERR("Failed to open: {}.", _filePath.getCPtr());
		}

		if (NULL != _size)
		{
			*_size = 0;
		}

		return NULL;
	}

	void AYBgfxCreator::unload(void* _ptr)
	{
		bx::free(&_defaultAllocator, _ptr);
	}

}