#include <cassert>

#include "../neat_ecs.hpp"
#include "../neat_test.hpp"

struct A {
    int a = 0;
};

struct B {
    int b = 0;
};

struct C {
    int c = 0;
};

using ecs = neat::ecs::engine<A, B, C>;

void test_deleted_entity_no_longer_exists() {
    ecs ecs;

    auto e1 = ecs.entities.create();
    auto e2 = ecs.entities.create();
    auto e3 = ecs.entities.create();
    auto e4 = ecs.entities.create();
    auto e5 = ecs.entities.create();
    auto e6 = ecs.entities.create();

    NEAT_TEST_ASSERT(ecs.entities.remove(e2));
    NEAT_TEST_ASSERT(ecs.entities.remove(e4));
    NEAT_TEST_ASSERT(ecs.entities.remove(e5));

    NEAT_TEST_ASSERT(ecs.entities.exists(e1));
    NEAT_TEST_ASSERT(ecs.entities.exists(e3));
    NEAT_TEST_ASSERT(ecs.entities.exists(e6));

    NEAT_TEST_ASSERT(not ecs.entities.exists(e2));
    NEAT_TEST_ASSERT(not ecs.entities.exists(e4));
    NEAT_TEST_ASSERT(not ecs.entities.exists(e5));
}

void test_deleted_entity_cant_be_deleted_again() {
    ecs ecs;

    auto e = ecs.entities.create();

    NEAT_TEST_ASSERT(ecs.entities.remove(e));
    NEAT_TEST_ASSERT(not ecs.entities.exists(e));
    NEAT_TEST_ASSERT(not ecs.entities.remove(e));
    NEAT_TEST_ASSERT(not ecs.entities.exists(e));
}

void test_get_component_returns_same() {
    ecs ecs;

    auto e = ecs.entities.create();

    ecs.components.add<A>(e, 100);
    NEAT_TEST_ASSERT(ecs.components.get<A>(e)->a == 100);

    ecs.components.get<A>(e)->a = 50;
    NEAT_TEST_ASSERT(ecs.components.get<A>(e)->a == 50);
}

int main() {
    NEAT_TEST_RUN(test_deleted_entity_no_longer_exists);
    NEAT_TEST_RUN(test_deleted_entity_cant_be_deleted_again);
    NEAT_TEST_RUN(test_get_component_returns_same);

    NEAT_TEST_PRINT_STATS();

    return 0;
}