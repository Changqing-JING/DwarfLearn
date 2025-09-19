#ifndef DEBUG_INFO
#define DEBUG_INFO
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "ByteReader.hpp"
#include "DebugAbbrev.hpp"
#include "DebugLoc.hpp"
#include "Tree.hpp"

#include "elf.h"

class DebugInfo {
public:
  struct DIEInfo {
    uint32_t offset;
    DebugAbbrev::Tag tag;
    std::string name;
    std::string typeName; // For base types, this stores the type name
  };

public:
  // Template function to support both ELF32 and ELF64
  template <typename ShdrType>
  static void parseDebugInfo(std::vector<uint8_t> const &elfFile, const ShdrType *const debugInfoSection, std::unordered_map<ptrdiff_t, DebugAbbrev::AbbrevTable> const &debugAbbrevSections,
                             char const *const debugStr, DebugLoc const &debugLoc) {
    {
      uint8_t const *const debugInfoData = elfFile.data() + debugInfoSection->sh_offset;
      ByteReader debugInfoReader(debugInfoData, static_cast<size_t>(debugInfoSection->sh_size));

      while (!debugInfoReader.reachedEnd()) {
        uint32_t const unit_length = debugInfoReader.getNumber<uint32_t>();

        parseDebugInfoTree(debugInfoReader, debugAbbrevSections, debugStr, unit_length, std::is_same_v<ShdrType, Elf32_Shdr>, debugLoc);
      }
    }
  }

  static const std::string vectorToStr(std::vector<uint8_t> const &vec);

  template <typename T>
  static const std::string numToHexString(T const num) {
    std::stringstream ss;
    ss << "0x" << std::hex;
    if (sizeof(num) > 1) {
      ss << num;
    } else {
      ss << static_cast<uint32_t>(num);
    }
    return ss.str();
  }

  static Tree<uint32_t> parseDebugInfoTree(ByteReader &debugInfoReader, std::unordered_map<ptrdiff_t, DebugAbbrev::AbbrevTable> const &debugAbbrevSections, char const *const debugStr,
                                           uint32_t const unitLength, bool const is32, DebugLoc const &debugLoc);

  static std::string resolveTypeName(uint32_t typeOffset, const std::unordered_map<uint32_t, DIEInfo> &dieStorage);

private:
};

#endif