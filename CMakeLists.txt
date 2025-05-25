message(STATUS "ToolChainFile: ${CMAKE_TOOLCHAIN_FILE}")


cmake_minimum_required(VERSION 3.12)
project(AliyatRenderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


#--------------第三方大型库-------------#
find_package(Boost CONFIG REQUIRED COMPONENTS json asio uuid)
find_package(OpenCV REQUIRED core imgproc highgui imgcodecs)

message(${Boost_INCLUDE_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

#-------------第三方小型库--------------#
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include)
link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)

#-------------项目自定义库--------------#
# include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src/Core/ModuleManager)

add_subdirectory(src/Core/MemoryPool)

add_subdirectory(src/Core/EventSystem)
add_subdirectory(src/Core/ResourceManager)
add_subdirectory(src/Core/Network)

add_subdirectory(src/Core/EngineCore)


file(GLOB ASSETS
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/*.dll"
	"${OpenCV_DIR}/x64/vc16/bin/*.dll"
	"${BOOST_LIBRARY_DIRS}/*.dll"
	)
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SRCS
	"src/core/*.cpp"
	"src/*.c"
	)

file(GLOB TPLIB "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/*")

find_package(OpenSSL REQUIRED)

add_executable(TESTLIBS test.cpp)
target_link_libraries(TESTLIBS PRIVATE engine_core Boost::asio OpenSSL::SSL)

#链接
#add_executable(BaseWorld main.cpp ${SRCS})
#add_executable(Server server.cpp ${SRCS})
#add_executable(Client client.cpp ${SRCS})

#target_link_libraries(BaseWorld utils ${TPLIB})
#target_link_libraries(Server utils ${TPLIB} ${OpenCV_LIBS} Boost::asio Boost::json Boost::uuid)
#target_link_libraries(Client utils ${TPLIB} ${OpenCV_LIBS} Boost::asio Boost::json Boost::uuid)

#--------------单元测试-------------#
#add_executable(UnitTest_EventSystem unitest/EventSystem/UnitTest.cpp)
#target_link_libraries(UnitTest_EventSystem utils ${TPLIB})

#add_executable(UnitTest_ResourceManager unitest/ResourceManager/UnitTest.cpp ${SRCS})
#target_link_libraries(UnitTest_ResourceManager utils ${TPLIB})

#add_executable(UnitTest_Network unitest/Network/UnitTest.cpp ${SRCS})
#target_link_libraries(UnitTest_Network PRIVATE utils ${TPLIB} ${OpenCV_LIBS} Boost::asio Boost::json)


