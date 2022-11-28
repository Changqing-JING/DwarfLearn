cmake_minimum_required(VERSION 3.5)

project(ELFLearn)

add_subdirectory(TargetFile)

aux_source_directory(src sourceFiles)

add_executable(${PROJECT_NAME} ${sourceFiles})

