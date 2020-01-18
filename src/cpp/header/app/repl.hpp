namespace Onyx {
namespace App {
// The canonical Read-Eval-Print-Loop implementation.
class REPL {
public:
  REPL(unsigned char workers);

  // Run the REPL.
  void run();

  // Evaluate code.
  void eval(char *input);
};
} // namespace App
} // namespace Onyx
