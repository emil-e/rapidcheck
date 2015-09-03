RapidCheck [![Build Status](https://travis-ci.org/emil-e/rapidcheck.svg?branch=master)](https://travis-ci.org/emil-e/rapidcheck) [![Build status](https://ci.appveyor.com/api/projects/status/8hms56ghn27agpcj/branch/master?svg=true)](https://ci.appveyor.com/project/emil-e/rapidcheck/branch/master)
==========
RapidCheck is a C++ framework for property based testing inspired by QuickCheck and other similar frameworks. In property based testing, you state facts about your code that given certain precondition should always be true. RapidCheck then generates random test data to try and find a case for which the property doesn't hold. If such a case is found, RapidCheck tries to find the smallest case (for some definition of smallest) for which the property is still false and then displays this as a counterexample. For example, if the input is an integer, RapidCheck tries to find the smallest integer for which the property is false.

Sounds interesting? Why don't you read the **[User Guide](doc/user_guide.md)** to learn more!

## Why RapidCheck? ##
There are existing implementations of property based testing but the ones that I have found are either (in my humble opinion) a bit clunky or are missing essential features such as test case shrinking.

Let's throw together a list of features:

- Write your properties in an imperative way that makes sense for C++
- Test case shrinking
- Great support for STL types, including maps and sets
- Advanced combinators for creating your own generators
- Stateful based on commands in the vein of Erlang QuickCheck
- Integration with popular testing frameworks such as Boost Test, Google Test and Google Mock

## Prerequisites and installation ##
RapidCheck makes extensive use of C++11 and thus requires a compliant compiler. RapidCheck continuous integration builds using Clang 3.4 and GCC 4.9 and any later versions should also work. MSVC 2013 lacks too many C++11 features to successfully compile RapidCheck but it is possible that MSVC 2015 will work when it's released.

RapidCheck uses CMake and is built like any other CMake project. If your own project uses CMake you can simply have RapidCheck as a subdirectory and add the following to your `CMakeLists.txt`:

    add_subdirectory("path/to/rapidcheck")
    target_link_libraries(my_target rapidcheck)

This will give you both linking and include directories.

## Quick introduction ##
A common first example is testing a reversal function. For such a function, double reversal should always result in the original list. In this example we will use the standard C++ `std::reverse` function:

```C++
#include <rapidcheck.h>

#include <vector>
#include <algorithm>
#include <set>

int main() {
  rc::check("double reversal yields the original value",
            [](const std::vector<int> &l0) {
              auto l1 = l0;
              std::reverse(begin(l1), end(l1));
              std::reverse(begin(l1), end(l1));
              RC_ASSERT(l0 == l1);
            });

  return 0;
}
```

The `check` function is used to check properties. The first parameter is an optional string which describes the property. The second parameter is a callable object that implements the property, in this case a lambda. Any parameters to the callable (in our case the `l0` parameter) will be randomly generated. The `RC_ASSERT` macro works just like any other assert macro. If the given condition is false, the property has been falsified.

The property above also forms part of a specification of the reversal function: "For any list of integers A, reversing and then reversing again should result in A".

If we were to run this, RapidCheck would (hopefully) output the following:

```
Using configuration: seed=9928307433081493900

- double reversal yields the original value
OK, passed 100 tests
```

Here, RapidCheck tells us that it ran 100 test cases and all of them passed. It also tells us the configuration that was used, in particular the random seed. If there was a bug in the implementation of `std::reverse` we could get the following output instead:

```
Falsifiable after 12 tests and 10 shrinks

std::tuple<std::vector<int>>:
([1, 0, 0, 0, 0, 0, 0, 0, 0, 0])

main.cpp:17:
RC_ASSERT(l0 == l1)

Expands to:
[1, 0, 0, 0, 0, 0, 0, 0, 0, 0] == [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
```

Here RapidCheck tells us that it found a case for which the property does not hold after running 12 tests. When it found this case, it shrunk it 10 times to arrive at the counterexample in the output. The counterexample contains each input value that was used for the failing case along with its type. Since RapidCheck views property arguments as tuples, the type is shown here as `std::tuple<std::vector<int>>`.

Can you guess what the bug is? The fact that there are exactly 10 items should give a clue. In this case, the bug is that the implementation sets the first element to `0` when `l0.size() >= 10`. This is also the reason for the initial `0`, the problem doesn't manifest when all elements are zero. How did this bug happen? Who knows!

## Thanks ##
Big thanks to my employer, Spotify, for making it possible for me to spend work time improving RapidCheck.
