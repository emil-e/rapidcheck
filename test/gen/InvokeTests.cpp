#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Predictable.h"

using namespace rc;

// Always increasing type for testing generatinon ordering. Note that this is
// very much a hack but it will catch regressions where arguments are not
// generated in the correct order.
struct IncInt { int value; };

namespace rc {

template<>
class Arbitrary<IncInt> : public gen::Generator<IncInt>
{
public:
    IncInt generate() const override
    {
        return IncInt { value()++ };
    }

    static int &value()
    {
        static int value = 0;
        return value;
    }

    static void reset()
    { value() = 0; }
};

} // namespace rc

void show(IncInt x, std::ostream &os) { os << x.value; }

TEST_CASE("gen::lambda") {
    prop("generates the return value of the given callable",
         [] (int x) {
             auto generator = gen::lambda([=] { return x; });
             for (int i = 0; i < gen::currentSize(); i++)
                 RC_ASSERT(*generator == x);
         });
}

TEST_CASE("gen::rescue") {
    prop("converts exceptions to generated values",
         [] (int x) {
             auto generator = gen::lambda([=] {
                 throw std::to_string(x);
                 return std::string("");
             });

             std::string str(
                 *gen::noShrink(
                     gen::rescue<std::string>(
                         generator,
                         [] (const std::string &ex) {
                             return ex;
                         })));

             RC_ASSERT(str == std::to_string(x));
         });
}

TEST_CASE("gen::anyInvocation") {
    prop("generates arguments in listing order",
         [] {
             Arbitrary<IncInt>::reset();
             auto tuple = *gen::noShrink(
                 gen::anyInvocation(
                     [] (IncInt a, IncInt b, IncInt c) {
                         return std::make_tuple(a.value, b.value, c.value);
                     }));

             RC_ASSERT(std::get<0>(tuple) < std::get<1>(tuple));
             RC_ASSERT(std::get<1>(tuple) < std::get<2>(tuple));
         });

    prop("uses the appropriate Arbitrary instance",
         [] {
             auto tuple = *gen::noShrink(
                 gen::anyInvocation(
                     [] (const Predictable &a,
                         Predictable &&b,
                         Predictable c)
             {
                 return std::make_tuple(a.value, b.value, c.value);
             }));
             RC_ASSERT(std::get<0>(tuple) == Predictable::predictableValue);
             RC_ASSERT(std::get<1>(tuple) == Predictable::predictableValue);
             RC_ASSERT(std::get<2>(tuple) == Predictable::predictableValue);
         });

    prop("uses the return value as the generated value",
         [] {
             int x = *gen::noShrink(
                 gen::anyInvocation([] (int a, int b) { return 12345; }));
             RC_ASSERT(x == 12345);
         });
}
