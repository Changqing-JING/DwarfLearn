cmake_minimum_required(VERSION 3.5)

project(ELFLearn)

if(MSVC)
add_compile_options(/W4 /MP)
else()
add_compile_options(-Wall -Wextra -Wpedantic -Wnon-virtual-dtor -Wformat=2
            -Wformat-security -Werror=format-security -Wcast-align -Wcast-qual -Wconversion
            -Wdouble-promotion -Wfloat-equal -Wmissing-include-dirs -Wold-style-cast
            -Wredundant-decls -Wshadow -Wsign-conversion -Wswitch -Wuninitialized
            -Wunused-parameter -Walloca -Wunused-result -Wunused-local-typedefs
            -Wwrite-strings -Wpointer-arith -Wfloat-conversion -Wnull-dereference -Wdiv-by-zero
            -Wswitch-default -Wno-switch-bool -Wunknown-pragmas 
        )
endif()

if(ENABLE_TARGETFILE)
    add_subdirectory(TargetFile)
endif()

aux_source_directory(src sourceFiles)

add_executable(${PROJECT_NAME} ${sourceFiles})

