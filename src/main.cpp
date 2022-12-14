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

class ByteReader {
public:
  ByteReader(uint8_t const *const start, size_t size) : cursor_(start), end_(start + size) {
  }

  template <typename T>
  T getNumber() {
    T num;

    if (reinterpret_cast<uintptr_t>(cursor_ + sizeof(T)) > reinterpret_cast<uintptr_t>(end_)) {
      throw std::runtime_error("over flow");
    }

    memcpy(&num, cursor_, sizeof(T));

    cursor_ += sizeof(num);

    return num;
  }

  std::string getString() {
    std::string str;

    do {
      uint8_t c = *cursor_;
      cursor_++;

      if (c != 0U) {
        str.push_back(static_cast<char>(c));
      } else {
        break;
      }
    } while (true);

    return str;
  }

  std::vector<std::string> getStringTable() {
    std::vector<std::string> stringTable;
    do {
      std::string str = getString();

      if (!str.empty()) {
        stringTable.push_back(str);
      } else {
        break;
      }
    } while (true);

    return stringTable;
  }

  uint64_t readLEB128(bool const signedInt, uint32_t const maxBits = 64U) {
    assert(maxBits <= 64U && "maxBits longer than 64 bits"); // GCOVR_EXCL_LINE
    uint64_t result = 0U;
    uint32_t bitsWritten = 0U;
    uint8_t byte = 0xFFU;
    while ((static_cast<uint32_t>(byte) & 0x80U) != 0U) {
      byte = getNumber<uint8_t>();

      uint32_t const lowByte = static_cast<uint32_t>(byte) & static_cast<uint32_t>(0x7FU);
      result |= static_cast<uint64_t>(lowByte) << static_cast<uint64_t>(bitsWritten);
      bitsWritten = bitsWritten + 7U;
      if (bitsWritten > maxBits) {
        // More bits written than allowed
        if (signedInt && ((static_cast<uint32_t>(byte) & static_cast<uint32_t>(static_cast<uint32_t>(1) << (static_cast<uint32_t>(6) - (bitsWritten - maxBits)))) != 0U)) {
          // If it is signed and negative (sign bit set) "1" padding allowed
          uint32_t const bitMask = (static_cast<uint32_t>(0xFF) << ((6U - (bitsWritten - maxBits)) + 1U)) & 0b01111111U;
          if ((static_cast<uint32_t>(byte) & bitMask) != bitMask) {
            throw std::runtime_error("Malformed LEB128 signed integer (Wrong padding)\n");
          }

        } else {
          // Zero padding allowed if unsigned or positive signed integer
          uint32_t const bitMask = (0xFFU << ((6U - (bitsWritten - maxBits)) + 1U)) & 0b01111111U;
          if ((static_cast<uint32_t>(byte) & bitMask) != 0U) {
            throw std::runtime_error("Malformed LEB128 unsigned integer (Wrong padding)\n");
          }
        }
      }
    }
    if ((signedInt && ((static_cast<uint32_t>(byte) & 0x40U) != 0U)) && (bitsWritten < 64U)) {
      // Sign extend
      uint64_t const signExtensionMask = 0xFFFF'FFFF'FFFF'FFFFLLU << bitsWritten;
      result |= signExtensionMask;
    }
    return result;
  }

  inline bool reachedEnd() {
    return cursor_ == end_;
  }

  uint8_t const *cursor_;
  uint8_t const *end_;
};

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

void parseDebugLine(std::vector<uint8_t> const &elfFile, std::map<uint32_t, Elf32_Shdr> const &debugLines) {
  for (std::pair<const unsigned int, Elf32_Shdr> const &pair : debugLines) {
    const Elf32_Shdr &hdr = pair.second;
    const uint8_t *const debugLineSectionData = elfFile.data() + hdr.sh_offset;
    ByteReader byteReader(debugLineSectionData, hdr.sh_size);
    uint32_t const unit_length = byteReader.getNumber<uint32_t>();

    if (unit_length >= hdr.sh_size) {
      throw std::runtime_error("wrong unit_length");
    }

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
    while (!byteReader.reachedEnd()) {
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
          uint64_t const newAddress = byteReader.getNumber<uint32_t>(); // Only works for 32bit machine
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
}

std::array<char, 12> const debugLineName = {".debug_line"};

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

    if (currentHeader->sh_type == SHT_GROUP) {

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
    }
  }

  for (uint32_t i = 0; i < numberOfSectionHeaders; i++) {
    Elf32_Shdr const *const currentHeader = sectionHeaderStart + i;
    const char *const sectionName = stringContentStart + currentHeader->sh_name;

    if (strncmp(sectionName, debugLineName.data(), debugLineName.size()) == 0) {
      debugLines[i] = (*currentHeader);
    }
  }

  parseDebugLine(fileBytes, debugLines);

  return 0;
}