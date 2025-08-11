set(MODULE_NAME network)


find_package(Boost CONFIG REQUIRED COMPONENTS asio)

file(GLOB_RECURSE SRCS
	"*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)
target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(${MODULE_NAME} PUBLIC 
	module_manager
	PRIVATE
	Boost::asio
)