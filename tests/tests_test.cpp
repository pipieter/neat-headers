
#include <cstring>

#include "../neat_test.hpp"

void test_success_1() {
    NEAT_TEST_ASSERT(3 == 3);
    NEAT_TEST_ASSERT_EQ(5, 5);
}

void test_success_2() {
    NEAT_TEST_ASSERT(true);
    NEAT_TEST_ASSERT_EQ(1 + 2, 3);
}

void test_failure() {
    NEAT_TEST_ASSERT(false);
    NEAT_TEST_ASSERT_EQ(1, 2);
}

int main(int argc, const char** argv) {
    bool failure = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--failure") == 0) {
            failure = true;
        }
    }

    NEAT_TEST_RUN(test_success_1);
    NEAT_TEST_RUN(test_success_2);
    if (failure) {
        NEAT_TEST_RUN(test_failure);
    }

    NEAT_TEST_PRINT_STATS();
}