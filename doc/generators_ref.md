Generators reference
====================
_This section is incomplete._

The following is a reference of all the included generators of RapidCheck. These generators are accessed by factory functions in the `rc::gen` namespace.

## Basic ##

### `arbitrary<T>()` ###
Generates an arbitrary value of type `T`. Support for new types can be added by specializing `struct rc::Arbitrary<T>` and providing a static method `arbitrary()` which returns an appropriate generator. For more information see the documentation on [generators](generators.md). The semantics of the returned generator depends entirely on the implementation of the `Arbitrary` specialization.

This generator is also used by RapidCheck whenever it implicitly needs a generator for some type, for example when generating arguments to properties.

```C++
// Example:
const auto str = *gen::arbitrary<std::vector<std::string>>();
```

### `tuple(Gen<Ts>... gens)` ###
Generates and `std::tuple` using the given generators to generate elements.

```C++
// Example:
const auto tuple = *gen::tuple(gen::arbitrary<std::string>(),
                               gen::arbitrary<int>(),
                               gen::arbitrary<float>());
```

### `pair(Gen<T1> gen1, Gen<T2> gen2)` ###
Similar to `tuple` but generates `std::pair` instead.

```C++
// Example:
const auto pair = *gen::pair(gen::arbitrary<std::string>(),
                             gen::arbitrary<int>());
```

### `construct<T>(Gen<Args>... gens)` ###
Generates objects of type `T` constructed using arguments from the given generators.

```C++
// Example:
const auto person = *gen::construct<T>(
    gen::arbitrary<std::string>(), // Name
    gen::arbitrary<std::string>(), // Hometown
    gen::inRange(0, 100));         // Age
```
