#ifndef NEAT_ALLOCATORS_HPP_
#define NEAT_ALLOCATORS_HPP_

#include <cstddef>
#include <cstdlib>
#include <vector>

#ifndef NEAT_ALLOCATORS_MALLOC
#define NEAT_ALLOCATORS_MALLOC std::malloc
#endif  // NEAT_ALLOCATORS_MALLOC

#ifndef NEAT_ARENA_FREE
#define NEAT_ARENA_FREE std::free
#endif  // NEAT_ARENA_FREE

namespace neat::allocators {

template <typename T>
class arena {
   private:
    T*   _begin;
    T*   _end;
    T*   _current;
    bool _failure;

   public:
    arena(std::size_t count);
    ~arena();

    T*   allocate();
    bool failure() const;
};

}  // namespace neat::allocators

#pragma region arena implementations

template <typename T>
neat::allocators::arena<T>::arena(std::size_t count) {
    _begin = (T*)NEAT_ALLOCATORS_MALLOC(sizeof(T) * count);
    if (_begin == nullptr) {
        _end     = nullptr;
        _current = nullptr;
        _failure = true;
    } else {
        _end     = _begin + count;
        _current = _begin;
        _failure = false;
    }
}

template <typename T>
neat::allocators::arena<T>::arena::~arena() {
    if (_begin != nullptr) {
        NEAT_ARENA_FREE(_begin);
    }
}

template <typename T>
T* neat::allocators::arena<T>::arena::allocate() {
    if (_current == _end) {
        _current = nullptr;
    }
    if (_current == nullptr) {
        _failure = true;
        return nullptr;
    }

    T* ptr = _current;
    _current++;
    return ptr;
}

template <typename T>
bool neat::allocators::arena<T>::arena ::failure() const {
    return _failure;
}

#pragma endregion arena implementations

#endif  // NEAT_ALLOCATORS_HPP_