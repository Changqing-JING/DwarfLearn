project(TargetFile)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} targetFileSources)

add_library(${PROJECT_NAME} OBJECT ${targetFileSources})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-gdwarf-3 -m32" LINK_FLAGS "-m32")