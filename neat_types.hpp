#ifndef NEAT_TYPES_HPP_
#define NEAT_TYPES_HPP_

#include <tuple>
#include <type_traits>

namespace neat::types {

template <typename T, typename... Ts> inline constexpr bool is_one_of       = (std::is_same<T, Ts> {} || ...);
template <typename T, typename... Ts> inline constexpr bool are_all_same    = (std::conjunction_v<std::is_same<T, Ts>...>);
template <typename... T> inline constexpr bool              are_all_classes = (std::is_class_v<T> && ...);

template <typename Subset, typename Set> inline constexpr bool       is_subset_of                                            = std::is_same_v<Subset, Set>;
template <typename... Subset, typename... Set> inline constexpr bool is_subset_of<std::tuple<Subset...>, std::tuple<Set...>> = (is_one_of<Subset, Set...> && ...);

template <typename T>
inline consteval long get_index() {
    return -1;
};

template <typename T, typename U, typename... Us>
inline consteval long get_index() {
    if constexpr (!is_one_of<T, U, Us...>)
        return -1;
    else if constexpr (std::is_same_v<T, U>)
        return 0;
    else
        return 1 + get_index<T, Us...>();
}

template <typename T, typename U> inline constexpr bool is_derived_from = (std::is_base_of_v<U, T>);

template <typename T> struct remove_all;
template <typename T> struct remove_all { using type = T; };
template <typename T> struct remove_all<T&> : remove_all<T> {};
template <typename T> struct remove_all<T*> : remove_all<T> {};
template <typename T> struct remove_all<T const> : remove_all<T> {};
template <typename T> struct remove_all<T volatile> : remove_all<T> {};
template <typename T> struct remove_all<T const volatile> : remove_all<T> {};
template <typename T> using remove_all_t = typename remove_all<T>::type;

template <typename T, typename U> inline constexpr bool are_same_underlying_types = (std::is_same_v<remove_all_t<T>, remove_all_t<U>>);

}  // namespace neat::types

#endif  // NEAT_TYPES_HPP_