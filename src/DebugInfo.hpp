#ifndef DEBUG_INFO
#define DEBUG_INFO
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "DebugAbbrev.hpp"
#include "Tree.hpp"

#include "elf.h"

class DebugInfo {

public:
  static const Tree<uint32_t> parseDebugInfo(std::vector<uint8_t> const &elfFile, Elf32_Shdr const *const debugInfoSection,
                                             std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry> const &debugAbbrevTable, char const *const debugStr);

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

private:
  static void handleDataRepresentation(std::vector<uint8_t> const &dataRepresentation);

  enum class DwarfExpressionOpcode : uint8_t {
    DW_OP_addr = 0x03,
    DW_OP_deref = 0x06,
    DW_OP_const1u = 0x08,
    DW_OP_const1s = 0x09,
    DW_OP_const2u = 0x0a,
    DW_OP_const2s = 0x0b,
    DW_OP_const4u = 0x0c,
    DW_OP_const4s = 0x0d,
    DW_OP_const8u = 0x0e,
    DW_OP_const8s = 0x0f,
    DW_OP_constu = 0x10,
    DW_OP_consts = 0x11,
    DW_OP_dup = 0x12,
    DW_OP_drop = 0x13,
    DW_OP_over = 0x14,
    DW_OP_pick = 0x15,
    DW_OP_swap = 0x16,
    DW_OP_rot = 0x17,
    DW_OP_xderef = 0x18,
    DW_OP_abs = 0x19,
    DW_OP_and = 0x1a,
    DW_OP_div = 0x1b,
    DW_OP_minus = 0x1c,
    DW_OP_mod = 0x1d,
    DW_OP_mul = 0x1e,
    DW_OP_neg = 0x1f,
    DW_OP_not = 0x20,
    DW_OP_or = 0x21,
    DW_OP_plus = 0x22,
    DW_OP_plus_uconst = 0x23,
    DW_OP_shl = 0x24,
    DW_OP_shr = 0x25,
    DW_OP_shra = 0x26,
    DW_OP_xor = 0x27,
    DW_OP_skip = 0x2f,
    DW_OP_bra = 0x28,
    DW_OP_eq = 0x29,
    DW_OP_ge = 0x2a,
    DW_OP_gt = 0x2b,
    DW_OP_le = 0x2c,
    DW_OP_lt = 0x2d,
    DW_OP_ne = 0x2e,
    DW_OP_lit0 = 0x30,
    DW_OP_lit1 = 0x31,
    DW_OP_lit31 = 0x4f,
    DW_OP_reg0 = 0x50,
    DW_OP_reg1 = 0x51,
    DW_OP_reg31 = 0x6f,
    DW_OP_breg0 = 0x70,
    DW_OP_breg1 = 0x71,
    DW_OP_breg31 = 0x8f,
    DW_OP_regx = 0x90,
    DW_OP_fbreg = 0x91,
    DW_OP_bregx = 0x92,
  };

  static std::string const dwarfExpressionOpcodeToString(DwarfExpressionOpcode const opCode);
};

#endif