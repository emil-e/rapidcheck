`Seq<T>`
=======
_This section is incomplete._

`Seq<T>` implements a lazy sequence (often called a "stream") of values of type `T`. It has a super simple interface for retrieving these values consisting only of the method `Maybe<T> next()`. As can be seen, it returns a `Maybe<T>` which is similar to `boost::optional`, Haskell's `Maybe` or other similar types that either contain a value or be empty. `next` successively returns the values in the sequence until the sequence is exhausted after which it will return an empty `Maybe` to signal that there are no more values.

`Seq`s are mutable since calling `next` will modify the `Seq` by advancing it. They also have value semantics since you can copy them and pass them around like any other value.
