#ifndef DEBUG_LINE_HPP
#define DEBUG_LINE_HPP
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "ByteReader.hpp"
#include "elf.h"

class DebugLine {

public:
  // Template function to support both ELF32 and ELF64
  template <typename ShdrType>
  static void parseDebugLine(std::vector<uint8_t> const &elfFile, std::map<uint32_t, ShdrType> const &debugLines) {
    for (std::pair<const unsigned int, ShdrType> const &pair : debugLines) {
      const ShdrType &hdr = pair.second;
      const uint8_t *const debugLineSectionData = elfFile.data() + hdr.sh_offset;
      ByteReader byteReader(debugLineSectionData, static_cast<size_t>(hdr.sh_size));
      while (!byteReader.reachedEnd()) {
        uint32_t const unit_length = byteReader.getNumber<uint32_t>();
        if (unit_length >= hdr.sh_size) {
          throw std::runtime_error("wrong unit_length");
        }
        parseUnit(byteReader, unit_length, std::is_same_v<ShdrType, Elf32_Shdr>);
      }
    }
  }

  static void parseUnit(ByteReader &byteReader, uint32_t const unit_length, const bool isElf32) {
    uint8_t const *unitStart = byteReader.cursor_;
    uint16_t const version = byteReader.getNumber<uint16_t>();

    if (version != 3U) {
      throw std::runtime_error("currently only support dwarf3");
    }

    uint32_t const header_length = byteReader.getNumber<uint32_t>();

    if (header_length > unit_length) {
      throw std::runtime_error("header_length too large");
    }

    uint8_t const minimum_instruction_length = byteReader.getNumber<uint8_t>();

    uint8_t const default_is_stmt = byteReader.getNumber<uint8_t>();
    static_cast<void>(default_is_stmt);

    int8_t const line_base = byteReader.getNumber<int8_t>();

    uint8_t const line_range = byteReader.getNumber<uint8_t>();

    uint8_t const opcode_base = byteReader.getNumber<uint8_t>();
    if (opcode_base == 0U) {
      throw std::runtime_error("opcode_base must be larger than 0");
    }

    std::vector<uint8_t> standard_opcode_lengths;

    for (uint8_t i = 1; i < opcode_base; i++) {
      uint8_t const opCodeArgumentLength = byteReader.getNumber<uint8_t>();
      standard_opcode_lengths.push_back(opCodeArgumentLength);
    }

    std::vector<std::string> const include_directories = byteReader.getStringTable();

    std::cout << "Include Directories:" << std::endl;
    for (std::string const &includeDir : include_directories) {
      std::cout << includeDir << std::endl;
    }

    std::vector<std::string> fileNameTable;

    std::cout << "file names:" << std::endl;
    do {
      std::string fileName = byteReader.getString();

      if (!fileName.empty()) {
        uint64_t const dirIndex = byteReader.readLEB128(false);

        uint64_t const modifyTime = byteReader.readLEB128(false);

        uint64_t const fileSize = byteReader.readLEB128(false);

        std::cout << dirIndex << " " << modifyTime << " " << fileSize << " " << fileName << std::endl;

        fileNameTable.push_back(std::move(fileName));
      } else {
        break;
      }

    } while (true);

    int32_t address = 0;
    int32_t lineNumber = 1;
    uint64_t file = 1U;
    std::cout << "start with file " << file << " " << fileNameTable[file - 1] << std::endl;
    while (true) {
      ptrdiff_t const offset{byteReader.cursor_ - unitStart};
      if (offset >= unit_length - 1U) {
        break; // End of the unit
      }
      std::cout << "0x" << std::hex << byteReader.getOffset() << std::dec << " ";
      uint8_t const opCode = byteReader.getNumber<uint8_t>();

      int32_t addressIncrement = 0;
      int32_t lineIncrement = 0;

      if (opCode >= opcode_base) { // special opcode
        std::cout << "special opcode " << static_cast<uint32_t>(opCode) << ": ";
        addressIncrement = ((opCode - opcode_base) / line_range) * minimum_instruction_length;
        lineIncrement = static_cast<int32_t>(line_base) + static_cast<int32_t>((opCode - opcode_base) % line_range);
      } else if (opCode > 0U) { // standard opcode
        std::cout << "standard opcode " << static_cast<uint32_t>(opCode) << ": ";
        assert(opCode <= standard_opcode_lengths.size());
        uint8_t const opCodeArgumentLength = standard_opcode_lengths[opCode - 1U];
        StandardOpCode const standardOpcode = static_cast<StandardOpCode>(opCode);
        switch (standardOpcode) {
        case (StandardOpCode::DW_LNS_copy): {
          // The opcode is not needed in current example, skip it
          break;
        }
        case (StandardOpCode::DW_LNS_advance_pc): {
          uint64_t const operand = byteReader.readLEB128(false);
          addressIncrement = static_cast<int32_t>(operand) * minimum_instruction_length;
          break;
        }
        case (StandardOpCode::DW_LNS_advance_line): {
          uint64_t const operand = byteReader.readLEB128(true);
          lineIncrement = static_cast<int32_t>(operand);
          break;
        }
        case (StandardOpCode::DW_LNS_set_file): {
          file = byteReader.readLEB128(false);
          // The index of file name table begin with 1, not 0. So the 1st in table is the 0st element in vector.
          uint64_t const vectorIndex = file - 1U;
          assert(vectorIndex < fileNameTable.size());
          std::cout << "Set File Name to entry " << (file) << " in the File Name Table: " << fileNameTable[vectorIndex];
          break;
        }
        case (StandardOpCode::DW_LNS_set_column): {
          if (opCodeArgumentLength != 1U) {
            throw std::runtime_error("opCodeArgumentLength mismatch");
          }
          uint64_t const column = byteReader.readLEB128(false);
          std::cout << "set column " << column;
          break;
        }
        case (StandardOpCode::DW_LNS_negate_stmt): {
          // The opcode is not needed in current example, skip it
          break;
        }
        case (StandardOpCode::DW_LNS_set_basic_block): {
          // The opcode is not needed in current example, skip it
          break;
        }
        case (StandardOpCode::DW_LNS_const_add_pc): {
          addressIncrement = static_cast<int32_t>(((255U - opcode_base) / line_range) * minimum_instruction_length);
          break;
        }
        case (StandardOpCode::DW_LNS_fixed_advance_pc): {
          uint16_t const operand = byteReader.getNumber<uint16_t>();
          addressIncrement = operand;
          break;
        }
        case (StandardOpCode::DW_LNS_set_epilogue_begin): {
          break;
        }
        default: {

          throw std::runtime_error("not implemented yet");
        }
        }
      } else { // extended opcode
        uint64_t const commandLength = byteReader.readLEB128(false);
        static_cast<void>(commandLength);
        uint8_t const subOpcode = byteReader.getNumber<uint8_t>();
        ExtendedOpCode const extendedOpCode = static_cast<ExtendedOpCode>(subOpcode);
        std::cout << "Extended opcode " << static_cast<uint32_t>(subOpcode) << ": ";
        switch (extendedOpCode) {
        case (ExtendedOpCode::DW_LNE_end_sequence): {
          address = 0;
          lineNumber = 1;
          file = 1;
          std::cout << "End of Sequence" << std::endl;
          break;
        }
        case (ExtendedOpCode::DW_LNE_set_address): {
          // Handle both 32-bit and 64-bit addresses
          uint64_t newAddress;
          if (isElf32) {
            // 32-bit ELF
            newAddress = byteReader.getNumber<uint32_t>();
          } else {
            // 64-bit ELF
            newAddress = byteReader.getNumber<uint64_t>();
          }
          address = static_cast<int32_t>(newAddress);
          std::cout << "set address to " << std::hex << address;
          break;
        }
        case (ExtendedOpCode::DW_LNE_set_discriminator): {
          uint64_t const discriminator = byteReader.readLEB128(false);
          static_cast<void>(discriminator);
          break;
        }
        default: {
          throw std::runtime_error("not implemented yet");
        }
        }
      }

      if (lineIncrement != 0 || addressIncrement != 0) {
        int32_t const newAddress = address + addressIncrement;
        int32_t const newLineNumber = lineNumber + lineIncrement;

        std::cout << "increase address by " << addressIncrement << " to 0x" << std::hex << newAddress << " and Line by " << std::dec << lineIncrement << " to " << newLineNumber;
        address = newAddress;
        lineNumber = newLineNumber;
      }

      std::cout << std::endl;
    }
  }

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