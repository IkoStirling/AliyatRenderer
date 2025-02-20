#pragma once
#include "core/core.h"
#include <set>


class Pool
{
public:

	Pool();
	virtual ~Pool();


	virtual GLuint getUnusedElement();
	virtual void initializePool();
	virtual void releaseElement(GLuint e);
private:
	GLuint maxElement;
	std::set<GLuint> availableElements;
	std::set<GLuint> usedElements;

};