set(MODULE_NAME memory_pool)

file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)
target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)