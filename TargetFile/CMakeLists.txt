project(TargetFile)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} targetFileSources)

add_executable(${PROJECT_NAME} ${targetFileSources})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")