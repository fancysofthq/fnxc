namespace Onyx {
namespace App {
// The Language Server Protocol application.
class LSP {
public:
  LSP(unsigned char workers);

  // Run the LSP server at *port*.
  void listen(unsigned short port);
};
} // namespace App
} // namespace Onyx
