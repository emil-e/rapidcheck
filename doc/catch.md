# Catch test integration

rapidcheck comes with some basic integrations for the catch test library.

## Usage

This support is available through the `extras/catch` module. You can either
directly add the `extras/gtest/inclode` directory to your include path:

```cmake
add_subdirectory(rapidcheck)
set include_directories(rapidcheck/extras/gtest/include)
add_executable(MyTest main.cpp)
```

Or link against the `rapidcheck_catch` cmake target:

```cmake
add_subdirectory(rapidcheck)
add_executable(MyTest main.cpp)
target_link_libraries(MyTest rapidcheck_catch)
```

Once you've done one of these, you can simply:

```cpp
#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h'
```

## Reference

### `rc::prop("My test description", []{return true;}, /*verbose=*/true)`

The `rc::prop` can be used in place of `SECTION` for convenient checking of
properties. The third parameter is optional and defaults to `false`.

```cpp
TEST_CASE("001: My first test case")
{
    rc::prop("My property description",
        [](int a, int b)
        {
            // rapidcheck will produce the `a` and `b`
            return (a + b) == (b + a); // return true for passed test, false for
                                       // failed test
        });

    // no problem mixing rapidcheck tests with regular catch assertions
    SECTION("Normal catch stuff")
    {
        WHEN("Something happens")
        {
            THEN("assert a result")
            {
                REQUIRE(1 == 1);
            }
        }
    }
}
```
