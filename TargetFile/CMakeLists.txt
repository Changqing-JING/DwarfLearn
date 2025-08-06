project(TargetFile)


add_library(TargetFile OBJECT targetFile.cpp)

set_target_properties(TargetFile PROPERTIES COMPILE_FLAGS "-gdwarf-3 -m32 -ffunction-sections" LINK_FLAGS "-m32")


add_executable(TargetFileWithOptimize  targetWithOptimize.cpp foo.cpp)

set_target_properties(TargetFileWithOptimize PROPERTIES COMPILE_FLAGS "-gdwarf-3 -ffunction-sections -O2")