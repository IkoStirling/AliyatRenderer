set(MODULE_NAME engine_core)

file(GLOB SRCS
	"src/*.cpp"
	)
add_library(${MODULE_NAME} STATIC
    ${SRCS}
)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${MODULE_NAME} PUBLIC 
	module_manager
	event_system
	resource_manager
	memory_pool
	network
	renderer
	input_system
	game_logic
	scene_manager
	physics_system
)