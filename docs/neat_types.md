# Neat types

Extensions to the STL type_traits library, meant to be used in static_asserts to ensure argument correctness.

# API

In the examples given below, the capital letter arguments are distinct types, structs, or classes.

## `neat::types::is_one_of<T, Us...>`

Checks if T is any of the other given types.

```C++
neat::types::is_any_of<B, A, B, C>         == true;
neat::types::is_any_of<D, A, B, C>         == false;
neat::types::is_any_of<A, const A, A&, A*> == false;
neat::types::is_any_of<A>                  == false;
```

## `neat::types::is_subset_of<std::tuple<Ts...>, std::tuple<Us...>>`

Checks if the types in the first argument is a subset of the types in the second argument. The arguments provided should be given as `std::tuples`. The amount of duplicates in the first or second arguments are not taken into account, e.g. in case `int` appears twice in the first argument but only once in the second argument, this will still count as a valid subset.

The `std::tuple` syntax is required to differentiate between the subset and set.

```C++
neat::types::is_subset_of<std::tuple<>, std::tuple<A>>           == true;
neat::types::is_subset_of<std::tuple<A, B>, std::tuple<A, B, C>> == true;
neat::types::is_subset_of<std::tuple<A, A>, std::tuple<A, B>>    == true;
neat::types::is_subset_of<std::tuple<A, B>, std::tuple<B, C>>    == false;
neat::types::is_subset_of<std::tuple<A>, std::tuple<A*, A&>>     == false;
```

## `neat::types::are_all_classes<Ts...>`

Checks if all given arguments are structs or classes.

```C++
neat::types::are_all_classes<A, B, C> == true;
neat::types::are_all_classes<A, A, A> == true;
neat::types::are_all_classes<>        == true;
neat::types::are_all_classes<A*>      == false;
neat::types::are_all_classes<A, int>  == false;
```

## `neat::types::get_index<T, Us...>()`

Gets the index if the first type in the other types, starting the count from zero. In case the type is not present in the other types, returns -1. In case the type appears multiple times, the first index is returned

```C++
neat::types::get_index<A, A, B, C>() ==  0;
neat::types::get_index<B, A, B, C>() ==  1;
neat::types::get_index<B, A, B, B>() ==  1;
neat::types::get_index<D, A, B, C>() == -1;
neat::types::get_index<A>()          == -1;
neat::types::get_index<A, A&, A*>()  == -1;
```

## `neat::types::is_derived_from<T, U>`

Checks if the first type is derived from the second type.

## `neat::types::are_all_same<T...>`

Checks if all given types are the same type.

```C++
neat::types::are_all_same<int, int, int>   == true;
neat::types::are_all_same<int>             == true;
neat::types::are_all_same<int, char>       == false;
neat::types::are_all_same<int, int*, int&> == false
```

## `neat::types::remove_all_t<T>`

Removes all qualifiers, references, and pointers from a type.

```C++
neat::types::remove_all_t<int>                  == int;
neat::types::remove_all_t<int&>                 == int;
neat::types::remove_all_t<const volatile int*&> == int;
```

## `neat::types::are_same_underlying_types<T, U>`

Checks if two types are identical after removing qualifiers, references, and pointers as determined by `neat::types::remove_all`.

```C++
neat::types::are_same_underlying_types<int, int>                         == true;
neat::types::are_same_underlying_types<int&, int>                        == true;
neat::types::are_same_underlying_types<const volatile int* const&, int&> == true;
neat::types::are_same_underlying_types<unsigned int, int>                == false;
```