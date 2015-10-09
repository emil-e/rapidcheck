#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "detail/MapParser.h"
#include "detail/ParseException.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("parseMap") {
  SECTION("parses empty string (or only whitespace) as empty map") {
    REQUIRE(parseMap("").empty());
    REQUIRE(parseMap("\n\t   ").empty());
  }

  SECTION("parses basic maps") {
    auto map = parseMap("foo=bar baz=1337");
    std::map<std::string, std::string> expected = {{"foo", "bar"},
                                                   {"baz", "1337"}};

    REQUIRE(map == expected);
  }

  SECTION("ignores whitespace") {
    auto map = parseMap(" \n  foo =   bar \t  baz =  1337 ");
    std::map<std::string, std::string> expected = {{"foo", "bar"},
                                                   {"baz", "1337"}};

    REQUIRE(map == expected);
  }

  SECTION("parses keys not followed by '=' as missing value") {
    auto map = parseMap("foo=bar baz");
    REQUIRE(map["baz"] == "");
  }

  SECTION("parses empty values") {
    auto map = parseMap("foo=bar baz=");
    REQUIRE(map["baz"] == "");
  }

  SECTION("parses double quoted string values") {
    auto map = parseMap("foo=bar baz=\"testing 1 2 3 \"  stuff=cool");
    REQUIRE(map["baz"] == "testing 1 2 3 ");
  }

  SECTION("parses single quoted string values") {
    auto map = parseMap("foo=bar baz='testing 1 2 3 ' stuff=cool");
    REQUIRE(map["baz"] == "testing 1 2 3 ");
  }

  SECTION("allows escaping of quotes in double quoted strings") {
    auto map = parseMap("foo=bar baz=\"quote \\\"some\\\" stuff\" stuff=cool");
    REQUIRE(map["baz"] == "quote \"some\" stuff");
  }

  SECTION("allows escaping of quotes in single quoted strings") {
    auto map = parseMap("foo=bar baz='quote \\'some\\' stuff' stuff=cool");
    REQUIRE(map["baz"] == "quote 'some' stuff");
  }

  SECTION("allows escaping of backslash in double quoted strings") {
    auto map = parseMap("foo=bar baz=\"this\\\\that\" stuff=cool");
    REQUIRE(map["baz"] == "this\\that");
  }

  SECTION("allows escaping of backslash in single quoted strings") {
    auto map = parseMap("foo=bar baz='this\\\\that' stuff=cool");
    REQUIRE(map["baz"] == "this\\that");
  }

  SECTION("allows escaping of normal characters in double quoted string") {
    auto map = parseMap("foo=bar baz=\"\\f\\o\\o\\b\\a\\r\" stuff=cool");
    REQUIRE(map["baz"] == "foobar");
  }

  SECTION("allows escaping of normal characters in single quoted string") {
    auto map = parseMap("foo=bar baz='\\f\\o\\o\\b\\a\\r' stuff=cool");
    REQUIRE(map["baz"] == "foobar");
  }

  SECTION("allows double quoted keys") {
    auto map = parseMap(" \"\\\"this=\\\\=that\\\"\"=foobar");
    REQUIRE(map["\"this=\\=that\""] == "foobar");
  }

  SECTION("allows single quoted keys") {
    auto map = parseMap(" '\\'this=\\\\=that\\''=foobar");
    REQUIRE(map["'this=\\=that'"] == "foobar");
  }

  SECTION("if there are duplicate keys, the last value takes precedence") {
    auto map = parseMap("foo=this foo=that");
    REQUIRE(map["foo"] == "that");
  }

  SECTION("throws ParseException on unterminated double quoted string") {
    REQUIRE_THROWS_AS(parseMap("foo=\"bar"), ParseException);
  }

  SECTION("throws ParseException on unterminated single quoted string") {
    REQUIRE_THROWS_AS(parseMap("foo='bar"), ParseException);
  }
}

TEST_CASE("mapToString") {
  prop("parseMap(mapToString(x)) == x",
       [](const std::map<std::string, std::string> &map) {
         RC_PRE(map.find("") == map.end());
         RC_ASSERT(parseMap(mapToString(map)) == map);
       });

  SECTION("does not quote strings that do not contains special chars") {
    std::map<std::string, std::string> map{{"foobar", "barfoo"}};

    REQUIRE(mapToString(map) == "foobar=barfoo");
  }

  SECTION("quotes strings that contain whitespace") {
    std::map<std::string, std::string> map{{"foo bar", "bar\tfoo"}};

    REQUIRE(mapToString(map) == "'foo bar'='bar\tfoo'");
  }

  SECTION("quotes strings that contain '='") {
    std::map<std::string, std::string> map{{"foo=bar", "bar=foo"}};

    REQUIRE(mapToString(map) == "'foo=bar'='bar=foo'");
  }

  SECTION("if string contains single quotes") {
    std::map<std::string, std::string> map{{"foo'bar", "bar'foo"}};

    SECTION("quotes and escapes if using single quoting") {
      REQUIRE(mapToString(map, false) == "'foo\\'bar'='bar\\'foo'");
    }
    SECTION("quotes but does not escape if using double quoting") {
      REQUIRE(mapToString(map, true) == "\"foo'bar\"=\"bar'foo\"");
    }
  }

  SECTION("if string contains double quotes") {
    std::map<std::string, std::string> map{{"foo\"bar", "bar\"foo"}};

    SECTION("quotes and escapes if using double quoting") {
      REQUIRE(mapToString(map, true) == "\"foo\\\"bar\"=\"bar\\\"foo\"");
    }
    SECTION("quotes but does not escape if using single quoting") {
      REQUIRE(mapToString(map, false) == "'foo\"bar'='bar\"foo'");
    }
  }

  SECTION("quotes and escapes strings that contains backslashes") {
    std::map<std::string, std::string> map{{"foo\\bar", "bar\\foo"}};

    REQUIRE(mapToString(map) == "'foo\\\\bar'='bar\\\\foo'");
  }
}
