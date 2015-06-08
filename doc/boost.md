Boost type support
==================
RapidCheck comes with support for (currently, a very limited set of) common Boost library types. This support is available through the `extras/boost` module. You can either directly add the `extras/boost/include` directory to your include path or link against the `rapidcheck_boost` target in your `CMakeLists.txt`. You can then simply `#include <rapidcheck/boost.h>`.

The Boost support is currently very limited so if you feel that you're missing some very essential type, please file an issue. Even better, submit a pull request complete with tests!

## Arbitrary ##
The following types currently have `Arbitrary` specializations:
- `boost::optional<T>`

## Generators reference ##
All the Boost generators are located in the `rc::gen::boost` namespace.

#### `Gen<boost::optional<T>> optional(Gen<T> gen)` ####
Equivalent to `gen::maybe` but for `boost::optional<T>` instead.

```
// Example:
const auto optionalSmallInt = *gen::boost::optional(gen::inRange(0, 100));
```
