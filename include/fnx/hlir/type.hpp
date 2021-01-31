namespace FNX {
namespace HLIR {

namespace Type {

struct Void {};

/// 16-bit floating-point value.
struct Half {};

/// 16-bit "brain" floating-point value (7-bit significand).
struct BFloat {};

/// 32-bit floating-point value.
struct Float {};

/// 64-bit floating-point value.
struct Double {};

/// 128-bit floating-point value (112-bit significand).
struct FP128 {};

/// 80-bit floating-point value (X87).
struct X86FP80 {};

/// 128-bit floating-point value (two 64-bits).
struct PPCfp128 {};

template <unsigned Size> struct Int {};

template <class T, unsigned Size> struct Vector {};

template <class T, unsigned Size> struct Array {};

template <class T, unsigned AddressSpace = 0> struct Pointer {};

}; // namespace Type

} // namespace HLIR
} // namespace FNX
