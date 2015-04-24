Generators
==========
To generate input data for properties, RapidCheck uses the concept of generators. A generator which generates values of type `T` has the type `Gen<T>`.

RapidCheck has several built-in generators, all of which have factory functions in the `rc::gen` namespace. The most basic one is the `arbitrary<T>` generator which generates completely arbitrary values (i.e. all possible values of that type may be generated) of type `T`. This is also the generator used for generating arguments to the property function. To use a specific generator for generating input data, you can use the `*` operator. This way you can also have one generator depend on the value of a previously generated value. Here's an example using the `arbitrary` and `inRange` generators:

    int max = *rc::gen::arbitrary<int>();
    int i = *rc::gen::inRange<int>(0, max);

In this example, an arbitrary integer will first be chosen followed by an integer between 0 and `max` (exclusive).

Using this style, our very first example could be rewritten like this:

    rc::check("double reversal yields the original value",
        [] {
          auto l0 = *rc::gen::arbitrary<std::vector<int>>();
          auto l1(l0);
          std::reverse(begin(l1), end(l1));
          std::reverse(begin(l1), end(l1));
          RC_ASSERT(l0 == l1);
        });
