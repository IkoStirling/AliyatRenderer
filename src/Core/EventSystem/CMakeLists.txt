set(MODULE_NAME event_system)
set(INTERFACE_NAME system_event_type)


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
target_include_directories(${MODULE_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 严格来说接口库是需要方链接，这里相当于直接include了interface目录
target_link_libraries(${MODULE_NAME} PUBLIC ${INTERFACE_NAME}) 

# CMAKE自动去重，不会链接两次
target_link_libraries(${MODULE_NAME} PUBLIC  memory_pool module_manager) 