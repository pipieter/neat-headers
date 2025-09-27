#include <cstdint>
#include <neat/types.hpp>
#include <type_traits>

struct A {
    int a;
};

struct B {
    float bf;
    char  bc;
};

class C {
};

class D : C {
    std::tuple<int, int, char> dt;
};

int main() {
    // is_one_of
    static_assert(neat::types::is_one_of<int, double, float, int>);
    static_assert(neat::types::is_one_of<char*, double, char*, int>);
    static_assert(not neat::types::is_one_of<int>);
    static_assert(not neat::types::is_one_of<int, double, float>);
    static_assert(not neat::types::is_one_of<void*, double, float, int*>);
    static_assert(not neat::types::is_one_of<int, const int, int&, int*>);

    // is_subset_of
    static_assert(neat::types::is_subset_of<C, C>);
    static_assert(neat::types::is_subset_of<std::tuple<int, char>, std::tuple<char, int, A>>);
    static_assert(neat::types::is_subset_of<std::tuple<int, int>, std::tuple<char, int, B>>);
    static_assert(neat::types::is_subset_of<std::tuple<>, std::tuple<C>>);
    static_assert(not neat::types::is_subset_of<C, D>);
    static_assert(not neat::types::is_subset_of<std::tuple<int, char>, std::tuple<char, double>>);

    // are_all_classes
    static_assert(neat::types::are_all_classes<A, B, C, D>);
    static_assert(neat::types::are_all_classes<A, A, A>);
    static_assert(neat::types::are_all_classes<>);
    static_assert(not neat::types::are_all_classes<int, A, D>);
    static_assert(not neat::types::are_all_classes<A*>);

    // get_index
    static_assert(neat::types::get_index<int, char, bool, int>() == 2);
    static_assert(neat::types::get_index<C, char, C, D>() == 1);
    static_assert(neat::types::get_index<C, C>() == 0);
    static_assert(neat::types::get_index<A, B, C, D>() == -1);
    static_assert(neat::types::get_index<A>() == -1);
    static_assert(neat::types::get_index<A, const A, A&, A*>() == -1);
    static_assert(neat::types::get_index<A, B, A, A, A>() == 1);

    // is_derived_from
    static_assert(neat::types::is_derived_from<D, C>);
    static_assert(not neat::types::is_derived_from<C, D>);
    static_assert(not neat::types::is_derived_from<D, A>);
    static_assert(not neat::types::is_derived_from<D, int>);

    // are_all_same
    static_assert(neat::types::are_all_same<int, int, int, int>);
    static_assert(neat::types::are_all_same<int>);
    static_assert(neat::types::are_all_same<unsigned char, uint8_t>);
    static_assert(not neat::types::are_all_same<int, int, int&, int>);
    static_assert(not neat::types::are_all_same<int, int*, int, int>);
    static_assert(not neat::types::are_all_same<int, const int, int, int>);

    // remove_all_t
    static_assert(std::is_same_v<neat::types::remove_all_t<int&>, int>);
    static_assert(std::is_same_v<neat::types::remove_all_t<const int>, int>);
    static_assert(std::is_same_v<neat::types::remove_all_t<const volatile int*&>, int>);
    static_assert(std::is_same_v<neat::types::remove_all_t<int>, int>);
    static_assert(not std::is_same_v<neat::types::remove_all_t<char>, int>);
    static_assert(not std::is_same_v<neat::types::remove_all_t<unsigned int>, int>);

    // are_same_underlying_types
    static_assert(neat::types::are_same_underlying_types<int&, int>);
    static_assert(neat::types::are_same_underlying_types<const int, int>);
    static_assert(neat::types::are_same_underlying_types<const volatile int* const&, int&>);
    static_assert(neat::types::are_same_underlying_types<int, int>);
    static_assert(not neat::types::are_same_underlying_types<char, int>);
    static_assert(not neat::types::are_same_underlying_types<unsigned int, int>);

    return 0;
}