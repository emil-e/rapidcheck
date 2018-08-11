package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lib",
    strip_include_prefix =
        "include",
    includes = [
        "include",
    ],
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    srcs = glob([
        "src/**/*.cpp",
        "src/**/*.h",
    ]),
)

cc_library(
    name = "boost",
    strip_include_prefix =
        "extras/boost_test/include",
    hdrs = glob([
        "extras/boost_test/include/**/*.h",
    ]),
    deps = [
        "@boost//:unit_test_framework",
        "lib",
    ]
)

cc_library(
    name = "rapidcheck",
    deps = [
        "boost",
    ]
)

cc_test(
    name = "bin/test-rapidcheck",
    timeout = "short",
    srcs = glob([
        "examples/boost_test/**/*.cpp",
        "examples/boost_test/**/*.h",
    ]),
    deps = [
        "boost",
    ],
)
