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

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    Elf32_Shdr const *const currentHeader = sectionHeaderStart + i;

    switch (currentHeader->sh_type) {

    case (SHT_GROUP): {

      uint32_t const *const groupSection = reinterpret_cast<const uint32_t *>(fileBytes.data() + currentHeader->sh_offset);

      uint32_t textSectionIndex = UINT32_MAX;
      uint32_t debugLineSectionIndex = UINT32_MAX;
      for (uint32_t j = 1U; j < currentHeader->sh_size / sizeof(uint32_t); j++) {
        uint32_t const groupMemberIndex = groupSection[j];
        Elf32_Shdr const *const groupMemberSection = sectionHeaderStart + groupMemberIndex;
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

  Elf32_Shdr const *debugInfoSection;
  Elf32_Shdr const *debugAbbrevSection;
  const char *debugStrSection;

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    Elf32_Shdr const *const currentHeader = sectionHeaderStart + i;
    const char *const sectionName = stringContentStart + currentHeader->sh_name;

    if (strncmp(sectionName, debugLineName.data(), debugLineName.size()) == 0) {
      debugLines[i] = (*currentHeader);
    } else if (strncmp(sectionName, debugInfoName.data(), debugInfoName.size()) == 0) {
      debugInfoSection = currentHeader;
    } else if (strncmp(sectionName, debugAbbrevName.data(), debugAbbrevName.size()) == 0) {
      debugAbbrevSection = currentHeader;
    } else if (strncmp(sectionName, debugStrName.data(), debugStrName.size()) == 0) {
      debugStrSection = reinterpret_cast<const char *>(fileBytes.data() + currentHeader->sh_offset);
    }
  }

  DebugLine::parseDebugLine(fileBytes, debugLines);

  std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry> const debugAbbrev = DebugAbbrev::parseDebugAbbrev(fileBytes, debugAbbrevSection);

  DebugInfo::parseDebugInfo(fileBytes, debugInfoSection, debugAbbrev, debugStrSection);

  return 0;
}