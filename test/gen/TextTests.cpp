#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Text.h"

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

#include "util/Util.h"
#include "util/Meta.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::character") {
  prop("never generates null characters",
       [](const GenParams &params) {
         const auto gen = gen::character<char>();
         onAnyPath(
             gen(params.random, params.size),
             [](const Shrinkable<char> &value, const Shrinkable<char> &shrink) {
               RC_ASSERT(shrink.value() != '\0');
             });
       });

  prop("first shrink is always 'a')",
       [](const GenParams &params) {
         const auto gen = gen::character<char>();
         const auto shrinkable = gen(params.random, params.size);
         RC_PRE(shrinkable.value() != 'a');
         RC_ASSERT(shrinkable.shrinks().next()->value() == 'a');
       });
}

struct StringProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("length is smaller or equal to size",
                     [](const GenParams &params) {
                       const auto gen = gen::string<T>();
                       const auto shrinkable = gen(params.random, params.size);
                       RC_ASSERT(shrinkable.value().size() <=
                                 static_cast<std::size_t>(params.size));
                     });

    templatedProp<T>(
        "finds minimum where string must be longer than a certain length",
        [](const Random &random) {
          const auto n = *gen::inRange<std::size_t>(0, 10);
          const auto size = *gen::inRange<int>(50, 100);
          const auto result =
              searchGen(random,
                        size,
                        gen::string<T>(),
                        [=](const T &x) { return x.size() >= n; });
          T expected(n, 'a');
          RC_ASSERT(result == expected);
        });

    templatedProp<T>("first shrink is empty",
                     [](const GenParams &params) {
                       const auto gen = gen::string<T>();
                       const auto shrinkable = gen(params.random, params.size);
                       RC_PRE(!shrinkable.value().empty());
                       RC_ASSERT(shrinkable.shrinks().next()->value().empty());
                     });

    templatedProp<T>(
        "the size of each shrink is the same or smaller than the original",
        [](const GenParams &params) {
          onAnyPath(
              gen::string<T>()(params.random, params.size),
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(containerSize(shrink.value()) <=
                          containerSize(value.value()));
              });
        });

    templatedProp<T>("none of the shrinks equal the original value",
                     [](const GenParams &params) {
                       onAnyPath(gen::string<T>()(params.random, params.size),
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != shrink.value());
                                 });
                     });
  }
};

struct StringUtf8Properties {
	template <typename T>
	static void exec() {
		templatedProp<T>("length is at most four times to size",
			[](const GenParams &params) {
			const auto gen = gen::stringUtf8<T>();
			const auto shrinkable = gen(params.random, params.size);
			RC_ASSERT(shrinkable.value().size() <=
				static_cast<std::size_t>(params.size) * 4);
		});

		templatedProp<T>("first shrink is empty",
			[](const GenParams &params) {
			const auto gen = gen::stringUtf8<T>();
			const auto shrinkable = gen(params.random, params.size);
			RC_PRE(!shrinkable.value().empty());
			RC_ASSERT(shrinkable.shrinks().next()->value().empty());
		});

		templatedProp<T>(
			"the size of each shrink is the same or smaller than the original",
			[](const GenParams &params) {
			onAnyPath(
				gen::stringUtf8<T>()(params.random, params.size),
				[](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
				RC_ASSERT(containerSize(shrink.value()) <=
					containerSize(value.value()));
			});
		});

		templatedProp<T>("none of the shrinks equal the original value",
			[](const GenParams &params) {
			onAnyPath(gen::stringUtf8<T>()(params.random, params.size),
				[](const Shrinkable<T> &value,
					const Shrinkable<T> &shrink) {
				RC_ASSERT(value.value() != shrink.value());
			});
		});
	}
};

struct CodepointContainerProperties {
	template <typename T>
	static void exec() {
		templatedProp<T>("length is at most size",
			[](const GenParams &params) {
			const auto gen = gen::unicodeCodepoints<T>();
			const auto shrinkable = gen(params.random, params.size);
			RC_ASSERT(shrinkable.value().size() <=
				static_cast<std::size_t>(params.size));
		});

		templatedProp<T>(
			"finds minimum where container must be longer than a certain length",
			[](const Random &random) {
			const auto n = *gen::inRange<std::size_t>(0, 10);
			const auto size = *gen::inRange<int>(50, 100);
			const auto result =
				searchGen(random,
					size,
					gen::unicodeCodepoints<T>(),
					[=](const T &x) { return x.size() >= n; });
			T expected(n, 'a');
			RC_ASSERT(result == expected);
		});

		templatedProp<T>("first shrink is empty",
			[](const GenParams &params) {
			const auto gen = gen::unicodeCodepoints<T>();
			const auto shrinkable = gen(params.random, params.size);
			RC_PRE(!shrinkable.value().empty());
			RC_ASSERT(shrinkable.shrinks().next()->value().empty());
		});

		templatedProp<T>(
			"the size of each shrink is the same or smaller than the original",
			[](const GenParams &params) {
			onAnyPath(
				gen::unicodeCodepoints<T>()(params.random, params.size),
				[](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
				RC_ASSERT(containerSize(shrink.value()) <=
					containerSize(value.value()));
			});
		});

		templatedProp<T>("none of the shrinks equal the original value",
			[](const GenParams &params) {
			onAnyPath(gen::unicodeCodepoints<T>()(params.random, params.size),
				[](const Shrinkable<T> &value,
					const Shrinkable<T> &shrink) {
				RC_ASSERT(value.value() != shrink.value());
			});
		});
	}
};


TEST_CASE("gen::string") {
  forEachType<StringProperties, std::string, std::wstring>();
}

TEST_CASE("gen::stringUtf8") {
	forEachType<StringUtf8Properties, std::string, std::wstring>();
}

TEST_CASE("gen::unicodeCodepoints") {
	forEachType<CodepointContainerProperties, std::vector<uint32_t>, std::vector<int64_t>>();
}



