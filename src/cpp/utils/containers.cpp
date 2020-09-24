#include "../../../include/fnx/utils/containers.hpp"

namespace FNX {
namespace Utils {

template <typename K, typename V>
std::unordered_map<V, K> inverse_map(std::unordered_map<K, V> &map) {
  std::unordered_map<V, K> inv;
  std::for_each(
      map.begin(), map.end(), [&inv](const std::pair<K, V> &p) {
        inv.insert(std::make_pair(p.second, p.first));
      });
  return inv;
}

} // namespace Utils
} // namespace FNX
