#include "../neat_allocators.hpp"
#include "../neat_test.hpp"

void test_arena_small_ints(void) {
    neat::allocators::arena<int> arena(4);

    int* a = arena.allocate();
    int* b = arena.allocate();
    int* c = arena.allocate();
    int* d = arena.allocate();

    NEAT_TEST_ASSERT(a != nullptr);
    NEAT_TEST_ASSERT(b != nullptr);
    NEAT_TEST_ASSERT(c != nullptr);
    NEAT_TEST_ASSERT(d != nullptr);
    NEAT_TEST_ASSERT(b == a + 1);
    NEAT_TEST_ASSERT(c == a + 2);
    NEAT_TEST_ASSERT(d == a + 3);
    NEAT_TEST_ASSERT(!arena.failure());

    int* e = arena.allocate();
    NEAT_TEST_ASSERT(e == nullptr);
    NEAT_TEST_ASSERT(arena.failure());
}

void test_bump_small(void) {
    neat::allocators::bump bump1(5);

    int* a = bump1.allocate<int>();
    int* b = bump1.allocate<int>();
    int* c = bump1.allocate<int>();
    int* d = bump1.allocate<int>();
    int* e = bump1.allocate<int>();

    NEAT_TEST_ASSERT(a != nullptr);
    NEAT_TEST_ASSERT(b != nullptr);
    NEAT_TEST_ASSERT(c != nullptr);
    NEAT_TEST_ASSERT(d != nullptr);
    NEAT_TEST_ASSERT(e != nullptr);
    NEAT_TEST_ASSERT(bump1.block_count() == 5);

    neat::allocators::bump bump2(3);

    int* f = bump2.allocate<int>();
    int* g = bump2.allocate<int>();
    int* h = bump2.allocate<int>();

    NEAT_TEST_ASSERT(f == nullptr);
    NEAT_TEST_ASSERT(g == nullptr);
    NEAT_TEST_ASSERT(h == nullptr);
    NEAT_TEST_ASSERT(bump2.block_count() == 0);

    char* i = bump2.allocate<char>();
    char* j = bump2.allocate<char>();
    char* k = bump2.allocate<char>();

    NEAT_TEST_ASSERT(i != nullptr);
    NEAT_TEST_ASSERT(j != nullptr);
    NEAT_TEST_ASSERT(k != nullptr);
    NEAT_TEST_ASSERT(bump2.block_count() == 1);
}

int main() {
    NEAT_TEST_RUN(test_arena_small_ints);
    NEAT_TEST_RUN(test_bump_small);

    NEAT_TEST_PRINT_STATS();
}