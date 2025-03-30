#pragma once
#include "core/core.h"
#include <set>


class Pool
{
public:

	Pool(GLuint _maxElement = 1000);
	virtual ~Pool();


	virtual GLuint getUnusedElement();
	virtual void initializePool();
	virtual void releaseElement(GLuint e);
private:
	GLuint maxElement;
	std::set<GLuint> availableElements;
	std::set<GLuint> usedElements;

};