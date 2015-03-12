RapidCheck [![Build Status](https://travis-ci.org/emil-e/rapidcheck.svg?branch=master)](https://travis-ci.org/emil-e/rapidcheck)
==========
RapidCheck is a C++ framework for property based testing inspired by QuickCheck and other similar frameworks. In property based testing, you state facts about some piece of code that should always be true given certain preconditions. RapidCheck then generates random test data to try and find a case for which the property doesn't hold. If such a case is found, RapidCheck tries to find the smallest case (for some definition of smallest) for which the property is not true. For example, if the input is an integer, RapidCheck tries to find the smallest integer for which the property is false.

A first example
===============
A common example is testing a list reversal function. For such a function, double reversal should always result in the original list. In this example we will use the standard C++ `std::reverse` function:

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
To generate input data for properties, RapidCheck uses the concept of generators. Every generator must inherit from `rc::gen::Generator<T>` where `T` is the generated type. A generator must at least implement the `generate` member function which simply returns a randomly generated value of `T`.

RapidCheck has several built-in generators, all of which have factory functions in the `rc::gen` namespace. The most basic one is the `arbitrary<T>` generator which generates completely arbitrary values (i.e. all possible values of that type may be generated) of type `T`. This is also the generator used for generating arguments to the property function. To use a specific generator for generating input data, you can use the `rc::pick` function which takes a generator and returns a generated value. This way you can also have one generator depend on the value of a previously generated value. Here's an example using the `arbitrary` and `ranged` generators:

    int max = rc::pick(rc::gen::arbitrary<int>());
    int i = rc::pick(rc::gen::ranged<int>(0, max));

In this example, an arbitrary integer will first be chosen followed by an integer between 0 and `max` (exclusive). There is also a version of pick which does not take any arguments. This version simply uses `arbitrary<T>` which means the first line in the example above could also be written like this:

    int max = rc::pick<int>();

Using this style, our very first example could be rewritten like this:

    rc::prop(
        "double reversal yields the original value",
        [] {
            auto l0(rc::pick<std::vector<int>>());
            auto l1(l0);
            std::reverse(l1.begin(), l1.end());
            std::reverse(l1.begin(), l1.end());
            RC_ASSERT(l0 == l1);
        });

This is semantically equivalent to the first version so which style to use is a matter of preference. An advantage of having input data as parameters is that it makes them easier to spot but the drawback is that you cannot use generators other than `arbitrary<T>` this way.

Please note that for RapidCheck to work properly, you must always use `rc::pick` when using a generator. If you call `generate` directly, shrinking  (see the section on shrinking for more information) will not work properly.

Implementing generators
-----------------------
To create your own generator, you simply inherit from `rc::gen::Generator<T>` (where `T` is the generated type) and implement the `generate` member function. For example, let's say we want to make a generator which generates strings which contains the sequence "foobar" repeated a number of times:

    public Foobar : public rc::gen::Generator<std::string>
    {
    public:
        std::string generate() const override
        {
            int times = rc::pick(rc::gen::ranged<int>(1, 10));
            std::string value;
            for (int i = 0; i < times; i++)
                value += "foobar";
            return value;
        }
    };

This generator can now be used like this:

    std::string foobar = rc::pick(Foobar());

Specializing `Arbitrary<T>`
---------------------------
Behind the scenes `arbitrary<T>` simply returns an instance of the generator Arbitrary<T>. The factory function `arbitrary<T>` is included for consistency with the rest of the generator factories. If you want to use custom types in your properties, it might be a good idea to specialize `Arbitrary<T>` for your own types. This way, you can use the short hand `rc::pick<T>()` and also have your type be generated as arguments to property functions.

Let's say we have a type which represents a person with a first name, a last name and an age:

    struct Person {
        std::string firstName;
        std::string lastName;
        int age;
    };

We can specialize `Arbitrary<T>` so that `arbitrary<T>()` returns a generator of arbitrary values of `Person`:

    template<>
    class Arbitrary<Person> : public rc::gen::Arbitrary<Person>
    {
    public:
        Person generate() const override
        {
            Person person;
            person.firstName = rc::pick<std::string>();
            person.lastName = rc::pick<std::string>();
            person.age = rc::pick(rc::gen::ranged<int>(0, 100));
            return person;
        }
    };

`Person` can now be used in a property which tests the equality and inequality operators:

    rc::prop(
        "equality is the opposite of inequality",
        [] (Person p1, Person p2) {
            RC_ASSERT((p1 == p2) == !(p1 != p2));
        });

 Since, for example, `arbitrary<std::map<K, V>>` uses `arbitrary<K>` and `arbitrary<V>` to generate its values, we can even do the following:

    rc::prop(
        "equality is the opposite of inequality",
        [] (const std::map<int, Person> &m1, const std::map<int, Person> &m2) {
            RC_ASSERT((m1 == m2) == !(m1 != m2));
        });

Size
----
Properties and generators in RapidCheck are run with an implicit parameter the determines the size of the values that are generated by the generators used. For most generators, this parameter determines the _maximum_ size of the value that is generated, i.e. the maximum magnitude of a numeric value or the maximum length of a string. By default, RapidCheck starts with a size of zero and gradually increases this. This has several advantages over simply generating large values all the time:

- Some bugs might only found when all values are small.
- A small size results in more duplicates and collissions. A small integer might be between `-1` and `1` which limits the selection to only three different values.
- It's computationally cheaper to generate small values. If values are expensive to generate and/or check, a bug will be found faster if it also manifests at smaller sizes.

The current size can be accessed by calling `rc::gen::currentSize()`. The minimum size is `0` and for most generators, this should result in only trivial values (i.e. empty lists, zero numeric values). There is no upper limit to the size but `kNominalSize` is used as a reference value and is defined to be `100`. For some generators, there is a natural upper limit (i.e. 255 when generating bytes) to the size of the values that can be generated so this might be a good size at which to "max out". However, for something like a container, it is a very bad idea to max out at the nominal value since the maximum size of an `std::vector` is extremely large. For the built in container generators, the size is used directly as the 

The size can also be changed for a given generator using the `rc:gen::resize` and `rc::gen:scale` combinators. For example, it might sometimes be a good idea to reduce the size of generated strings for performance reasons:

    using namespace rc;
    using namespace rc::gen;
    auto elementGenerator = scale(0.25, arbitrary<std::string>());
    auto stringVector = pick(collection<std::vector<std::string>>(elementGenerator));

The string generator will see a size which is one quarter of the original size.

Generator requirements
----------------------
A valid generator must follow a few rules:

- `generate` must only use other generators using `rc::pick` as a source of randomness. For example, it may not use `rand()` to generate random numbers. RapidCheck depends on the fact that it has complete control over what is returned from `rc::pick`.
- Member functions of a generator must not have any visible side effects.
- T must be either copy-constructible/copy-assignable or move-constructible/move-assignable.
- A generator must be copy-constructible and copy-assignable. Given the constraints above, the default copy-constructor and copy-assignment operator should suffice for most implementations.

Shrinking
=========
When RapidCheck finds a failing test case, it tries to find the most trivial case for which the property still fails. This process is known as shrinking and the intention is to make it easier to spot the exact bug just by looking at the counter-example. It is much easier to guess why an algorithm fails on a list of two elements than it is to guess why it fails on a list of one hundred elements if the bug is simply that it breaks when the list contains duplicates.

To shrink a generated value, RapidCheck calls the `shrink` member function of the generator that originally generated the value and passes the value that should be shrunk. This function should return a `std::unique_ptr` to an instance of `rc::shrink::Iterator<T>` where `T` is the shrunk type. For convenience, the typedef `rc::shrink::IteratorUP<T>` is provided for such a pointer.

`Iterator<T>` is an abstract base class template and subclasses must implement the `hasNext` which returns true and `next` member functions. `next` returns the next possible shrink of the start value and `hasNext` returns `true` or `false` depending on whether there are more possible values of if all values have been exhausted. The values returned by `next` does not have to be values that also fail the property but they must be values that could have been generated by the generator. For exmaple, a generator which generates only even integers should not return an iterator which suggests odd integers.

To simplify implementation of the `shrink` member function, the `rc::shrink` namespace provides a number of common implementations and combinators so that you don't have to subclass `Iterator<T>` yourself most of the time. The default implementation of `shrink` returns `rc::shrink::nothing<T>()` which is simply an iterator for which `hasNext` always returns false.

Implicit shrinking
------------------
The iterator returned by `shrink` implements what is referred to as "explicit shrinking". This is because the author of the generator explicitly defines how generated values may be shrunk. However, before calling `shrink`, RapidCheck will try to shrink the "sub-values" that make up the generated value. This behavior is enabled by default and is therefore referred to as "implicit shrinking".

Let's take this type and associated generator as an example:

    struct IntPair {
        int a, b;
    };

    template<>
    class Arbitrary<IntPair> : public rc::gen::Generator<IntPair>
    {
    public:
        IntPair generate() const override
        {
            IntPair pair;
            pair.a = rc::pick<int>();
            pair.b = rc::pick<int>();
            return pair;
        }
    };

When generating, the `Arbitrary<IntPair>` generator picks two arbitrary `int`s which make up the final value. When shrinking values from this generator, RapidCheck will first try to shrink `a` until fully exhausted and then try to shrink `b` until fully exhausted. Finally, RapidCheck will call `shrink` and try to shrink the value using this iterator. In the case of this generator, `shrink` has not been implemented and the iterator will thus not return any shrinks. However, good thing is that we get basic shrinking for free!

By default, RapidCheck will try to shrink every value returned by a call to `pick` and most of the time, this is what you want.

Some notes
----------
Conceptually, shrinking is an optimization problem where we want to find the most trivial argument set (the global minimum) which still fails the property. However, what makes a value "trivial" is entirely dependant on the context in which it is used. 42 is smaller than 100 but if the number represents a percentage, one could argue that 100% percent is a more trivial value than 42%. Furthermore, there is no guarantee that we actually find the global minimum, we might only find a local minimum.

As such, shrinking should be viewed as a best effort process intended to make the actual bug easier to spot in the counter-example, not a mathematical solver which yields some objective minimum. What is an appropriate shrinking strategy for a given generator depends entirely on how the value will be used, what bugs are to expected, and so forth.

Here are some tips to implement effective shrinking:

- When RapidCheck finds an acceptable value, it will not try any further values returned by the iterator. For this reason, more extreme suggestions should be returned before more modest suggestions to reduce the number of iterations required to find the minimum. For example, when shrinking the integer `10`, `0` should be suggested before `9` and when shrinking a container, an empty container should be suggested before one where some value is smaller.
- The sequence of values do not always have to be exhaustive. When RapidCheck finds an acceptable value, it will repeat the shrinking process until the value cannot be shrunk anymore. Take the example of shrinking an unsigned integer value. If we want to shrink an unsigned integer `x`, we could simply suggest `x - 1`. If the minimum value is `0`, it will eventually be reached but it might take a lot of iterations. However, if we do not also suggest the immediate value, we might miss some possible shrinking.
- Sometimes, the shrinking should be more exhaustive. When shrinking a list, a possible shrink might be to remove three consecutive elements but if we only remove one element at a time, we might miss this.

Displaying counterexamples
==========================
TBA

Stateful testing
================
TBA
