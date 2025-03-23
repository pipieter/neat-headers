#ifndef NEAT_TEST_HPP_
#define NEAT_TEST_HPP_

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string_view>

#define NEAT_TEST_RUN(func)       neat::test::run((func), (#func))
#define NEAT_TEST_ASSERT(b)       neat::test::assert_true((b), __FILE__, __LINE__)
#define NEAT_TEST_ASSERT_EQ(a, b) neat::test::assert_eq((a), (b), __FILE__, __LINE__)
#define NEAT_TEST_PRINT_STATS()   neat::test::stats()

namespace neat::test {

inline void                run(void (&func)(), const char* name);
inline void                assert_true(bool value, const char* file, int line);
template <typename T> void assert_eq(T value, T expected, const char* file, int line);

inline void stats();

namespace colors {

const std::string_view black  = "\e[0;30m";
const std::string_view blue   = "\e[0;34m";
const std::string_view cyan   = "\e[0;36m";
const std::string_view green  = "\e[0;32m";
const std::string_view purple = "\e[0;35m";
const std::string_view red    = "\e[0;31m";
const std::string_view white  = "\e[0;37m";
const std::string_view yellow = "\e[0;33m";
const std::string_view reset  = "\e[0m";

};  // namespace colors

static struct {
    size_t tests_run         = 0;
    size_t asserts_run       = 0;
    size_t asserts_failed    = 0;
    size_t asserts_succeeded = 0;
} data;

}  // namespace neat::test

#pragma region implementations

inline void neat::test::run(void (&func)(), const char* file) {
    std::cout << colors::green << "Running " << file << colors::reset << std::endl;
    data.tests_run++;
    func();
}

inline void neat::test::assert_true(bool value, const char* file, int line) {
    data.asserts_run++;
    if (!value) {
        std::cout << colors::red << "- Assertion failed " << file << ":" << line << colors::reset << std::endl;
        data.asserts_failed++;
    } else {
        data.asserts_succeeded++;
    }
}

template <typename T> void neat::test::assert_eq(T value, T expected, const char* file, int line) {
    data.asserts_run++;
    if (value != expected) {
        std::cout << colors::red << "- neat::test::assert_eq failed " << file << ":" << line
                  << ", expected " << expected << ", received " << value << colors::reset << std::endl;
        data.asserts_failed++;
    } else {
        data.asserts_succeeded++;
    }
}

inline void neat::test::stats() {
    auto summary_color = data.asserts_succeeded == data.asserts_run ? colors::green : colors::red;

    double success_rate = std::round(100.0 * (double)data.asserts_succeeded / (double)data.asserts_run);
    std::cout << colors::yellow
              << "Tests run: " << data.tests_run << std::endl
              << "Assertions made:   " << std::setw(6) << data.asserts_run << std::endl
              << "Assertions passed: " << std::setw(6) << data.asserts_succeeded << std::endl
              << "Assertions failed: " << summary_color << std::setw(6) << data.asserts_failed << colors::yellow << std::endl
              << "Success rate:      " << summary_color << std::setw(6) << success_rate << "%" << colors::reset << std::endl;
}

#pragma endregion implementations

#endif  // NEAT_TEST_HPP_