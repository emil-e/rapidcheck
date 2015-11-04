#define BOOST_TEST_MODULE main
#include <boost/test/included/unit_test.hpp>
#include <rapidcheck/boost_test.h>

BOOST_AUTO_TEST_SUITE(RapidCheckExample)

// Should succeed:
RC_BOOST_PROP(copyOfStringIsIdenticalToOriginal, (const std::string &str)) {
  RC_CLASSIFY(str.empty());
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}

// Should obviously fail:
RC_BOOST_PROP(dividingByTenMakesAllNumbersEqual, (int a, int b)) {
  RC_ASSERT((a / 10) == (b / 10));
}

// If you don't have any arguments, you have to have an empty paren:
RC_BOOST_PROP(inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}

// You can also create fixtures...
class MyFixture {
protected:
  MyFixture()
      : counter(0) {}

  void increment() { counter++; }

  std::size_t counter;
};

// ...and use them like this:
RC_BOOST_FIXTURE_PROP(shouldInstantiateFixtureOnEachRun,
                      MyFixture,
                      (const std::vector<int> &ints)) {
  for (std::size_t i = 0; i < ints.size(); i++) {
    increment();
  }

  RC_ASSERT(counter == ints.size());
}

// A normal Boost test can use the same fixture:
BOOST_FIXTURE_TEST_CASE(incrementIncrementsByOne, MyFixture) {
  BOOST_REQUIRE_EQUAL(0U, counter);
  increment();
  BOOST_REQUIRE_EQUAL(1U, counter);
}

BOOST_AUTO_TEST_SUITE_END()
