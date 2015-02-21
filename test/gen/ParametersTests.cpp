#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

TEST_CASE("gen::resize") {
    prop("changes the generation size",
         [] {
             auto size = *gen::positive<int>();
             auto generator = gen::noShrink(
                 gen::resize(
                     size,
                     gen::lambda([] { return gen::currentSize(); })));
             RC_ASSERT(*generator == size);
         });
}

TEST_CASE("gen::noShrink") {
    prop("sets the NoShrink parameter",
         [] {
             detail::ImplicitParam<detail::param::NoShrink> noShrink(
                 *gen::arbitrary<bool>());
             bool wasNoShrink = *gen::noShrink(gen::lambda([]{
                 return detail::ImplicitParam<detail::param::NoShrink>::value();
             }));
             RC_ASSERT(wasNoShrink);
         });

    prop("blocks explicit shrinking",
         [] {
             auto generator = gen::arbitrary<int>();
             auto value = *generator;
             RC_ASSERT(!gen::noShrink(generator).shrink(value).next());
         });
}
