This project is a learning project which shows decoding of dwarf3 .debug_line section in a ELF32.
## Linux
```shell
sudo apt install gcc-multilib g++-multilib
```
### Build
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TARGETFILE=1 ..
cmake --build .
```
Note the test target file can only be built on Linux
### Run
```shell
./build/ELFLearn build/TargetFile/CMakeFiles/TargetFile.dir/targetfile.cpp.o
```

## Windows
### Build
```shell
mkdir build_win
cd build_win
cmake -A x64 ..
cmake --build .
```
### Run
```
build_win\Debug\ELFLearn.exe path_to_elf_file
```

## Readelf commands
Read .debug_info
```shell
readelf --debug-dump=info ./build/TargetFile/CMakeFiles/TargetFile.dir/targetfile.cpp.o
```

## Docs
[Dwarf GNU extension](https://sourceware.org/elfutils/DwarfExtensions)