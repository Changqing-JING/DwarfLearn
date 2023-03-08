#ifndef DEBUG_INFO
#define DEBUG_INFO
#include <cstdint>
#include <vector>
#include "DebugAbbrev.hpp"
#include "Tree.hpp"

#include "elf.h"

class DebugInfo {

public:
  static const Tree<uint32_t> parseDebugInfo(std::vector<uint8_t> const &elfFile, Elf32_Shdr const *const debugInfoSection,
                                             std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry> const &debugAbbrevTable, char const *const debugStr);

  static const std::string vectorToStr(std::vector<uint8_t> const &vec);
  static const std::string numToHexString(uint64_t const num);
};

#endif