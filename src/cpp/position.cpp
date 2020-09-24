#include "../../include/fnx/position.hpp"

namespace FNX {

Position::Position(uint32_t row, uint32_t col, uint32_t offset) :
    row(row), col(col), offset(offset) {}

Position::Position() {}

} // namespace FNX
