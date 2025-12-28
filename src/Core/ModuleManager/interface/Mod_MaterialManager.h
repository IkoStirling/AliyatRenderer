#pragma once
#include "IAYModule.h"

namespace ayt::engine::resource
{
	struct Material;
}
namespace ayt::engine::render
{
	using ::ayt::engine::resource::Material;

	class Mod_MaterialManager : public ::ayt::engine::modules::IModule
	{
	public:
		virtual uint32_t createMaterial(const Material& mat) = 0;
		virtual uint32_t createMaterial(Material&& mat) = 0;
		virtual const Material& getMaterial(uint32_t id) const = 0;
		virtual const Material& getMaterial(const std::string& name) const = 0;
	};
}