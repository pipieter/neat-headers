#include <cmath>
#include <neat/math.hpp>
#include <neat/test.hpp>

void test_abs(void) {
    NEAT_TEST_ASSERT_EQ(neat::math::abs(500), 500);
    NEAT_TEST_ASSERT_EQ(neat::math::abs(-500), 500);
    NEAT_TEST_ASSERT_EQ(neat::math::abs(-1.00), 1.00);
}

void test_approach(void) {
    NEAT_TEST_ASSERT_EQ(neat::math::approach(50, 100, 25), 75);
    NEAT_TEST_ASSERT_EQ(neat::math::approach(100, 60, 20), 80);
    NEAT_TEST_ASSERT_EQ(neat::math::approach(100u, 60u, 20u), 80u);
    NEAT_TEST_ASSERT_EQ(neat::math::approach(100.0, 60.0, 20.0), 80.0);
}

void test_collide(void) {
    NEAT_TEST_ASSERT(neat::math::collide(neat::math::point<float> {1, 1}, neat::math::rectangle<float> {0, 0, 2, 2}));
    NEAT_TEST_ASSERT(neat::math::collide(neat::math::circle<float> {3, 1, 2}, neat::math::rectangle<float> {0, 0, 2, 2}));

    NEAT_TEST_ASSERT(not neat::math::collide(neat::math::point<float> {3, 1}, neat::math::rectangle<float> {0, 0, 2, 2}));
    NEAT_TEST_ASSERT(not neat::math::collide(neat::math::circle<float> {3, 1, 1}, neat::math::rectangle<float> {0, 0, 2, 2}));
}

void test_smoothstep_inverse(void) {
    for (float t = -2.00; t < 2.00; t += 0.01) {
        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::inverse::cosine(neat::math::smoothstep::cosine(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));
        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::inverse::linear(neat::math::smoothstep::linear(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));
        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::inverse::cubic(neat::math::smoothstep::cubic(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));

        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::cosine(neat::math::smoothstep::inverse::cosine(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));
        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::linear(neat::math::smoothstep::inverse::linear(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));
        NEAT_TEST_ASSERT(neat::math::equals(neat::math::smoothstep::cubic(neat::math::smoothstep::inverse::cubic(t)), neat::math::clamp<float>(t, 0, 1), 1e-4));
    }
}

int main() {
    NEAT_TEST_RUN(test_abs);
    NEAT_TEST_RUN(test_collide);
    NEAT_TEST_RUN(test_approach);
    NEAT_TEST_RUN(test_smoothstep_inverse);

    NEAT_TEST_PRINT_STATS();
}