set(MODULE_NAME module_manager)
set(INTERFACE_NAME system_module_type)


add_library(${INTERFACE_NAME} INTERFACE
)
target_include_directories(${INTERFACE_NAME} INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}/interface
)


file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)

find_package(glm CONFIG REQUIRED)

target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include 
	)
target_link_libraries(${MODULE_NAME} PUBLIC ${INTERFACE_NAME}
	# spdlog::spdlog_header_only
	glm::glm
	configer
)