#include "VariableLocation.hpp"
#include <iostream>
#include "ByteReader.hpp"

void VariableLocation::handleVariableLocation(std::span<const uint8_t> const dataRepresentation) {
  ByteReader dataRepresentationReader(dataRepresentation.data(), dataRepresentation.size());
  while (!dataRepresentationReader.reachedEnd()) {
    handleVariableLocation(dataRepresentationReader);
  }
}

void VariableLocation::handleBasicOpCode(DwarfExpressionOpcode const opCode, ByteReader &byteCodeReader) {
  if (opCode == DwarfExpressionOpcode::DW_OP_fbreg) {
    int64_t const opNum = static_cast<int64_t>(byteCodeReader.readLEB128(true));
    std::cout << "(" << dwarfExpressionOpcodeToString(opCode) << " " << opNum << ")";
  } else if (static_cast<uint32_t>(opCode) >= static_cast<uint32_t>(DwarfExpressionOpcode::DW_OP_reg0) && static_cast<uint32_t>(opCode) <= static_cast<uint32_t>(DwarfExpressionOpcode::DW_OP_reg31)) {
    uint64_t const regIndex = static_cast<uint32_t>(opCode) - static_cast<uint32_t>(DwarfExpressionOpcode::DW_OP_reg0);

    std::cout << "reg " << regIndex << std::endl;
  } else if (opCode == DwarfExpressionOpcode::DW_OP_regx) {
    // DW_OP_regx has one operand: register number (unsigned LEB128)
    uint64_t const regIndex = byteCodeReader.readLEB128(false);
    std::cout << "(" << dwarfExpressionOpcodeToString(opCode) << " " << regIndex << ") ";
  } else if (opCode == DwarfExpressionOpcode::DW_OP_GNU_entry_value) {
    std::cout << "(" << dwarfExpressionOpcodeToString(opCode) << ") ";
    uint64_t const size = byteCodeReader.readLEB128(false);
    handleVariableLocation(std::span<const uint8_t>(byteCodeReader.cursor_, size));
    byteCodeReader.step(size);
  }

  else {
    throw std::runtime_error("not implemented yet");
  }
}

void VariableLocation::handleVariableLocation(ByteReader &byteCodeReader) {
  DwarfExpressionOpcode const opCode = static_cast<DwarfExpressionOpcode>(byteCodeReader.getNumber<uint8_t>());

  if (opCode == DwarfExpressionOpcode::DW_OP_GNU_entry_value) {
    std::cout << "(" << dwarfExpressionOpcodeToString(opCode) << ") ";

    uint64_t const size = byteCodeReader.readLEB128(false);
    DwarfExpressionOpcode const subOpcode = static_cast<DwarfExpressionOpcode>(byteCodeReader.getNumber<uint8_t>());
    handleBasicOpCode(subOpcode, byteCodeReader);
    byteCodeReader.step(size);

  } else {
    handleBasicOpCode(opCode, byteCodeReader);
  }
}

std::string const VariableLocation::dwarfExpressionOpcodeToString(DwarfExpressionOpcode const opCode) {

  switch (opCode) {
  case (DwarfExpressionOpcode::DW_OP_addr): {
    return "DW_OP_addr";
  }
  case (DwarfExpressionOpcode::DW_OP_deref): {
    return "DW_OP_deref";
  }
  case (DwarfExpressionOpcode::DW_OP_const1u): {
    return "DW_OP_const1u";
  }
  case (DwarfExpressionOpcode::DW_OP_const1s): {
    return "DW_OP_const1s";
  }
  case (DwarfExpressionOpcode::DW_OP_const2u): {
    return "DW_OP_const2u";
  }
  case (DwarfExpressionOpcode::DW_OP_const2s): {
    return "DW_OP_const2s";
  }
  case (DwarfExpressionOpcode::DW_OP_const4u): {
    return "DW_OP_const4u";
  }
  case (DwarfExpressionOpcode::DW_OP_const4s): {
    return "DW_OP_const4s";
  }
  case (DwarfExpressionOpcode::DW_OP_const8u): {
    return "DW_OP_const8u";
  }
  case (DwarfExpressionOpcode::DW_OP_const8s): {
    return "DW_OP_const8s";
  }
  case (DwarfExpressionOpcode::DW_OP_constu): {
    return "DW_OP_constu";
  }
  case (DwarfExpressionOpcode::DW_OP_consts): {
    return "DW_OP_consts";
  }
  case (DwarfExpressionOpcode::DW_OP_dup): {
    return "DW_OP_dup";
  }
  case (DwarfExpressionOpcode::DW_OP_drop): {
    return "DW_OP_drop";
  }
  case (DwarfExpressionOpcode::DW_OP_over): {
    return "DW_OP_over";
  }
  case (DwarfExpressionOpcode::DW_OP_pick): {
    return "DW_OP_pick";
  }
  case (DwarfExpressionOpcode::DW_OP_swap): {
    return "DW_OP_swap";
  }
  case (DwarfExpressionOpcode::DW_OP_rot): {
    return "DW_OP_rot";
  }
  case (DwarfExpressionOpcode::DW_OP_xderef): {
    return "DW_OP_xderef";
  }
  case (DwarfExpressionOpcode::DW_OP_abs): {
    return "DW_OP_abs";
  }
  case (DwarfExpressionOpcode::DW_OP_and): {
    return "DW_OP_and";
  }
  case (DwarfExpressionOpcode::DW_OP_div): {
    return "DW_OP_div";
  }
  case (DwarfExpressionOpcode::DW_OP_minus): {
    return "DW_OP_minus";
  }
  case (DwarfExpressionOpcode::DW_OP_mod): {
    return "DW_OP_mod";
  }
  case (DwarfExpressionOpcode::DW_OP_mul): {
    return "DW_OP_mul";
  }
  case (DwarfExpressionOpcode::DW_OP_neg): {
    return "DW_OP_neg";
  }
  case (DwarfExpressionOpcode::DW_OP_not): {
    return "DW_OP_not";
  }
  case (DwarfExpressionOpcode::DW_OP_or): {
    return "DW_OP_or";
  }
  case (DwarfExpressionOpcode::DW_OP_plus): {
    return "DW_OP_plus";
  }
  case (DwarfExpressionOpcode::DW_OP_plus_uconst): {
    return "DW_OP_plus_uconst";
  }
  case (DwarfExpressionOpcode::DW_OP_shl): {
    return "DW_OP_shl";
  }
  case (DwarfExpressionOpcode::DW_OP_shr): {
    return "DW_OP_shr";
  }
  case (DwarfExpressionOpcode::DW_OP_shra): {
    return "DW_OP_shra";
  }
  case (DwarfExpressionOpcode::DW_OP_xor): {
    return "DW_OP_xor";
  }
  case (DwarfExpressionOpcode::DW_OP_skip): {
    return "DW_OP_skip";
  }
  case (DwarfExpressionOpcode::DW_OP_bra): {
    return "DW_OP_bra";
  }
  case (DwarfExpressionOpcode::DW_OP_eq): {
    return "DW_OP_eq";
  }
  case (DwarfExpressionOpcode::DW_OP_ge): {
    return "DW_OP_ge";
  }
  case (DwarfExpressionOpcode::DW_OP_gt): {
    return "DW_OP_gt";
  }
  case (DwarfExpressionOpcode::DW_OP_le): {
    return "DW_OP_le";
  }
  case (DwarfExpressionOpcode::DW_OP_lt): {
    return "DW_OP_lt";
  }
  case (DwarfExpressionOpcode::DW_OP_ne): {
    return "DW_OP_ne";
  }
  case (DwarfExpressionOpcode::DW_OP_lit0): {
    return "DW_OP_lit0";
  }
  case (DwarfExpressionOpcode::DW_OP_lit1): {
    return "DW_OP_lit1";
  }
  case (DwarfExpressionOpcode::DW_OP_lit31): {
    return "DW_OP_lit31";
  }
  case (DwarfExpressionOpcode::DW_OP_reg0): {
    return "DW_OP_reg0";
  }
  case (DwarfExpressionOpcode::DW_OP_reg1): {
    return "DW_OP_reg1";
  }
  case (DwarfExpressionOpcode::DW_OP_reg31): {
    return "DW_OP_reg31";
  }
  case (DwarfExpressionOpcode::DW_OP_breg0): {
    return "DW_OP_breg0";
  }
  case (DwarfExpressionOpcode::DW_OP_breg1): {
    return "DW_OP_breg1";
  }
  case (DwarfExpressionOpcode::DW_OP_breg31): {
    return "DW_OP_breg31";
  }
  case (DwarfExpressionOpcode::DW_OP_regx): {
    return "DW_OP_regx";
  }
  case (DwarfExpressionOpcode::DW_OP_fbreg): {
    return "DW_OP_fbreg";
  }
  case (DwarfExpressionOpcode::DW_OP_bregx): {
    return "DW_OP_bregx";
  }
  case (DwarfExpressionOpcode::DW_OP_GNU_entry_value): {
    return "DW_OP_GNU_entry_value";
  }

  default: {
    throw std::runtime_error("No implemented yet");
    break;
  }
  }
}