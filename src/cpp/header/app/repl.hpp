namespace Onyx {
namespace App {
// The Read-Eval-Print-Loop application.
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
