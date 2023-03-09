#include "DebugInfo.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "ByteReader.hpp"
#include "Tree.hpp"

const Tree<uint32_t> DebugInfo::parseDebugInfo(std::vector<uint8_t> const &elfFile, Elf32_Shdr const *const debugInfoSection,
                                               std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry> const &debugAbbrevTable, char const *const debugStr) {
  uint8_t const *const debugInfoData = elfFile.data() + debugInfoSection->sh_offset;
  ByteReader debugInfoReader(debugInfoData, debugInfoSection->sh_size);

  uint32_t const unit_length = debugInfoReader.getNumber<uint32_t>();
  uint16_t const version = debugInfoReader.getNumber<uint16_t>();
  uint32_t const debug_abbrev_offset = debugInfoReader.getNumber<uint32_t>();
  uint8_t const address_size = debugInfoReader.getNumber<uint8_t>();

  std::cout << "dump Debug Info:" << std::endl;

  std::cout << "unit_length: " << unit_length << ", version: " << version << ", debug_abbrev_offset: " << debug_abbrev_offset << ", address_size: " << address_size << std::endl;

  Tree<uint32_t> debugInfoTree;
  std::vector<TreeNode<uint32_t> *> treeNodeStack;

  uint32_t debugInfoIndex = 0;
  while (!debugInfoReader.reachedEnd()) {
    uint64_t const abbrevIndex = debugInfoReader.readLEB128(false);
    if (abbrevIndex != 0) {

      std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry>::const_iterator it = debugAbbrevTable.find(abbrevIndex);
      assert(it != debugAbbrevTable.end());
      DebugAbbrev::AbbrevEntry const &abbrevEntry = it->second;
      std::cout << "section abbrevIndex " << abbrevIndex << "------------------" << std::endl;
      std::cout << "abbrev tag " << DebugAbbrev::tagToString(abbrevEntry.tag) << std::endl;
      for (DebugAbbrev::AttributeSpecification const &attributeSpec : abbrevEntry.attributeSpecifications) {
        const std::string attributeNameStr = DebugAbbrev::attributeNameToString(attributeSpec.attributeName);
        std::cout << attributeNameStr << ": ";
        std::string formStr;
        switch (attributeSpec.form) {
        case (DebugAbbrev::Form::DW_FORM_strp): {
          uint32_t const offset = debugInfoReader.getNumber<uint32_t>();
          char const *const indirectStr = debugStr + offset;
          formStr = indirectStr;
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_string): {
          const std::string str = debugInfoReader.getString();
          formStr = str;
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_data1): {
          uint8_t const num = debugInfoReader.getNumber<uint8_t>();
          formStr = numToHexString(num);
          break;
        }

        case (DebugAbbrev::Form::DW_FORM_data2): {
          uint16_t const num = debugInfoReader.getNumber<uint16_t>();
          formStr = numToHexString(num);
          break;
        }

        case (DebugAbbrev::Form::DW_FORM_data4): {
          uint32_t const num = debugInfoReader.getNumber<uint32_t>();
          formStr = numToHexString(num);
          break;
        }

        case (DebugAbbrev::Form::DW_FORM_addr): {
          uint32_t const addr = debugInfoReader.getNumber<uint32_t>();
          formStr = numToHexString(addr);
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_flag): {
          uint8_t const flag = debugInfoReader.getNumber<uint8_t>();
          formStr = numToHexString(flag);
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_ref1): {
          uint8_t const reference = debugInfoReader.getNumber<uint8_t>();
          formStr = numToHexString(reference);
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_ref2): {
          uint16_t const reference = debugInfoReader.getNumber<uint16_t>();
          formStr = numToHexString(reference);
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_ref4): {
          uint32_t const reference = debugInfoReader.getNumber<uint32_t>();
          formStr = numToHexString(reference);
          break;
        }
        case (DebugAbbrev::Form::DW_FORM_block1):
        case (DebugAbbrev::Form::DW_FORM_block2):
        case (DebugAbbrev::Form::DW_FORM_block4): {
          uint32_t blockLength;
          if (attributeSpec.form == DebugAbbrev::Form::DW_FORM_block1) {
            blockLength = debugInfoReader.getNumber<uint8_t>();
          } else if (attributeSpec.form == DebugAbbrev::Form::DW_FORM_block2) {
            blockLength = debugInfoReader.getNumber<uint16_t>();
          } else {
            blockLength = debugInfoReader.getNumber<uint32_t>();
          }

          const std::vector<uint8_t> blockData = debugInfoReader.getArray(blockLength);
          std::string const dataString = vectorToStr(blockData);
          formStr = numToHexString(blockLength) + dataString;

          if (attributeSpec.attributeName == DebugAbbrev::AttributeName::DW_AT_location) {
            handleDataRepresentation(blockData);
          }

          break;
        }

        default: {
          throw std::runtime_error("not implemented yet");
        }
        }
        std::cout << formStr << std::endl;
      }

      if (debugInfoTree.hasRoot()) {
        TreeNode<uint32_t> *const currentParent = treeNodeStack.back();
        TreeNode<uint32_t> &child = currentParent->addChild(debugInfoIndex);

        if (abbrevEntry.hasChildren) {
          treeNodeStack.push_back(&child);
        }

      } else {
        TreeNode<uint32_t> &root = debugInfoTree.setRoot(debugInfoIndex);
        treeNodeStack.push_back(&root);
      }

      debugInfoIndex++;
    } else {
      treeNodeStack.pop_back();
    }
  }

  return debugInfoTree;
}

void DebugInfo::handleDataRepresentation(std::vector<uint8_t> const &dataRepresentation) {
  ByteReader dataRepresentationReader(dataRepresentation.data(), dataRepresentation.size());

  DwarfExpressionOpcode const opCode = static_cast<DwarfExpressionOpcode>(dataRepresentationReader.getNumber<uint8_t>());
  int64_t opNum;
  switch (opCode) {
  case (DwarfExpressionOpcode::DW_OP_fbreg): {
    opNum = static_cast<int64_t>(dataRepresentationReader.readLEB128(true));
    break;
  }

  default: {
    throw std::runtime_error("not implemented yet");
    break;
  }
  }

  std::cout << "(" << dwarfExpressionOpcodeToString(opCode) << " " << opNum << ")";
}

const std::string DebugInfo::vectorToStr(std::vector<uint8_t> const &vec) {
  std::stringstream ss;
  ss << " ";
  for (uint8_t const num : vec) {
    ss << "0x" << std::hex << static_cast<uint32_t>(num) << " ";
  }
  return ss.str();
}

std::string const DebugInfo::dwarfExpressionOpcodeToString(DwarfExpressionOpcode const opCode) {

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

  default: {
    throw std::runtime_error("No implemented yet");
    break;
  }
  }
}