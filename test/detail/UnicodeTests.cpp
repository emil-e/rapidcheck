#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Text.h"

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

#include "util/Util.h"
#include "util/Meta.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::unicodeCodepoint") {
	prop("never generates null characters and always within range of valid unicode",
		[](const GenParams &params) {

		const auto gen = gen::unicodeCodepoint<uint32_t>();
		onAnyPath(
			gen(params.random, params.size),
			[](const Shrinkable<uint32_t> &value, const Shrinkable<uint32_t> &shrink) {
			RC_ASSERT(shrink.value() > 0u && shrink.value() < 0x10FFFFu);
		});
	});

	prop("first shrink is always 'a')",
		[](const GenParams &params) {
		const auto gen = gen::unicodeCodepoint<uint32_t>();
		const auto shrinkable = gen(params.random, params.size);
		RC_PRE(shrinkable.value() != 'a');
		RC_ASSERT(shrinkable.shrinks().next()->value() == 'a');
	});
}

TEST_CASE("detail::makeCharacterUtf8") {

	// Test known values
	auto char1 = rc::detail::makeCharacterUtf8<std::string>(0x00C4u);
	REQUIRE(char1.size() == 2u);
	REQUIRE(char1[0] == static_cast<char>(0xC3u));
	REQUIRE(char1[1] == static_cast<char>(0x84u));
	auto char2 = rc::detail::makeCharacterUtf8<std::string>(0x0034u);
	REQUIRE(char2[0] == static_cast<char>(0x34u));
	REQUIRE(char2.size() == 1u);

	prop("Only results in valid utf8 characters",
		[](const GenParams &params) {
		const auto gen = gen::unicodeCodepoint<uint32_t>();

		// Test some known values

		onAnyPath(
			gen(params.random, params.size),
			[](const Shrinkable<uint32_t> &value, const Shrinkable<uint32_t> &shrink) {
			auto utf8 = rc::detail::makeCharacterUtf8<std::string>(shrink.value());

			RC_ASSERT(utf8.size() > 0u);
			size_t expectedBytes = 1;
			if ((utf8[0] & 0b10000000) == 0)
			{
				expectedBytes = 1;
			}
			else if ((utf8[0] & 0b11100000) == 0b11000000)
			{
				expectedBytes = 2;
			}
			else if ((utf8[0] & 0b11110000) == 0b11100000)
			{
				expectedBytes = 3;
			}
			else if ((utf8[0] & 0b11111000) == 0b11110000)
			{
				expectedBytes = 4;
			}
			else
			{
				RC_ASSERT(false);
			}
			RC_ASSERT(utf8[0] != 0u);
			for (size_t i = 1; i < expectedBytes; ++i)
			{
				RC_ASSERT((utf8[i] & 0b11000000) == 0b10000000);
			}
		});
	});
}