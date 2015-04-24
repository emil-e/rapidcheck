Shrinkable<T>
=============

`Shrinkable<T>` is a fundamental template class in RapidCheck. An instance of `Shrinkable<T>` represents some value of type `T` and provides a way to access all the ways in which this value can be shrunk. It has value semantics which means that it can be copied and passed around just like any other value. It has two member functions:

- `T value() const` returns the represented value of this `Shrinkable`.
- `Seq<Shrinkable<T>> shrinks() const` returns a `Seq` (see [the documentation for `Seq`](Seq.md) for more information) of all the possible ways of shrinking the value from the smallest to the largest.

There are two important things to note here:

- `value()` is a member function, not a member variable. This means that the way that the `Shrinkable` obtains the value `T` that it returns is abstracted away. If this wasn't the case, we would have to impose the following restrictions on `T`:
  - `T` would need to have a copy constructor. Otherwise, we would only be able to retrieve `T` once. This means we couldn't generate, for example, `std::unique_ptr`s to objects. Non-copyable objects are very common in mainstream C++ code so this would be undesirable.
  - `T`'s copy constructor would have to produce copies that shared no state with the original value. Without this guarantee, we would not be able to repeatedly provide semantically equivalent objects since the consumer of `T` might modify the copy which could also modify the original. The obvious example of a type which certainly doesn't provide the non-shared-state guarantee is `std::shared_ptr` whose entire purpose is having a copy constructor that yields copies with shared state.
- `shrinks()` returns a `Seq` of `Shrinkable<T>` and not just `T`. This means that a `Shrinkable<T>` is a value of `T` combined with a tree of possible ways of shrinking it, not just a list. This allows RapidCheck to _recursively_ search this for the smallest value value satisfying some condition (usually failing the property).

TODO
