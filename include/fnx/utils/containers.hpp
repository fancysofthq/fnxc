#include <unordered_map>

namespace FNX {
namespace Utils {

// Inverse a map, swapping keys and values.
template <typename K, typename V>
std::unordered_map<V, K>
inverse_map(const std::unordered_map<K, V> &map);

} // namespace Utils
} // namespace FNX
