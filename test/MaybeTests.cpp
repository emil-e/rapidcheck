#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Maybe.h"

#include "util/Logger.h"
#include "util/TemplateProps.h"
#include "util/Generators.h"
#include "util/AppleOrange.h"
#include "util/DestructNotifier.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("Nothing") {
    SECTION("convertible to boolean false") {
        REQUIRE_FALSE(Nothing);
    }
}

TEST_CASE("Maybe") {
    Logger foo("foo");
    Logger bar("bar");

    SECTION("default constructor") {
        SECTION("constructed object is not initialized") {
            REQUIRE_FALSE(Maybe<int>());
        }
    }

    SECTION("NothingType constructor") {
        SECTION("constructed object is not initialized") {
            REQUIRE_FALSE(Maybe<int>(Nothing));
        }
    }

    SECTION("value move constructor") {
        Maybe<Logger> maybe(foo);

        SECTION("initializes object") {
            REQUIRE(maybe);
        }

        SECTION("copy constructs value") {
            maybe->requireExact(
                "constructed as foo",
                "copy constructed");
        }
    }

    SECTION("value move constructor") {
        Maybe<Logger> maybe(std::move(foo));

        SECTION("initializes object") {
            REQUIRE(maybe);
        }

        SECTION("move constructs value") {
            maybe->requireExact(
                "constructed as foo",
                "move constructed");
        }
    }

    SECTION("value copy assignment") {
        SECTION("if uninitialized") {
            Maybe<Logger> maybe;
            maybe = foo;

            SECTION("initializes object") {
                REQUIRE(maybe);
            }

            SECTION("copy constructs value") {
                maybe->requireExact(
                    "constructed as foo",
                    "copy constructed");
            }
        }

        SECTION("if initialized") {
            Maybe<Logger> maybe(foo);
            maybe = bar;

            SECTION("remains initialized") {
                REQUIRE(maybe);
            }

            SECTION("copy assigns value") {
                maybe->requireExact(
                    "constructed as bar",
                    "copy assigned");
            }

            SECTION("self assign leaves self unchanged") {
                maybe = *maybe;
                REQUIRE(maybe->id == "bar");
            }
        }
    }

    SECTION("value move assignment") {
        SECTION("if uninitialized") {
            Maybe<Logger> maybe;
            maybe = std::move(foo);

            SECTION("initializes object") {
                REQUIRE(maybe);
            }

            SECTION("move constructs value") {
                maybe->requireExact(
                    "constructed as foo",
                    "move constructed");
            }
        }

        SECTION("if initialized") {
            Maybe<Logger> maybe(foo);
            maybe = std::move(bar);

            SECTION("remains initialized") {
                REQUIRE(maybe);
            }

            SECTION("move assigns value") {
                maybe->requireExact(
                    "constructed as bar",
                    "move assigned");
            }
        }
    }

    SECTION("NothingType assignment") {
        SECTION("deinitializes object") {
            Maybe<int> maybe(1337);
            maybe = Nothing;
            REQUIRE_FALSE(maybe);
        }
    }

    SECTION("copy constructor") {
        SECTION("constructs empty object if source is empty") {
            Maybe<int> maybe1;
            Maybe<int> maybe2(maybe1);
            REQUIRE_FALSE(maybe2);
        }

        SECTION("copy constructs value") {
            Maybe<Logger> maybe1(foo);
            Maybe<Logger> maybe2(maybe1);
            maybe2->requireExact(
                "constructed as foo",
                "copy constructed",
                "copy constructed");
        }
    }

    SECTION("copy assignment") {
        SECTION("if uninitialized") {
            Maybe<Logger> lhs;

            SECTION("if source is uninitialized, nothing happens") {
                Maybe<Logger> rhs;
                lhs = rhs;
                REQUIRE_FALSE(lhs);
            }

            SECTION("if source is initialized") {
                Maybe<Logger> rhs(foo);
                lhs = rhs;

                SECTION("gets initialized") {
                    REQUIRE(lhs);
                }

                SECTION("value gets copy constructed") {
                    lhs->requireExact(
                        "constructed as foo",
                        "copy constructed",
                        "copy constructed");
                }
            }

            SECTION("self assign leaves self unchanged") {
                lhs = lhs;
                REQUIRE_FALSE(lhs);
            }
        }

        SECTION("if initialized") {
            Maybe<Logger> lhs(bar);

            SECTION("if source is uninitialized, becomes uninitialized") {
                Maybe<Logger> rhs;
                lhs = rhs;
                REQUIRE_FALSE(lhs);
            }

            SECTION("if source is initialized") {
                Maybe<Logger> rhs(foo);
                lhs = rhs;

                SECTION("gets initialized") {
                    REQUIRE(lhs);
                }

                SECTION("value gets copy assigned") {
                    lhs->requireExact(
                        "constructed as foo",
                        "copy constructed",
                        "copy assigned");
                }
            }

            SECTION("self assign leaves self unchanged") {
                lhs = lhs;
                REQUIRE(lhs->id == "bar");
            }
        }
    }

    SECTION("move constructor") {
        SECTION("constructs empty object if source is empty") {
            Maybe<int> maybe1;
            Maybe<int> maybe2(std::move(maybe1));
            REQUIRE_FALSE(maybe2);
        }

        SECTION("move constructs value") {
            Maybe<Logger> maybe1(foo);
            Maybe<Logger> maybe2(std::move(maybe1));
            maybe2->requireExact(
                "constructed as foo",
                "copy constructed",
                "move constructed");
        }
    }

    SECTION("move assignment") {
        SECTION("if uninitialized") {
            Maybe<Logger> lhs;

            SECTION("if source is uninitialized, nothing happens") {
                Maybe<Logger> rhs;
                lhs = std::move(rhs);
                REQUIRE_FALSE(lhs);
            }

            SECTION("if source is initialized") {
                Maybe<Logger> rhs(foo);
                lhs = std::move(rhs);

                SECTION("gets initialized") {
                    REQUIRE(lhs);
                }

                SECTION("value gets move constructed") {
                    lhs->requireExact(
                        "constructed as foo",
                        "copy constructed",
                        "move constructed");
                }
            }
        }

        SECTION("if initialized") {
            Maybe<Logger> lhs(bar);

            SECTION("if source is uninitialized, becomes uninitialized") {
                Maybe<Logger> rhs;
                lhs = std::move(rhs);
                REQUIRE_FALSE(lhs);
            }

            SECTION("if source is initialized") {
                Maybe<Logger> rhs(foo);
                lhs = std::move(rhs);

                SECTION("gets initialized") {
                    REQUIRE(lhs);
                }

                SECTION("value gets move assigned") {
                    lhs->requireExact(
                        "constructed as foo",
                        "copy constructed",
                        "move assigned");
                }
            }
        }
    }

    SECTION("init") {

        SECTION("if not initialized") {
            Maybe<Logger> maybe;
            maybe.init("foobar");

            SECTION("initializes object") {
                REQUIRE(maybe);
            }

            SECTION("constructs value in place") {
                maybe->requireExact("constructed as foobar");
            }
        }

        SECTION("if already initialized") {
            std::vector<std::string> log;
            Maybe<DestructNotifier> maybe(DestructNotifier("foobar", &log));
            maybe.init("foobaz", &log);

            SECTION("gets initialized with new value") {
                REQUIRE(maybe);
                REQUIRE(maybe->id() == "foobaz");
            }
            SECTION("calls the destructor of the old value") {
                REQUIRE(log == std::vector<std::string>{"foobar"});
            }
        }
    }

    SECTION("reset") {
        SECTION("if initialized") {
            std::vector<std::string> log;
            Maybe<DestructNotifier> maybe(DestructNotifier("foobar", &log));
            maybe.reset();

            SECTION("deinitializes object") {
                REQUIRE_FALSE(maybe);
            }
            SECTION("calls the destructor of the value object") {
                REQUIRE(log == std::vector<std::string>{"foobar"});
            }
        }

        SECTION("does nothing if object is not initialized") {
            Maybe<int> maybe;
            maybe.reset();
            REQUIRE_FALSE(maybe);
        }
    }

    SECTION("operator->") {
        SECTION("returns a pointer to the value") {
            Maybe<int> maybe;
            REQUIRE(maybe.operator->() == &*maybe);
        }
    }

    SECTION("destructor") {
        SECTION("calls value destructor if initialized") {
            std::vector<std::string> log;

            {
                Maybe<DestructNotifier> maybe;
                maybe.init("foobar", &log);
            }

            REQUIRE(log == std::vector<std::string>{"foobar"});
        }
    }

    SECTION("operator==/operator!=") {
        propConformsToEquals<Maybe<std::string>>();

        SECTION("uninitialized values are equal") {
            REQUIRE(Maybe<Apple>() == Maybe<Orange>());
        }

        SECTION("uninitialized value are not equal to initialized values") {
            REQUIRE(Maybe<Apple>() != Maybe<Orange>(Orange("foo")));
        }

        SECTION("initialized value are equal if value is equal") {
            REQUIRE(Maybe<Apple>(Apple("foo")) == Maybe<Orange>(Orange("foo")));
        }

        SECTION("initialized value are inequal if value is inequal") {
            REQUIRE(Maybe<Apple>(Apple("foo")) != Maybe<Orange>(Orange("bar")));
        }
    }
}
