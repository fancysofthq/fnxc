#pragma once

#include <filesystem>

#include "../unit.hpp"

namespace FNX {
namespace C {

// A C source file.
class File : FNX::Unit {
public:
  File(std::filesystem::path);
};

} // namespace C
} // namespace FNX
