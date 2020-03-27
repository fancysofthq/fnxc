namespace Onyx {
namespace Compiler {
// Optimizes the AST before codegen.
//
// AST always generates monomorphic functions, flattened to final
// types. It's up to an optimizer to either leave it as-is or join
// such calls in union switches, turning functions from mono- to
// poly-morphic; such optimization would happen before LLVM IR emit.
class Optimizer {};
} // namespace Compiler
} // namespace Onyx
