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
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "ByteReader.hpp"
#include "DebugAbbrev.hpp"
#include "DebugInfo.hpp"
#include "DebugLine.hpp"
#include "DebugLoc.hpp"
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
std::array<char, 11> constexpr debugLocName = {".debug_loc"};

// Template function declarations for ELF32/64 handling
template <typename EhdrType, typename ShdrType>
int processElfFile(const std::vector<uint8_t> &fileBytes);

int main(int argc, char *argv[]) {

  if (argc < 2) {
    return 1;
  }

  std::vector<uint8_t> const fileBytes = readFile(argv[1]);

  // Check basic ELF magic
  if (fileBytes.size() < EI_NIDENT || fileBytes[EI_MAG0] != ELFMAG0 || fileBytes[EI_MAG1] != ELFMAG1 || fileBytes[EI_MAG2] != ELFMAG2 || fileBytes[EI_MAG3] != ELFMAG3) {
    printf("file is not a valid ELF file\n");
    exit(1);
  }

  // Determine ELF class (32-bit or 64-bit)
  unsigned char elfClass = fileBytes[EI_CLASS];

  if (elfClass == ELFCLASS32) {
    printf("Processing ELF32 file\n");
    return processElfFile<Elf32_Ehdr, Elf32_Shdr>(fileBytes);
  } else if (elfClass == ELFCLASS64) {
    printf("Processing ELF64 file\n");
    return processElfFile<Elf64_Ehdr, Elf64_Shdr>(fileBytes);
  } else {
    printf("Unsupported ELF class: %d\n", elfClass);
    exit(1);
  }
}

template <typename EhdrType, typename ShdrType>
int processElfFile(const std::vector<uint8_t> &fileBytes) {
  const EhdrType *const elfHeader = reinterpret_cast<const EhdrType *>(fileBytes.data());

  const auto sectionHeaderOffset = elfHeader->e_shoff;
  const auto sectionHeaderSize = elfHeader->e_shentsize;
  const auto numberOfSectionHeaders = elfHeader->e_shnum;

  if (sectionHeaderSize != sizeof(ShdrType)) {
    printf("wrong section header size\n");
    exit(1);
  }

  const ShdrType *const sectionHeaderStart = reinterpret_cast<const ShdrType *>(fileBytes.data() + sectionHeaderOffset);

  const ShdrType *stringTable = sectionHeaderStart + elfHeader->e_shstrndx;

  std::map<uint32_t, ShdrType> debugLines; // key is section index, value is section header

  const char *const stringContentStart = reinterpret_cast<const char *>(fileBytes.data() + stringTable->sh_offset);

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    const ShdrType *const currentHeader = sectionHeaderStart + i;

    switch (currentHeader->sh_type) {

    case (SHT_GROUP): {

      const uint32_t *const groupSection = reinterpret_cast<const uint32_t *>(fileBytes.data() + currentHeader->sh_offset);

      uint32_t textSectionIndex = UINT32_MAX;
      uint32_t debugLineSectionIndex = UINT32_MAX;
      for (uint32_t j = 1U; j < currentHeader->sh_size / sizeof(uint32_t); j++) {
        uint32_t const groupMemberIndex = groupSection[j];
        const ShdrType *const groupMemberSection = sectionHeaderStart + groupMemberIndex;
        const char *const sectionName = stringContentStart + groupMemberSection->sh_name;
        if (strncmp(sectionName, ".text", 5U) == 0) {
          if (textSectionIndex == UINT32_MAX) {
            textSectionIndex = groupMemberIndex;
          } else {
            throw std::runtime_error("two .text in group");
          }

        } else if (strncmp(sectionName, debugLineName.data(), debugLineName.size()) == 0) {
          if (debugLineSectionIndex == UINT32_MAX) {
            debugLineSectionIndex = groupMemberIndex;
          } else {
            throw std::runtime_error("two .debugline in group");
          }
        }
      }

      if ((debugLineSectionIndex != UINT32_MAX) && (textSectionIndex == UINT32_MAX)) {
        throw std::runtime_error("debug_line section without code section");
      } else if ((debugLineSectionIndex != UINT32_MAX) && (textSectionIndex != UINT32_MAX)) {
        printf("text section %d map to debug_line %d\n", textSectionIndex, debugLineSectionIndex);
        debugLineTextMap[debugLineSectionIndex] = textSectionIndex;
      }
      break;
    }
    default: {
      break;
    }
    }
  }

  const ShdrType *debugInfoSection = nullptr;
  const ShdrType *debugAbbrevSection = nullptr;
  const ShdrType *debugLocSection = nullptr;
  const char *debugStrSection = nullptr;

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    const ShdrType *const currentHeader = sectionHeaderStart + i;
    const char *const sectionName = stringContentStart + currentHeader->sh_name;

    if (strncmp(sectionName, debugLineName.data(), debugLineName.size()) == 0) {
      debugLines[i] = (*currentHeader);
    } else if (strncmp(sectionName, debugInfoName.data(), debugInfoName.size()) == 0) {
      debugInfoSection = currentHeader;
    } else if (strncmp(sectionName, debugAbbrevName.data(), debugAbbrevName.size()) == 0) {
      debugAbbrevSection = currentHeader;
    } else if (strncmp(sectionName, debugStrName.data(), debugStrName.size()) == 0) {
      debugStrSection = reinterpret_cast<const char *>(fileBytes.data() + currentHeader->sh_offset);
    } else if (strncmp(sectionName, debugLocName.data(), debugLocName.size()) == 0) {
      debugLocSection = currentHeader;
    }
  }

  // Use the template function directly with the native types
  DebugLine::parseDebugLine<ShdrType>(fileBytes, debugLines);
  if (debugAbbrevSection != nullptr) {
    std::unordered_map<ptrdiff_t, DebugAbbrev::AbbrevTable> const debugAbbrev = DebugAbbrev::parseDebugAbbrev<ShdrType>(fileBytes, debugAbbrevSection);
    DebugLoc debugLoc;
    if (debugLocSection) {
      debugLoc = DebugLoc(fileBytes, debugLocSection);
    }

    // Now DebugInfo also supports templates for both ELF32 and ELF64
    if (debugInfoSection && debugAbbrevSection && debugStrSection) {
      DebugInfo::parseDebugInfo<ShdrType>(fileBytes, debugInfoSection, debugAbbrev, debugStrSection, debugLoc);
    }
  }

  return 0;
}