#include "ByteReader.hpp"

ByteReader::ByteReader(uint8_t const *const start, size_t size) : cursor_(start), end_(start + size) {
}

const std::vector<uint8_t> ByteReader::getArray(uint32_t const length) {
  std::vector<uint8_t> res;

  for (uint32_t i = 0; i < length; i++) {
    res.push_back(*cursor_);
    cursor_++;
  }
  return res;
}

std::string const ByteReader::getString() {
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

std::vector<std::string> ByteReader::getStringTable() {
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

uint64_t ByteReader::readLEB128(bool const signedInt, uint32_t const maxBits) {
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