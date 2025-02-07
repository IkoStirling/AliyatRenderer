
file(GLOB_RECURSE UTILS ./ *.cpp)

add_library(utils ${UTILS})

target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/utils)