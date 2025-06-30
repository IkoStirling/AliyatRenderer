#pragma once
//#include <spdlog/spdlog.h>
//#include <spdlog/sinks/stdout_color_sinks.h> 
#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout<< x << std::endl
#else
#define DEBUG_PRINT(x) 
#endif

#define PARAM_TO_STRING(x) #x

