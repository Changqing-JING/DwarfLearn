#ifndef DEBUG_LINE_HPP
#define DEBUG_LINE_HPP
#include <cstdint>
#include <map>
#include <vector>
#include "elf.h"

class DebugLine {

public:
  static void parseDebugLine(std::vector<uint8_t> const &elfFile, std::map<uint32_t, Elf32_Shdr> const &debugLines);

private:
  enum class StandardOpCode : uint8_t {
    DW_LNS_copy = 1U,
    DW_LNS_advance_pc = 2U,
    DW_LNS_advance_line = 3U,
    DW_LNS_set_file = 4U,
    DW_LNS_set_column = 5U,
    DW_LNS_negate_stmt = 6U,
    DW_LNS_set_basic_block = 7U,
    DW_LNS_const_add_pc = 8U,
    DW_LNS_fixed_advance_pc = 9U,
    DW_LNS_set_prologue_end = 10U,
    DW_LNS_set_epilogue_begin = 11U,
    DW_LNS_set_isa = 12U
  };

  enum class ExtendedOpCode : uint8_t {
    DW_LNE_end_sequence = 1U,
    DW_LNE_set_address = 2U,
    DW_LNE_define_file = 3U,
    DW_LNE_set_discriminator = 4U,
  };
};

#endif