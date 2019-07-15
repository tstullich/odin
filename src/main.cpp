#include <stdexcept>

#include "renderer/application.hpp"

int main(int argc, char* argv[]) {
  try {
    odin::Application app(argc, argv);
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}