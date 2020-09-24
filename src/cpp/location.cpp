#include "../../include/fnx/location.hpp"

namespace FNX {

Location::Location(Position begin, Position end) :
    begin(begin), end(end) {}

Location::Location(Position pos) : begin(pos), end(pos) {}

} // namespace FNX
