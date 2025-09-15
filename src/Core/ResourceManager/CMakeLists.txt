set(MODULE_NAME resource_manager)
set(INTERFACE_NAME system_event_type_rosource)

find_package(OpenCV REQUIRED core imgproc highgui imgcodecs)
add_library(${INTERFACE_NAME} INTERFACE
)
target_include_directories(${INTERFACE_NAME} INTERFACE 
	${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(${INTERFACE_NAME} INTERFACE
    ${OpenCV_LIBS}
)


file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)


find_package(OpenAL REQUIRED)
find_package(FFMPEG REQUIRED) # 注意 FFMPEG是个C库，c++中必须用extern "C"去包裹头文件 
find_package(assimp CONFIG REQUIRED)

target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/interface
	PRIVATE 
	${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(${MODULE_NAME} PUBLIC
	${INTERFACE_NAME}
	module_manager
	event_system
	material_manager
	sound_engine
	PRIVATE
	${FFMPEG_LIBRARIES}
	OpenAL::OpenAL
	assimp::assimp
)
