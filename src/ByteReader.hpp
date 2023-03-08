#ifndef BYTE_READER_HPP
#define BYTE_READER_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

class ByteReader {
public:
  ByteReader(uint8_t const *const start, size_t size);

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

  const std::vector<uint8_t> getArray(uint32_t const length);

  std::string const getString();

  std::vector<std::string> getStringTable();

  uint64_t readLEB128(bool const signedInt, uint32_t const maxBits = 64U);

  inline bool reachedEnd() noexcept {
    return cursor_ == end_;
  }

  uint8_t const *cursor_;
  uint8_t const *end_;
};

#endif