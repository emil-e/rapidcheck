#include <catch2/catch.hpp>

#include "rapidcheck/detail/ShowType.h"

#include "util/ShowTypeTestUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

TEST_CASE("typeToString") {
  SECTION("shows primitive types correctly") {
    REQUIRE(typeToString<void>() == "void");

    REQUIRE(typeToString<bool>() == "bool");

    REQUIRE(typeToString<char>() == "char");
    REQUIRE(typeToString<unsigned char>() == "unsigned char");

    REQUIRE(typeToString<short>() == "short");
    REQUIRE(typeToString<unsigned short>() == "unsigned short");

    REQUIRE(typeToString<int>() == "int");
    REQUIRE(typeToString<unsigned int>() == "unsigned int");

    REQUIRE(typeToString<long>() == "long");
    REQUIRE(typeToString<unsigned long>() == "unsigned long");

    REQUIRE(typeToString<long long>() == "long long");
    REQUIRE(typeToString<unsigned long long>() == "unsigned long long");
  }

  SECTION("uses a ShowType specialization if there is one") {
    REQUIRE(typeToString<Foo>() == "FFoo");
    REQUIRE(typeToString<Bar>() == "BBar");
    REQUIRE(typeToString<Baz>() == "BBaz");
  }

  SECTION("shows 'decorated' types correctly") {
    REQUIRE(typeToString<Foo *>() == "FFoo *");
    REQUIRE(typeToString<Foo **>() == "FFoo **");
    REQUIRE(typeToString<Foo ****>() == "FFoo ****");
    REQUIRE(typeToString<Foo &>() == "FFoo &");
    REQUIRE(typeToString<Foo &&>() == "FFoo &&");
    REQUIRE(typeToString<const Foo>() == "const FFoo");
    REQUIRE(typeToString<volatile Foo>() == "volatile FFoo");
    REQUIRE(typeToString<const Foo **>() == "const FFoo **");
    REQUIRE(typeToString<const volatile Foo *&>() == "const volatile FFoo * &");
  }

  SECTION("non-specialized types") {
    REQUIRE(typeToString<NonspecializedStruct>() ==
            "rc::test::NonspecializedStruct");
    REQUIRE(typeToString<NonspecializedClass>() ==
            "rc::test::NonspecializedClass");
    REQUIRE(typeToString<NonspecializedEnum>() ==
            "rc::test::NonspecializedEnum");
    REQUIRE(typeToString<NonspecializedEnumClass>() ==
            "rc::test::NonspecializedEnumClass");

    REQUIRE(typeToString<NonspecializedTemplate<NonspecializedStruct>>() ==
            "rc::test::NonspecializedTemplate<rc::test::NonspecializedStruct>");
    REQUIRE(typeToString<NonspecializedStruct *>() ==
            "rc::test::NonspecializedStruct *");
    REQUIRE(typeToString<const NonspecializedStruct *>() ==
            "const rc::test::NonspecializedStruct *");
  }

  SECTION("std::string") {
    REQUIRE(typeToString<std::string>() == "std::string");
  }
  SECTION("std::wstring") {
    REQUIRE(typeToString<std::wstring>() == "std::wstring");
  }
  SECTION("std::u16string") {
    REQUIRE(typeToString<std::u16string>() == "std::u16string");
  }
  SECTION("std::u32string") {
    REQUIRE(typeToString<std::u32string>() == "std::u32string");
  }

  SECTION("std::vector") {
    REQUIRE(typeToString<std::vector<Foo *>>() == "std::vector<FFoo *>");
  }
  SECTION("std::deque") {
    REQUIRE(typeToString<std::deque<Foo *>>() == "std::deque<FFoo *>");
  }
  SECTION("std::forward_list") {
    REQUIRE(typeToString<std::forward_list<Foo *>>() ==
            "std::forward_list<FFoo *>");
  }
  SECTION("std::list") {
    REQUIRE(typeToString<std::list<Foo *>>() == "std::list<FFoo *>");
  }

  SECTION("std::set") {
    REQUIRE(typeToString<std::set<Foo *>>() == "std::set<FFoo *>");
  }
  SECTION("std::map") {
    using MapType = std::map<Foo *, Bar **>;
    REQUIRE(typeToString<MapType>() == "std::map<FFoo *, BBar **>");
  }

  SECTION("std::multiset") {
    REQUIRE(typeToString<std::multiset<Foo *>>() == "std::multiset<FFoo *>");
  }
  SECTION("std::multimap") {
    using MapType = std::multimap<Foo *, Bar **>;
    REQUIRE(typeToString<MapType>() == "std::multimap<FFoo *, BBar **>");
  }

  SECTION("std::unordered_set") {
    REQUIRE(typeToString<std::unordered_set<Foo *>>() ==
            "std::unordered_set<FFoo *>");
  }
  SECTION("std::unordered_map") {
    using MapType = std::unordered_map<Foo *, Bar **>;
    REQUIRE(typeToString<MapType>() == "std::unordered_map<FFoo *, BBar **>");
  }

  SECTION("std::unordered_multiset") {
    REQUIRE(typeToString<std::unordered_multiset<Foo *>>() ==
            "std::unordered_multiset<FFoo *>");
  }
  SECTION("std::unordered_multimap") {
    using MapType = std::unordered_multimap<Foo *, Bar **>;
    REQUIRE(typeToString<MapType>() ==
            "std::unordered_multimap<FFoo *, BBar **>");
  }

  SECTION("std::array") {
    using ArrayType = std::array<Bar **, 1337>;
    REQUIRE(typeToString<ArrayType>() == "std::array<BBar **, 1337>");
  }

  SECTION("std::stack") {
    REQUIRE(typeToString<std::stack<Bar **>>() == "std::stack<BBar **>");
  }

  SECTION("std::pair") {
    using PairType = std::pair<Foo *, Bar **>;
    REQUIRE(typeToString<PairType>() == "std::pair<FFoo *, BBar **>");
  }

  SECTION("std::tuple") {
    using Tuple0 = std::tuple<>;
    using Tuple1 = std::tuple<Foo *>;
    using Tuple2 = std::tuple<Foo *, Bar **>;
    using Tuple3 = std::tuple<Foo *, Bar **, const volatile Baz &>;
    REQUIRE(typeToString<Tuple0>() == "std::tuple<>");
    REQUIRE(typeToString<Tuple1>() == "std::tuple<FFoo *>");
    REQUIRE(typeToString<Tuple2>() == "std::tuple<FFoo *, BBar **>");
    REQUIRE(typeToString<Tuple3>() ==
            "std::tuple<FFoo *, BBar **, const volatile BBaz &>");
  }

  SECTION("std::unique_ptr") {
    using UP2 = std::unique_ptr<Foo *, Bar>;
    REQUIRE(typeToString<std::unique_ptr<Foo *>>() ==
            "std::unique_ptr<FFoo *>");
    REQUIRE(typeToString<UP2>() == "std::unique_ptr<FFoo *>");
  }

  SECTION("std::shared_ptr") {
    REQUIRE(typeToString<std::shared_ptr<const Foo *>>() ==
            "std::shared_ptr<const FFoo *>");
  }
}
