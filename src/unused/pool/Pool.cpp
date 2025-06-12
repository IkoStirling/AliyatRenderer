#include "Pool.h"

Pool::Pool(GLuint _maxElement):
	maxElement(_maxElement)
{
	for (GLuint i = 0; i < maxElement; i++)
	{
		availableElements.insert(i);
	}
}

Pool::~Pool()
{

}

GLuint Pool::getUnusedElement()
{
	if (availableElements.empty())
		return -1;
	GLuint e = *availableElements.begin();
	availableElements.erase(e);
	usedElements.insert(e);
	return e;
}

void Pool::initializePool()
{

}

void Pool::releaseElement(GLuint e)
{
	if (e > 0 && e < maxElement)
	{
		availableElements.insert(e);
	}
}
