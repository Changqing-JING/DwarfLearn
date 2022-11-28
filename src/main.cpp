#include <cstdint>
#include <elf.h>
#include <sys/types.h>
#include <vector>
#include <fstream>
#include <iterator>
#include <array>
#include <cassert>


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
  vec.reserve(fileSize);

  // read the data:
  vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

  return vec;
}

int main(int argc, char* argv[]){

    if(argc<2){
        return 1;
    }
    
    std::vector<uint8_t> fileBytes = readFile(argv[1]);

    std::array<uint8_t, EI_NIDENT> elf32Magic = {0x7f, 0x45, 0x4c, 0x46, 01, 01, 01, 00, 00 ,00, 00, 00 ,00 ,00, 00, 00};


    Elf32_Ehdr* elf32Header = reinterpret_cast<Elf32_Ehdr*>(fileBytes.data());

    for(uint16_t i = 0;i<elf32Magic.size();i++){
      if(elf32Magic[i]!=elf32Header->e_ident[i]){
        printf("file is not a valid elf32\n");
        exit(1);
      }
    }

    const uint32_t sectionHeaderOffset = elf32Header->e_shoff;
    const uint32_t sectionHeaderSize = elf32Header->e_shentsize;
    const uint32_t numberOfSectionHeaders = elf32Header->e_shnum;

    if(sectionHeaderSize != sizeof( Elf32_Shdr)){
      printf("wrong section header size\n");
      exit(1);
    }

    Elf32_Shdr* sectionHeaderStart = reinterpret_cast<Elf32_Shdr*>(fileBytes.data() + sectionHeaderOffset);

    Elf32_Shdr* stringTable = sectionHeaderStart + numberOfSectionHeaders-1U;

    if(stringTable->sh_type != SHT_STRTAB){
      printf("string table not found\n");
      exit(1);
    }

    const char* stringContentStart = reinterpret_cast<const char*>(fileBytes.data() + stringTable->sh_offset);

    for(uint32_t i = 0;i<numberOfSectionHeaders;i++){
      Elf32_Shdr* currentHeader = sectionHeaderStart + i;
      const char* sectionName = stringContentStart + currentHeader->sh_name;
      printf("%s, offset %x, size %x\n",sectionName, currentHeader->sh_offset, currentHeader->sh_size);
    }

    return 0;
}