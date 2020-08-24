#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The API documentation compiler application.
// It emits a compiled documentation in given format.
class API : Shared::BC {
public:
  enum Format {
    Binary,
    MessagePack,
    JSON,
    DSON,
    YAML,
    XML,
    CHeader
  };

  API(filesystem::path input,
      filesystem::path output,
      bool is_extern,
      Format format,
      unsigned char workers);

  // Compile documentation.
  void compile();
};
} // namespace App
} // namespace Onyx
