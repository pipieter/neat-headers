#include <cassert>

#include "../neat_ecs.hpp"

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

    assert(ecs.entities.remove(e2));
    assert(ecs.entities.remove(e4));
    assert(ecs.entities.remove(e5));

    assert(ecs.entities.exists(e1));
    assert(ecs.entities.exists(e3));
    assert(ecs.entities.exists(e6));

    assert(not ecs.entities.exists(e2));
    assert(not ecs.entities.exists(e4));
    assert(not ecs.entities.exists(e5));
}

void test_deleted_entity_cant_be_deleted_again() {
    ecs ecs;

    auto e = ecs.entities.create();

    assert(ecs.entities.remove(e));
    assert(not ecs.entities.exists(e));
    assert(not ecs.entities.remove(e));
    assert(not ecs.entities.exists(e));
}

void test_get_component_returns_same() {
    ecs ecs;

    auto e = ecs.entities.create();

    ecs.components.add<A>(e, 100);
    assert(ecs.components.get<A>(e)->a == 100);

    ecs.components.get<A>(e)->a = 50;
    assert(ecs.components.get<A>(e)->a == 50);
}

int main() {
    test_deleted_entity_no_longer_exists();
    test_deleted_entity_cant_be_deleted_again();
    test_get_component_returns_same();

    return 0;
}