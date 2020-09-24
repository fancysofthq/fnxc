#pragma once

#include "../unit.hpp"

namespace FNX {
namespace CNX {

/**
 * @brief A constant C expression met in Onyx context.
 *
 * For example, `${FOO}` or `${42ull}`.
 */
class Constexpr : public FNX::Unit {};

} // namespace CNX
} // namespace FNX
