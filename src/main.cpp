#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include "ByteReader.hpp"
#include "DebugAbbrev.hpp"
#include "DebugInfo.hpp"
#include "DebugLine.hpp"
#include "elf.h"

std::unordered_map<uint32_t, uint32_t> debugLineTextMap; // key is section index of debug line, value is section index of text

std::vector<uint8_t> readFile(const char *const filename) {
  // open the file:
  std::ifstream file(filename, std::ios::binary);

  // Stop eating new lines in binary mode!!!
  file.unsetf(std::ios::skipws);

  // get its size:
  std::streampos fileSize;

  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  // reserve capacity
  std::vector<uint8_t> vec;
  vec.reserve(static_cast<size_t>(fileSize));

  // read the data:
  vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

  return vec;
}

std::array<char, 12> constexpr debugLineName = {".debug_line"};
std::array<char, 12> constexpr debugInfoName = {".debug_info"};
std::array<char, 14> constexpr debugAbbrevName = {".debug_abbrev"};
std::array<char, 11> constexpr debugStrName = {".debug_str"};

int main(int argc, char *argv[]) {

  if (argc < 2) {
    return 1;
  }

  std::vector<uint8_t> const fileBytes = readFile(argv[1]);

  std::array<uint8_t, EI_NIDENT> const elf32Magic = {0x7f, 0x45, 0x4c, 0x46, 01, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00};

  Elf32_Ehdr const *const elf32Header = reinterpret_cast<Elf32_Ehdr const *>(fileBytes.data());

  for (uint16_t i = 0; i < elf32Magic.size(); i++) {
    if (elf32Magic[i] != elf32Header->e_ident[i]) {
      printf("file is not a valid elf32\n");
      exit(1);
    }
  }

  const uint32_t sectionHeaderOffset = elf32Header->e_shoff;
  const uint32_t sectionHeaderSize = elf32Header->e_shentsize;
  const uint32_t numberOfSectionHeaders = elf32Header->e_shnum;

  if (sectionHeaderSize != sizeof(Elf32_Shdr)) {
    printf("wrong section header size\n");
    exit(1);
  }

  Elf32_Shdr const *const sectionHeaderStart = reinterpret_cast<Elf32_Shdr const *>(fileBytes.data() + sectionHeaderOffset);

  Elf32_Shdr const *stringTable = sectionHeaderStart + elf32Header->e_shstrndx;

  std::map<uint32_t, Elf32_Shdr> debugLines; // key is section index, value is section header

  const char *const stringContentStart = reinterpret_cast<const char *>(fileBytes.data() + stringTable->sh_offset);

  std::array<uint32_t, 3> protectedAddrList = {0xA00F0000, 0x800F0000, 0x81400000};

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    Elf32_Shdr const *const currentHeader = sectionHeaderStart + i;
    const char *const sectionName = stringContentStart + currentHeader->sh_name;

    for (uint32_t const protectedAddr : protectedAddrList) {
      uint32_t const sectionStartAddr = currentHeader->sh_addr;
      uint32_t const sectionEndAddr = sectionStartAddr + currentHeader->sh_size;
      if ((sectionStartAddr <= protectedAddr) && (sectionEndAddr >= protectedAddr)) {
        printf("%s, %x, %d\n", sectionName, sectionStartAddr, currentHeader->sh_size);
      }
    }
  }

  return 0;
}