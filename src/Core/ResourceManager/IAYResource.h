#pragma once

class IAYResource
{
public:
	virtual ~IAYResource() = default;
	virtual bool load(const std::string& filepath) = 0;
	virtual bool unload() = 0;
};