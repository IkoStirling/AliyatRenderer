cmake_minimum_required(VERSION 3.12)

project(AliyatRenderer)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")


#---------------------------#
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include)
link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)

add_subdirectory(utils)

file(GLOB ASSETS
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/*.dll"
	)
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})


#Á´½Ó
aux_source_directory(src SRCS)
add_executable(BaseWorld ${SRCS})

 
file(GLOB TPLIB "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/*")
target_link_libraries(BaseWorld utils ${TPLIB})

#target_link_libraries(BaseWorld glfw3.lib)

#---------------------------#

#add_custom_target(clean_cmake_files COMMAND ${CMAKE_COMMAND} -P clean_cmake_files.cmake)


