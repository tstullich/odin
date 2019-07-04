#ifndef ODIN_FILE_READER_HPP
#define ODIN_FILE_READER_HPP

#include <fstream>
#include <stdexcept>
#include <vector>

namespace odin {
class FileReader {
 public:
  static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
  }

 private:
  FileReader();
};
}  // namespace odin
#endif  // ODIN_FILE_READER_HPP