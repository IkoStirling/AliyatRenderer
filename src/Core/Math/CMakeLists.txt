set(MODULE_NAME aymath_header_only)
set(INTERFACE_NAME ${MODULE_NAME}_xxx)

add_library(${MODULE_NAME} INTERFACE
)

find_package(glm CONFIG REQUIRED)

target_include_directories(${MODULE_NAME} INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CMAKE自动去重，不会链接两次
target_link_libraries(${MODULE_NAME} 
	INTERFACE 
	glm::glm
) 

