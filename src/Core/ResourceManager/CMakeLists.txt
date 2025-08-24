set(MODULE_NAME resource_manager)
set(INTERFACE_NAME system_event_type_rosource)


add_library(${INTERFACE_NAME} INTERFACE
)
target_include_directories(${INTERFACE_NAME} INTERFACE 
	
)


file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)

find_package(OpenCV REQUIRED core imgproc highgui imgcodecs)
find_package(OpenAL REQUIRED)
find_package(FFMPEG REQUIRED)
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
	PRIVATE
	${OpenCV_LIBS}
	${FFMPEG_LIBRARIES}
	OpenAL::OpenAL
	assimp::assimp
)
