message(STATUS "ToolChainFile: ${CMAKE_TOOLCHAIN_FILE}")

cmake_minimum_required(VERSION 3.12)
project(AliyatRenderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)


if (MSVC)
	add_compile_options(/wd4828)  # 忽略 C4828
	add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BX_CONFIG_DEBUG=1)
else()
    add_compile_definitions(BX_CONFIG_DEBUG=0)
endif()

add_subdirectory(thirdParty/bgfx.cmake)


#-------------第三方小型库--------------#
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include)
link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)



#-------------项目自定义库--------------#

add_subdirectory(src/Core/ConfigWrapper)
add_subdirectory(src/Core/SQL)
add_subdirectory(src/Core/Command)
add_subdirectory(src/Core/ModuleManager)

add_subdirectory(src/Core/MemoryPool)
add_subdirectory(src/Core/EventSystem)
add_subdirectory(src/Core/TimerManager)
add_subdirectory(src/Core/MaterialManager)
add_subdirectory(src/Core/ResourceManager)
add_subdirectory(src/Core/SoundEngine)
add_subdirectory(src/Core/Renderer)
add_subdirectory(src/Core/InputSystem)
#add_subdirectory(src/Core/Network)
add_subdirectory(src/Core/SceneManager)
add_subdirectory(src/Core/PhysicsSystem)
add_subdirectory(src/Core/ECSEngine)

add_subdirectory(src/Core/GameLogic)
add_subdirectory(src/Core/EngineCore)


add_subdirectory(utils)

file(GLOB ASSETS
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/*.dll"
	)
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

# 快捷数据库工具
add_executable(aysql_tool sql.cpp)
target_link_libraries(aysql_tool PRIVATE sql command)

# 快捷配置文件工具
find_package(httplib CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
add_executable(ayconfig_tool "ConfigResolver.cpp")
target_link_libraries(ayconfig_tool PRIVATE module_manager httplib::httplib efsw::efsw)
if (WIN32)
    set_target_properties(ayconfig_tool PROPERTIES
        LINK_OPTIONS "/SUBSYSTEM:WINDOWS"
    )
endif()

add_executable(TESTLIBS test.cpp)
target_link_libraries(TESTLIBS PRIVATE engine_core)

file(GLOB RENDER_LIBS "${CMAKE_SOURCE_DIR}/thirdParty/lib/*")
add_executable(bgfxT bgfxT.cpp)
target_link_libraries(bgfxT PRIVATE bgfx ${RENDER_LIBS})
target_include_directories(bgfxT PRIVATE 
    thirdParty/bgfx.cmake/bx/include
)




#--------------Project0001-------------#
#add_subdirectory(src/Project0001)
#add_executable(Project0001 src/Project0001/main.cpp )
#target_link_libraries(Project0001 PRIVATE project_0001)


#file(GLOB_RECURSE SRCS
#	"src/unused/*.cpp"
#	"src/*.c"
#	)
#file(GLOB TPLIB "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/*")
#add_executable(BaseWorld main.cpp ${SRCS})
#add_executable(Server server.cpp ${SRCS})
#add_executable(Client client.cpp ${SRCS})

#target_link_libraries(BaseWorld utils ${TPLIB})
#target_link_libraries(Server utils ${TPLIB} ${OpenCV_LIBS} Boost::asio Boost::json Boost::uuid)
#target_link_libraries(Client utils ${TPLIB} ${OpenCV_LIBS} Boost::asio Boost::json Boost::uuid)

#--------------单元测试-------------#
#add_executable(UnitTest_EventSystem unitest/EventSystem/UnitTest.cpp)
#target_link_libraries(UnitTest_EventSystem utils event_system)

#add_executable(UnitTest_ResourceManager unitest/ResourceManager/UnitTest.cpp)
#target_link_libraries(UnitTest_ResourceManager PRIVATE engine_core)

#add_executable(UnitTest_Network unitest/Network/UnitTest.cpp)
#target_link_libraries(UnitTest_Network PRIVATE engine_core)

#add_executable(UnitTest_MemoryPool unitest/MemoryPool/Unitest.cpp)
#target_link_libraries(UnitTest_MemoryPool PRIVATE memory_pool)
