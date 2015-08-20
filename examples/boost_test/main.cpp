#define BOOST_TEST_MODULE main
#include <boost/test/included/unit_test.hpp>
#include <rapidcheck/boost_test.h>

RC_BOOST_PROP(copyOfStringIsIdenticalToOriginal, (const std::string &str)) {
  RC_CLASSIFY(str.empty());
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}

// Should obviously fail
RC_BOOST_PROP(dividingByTenMakesAllNumbersEqual, (int a, int b)) {
  RC_ASSERT((a / 10) == (b / 10));
}

// If you don't have any arguments, you have to have an empty paren
RC_BOOST_PROP(inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}
