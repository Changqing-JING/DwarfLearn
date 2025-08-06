#include "DebugInfo.hpp"
#include "VariableLocation.hpp"

const std::string DebugInfo::vectorToStr(std::vector<uint8_t> const &vec) {
  std::stringstream ss;
  ss << " ";
  for (uint8_t const num : vec) {
    ss << "0x" << std::hex << static_cast<uint32_t>(num) << " ";
  }
  return ss.str();
}

Tree<uint32_t> DebugInfo::parseDebugInfoTree(ByteReader &debugInfoReader, std::unordered_map<ptrdiff_t, DebugAbbrev::AbbrevTable> const &debugAbbrevSections, char const *const debugStr,
                                             uint32_t const unitLength, bool const is32, DebugLoc const &debugLoc) {
  uint8_t const *start = debugInfoReader.cursor_;

  uint16_t const version = debugInfoReader.getNumber<uint16_t>();
  uint32_t const debug_abbrev_offset = debugInfoReader.getNumber<uint32_t>();
  uint8_t const address_size = debugInfoReader.getNumber<uint8_t>();

  std::cout << "dump Debug Info:" << std::endl;

  std::cout << "unit_length: " << unitLength << ", version: " << version << ", debug_abbrev_offset: " << debug_abbrev_offset << ", address_size: " << static_cast<uint32_t>(address_size) << std::endl;
  DebugAbbrev::AbbrevTable const &debugAbbrevTable = debugAbbrevSections.at(debug_abbrev_offset);

  Tree<uint32_t> debugInfoTree;
  std::vector<TreeNode<uint32_t> *> treeNodeStack;

  uint32_t debugInfoIndex = 0;

  while (debugInfoReader.cursor_ - start < unitLength) {
    uint64_t const abbrevIndex = debugInfoReader.readLEB128(false);
    if (abbrevIndex != 0) {

      std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry>::const_iterator it = debugAbbrevTable.find(abbrevIndex);
      assert(it != debugAbbrevTable.end() && "abbrevIndex not found in debugAbbrevTable");
      DebugAbbrev::AbbrevEntry const &abbrevEntry = it->second;
      std::cout << std::hex << "0x" << debugInfoReader.getOffset() << std::dec << ": section abbrevIndex " << abbrevIndex << "------------------" << std::endl;
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
          if (attributeSpec.attributeName == DebugAbbrev::AttributeName::DW_AT_location) {
            debugLoc.decodeAt(num);
          }
          break;
        }

        case (DebugAbbrev::Form::DW_FORM_addr): {
          // Handle both 32-bit and 64-bit addresses
          if (is32) {
            uint32_t const addr = debugInfoReader.getNumber<uint32_t>();
            formStr = numToHexString(addr);
          } else {
            uint64_t const addr = debugInfoReader.getNumber<uint64_t>();
            formStr = numToHexString(addr);
          }
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
            VariableLocation::handleVariableLocation(blockData);
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
      assert(treeNodeStack.size() > 0U);
      treeNodeStack.pop_back();
    }
  }

  return debugInfoTree;
}