# Neat test

Small testing framework designed for the tests in this collection.

# API

All exposed functions use macros, as C++ is currently unable to stringify names without their use. The following functions are available:

```C++
NEAT_TEST_RUN(function);
NEAT_TEST_ASSERT(boolean);
NEAT_TEST_ASSERT_EQ(a, b);
NEAT_TEST_PRINT_STATS();
```

`NEAT_TEST_RUN` expects a function without any parameters.
