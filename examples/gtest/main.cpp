#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

// Should succeed:
RC_GTEST_PROP(MyTestCase,
              copyOfStringIsIdenticalToOriginal,
              (const std::string &str)) {
  RC_CLASSIFY(str.empty());
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}

// Should obviously fail:
RC_GTEST_PROP(MyTestCase, dividingByTenMakesAllNumbersEqual, (int a, int b)) {
  RC_ASSERT((a / 10) == (b / 10));
}

// If you don't have any arguments, you have to have an empty paren:
RC_GTEST_PROP(MyTestCase, inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}

// You can also create fixtures...
class MyFixture : public ::testing::Test {
protected:
  MyFixture()
      : counter(0) {}

  void SetUp() override {
    // SetUp works as usual...
  }

  void increment() { counter++; }

  void TearDown() override {
    // ...as does TearDown
  }

  int counter;
};

// ...and use them like this:
RC_GTEST_FIXTURE_PROP(MyFixture,
                      shouldInstantiateFixtureOnEachRun,
                      (const std::vector<int> &ints)) {
  for (const auto x : ints) {
    increment();
  }

  RC_ASSERT(counter == ints.size());
}

// A normal Google test can use the same fixture:
TEST_F(MyFixture, incrementIncrementsByOne) {
  ASSERT_EQ(0, counter);
  increment();
  ASSERT_EQ(1, counter);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
