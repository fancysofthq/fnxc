#pragma once

#include <tuple>
#include <variant>

namespace FNX {
namespace Utils {

namespace FlattenVariantImpl {

// A flattened variant implementation by Vittorio Romeo [1], licensed
// under CC BY-SA 3.0. Minor changes were made to adapt the code.
// This file is exemptive from the wrapping FNX license, and is
// licensed under CC BY-SA 3.0 as well.
//
// [1]: https://stackoverflow.com/a/39273356/3645337
//

// Type of the concatenation of all 'Ts...' tuples.
template <typename... Ts>
using cat = decltype(std::tuple_cat(std::declval<Ts>()...));

template <typename TResult, typename... Ts> struct flatten_variant;

// Base case: no more types to process.
template <typename TResult> struct flatten_variant<TResult> {
  using type = TResult;
};

// Case: T is not a variant.
// Return concatenation of previously processed types,
// T, and the flattened remaining types.
template <typename TResult, typename T, typename... TOther>
struct flatten_variant<TResult, T, TOther...> {
  using type =
      cat<TResult,
          std::tuple<T>,
          typename flatten_variant<TResult, TOther...>::type>;
};

// Case: T is a variant.
// Return concatenation of previously processed types,
// the types inside the variant, and the flattened remaining types.
// The types inside the variant are recursively flattened in a new
// flatten_variant instantiation.
template <typename TResult, typename... Ts, typename... TOther>
struct flatten_variant<TResult, std::variant<Ts...>, TOther...> {
  using type =
      cat<TResult,
          typename flatten_variant<std::tuple<>, Ts...>::type,
          typename flatten_variant<TResult, TOther...>::type>;
};

template <typename T> struct to_variant;

template <typename... Ts> struct to_variant<std::tuple<Ts...>> {
  using type = std::variant<Ts...>;
};
} // namespace FlattenVariantImpl

/// Flatten a variant.
///
/// @example utils/flattened_variant.cpp
///
/// @tparam T The flattened variant.
template <typename T>
using flatten_variant_t = typename FlattenVariantImpl::to_variant<
    typename FlattenVariantImpl::flatten_variant<std::tuple<>, T>::
        type>::type;

} // namespace Utils
} // namespace FNX
