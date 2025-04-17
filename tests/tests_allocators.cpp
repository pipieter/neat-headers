#include "../neat_allocators.hpp"
#include "../neat_test.hpp"

void test_small_ints(void) {
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

int main() {
    NEAT_TEST_RUN(test_small_ints);

    NEAT_TEST_PRINT_STATS();
}