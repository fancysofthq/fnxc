#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The API builder application.
//
// It outputs a compiled API specification in given format.
// In `--lib` mode it generates a C header file instead,
// mentioning exported entities only.
//
// ```
// onyx api main.nx && touch main.mp
// onyx api main.nx --format=json && touch main.json
// onyx api main.nx --lib && touch main.h
// ```
class API : Shared::BC {
public:
  enum Format { MP, JSON, YAML, XML };

  API(filesystem::path input,
      filesystem::path output,
      bool lib,
      Format format,
      unsigned char workers);

  // Compile an API.
  void compile();
};
} // namespace App
} // namespace Onyx
