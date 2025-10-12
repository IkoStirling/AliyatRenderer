set(MODULE_NAME utils)


find_package(OpenSSL REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS asio)

file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)
target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(${MODULE_NAME} PUBLIC 
	PUBLIC
	Boost::asio
	OpenSSL::SSL
)