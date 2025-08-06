#ifndef DEBUG_LOC_HPP
#define DEBUG_LOC_HPP

#include <cstdint>
#include <vector>

class DebugLoc {
public:
  DebugLoc() : start_(nullptr), size_(0) {
  }
  template <typename ShdrType>
  DebugLoc(std::vector<uint8_t> const &elfFile, const ShdrType *const debugLocSection) : start_(elfFile.data() + debugLocSection->sh_offset), size_(debugLocSection->sh_size) {
  }

  void decodeAt(size_t const offset) const;

private:
  uint8_t const *start_;
  size_t size_;
};

#endif