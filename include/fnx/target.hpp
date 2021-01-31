#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// NOTE: An ISE defines an *integral* part of a processing unit.

namespace FNX {
namespace Target {

/// An Instruction Set Architecture.
enum class ISA {
  I386,   ///< x86
  AMD64,  ///< x86-64
  T32,    ///< ARMv6, ARMv7, a.k.a.\ "Thumb"
  A32,    ///< ARMv6, ARMv7, a.k.a.\ "ARM"
  A64,    ///< ARMv8
  WASM32, ///< WebAssembly
};

const std::unordered_map<std::string, ISA> isa_map = {
    {"i386", ISA::I386},
    {"amd64", ISA::AMD64},
    {"thumb", ISA::T32},
    {"arm", ISA::A32},
    {"arm64", ISA::A64},
    {"wasm32", ISA::WASM32},
};

enum class ISAFamily {
  X86,
  ARM,
  WASM,
};

inline ISAFamily family(ISA isa) {
  switch (isa) {
  case ISA::I386:
    return ISAFamily::X86;
  case ISA::AMD64:
    return ISAFamily::X86;
  case ISA::T32:
    return ISAFamily::ARM;
  case ISA::A32:
    return ISAFamily::ARM;
  case ISA::A64:
    return ISAFamily::ARM;
  case ISA::WASM32:
    return ISAFamily::WASM;
  }
}

inline std::string name(ISAFamily family) {
  switch (family) {
  case ISAFamily::X86:
    return "x86";
  case ISAFamily::ARM:
    return "arm";
  case ISAFamily::WASM:
    return "wasm";
  }
}

// The x86 ISA family.
namespace X86 {
/// An x86 Instruction Set Extensions.
enum class ISE {
  FMA3,
  F16C,

  MMX,

  SSE,
  SSE2,
  SSE3,
  SSSE3,
  SSE41,
  SSE42,
  SSE4A,

  AVX,
  AVX2,
  AVX512,

  AES,
  SHA,

  MOVBE,
  POPCNT,
  PCLMUL,
  FSGSBASE,
  RDRND,

  BMI,
  BMI2,

  VTx,
  VTd,

  TXT,
  TSX,
  RDSEED,
  ADCX,
  PREFETCHW,
  CLFLUSHOPT,
  XSAVE,
  SGX,
  MPX,
};

const std::unordered_map<std::string, ISE> ise_map = {
    {"MMX", ISE::MMX},
    {"SSE", ISE::SSE},
    {"SSE2", ISE::SSE2},
    {"SSE3", ISE::SSE3},
    {"SSSE3", ISE::SSSE3},
    {"SSE4.1", ISE::SSE41},
    {"SSE4.2", ISE::SSE42},
    {"SSE4A", ISE::SSE4A},
    {"AVX", ISE::AVX},
    {"AVX2", ISE::AVX2},
    {"AVX-512", ISE::AVX512},
    {"FMA3", ISE::FMA3},
    {"AES", ISE::AES},
    {"SHA", ISE::SHA},
    {"MOVBE", ISE::MOVBE},
    {"POPCNT", ISE::POPCNT},
    {"PCLMUL", ISE::PCLMUL},
    {"FSGSBASE", ISE::FSGSBASE},
    {"RDRND", ISE::RDRND},
    {"F16C", ISE::F16C},
    {"BMI", ISE::BMI},
    {"BMI2", ISE::BMI2},
    {"VT-x", ISE::VTx},
    {"VT-d", ISE::VTd},
    {"TXT", ISE::TXT},
    {"TSX", ISE::TSX},
    {"RDSEED", ISE::RDSEED},
    {"ADCX", ISE::ADCX},
    {"PREFETCHW", ISE::PREFETCHW},
    {"CLFLUSHOPT", ISE::CLFLUSHOPT},
    {"XSAVE", ISE::XSAVE},
    {"SGX", ISE::SGX},
    {"MPX", ISE::MPX},
};

const std::unordered_set<ISE> default_ise = {};

/// A well-known x86-64 Processing Unit.
struct PU {
  const ISA isa;
  const std::unordered_set<ISE> ise;
  PU(ISA, const std::unordered_set<ISE>);
};

const std::unordered_map<std::string, PU> well_known_pus = {
    {"amd3200g",
     PU(ISA::AMD64,
        {
            ISE::MMX,
            ISE::SSE,
            ISE::SSE2,
            ISE::SSE3,
            ISE::SSSE3,
            ISE::SSE41,
            ISE::SSE42,
            ISE::SSE4A,
            ISE::AES,
            ISE::AVX,
            ISE::AVX2,
            ISE::FMA3,
            ISE::SHA,
        })},
};
} // namespace X86

// The ARM ISA family.
namespace ARM {
enum class ISE {
  Neon,
  Helium,
  Jazelle,
  SVE,
  DSP,
  VFP,
  VFPv2,
  VFPv3,
  VFPv3U,
  VFPv3HP,
  VFPv3UHP,
  VFPv4,
  VFPv5,
};
} // namespace ARM

// The WebAssembly ISA family.
namespace WASM {
enum class ISE;
}

enum class OS {
  Linux,
  BSD,
  Windows,
  Darwin,
  WASI,
};

const std::unordered_map<std::string, OS> os_map = {
    {"linux", OS::Linux},
    {"bsd", OS::BSD},
    {"win32", OS::Windows},
    {"darwin", OS::Darwin},
    {"wasi", OS::WASI},
};

struct C {
  enum Vendor {
    ISO,       // No specific vendor
    GNU,       // GNUC
    Microsoft, // MSVC
    Apple,     // CLang
  };

  static const std::unordered_map<std::string, Vendor> vendor_map;

  enum Standard {
    ANSI,
    C99,
    C11,
    C18,
  };

  static const std::unordered_map<std::string, Standard> std_map;

  static inline std::optional<C> default_for(OS os) {
    switch (os) {
    case OS::Linux:
      // clang-format off
      //
      // From https://www.kernel.org/doc/html/latest/process/programming-language.html:
      //
      // > More precisely, the kernel is typically compiled with gcc [gcc] under -std=gnu89
      // [gcc-c-dialect-options]: the GNU dialect of ISO C90 (including some C99 features).
      //
      // clang-format on
      return C(GNU, ANSI);
    case OS::BSD:
      return C(Apple, ANSI);
    case OS::Windows:
      return C(Microsoft, C11);
    case OS::Darwin:
      return C(Apple, C11);
    case OS::WASI:
      // TODO: IDK.
      return std::nullopt;
    }
  }

  const std::variant<std::string, Vendor> vendor;
  const Standard standard;

  C(std::variant<std::string, Vendor> vendor, Standard);
};

const std::unordered_map<std::string, C::Vendor> C::vendor_map = {
    {"iso", Vendor::ISO},
    {"gcc", Vendor::GNU},
    {"msvc", Vendor::Microsoft},
    {"clang", Vendor::Apple},
};

const std::unordered_map<std::string, C::Standard> C::std_map = {
    {"ansi", Standard::ANSI},
    {"c99", Standard::C99},
    {"c11", Standard::C11},
    {"c18", Standard::C18},
};

namespace Executable {
enum Format {
  ELF,   ///> ELF (Unix)
  PE,    ///> Portable Executable (Windows)
  MachO, ///> Mach object (MacOS, iOS)
  WASM,  ///> Native WebAssembly module
};

inline static Format default_for(OS os) {
  switch (os) {
  case OS::Linux:
  case OS::BSD:
    return ELF;
  case OS::Windows:
    return PE;
  case OS::Darwin:
    return MachO;
  case OS::WASI:
    return WASM;
  }
}

const std::unordered_map<std::string, Format> format_map = {
    {"elf", ELF},
    {"pe", PE},
    {"macho", MachO},
    {"wasm", WASM},
};
} // namespace Executable

namespace Debug {
struct DWARF {
  enum class Version { V3, V4, V5 };
  enum class Extensions { GDB, LLDB };
};
}; // namespace Debug

struct Target {
  const std::variant<ISA> isa;
  const std::unordered_set<
      std::variant<X86::ISE, ARM::ISE, WASM::ISE>>
      ise;

  /// OS id and its optional version string.
  const std::optional<std::tuple<OS, std::optional<std::string>>> os;

  const std::optional<C> c;

  /// Would not be set in documentation generation mode (?).
  const std::optional<Executable::Format> file_format;

  // Ditto.
  const std::optional<std::variant<Debug::DWARF>> debug;
};

}; // namespace Target
} // namespace FNX
