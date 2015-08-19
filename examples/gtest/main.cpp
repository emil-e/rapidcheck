#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

RC_GTEST_PROP(MyTestCase,
              copyOfStringIsIdenticalToOriginal,
              (const std::string &str)) {
  RC_CLASSIFY(str.empty());
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}

// Should obviously fail
RC_GTEST_PROP(MyTestCase, dividingByTenMakesAllNumbersEqual, (int a, int b)) {
  RC_ASSERT((a / 10) == (b / 10));
}

// If you don't have any arguments, you have to have an empty paren
RC_GTEST_PROP(MyTestCase, inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
