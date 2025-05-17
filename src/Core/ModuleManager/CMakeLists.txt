set(MODULE_NAME module_manager)

file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)
message(STATUS "cur dir: ${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)