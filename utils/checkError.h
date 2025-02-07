#pragma once

#ifdef NDEBUG
#define GL_CALL(func) func; 
#else
#define GL_CALL(func) {\
		func; \
		checkError(); \
}
#endif // NDEBUG

void checkError();


