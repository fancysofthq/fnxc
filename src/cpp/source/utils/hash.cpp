#include "../../header/utils/hash.hpp"

namespace std {
size_t hash<std::experimental::filesystem::path>::
operator()(std::experimental::filesystem::path const path) const {
  return std::hash<string>{}(path.generic_u8string());
}
} // namespace std
