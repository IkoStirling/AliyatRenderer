cmake_minimum_required(VERSION 3.12)

project(AliyatRenderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------第三方大型库-------------#

set(OpenCV_DIR "../opencv410/opencv/build")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})

set(BOOST_ROOT "../boost_1_87_0")
set(BOOST_LIBRARY_DIR "../boost_1_87_0/stage/lib")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DEBUG_RUNTIME ON)

find_package(Boost 1.87 REQUIRED COMPONENTS system thread json)

link_directories(${BOOST_LIBRARY_DIR})

#-------------第三方小型库--------------#

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include)
link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/src)

#-------------项目自定义库--------------#

add_subdirectory(utils)

file(GLOB ASSETS
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/*.dll"
	"${OpenCV_DIR}/x64/vc16/bin/*.dll"
	"${BOOST_LIBRARY_DIRS}/*.dll"
	)
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

#链接
file(GLOB_RECURSE SRCS
	"src/*.cpp"
	"src/*.c"
	)

add_executable(BaseWorld main.cpp ${SRCS})
add_executable(Server server.cpp ${SRCS})
add_executable(Client client.cpp ${SRCS})
 
file(GLOB TPLIB "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/*")
target_link_libraries(BaseWorld utils ${TPLIB} ${OpenCV_LIBS})
target_link_libraries(Server utils ${TPLIB} ${OpenCV_LIBS})
target_link_libraries(Client utils ${TPLIB} ${OpenCV_LIBS})

#--------------单元测试-------------#
add_executable(UnitTest_EventSystem unitest/EventSystem/UnitTest.cpp ${SRCS})
target_link_libraries(UnitTest_EventSystem utils ${TPLIB} ${OpenCV_LIBS})

add_executable(UnitTest_ResourceManager unitest/ResourceManager/UnitTest.cpp ${SRCS})
target_link_libraries(UnitTest_ResourceManager utils ${TPLIB} ${OpenCV_LIBS})

add_executable(UnitTest_Network unitest/Network/UnitTest.cpp ${SRCS})
#target_include_directories(UnitTest_Network PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(UnitTest_Network
	PRIVATE
	utils 
	${TPLIB} 
	${OpenCV_LIBS} 

)



#add_custom_target(clean_cmake_files COMMAND ${CMAKE_COMMAND} -P clean_cmake_files.cmake)


