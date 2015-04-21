RapidCheck [![Build Status](https://travis-ci.org/emil-e/rapidcheck.svg?branch=master)](https://travis-ci.org/emil-e/rapidcheck)
==========
RapidCheck is a C++ framework for property based testing inspired by QuickCheck and other similar frameworks. In property based testing, you state facts about your code that given certain precondition should always be true. RapidCheck then generates random test data to try and find a case for which the property doesn't hold. If such a case is found, RapidCheck tries to find the smallest case (for some definition of smallest) for which the property is still false and then displays this as a counterexample. For example, if the input is an integer, RapidCheck tries to find the smallest integer for which the property is false.

A first example
===============
A common first example is testing a list reversal function. For such a function, double reversal should always result in the original list. In this example we will use the standard C++ `std::reverse` function:

    rc::prop(
        "double reversal yields the original value",
        [] (const std::vector<int> &l0) {
            auto l1(l0);
            std::reverse(l1.begin(), l1.end());
            std::reverse(l1.begin(), l1.end());
            RC_ASSERT(l0 == l1);
        });

The `prop` function is used to check properties. The first parameter is a string which describes the property. The second parameter is a callable object that implements the property, in this case a lambda. Any parameters to the callable (in our case the `l0` parameter) will be randomly generated. The `RC_ASSERT` macro works just like any other assert macro. If the given condition is false, the property has been falsified.

The property above also forms part of a specification of the reversal function: "For any list of integers A, reversing and then reversing again should result in A".

Generators
==========
To generate input data for properties, RapidCheck uses the concept of generators. A generator is a type `Gen<T>` where the type parameter `T` is the type of the values generator by that generator.

RapidCheck has several built-in generators, all of which have factory functions in the `rc::gen` namespace. The most basic one is the `arbitrary<T>` generator which generates completely arbitrary values (i.e. all possible values of that type may be generated) of type `T`. This is also the generator used for generating arguments to the property function. To use a specific generator for generating input data, you can use the `*` operator. This way you can also have one generator depend on the value of a previously generated value. Here's an example using the `arbitrary` and `ranged` generators:

    int max = *rc::gen::arbitrary<int>();
    int i = *rc::gen::inRange<int>(0, max);

In this example, an arbitrary integer will first be chosen followed by an integer between 0 and `max` (exclusive).

Using this style, our very first example could be rewritten like this:

    rc::prop(
        "double reversal yields the original value",
        [] {
            auto l0 = *rc::gen::arbitrary<std::vector<int>>();
            auto l1(l0);
            std::reverse(l1.begin(), l1.end());
            std::reverse(l1.begin(), l1.end());
            RC_ASSERT(l0 == l1);
        });

The above example is semantically equivalent to our initial example. However, this is only because there is only one generated value. If more than one value is generated using `*`, RapidCheck assumes that every generated value depends on the values that were generated before it. This puts limitations on the ways that RapidCheck can safely shrink the generated values. Because of this, if your values are actually independant, it makes sense to put them in the arguments if possible.
