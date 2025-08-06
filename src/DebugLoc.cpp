#include "DebugLoc.hpp"
#include <iostream>
#include "ByteReader.hpp"
#include "VariableLocation.hpp"

void DebugLoc::decodeAt(size_t const offset) const {
  assert(offset < size_);
  ByteReader debugLocReader(start_ + offset, size_ - offset);
  while (true) {
    uint64_t startAddress = debugLocReader.getNumber<uint64_t>();
    uint64_t endAddress = debugLocReader.getNumber<uint64_t>();
    if ((startAddress == 0) && (endAddress == 0)) {
      break; // End of the debug location entries
    }
    std::cout << std::hex << "[" << startAddress << ", " << endAddress << std::dec << "):";
    uint16_t const locationSize = debugLocReader.getNumber<uint16_t>();
    VariableLocation::handleVariableLocation(std::span<const uint8_t>(debugLocReader.cursor_, locationSize));
    debugLocReader.step(locationSize);
    std::cout << std::endl;
  }
}