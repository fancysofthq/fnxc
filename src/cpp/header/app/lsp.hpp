namespace Onyx {
namespace App {
// The canonical Language Server Protocol implementation.
class LSP {
public:
  LSP(unsigned char workers);

  // Run the LSP server at *port*.
  void listen(unsigned short port);
};
} // namespace App
} // namespace Onyx
