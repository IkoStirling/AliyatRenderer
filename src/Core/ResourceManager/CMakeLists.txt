set(MODULE_NAME resource_manager)
set(INTERFACE_NAME system_event_type_rosource)


add_library(${INTERFACE_NAME} INTERFACE
)
target_include_directories(${INTERFACE_NAME} INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}/interface
)


file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)

find_package(OpenCV REQUIRED core imgproc highgui imgcodecs)


target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${MODULE_NAME} PUBLIC ${INTERFACE_NAME} module_manager event_system
	PRIVATE
	${OpenCV_LIBS}
)
