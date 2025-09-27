# Neat allocators

Single-header library for specialized memory allocators.

# Example usage

```C++
#include <neat/allocators.hpp>

int main() {
    neat::allocators::arena<int> arena(4); // Allocates enough memory for four ints
    int* a =  arena.allocate(); // Success
    int* b =  arena.allocate(); // Success
    int* c =  arena.allocate(); // Success
    int* d =  arena.allocate(); // Success
    int* e =  arena.allocate(); // Failure, nullptr

    // At function end, arena is destroyed and allocated memory is free
    return 0;
}
```