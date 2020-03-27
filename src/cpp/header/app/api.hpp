#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical API builder application. It outputs a compiled API
// specification in given `-[-F]ormat`:
//
// Format          | Flag        | Extension  | Notes
// ---             | ---         | ---        | ---
// Onyx Binary API | `-Fbin`     | `.binapi`  | The default one
// MessagePack     | `-Fmsgpack` | `.msgpack`
// JSON            | `-Fjson`    | `.json`
// YAML            | `-Fyaml`    | `.yaml`
// XML             | `-Fxml`     | `.xml`
// C header file   | `-Fheader`  | `.h`       | Only for `-[-e]xtern`
//
// Passing the `-[-e]xtern` flag outputs `extern` entities API only
// as a C header file by default. It is still possible to define
// other formats in this case, though.
//
// Examples:
//
// ```console
// # Export application API in the default binary API format
// onyxc api main.nx && [ -e main.binapi ]
//
// # Export application API in JSON
// onyxc api main.nx -Fjson -o doc/api.json && [ -e doc/api.json ]
//
// # Export external entities API as a C header file
// onyxc api main.nx -e && [ -e main.h ]
//
// # Export external entities API in MessagePack format
// onyxc api main.nx -e --format=msgpack && [ -e main.msgpack ]
// ```
class API : Shared::BC {
public:
  enum Format { Binary, MessagePack, JSON, YAML, XML, Header };

  API(filesystem::path input,
      filesystem::path output,
      bool is_extern,
      Format format,
      unsigned char workers);

  // Compile an API.
  void compile();
};
} // namespace App
} // namespace Onyx
