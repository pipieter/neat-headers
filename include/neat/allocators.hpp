#ifndef NEAT_ALLOCATORS_HPP_
#define NEAT_ALLOCATORS_HPP_

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#ifndef NEAT_ALLOCATORS_MALLOC
#define NEAT_ALLOCATORS_MALLOC std::malloc
#endif  // NEAT_ALLOCATORS_MALLOC

#ifndef NEAT_ALLOCATORS_REALLOC
#define NEAT_ALLOCATORS_REALLOC std::realloc
#endif  // NEAT_ALLOCATORS_REALLOC

#ifndef NEAT_ALLOCATORS_FREE
#define NEAT_ALLOCATORS_FREE std::free
#endif  // NEAT_ALLOCATORS_FREE

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

class bump {
   private:
    struct block {
        uint8_t*    data;
        uint8_t*    current;
        std::size_t remaining;
    };

    block*      _blocks;
    std::size_t _block_count;
    std::size_t _block_capacity;
    std::size_t _block_size;

    void* add_block();

   public:
    bump(std::size_t block_size = 4096);
    ~bump();

    void*                    allocate(std::size_t size);
    template <typename T> T* allocate();
    std::size_t              block_count() const;
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
        NEAT_ALLOCATORS_FREE(_begin);
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

#pragma region bump implementations
inline neat::allocators::bump::bump(std::size_t size) {
    _blocks         = nullptr;
    _block_count    = 0;
    _block_capacity = 0;
    _block_size     = size;
}

inline neat::allocators::bump::~bump() {
    for (size_t i = 0; i < _block_count; i++) {
        NEAT_ALLOCATORS_FREE(_blocks[i].data);
    }
    NEAT_ALLOCATORS_FREE(_blocks);
}

inline void* neat::allocators::bump::allocate(std::size_t size) {
    if (size > _block_size)
        return nullptr;

    for (size_t i = 0; i < _block_count; i++) {
        if (_blocks[i].remaining >= size) {
            void* ptr = (void*)_blocks[i].current;
            _blocks[i].current += size / sizeof(uint8_t);
            _blocks[i].remaining -= size;
            return ptr;
        }
    }

    block* new_block = (block*)add_block();
    if (!new_block)
        return nullptr;
    void* ptr = new_block->current;
    new_block->current += size / sizeof(uint8_t);
    new_block->remaining -= size;
    return ptr;
}

template <typename T>
T* neat::allocators::bump::allocate() {
    return (T*)allocate(sizeof(T));
}

inline void* neat::allocators::bump::add_block() {
    // Add new block capacity
    while (_block_count >= _block_capacity) {
        size_t new_capacity = _block_capacity == 0 ? 1 : (_block_capacity * 2ull);
        block* new_blocks   = (block*)NEAT_ALLOCATORS_REALLOC(_blocks, sizeof(block) * new_capacity);
        if (new_blocks == nullptr) {
            return nullptr;
        } else {
            _block_capacity = new_capacity;
            _blocks         = new_blocks;
        }
    }

    uint8_t* data = (uint8_t*)NEAT_ALLOCATORS_MALLOC(_block_size);
    if (!data) {
        return nullptr;
    }
    block* new_block     = &_blocks[_block_count];
    new_block->data      = data;
    new_block->current   = data;
    new_block->remaining = _block_size;
    _block_count++;
    return new_block;
}

inline std::size_t neat::allocators::bump::block_count() const {
    return _block_count;
}

#pragma endregion bump implementations

#endif  // NEAT_ALLOCATORS_HPP_